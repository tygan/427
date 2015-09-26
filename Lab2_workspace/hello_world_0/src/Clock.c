#include "xgpio.h"          // Provides access to PB GPIO driver.
#include <stdio.h>          // xil_printf and so forth.
#include "platform.h"       // Enables caching and other system stuff.
#include "mb_interface.h"   // provides the microblaze interrupt enables, etc.
#include "xintc_l.h"        // Provides handy macros for the interrupt controller.
#define SECOND 100
#define DEBOUNCE 0
#define MINBTN 1
#define SECBTN 2
#define DOWNBTN 4
#define HOURBTN 8
#define UPBTN 16
#define MAX_HOUR 23
#define MAX_MIN 59
#define MAX_SEC 59
#define	TEN 10

XGpio gpLED;  // This is a handle for the LED GPIO block.
XGpio gpPB;   // This is a handle for the push-button GPIO block.
int currentButtonState, realButtonState, prevButtonState;
int count2,debounceTimer, btnTimer;
int hour, min, sec;
int firstTime, btnHoldTimer, btnHoldTimer2;

// This is invoked in response to a timer interrupt.
void printTime(){
	if(hour < TEN){		//Ensure HH:MM:SS when numbers are single digits
		xil_printf("0");
	}
	xil_printf("%d:", hour);	//Print hour
	if(min < TEN){
		xil_printf("0");
	}
	xil_printf("%d:", min);		//Print minute
	if(sec < TEN){
		xil_printf("0");
	}
	xil_printf("%d\r", sec);	//Print second
}
int changeTime(int isHour, int inc, int value){
	if(isHour){
		if(inc){	//if incrementing
			if(value == MAX_HOUR){	//wrap hour to 0 if 23
				return 0;
			}
			else{
				return value + 1;
			}
		}
		else{		//if decrementing
			if(value == 0){
				return MAX_HOUR;
			}
			else{
				return value - 1;
			}
		}
	}
	else{	//if minute or second
		if(inc){	//if incrementing
			if(value == MAX_MIN){	//wrap minute/sec to 0 if 59
				return 0;
			}
			else{
				return value + 1;
			}
		}
		else{		//if decrementing
			if(value == 0){
				return MAX_MIN;
			}
			else{
				return value - 1;
			}
		}
	}
}

void button_decoder() {		//Changes time based on button input
	int upFlag = 0;			//for up button
	int downFlag = 0;		//for down button
	if(realButtonState & UPBTN){
		upFlag = 1;
	}
	else if(realButtonState & DOWNBTN){
		downFlag = 1;
	}
	if(realButtonState & HOURBTN){		//if hour button was pressed
		if(upFlag){
			hour = changeTime(1, upFlag, hour);
		}
		else if(downFlag){
			hour = changeTime(1, !downFlag, hour);
		}
	}
	if(realButtonState & MINBTN){		//if minute button was pressed
		if(upFlag){
			min = changeTime(0, upFlag, min);
		}
		else if(downFlag){
			min = changeTime(0, !downFlag, min);
		}
	}
	if(realButtonState & SECBTN){		//if second button was pressed
		if(upFlag){
			sec = changeTime(0, upFlag, sec);
		}
		else if(downFlag){
			sec = changeTime(0, !downFlag, sec);
		}
	}
	printTime();		//Reprint time for every time change
}

//1) debounce switches, and 2) advances the time.
void timer_interrupt_handler() {

	prevButtonState = realButtonState;		//for debouncing
	realButtonState = currentButtonState;

	if(realButtonState != 0){	//if any button(s) are pressed
		if(realButtonState != prevButtonState){	//if change in buttons pressed, use buttons
			button_decoder();
			btnHoldTimer = 0;		//reset timer
		}
		if(btnHoldTimer >= SECOND){	//wait for button to be held for 1 second
			if(btnHoldTimer2 >= 50){	//update time every half second
				button_decoder();
				btnHoldTimer2 = 0;
			}
			else{
				btnHoldTimer2++;
			}
		}
		else{
			btnHoldTimer++;
		}
	}
	else	//increase time counters if no button is pressed
	{
		btnHoldTimer = 0;	//reset button hold timer if button is not pressed
		if(count2 == SECOND){
			if(sec == MAX_SEC){	//count up to 1 second
				sec = 0;
				if(min == MAX_MIN){	//count up to 1 minute
					min = 0;
					if(hour == MAX_HOUR){	//count up to 1 hour
						hour = 0;
					}
					else{
						hour++;
					}
				}
				else{
					min++;
				}
			}else{
				sec++;
			}
			count2 = 0;	//reset time counter
			printTime();
		}
		else{
			count2++;
		}

	}
}

// This is invoked each time there is a change in the button state (result of a push or a bounce).
void pb_interrupt_handler() {
  // Clear the GPIO interrupt.
  XGpio_InterruptGlobalDisable(&gpPB);                // Turn off all PB interrupts for now.
  currentButtonState = XGpio_DiscreteRead(&gpPB, 1);  // Get the current state of the buttons.
  debounceTimer = 0;	//reset debounce timer
  XGpio_InterruptClear(&gpPB, 0xFFFFFFFF);            // Ack the PB interrupt.
  XGpio_InterruptGlobalEnable(&gpPB);                 // Re-enable PB interrupts.
}

// Main interrupt handler, queries the interrupt controller to see what peripheral
// fired the interrupt and then dispatches the corresponding interrupt handler.
// This routine acks the interrupt at the controller level but the peripheral
// interrupt must be ack'd by the dispatched interrupt handler.
void interrupt_handler_dispatcher(void* ptr) {
	int intc_status = XIntc_GetIntrStatus(XPAR_INTC_0_BASEADDR);
	// Check the FIT interrupt first.
	if (intc_status & XPAR_FIT_TIMER_0_INTERRUPT_MASK){
		XIntc_AckIntr(XPAR_INTC_0_BASEADDR, XPAR_FIT_TIMER_0_INTERRUPT_MASK);
		timer_interrupt_handler();
	}
	// Check the push buttons.
	if (intc_status & XPAR_PUSH_BUTTONS_5BITS_IP2INTC_IRPT_MASK){
		XIntc_AckIntr(XPAR_INTC_0_BASEADDR, XPAR_PUSH_BUTTONS_5BITS_IP2INTC_IRPT_MASK);
		pb_interrupt_handler();
	}
}

int main (void) {
    init_platform();
    // Initialize the GPIO peripherals.
    int success;
    print("hello world\n\r");
    success = XGpio_Initialize(&gpPB, XPAR_PUSH_BUTTONS_5BITS_DEVICE_ID);
    // Set the push button peripheral to be inputs.
    XGpio_SetDataDirection(&gpPB, 1, 0x0000001F);
    // Enable the global GPIO interrupt for push buttons.
    XGpio_InterruptGlobalEnable(&gpPB);
    // Enable all interrupts in the push button peripheral.
    XGpio_InterruptEnable(&gpPB, 0xFFFFFFFF);

    microblaze_register_handler(interrupt_handler_dispatcher, NULL);
    XIntc_EnableIntr(XPAR_INTC_0_BASEADDR,
    		(XPAR_FIT_TIMER_0_INTERRUPT_MASK | XPAR_PUSH_BUTTONS_5BITS_IP2INTC_IRPT_MASK));
    XIntc_MasterEnable(XPAR_INTC_0_BASEADDR);
    microblaze_enable_interrupts();

    currentButtonState = 0;	//initialize variables
    realButtonState = 0;
    prevButtonState = 0;
    btnTimer = 0;
    btnHoldTimer2 = 0;
    count2 = 0;
    firstTime = 1;
    btnHoldTimer = 0;
    hour = 00;
    min = 00;
    sec = 00;

    while(1);  // Program never ends.

    cleanup_platform();

    return 0;
}
