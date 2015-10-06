/*
 * Copyright (c) 2009 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/*
 * helloworld.c: simple test application
 */
#include "xgpio.h"          // Provides access to PB GPIO driver.
#include <stdio.h>          // xil_printf and so forth.
#include "platform.h"       // Enables caching and other system stuff.
#include "mb_interface.h"   // provides the microblaze interrupt enables, etc.
#include "xintc_l.h"        // Provides handy macros for the interrupt controller.
#include "xgpio.h"          // Provides access to PB GPIO driver.
#include <stdio.h>          // xil_printf and so forth.
#include "platform.h"       // Enables caching and other system stuff.
#include "mb_interface.h"   // provides the microblaze interrupt enables, etc.
#include "xintc_l.h"        // Provides handy macros for the interrupt controller.
#include <stdio.h>
#include "platform.h"
#include "xparameters.h"
#include "xaxivdma.h"
#include "xio.h"
#include "time.h"
#include "unistd.h"
#include "shapes.h"
#define MIDDLEBTN 1
#define RIGHTBTN 2
#define DOWNBTN 4
#define LEFTBTN 8
#define UPBTN 16
#define DEBUG
void print(char *str);

#define FRAME_BUFFER_ADDR 0xC1000000  // Starting location in DDR where we will store the images that we display.
#define MAX_SILLY_TIMER 10000000;	//For nostalgia

#define ALIEN_HEIGHT 16
#define ALIEN_WIDTH 24

#define TANK_WIDTH 48
#define TANK_HEIGHT 16

#define BUNKER_WIDTH 48
#define BUNKER_HEIGHT 36

#define BUNKER_DAMAGE_HEIGHT 12
#define BUNKER_DAMAGE_WIDTH 12

#define X_DAMAGE 16
#define Y_DAMAGE 3
#define NUM_ALIENS 11 * 32

#define BUNKER_0 48
#define BUNKER_1 49
#define BUNKER_2 50
#define BUNKER_3 51

#define TANK_BULLET_HEIGHT 10
#define TANK_BULLET_WIDTH 3
#define ALIEN_MISSILE_HEIGHT 10
#define ALIEN_MISSILE_WIDTH 3

#define BUNKER_NUM_0_X 72
#define BUNKER_NUM_1_X 216
#define BUNKER_NUM_2_X 360
#define BUNKER_NUM_3_X 504
#define BUNKER_Y 300

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define ALIENS_TALL 5
#define ALIENS_WIDE 11
#define ALIEN_BUFFER 8
#define ALIENS_START_X 160
#define ALIENS_START_Y 40
#define ALIEN_HORIZ_MOVE 4
#define ALIEN_VERTICAL_MOVE 8
#define DOWN 0
#define LEFT 1
#define RIGHT 2

#define BLACK 0x00000000
#define WHITE 0x00FFFFFF
#define GREEN 0x0000FF00

#define WORD_WIDTH 32
XGpio gpLED;  // This is a handle for the LED GPIO block.
XGpio gpPB;   // This is a handle for the push-button GPIO block.
int currentButtonState;
int aliens_in = 1;
int first_row = 0;
int last_row = ALIENS_WIDE - 1;
int exists_missile = 0;
int aliens_alive[ALIENS_TALL][ALIENS_WIDE];
int bunkerHealth[Y_DAMAGE][X_DAMAGE] = {{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
										{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
										{3,0,0,3,3,0,0,3,3,0,0,3,3,0,0,3}};
int tankBulletCoordinates[2];
int alienMissileCoordinates[2];
int tankPosX;
int tankPosY;
int draw;

void print_aliens(int corner_top, int corner_left, int direction){
	unsigned int * framePointer = (unsigned int *) FRAME_BUFFER_ADDR;
	int i;

	aliens_in = !aliens_in;		//toggle aliens from out to in
	if(direction == DOWN){		//if moving aliens down, erase aliens on top
		int i,j;
		for(i=corner_top - (ALIEN_HEIGHT + ALIEN_BUFFER); i<corner_top; i++){
			for(j=corner_left; j<corner_left + ALIENS_WIDE*(ALIEN_BUFFER+ALIEN_WIDTH); j++){
				framePointer[i*SCREEN_WIDTH + j] = BLACK;
			}
		}
	}
	if(direction == RIGHT){		//if moving aliens to the right, erase aliens on the left
		int i,j;
		for(i=corner_top; i<corner_top+ALIENS_TALL*(ALIEN_HEIGHT+ALIEN_BUFFER); i++){
			for(j=corner_left-ALIEN_BUFFER; j<corner_left; j++){
				framePointer[i*SCREEN_WIDTH + j] = BLACK;
			}
		}
	}
	for(i=0; i<ALIENS_TALL; i++){	//for all rows
		int j;
		for(j=0; j<ALIENS_WIDE; j++){	//for all columns
			int inner_row, inner_column;
			for (inner_row=0; inner_row<ALIEN_HEIGHT+ALIEN_BUFFER; inner_row++) {	//draw entire rectangle with alien and buffer
				for (inner_column = 0; inner_column<ALIEN_WIDTH+ALIEN_BUFFER; inner_column++) {
					if(aliens_alive[i][j]){		//if alien is alive, draw
						if(inner_row<ALIEN_HEIGHT && inner_column<ALIEN_WIDTH){	//if inside alien rectangle, draw alien
							if(i==0){	//for top row
								if(aliens_in){
									if ((alien_top_in_24x16[inner_row] & (1<<(ALIEN_WIDTH-1-inner_column)))) {
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*SCREEN_WIDTH + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = WHITE;
									}else{	//draw black around alien
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*SCREEN_WIDTH + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = BLACK;
									}
								}
								else{
									if ((alien_top_out_24x16[inner_row] & (1<<(ALIEN_WIDTH-1-inner_column)))) {
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*SCREEN_WIDTH + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = WHITE;
									}else{	//draw black around alien
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*SCREEN_WIDTH + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = BLACK;
									}
								}
							}
							else if(i==1 || i==2){	//for middle 2 rows
								if(aliens_in){
									if ((alien_middle_in_24x16[inner_row] & (1<<(ALIEN_WIDTH-1-inner_column)))) {
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*SCREEN_WIDTH + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = WHITE;
									}else{	//draw black around alien
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*SCREEN_WIDTH + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = BLACK;
									}
								}
								else{
									if ((alien_middle_out_24x16[inner_row] & (1<<(ALIEN_WIDTH-1-inner_column)))) {
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*SCREEN_WIDTH + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = WHITE;
									}else{	//draw black around alien
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*SCREEN_WIDTH + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = BLACK;
									}
								}
							}
							else if(i==3 || i==4){	//for bottom 2 rows
								if(aliens_in){
									if ((alien_bottom_in_24x16[inner_row] & (1<<(ALIEN_WIDTH-1-inner_column)))) {
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*SCREEN_WIDTH + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = WHITE;
									}else{	//draw black around alien
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*SCREEN_WIDTH + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = BLACK;
									}
								}
								else{
									if ((alien_bottom_out_24x16[inner_row] & (1<<(ALIEN_WIDTH-1-inner_column)))) {
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*SCREEN_WIDTH + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = WHITE;
									}else{	//draw black around alien
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*SCREEN_WIDTH + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = BLACK;
									}
								}
							}
						}
						else{	//if outside alien rectangle, draw black
							framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*SCREEN_WIDTH + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = BLACK;
						}
					}
					else{	//if alien is not alive, just draw black
						framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*SCREEN_WIDTH + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = BLACK;
					}
				}
			}
		}
	}
}

void erase_alien(int corner_top, int corner_left){	//erases alien of given coordinates
	unsigned int * framePointer = (unsigned int *) FRAME_BUFFER_ADDR;
	int row,column;
	for (row=corner_top; row<corner_top+ALIEN_HEIGHT; row++) {
			for (column = corner_left; column<corner_left+ALIEN_WIDTH; column++) {
				framePointer[row*SCREEN_WIDTH + column] = BLACK;
			}
	}
}

void drawTank(left_corner, top_corner, draw){
	int color;
	if(draw == 1){
		color = GREEN;
	}else{
		color = BLACK;
	}
	int bunkerX = 0;
	int bunkerY = 0;
	int dupBitX = 0;
	int dupBitY = 0;
	unsigned int * framePointer = (unsigned int *) FRAME_BUFFER_ADDR;
	int row, column;
	for (row=top_corner; row<top_corner+TANK_HEIGHT; row++) {
	    for (column = left_corner; column<left_corner+TANK_WIDTH; column++) {
			if ((tank_22x8[bunkerY] & (1<<bunkerX))) {
				framePointer[(row)*SCREEN_WIDTH + (column)] = color;
			}else{
				framePointer[(row)*SCREEN_WIDTH + (column)] = BLACK;
			}
			if(dupBitX == 1)
			{
				bunkerX++;
				dupBitX = 0;
			}else{
				dupBitX = 1;
			}
		}
	    bunkerX = 0;
		if(dupBitY == 1){
			bunkerY++;
			dupBitY = 0;
		}else{
			dupBitY = 1;
		}
	 }
}
void drawBunkerDamage(x_pos, y_pos, damageType){
	unsigned int * framePointer = (unsigned int *) FRAME_BUFFER_ADDR;
	int row, column;
	for (row=y_pos; row<y_pos+BUNKER_DAMAGE_HEIGHT; row++) {
	    for (column = x_pos; column<x_pos+BUNKER_DAMAGE_WIDTH; column++) {
	    	if(damageType == 3){
	    		if ((bunkerDamage0_12x12[row-y_pos] & (1<<(BUNKER_DAMAGE_WIDTH-1-(column-x_pos))))) {
					framePointer[(row)*SCREEN_WIDTH + (column)] = BLACK;
				}
	    	}else if(damageType == 2){
	    		if ((bunkerDamage1_12x12[row-y_pos] & (1<<(BUNKER_DAMAGE_WIDTH-1-(column-x_pos))))) {
					framePointer[(row)*SCREEN_WIDTH + (column)] = BLACK;
				}
	    	}else if(damageType == 1){
	    		if ((bunkerDamage2_12x12[row-y_pos] & (1<<(BUNKER_DAMAGE_WIDTH-1-(column-x_pos))))) {
					framePointer[(row)*SCREEN_WIDTH + (column)] = BLACK;
				}
	    	}else if(damageType == 0){
				framePointer[(row)*SCREEN_WIDTH + (column)] = BLACK;
	    	}

		}
	 }
}
void erodeBunker(bunkerNumber){
	//This will get the current damage of the place you are going to erode.
	int bunkPosY;
	int bunkDamage;
	int bunkYOffset = 0;
	int skipDraw = 0;
	int randNum = rand() % 4;
	int bunkPosX = (bunkerNumber * 4) + randNum;
	if(bunkerHealth[0][bunkPosX] <= -1){//the first row
		if(bunkerHealth[1][bunkPosX] <= -1){//second row
			if(bunkerHealth[2][bunkPosX] <= -1) {//third row
				skipDraw = 1;//do nothing
			}else{
				bunkDamage = bunkerHealth[2][bunkPosX];
				bunkerHealth[2][bunkPosX]--;
				bunkYOffset = 2;
			}
		}else{
			bunkDamage = bunkerHealth[1][bunkPosX];
			bunkerHealth[1][bunkPosX]--;
			bunkYOffset = 1;
		}
	}else{
		bunkDamage = bunkerHealth[0][bunkPosX];
		bunkerHealth[0][bunkPosX]--;
		bunkYOffset = 0;
	}
	//This will get the x position of the bunker to erode.
	if(bunkerNumber == 0){
		bunkPosX = BUNKER_NUM_0_X + randNum*BUNKER_DAMAGE_WIDTH;
	}else if(bunkerNumber == 1){
		bunkPosX = BUNKER_NUM_1_X + randNum*BUNKER_DAMAGE_WIDTH;
	}else if(bunkerNumber == 2){
		bunkPosX = BUNKER_NUM_2_X + randNum*BUNKER_DAMAGE_WIDTH;
	}else if(bunkerNumber == 3){
		bunkPosX = BUNKER_NUM_3_X + randNum*BUNKER_DAMAGE_WIDTH;
	}
	//This will get the y position of the bunker to erode.
	if(bunkYOffset == 0){
		bunkPosY = BUNKER_Y;
	}else if(bunkYOffset == 1){
		bunkPosY = BUNKER_Y + BUNKER_DAMAGE_HEIGHT;
	}else if(bunkYOffset == 2){
		bunkPosY = BUNKER_Y + (BUNKER_DAMAGE_HEIGHT * 2);
	}
	if(skipDraw == 0){
		drawBunkerDamage(bunkPosX, bunkPosY, bunkDamage);
	}
}


void drawBunker(left_corner, top_corner, draw){
	int color;
	if(draw == 1){
		color = GREEN;
	}else{
		color = BLACK;
	}
	unsigned int * framePointer = (unsigned int *) FRAME_BUFFER_ADDR;
	int row, column;
	int bunkerX = 0;
	int bunkerY = 0;
	int dupBitX = 0;
	int dupBitY = 0;
	for (row=top_corner; row<top_corner+BUNKER_HEIGHT; row++) {
	    for (column = left_corner; column<left_corner+BUNKER_WIDTH; column++) {
			if ((bunker_24x18[bunkerY] & (1<<bunkerX))) {
				framePointer[(row)*SCREEN_WIDTH + (column)] = color;
			}else{
				framePointer[(row)*SCREEN_WIDTH + (column)] = BLACK;
			}
			if(dupBitX == 1)
			{
				bunkerX++;
				dupBitX = 0;
			}else{
				dupBitX = 1;
			}
		}
	    bunkerX = 0;
	    if(dupBitY == 1){
	    	bunkerY++;
	    	dupBitY = 0;
	    }else{
	    	dupBitY = 1;
	    }
	 }
}

void drawTankBullet(x,y,draw){
	int color;
	if(draw == 1){
		color = GREEN;
	}else{
		color = BLACK;
	}
	unsigned int * framePointer = (unsigned int *) FRAME_BUFFER_ADDR;
	int row, column;
	for (row=y; row<y+TANK_BULLET_HEIGHT; row++) {
		for (column = x; column<x+TANK_BULLET_WIDTH; column++) {
			if ((alienMissileForwardSlash_3x10[row-y] & (1<<(TANK_BULLET_WIDTH-1-(column-x))))) {
				framePointer[(row)*SCREEN_WIDTH + (column)] = color;
			}else{
				framePointer[(row)*SCREEN_WIDTH + (column)] = BLACK;
			}
		}
	}
}

void drawAlienMissile(x,y,draw){
	int color;
	if(draw == 1){
		color = WHITE;
	}else{
		color = BLACK;
	}
	unsigned int * framePointer = (unsigned int *) FRAME_BUFFER_ADDR;
	int row, column;
	for (row=y; row<y+ALIEN_MISSILE_HEIGHT; row++) {
		for (column = x; column<x+ALIEN_MISSILE_WIDTH; column++) {
			if ((alienMissileForwardSlash_3x10[row-y] & (1<<(ALIEN_MISSILE_HEIGHT-1-(column-x))))) {
				framePointer[(row)*SCREEN_WIDTH + (column)] = color;
			}else{
				framePointer[(row)*SCREEN_WIDTH + (column)] = color;
			}
		}
	}
}

void moveBullets(){
	int draw = 0;
	int tankBulletX = tankBulletCoordinates[0];
	int tankBullety = tankBulletCoordinates[1];
	drawTankBullet(tankBulletX, tankBullety, draw);//erase previous tank bullet
	if(tankBullety > 0){
		tankBullety -= 2;
		draw = 1;
		drawTankBullet(tankBulletX, tankBullety, draw);//draw new tank bullet
		tankBulletCoordinates[0] = tankBulletX;
		tankBulletCoordinates[1] = tankBullety;
	}

	if(exists_missile){
		draw = 0;
		int alienMissileX = alienMissileCoordinates[0];
		int alienMissileY = alienMissileCoordinates[1];
		drawAlienMissile(alienMissileX, alienMissileY, draw);//erase previous alien missile
		if(alienMissileY < SCREEN_HEIGHT - ALIEN_MISSILE_HEIGHT){
			alienMissileY += 2;
			draw = 1;
			drawAlienMissile(alienMissileX, alienMissileY, draw);//draw new tank bullet
			alienMissileCoordinates[0] = alienMissileX;
			alienMissileCoordinates[1] = alienMissileY;
		}
		else{
			exists_missile = 0;
		}
	}
}

void reevaluate_aliens(){
	int x, y, row_not_empty;
	x = -1;
	row_not_empty = 0;
	while(!row_not_empty){
		x++;
		for(y=0;y<ALIENS_TALL;y++){
			row_not_empty|=aliens_alive[y][x];
		}
	}
	first_row = x;

	x = ALIENS_WIDE;
	row_not_empty=0;
	while(!row_not_empty){
		x--;
		for(y=0;y<ALIENS_TALL;y++){
			row_not_empty|=aliens_alive[y][x];
		}
	}
	last_row = x;
}
void button_decoder() {
	int upFlag = 0;			//for up button
	int downFlag = 0;		//for down button
	if(currentButtonState & LEFTBTN){//move tank left
		if(tankPosX > 0){
			xil_printf("going left");
			 draw = 1;
			 tankPosX -= 2;
			 drawTank(tankPosX, tankPosY, draw);
		 }
	}else if(currentButtonState & RIGHTBTN){//move tank right
		 if(tankPosX < SCREEN_WIDTH - TANK_WIDTH){
			 xil_printf("going right");
			 draw = 1;
			 tankPosX += 2;
			 drawTank(tankPosX, tankPosY, draw);
		 }
	}else if(currentButtonState & MIDDLEBTN){//fire tank bullet
		xil_printf("fire bullet");
		draw = 1;
		drawTankBullet(tankPosX,tankPosY,draw);
	}
}
void timer_interrupt_handler() {
	//if(currentButtonState != 0){	//if any button(s) are pressed
//	xil_printf("going to decode button");
	button_decoder();
	//}
	//call move aliens function every once in a while...
}
// This is invoked each time there is a change in the button state (result of a push or a bounce).
void pb_interrupt_handler() {
  // Clear the GPIO interrupt.
  XGpio_InterruptGlobalDisable(&gpPB);                // Turn off all PB interrupts for now.
  currentButtonState = XGpio_DiscreteRead(&gpPB, 1);  // Get the current state of the buttons.
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
//		xil_printf("calling timer interrupt handler");
		timer_interrupt_handler();
	}
	// Check the push buttons.
	if (intc_status & XPAR_PUSH_BUTTONS_5BITS_IP2INTC_IRPT_MASK){
		XIntc_AckIntr(XPAR_INTC_0_BASEADDR, XPAR_PUSH_BUTTONS_5BITS_IP2INTC_IRPT_MASK);
//		xil_printf("calling pb interrupt handler");
		pb_interrupt_handler();
	}
}
int main()
{
	init_platform();                   // Necessary for all programs.
	int Status;                        // Keep track of success/failure of system function calls.

	////////////////////////////////////////////////////////////
	//Initialize interrupts and FIT
	////////////////////////////////////////////////////////////
	int success;
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
	////////////////////////////////////////////////////////////

	XAxiVdma videoDMAController;
	// There are 3 steps to initializing the vdma driver and IP.
	// Step 1: lookup the memory structure that is used to access the vdma driver.
    XAxiVdma_Config * VideoDMAConfig = XAxiVdma_LookupConfig(XPAR_AXI_VDMA_0_DEVICE_ID);
    // Step 2: Initialize the memory structure and the hardware.
    if(XST_FAILURE == XAxiVdma_CfgInitialize(&videoDMAController, VideoDMAConfig,	XPAR_AXI_VDMA_0_BASEADDR)) {
    	xil_printf("VideoDMA Did not initialize.\r\n");
    }
    // Step 3: (optional) set the frame store number.
    if(XST_FAILURE ==  XAxiVdma_SetFrmStore(&videoDMAController, 2, XAXIVDMA_READ)) {
    	xil_printf("Set Frame Store Failed.");
    }
    // Initialization is complete at this point.

    // Setup the frame counter. We want two read frames. We don't need any write frames but the
    // function generates an error if you set the write frame count to 0. We set it to 2
    // but ignore it because we don't need a write channel at all.
    XAxiVdma_FrameCounter myFrameConfig;
    myFrameConfig.ReadFrameCount = 2;
    myFrameConfig.ReadDelayTimerCount = 10;
    myFrameConfig.WriteFrameCount =2;
    myFrameConfig.WriteDelayTimerCount = 10;
    Status = XAxiVdma_SetFrameCounter(&videoDMAController, &myFrameConfig);
    if (Status != XST_SUCCESS) {
	   xil_printf("Set frame counter failed %d\r\n", Status);
	   if(Status == XST_VDMA_MISMATCH_ERROR)
		   xil_printf("DMA Mismatch Error\r\n");
    }
    // Now we tell the driver about the geometry of our frame buffer and a few other things.
    // Our image is 480 x 640.
    XAxiVdma_DmaSetup myFrameBuffer;
    myFrameBuffer.VertSizeInput = SCREEN_WIDTH;    // 480 vertical pixels.
    myFrameBuffer.HoriSizeInput = SCREEN_HEIGHT*4;  // 640 horizontal (32-bit pixels).
    myFrameBuffer.Stride = SCREEN_HEIGHT*4;         // Dont' worry about the rest of the values.
    myFrameBuffer.FrameDelay = 0;
    myFrameBuffer.EnableCircularBuf=1;
    myFrameBuffer.EnableSync = 0;
    myFrameBuffer.PointNum = 0;
    myFrameBuffer.EnableFrameCounter = 0;
    myFrameBuffer.FixedFrameStoreAddr = 0;
    if(XST_FAILURE == XAxiVdma_DmaConfig(&videoDMAController, XAXIVDMA_READ, &myFrameBuffer)) {
    	xil_printf("DMA Config Failed\r\n");
     }
    // We need to give the frame buffer pointers to the memory that it will use. This memory
    // is where you will write your video data. The vdma IP/driver then streams it to the HDMI
    // IP.
     myFrameBuffer.FrameStoreStartAddr[0] = FRAME_BUFFER_ADDR;
     myFrameBuffer.FrameStoreStartAddr[1] = FRAME_BUFFER_ADDR + 4*640*480;

     if(XST_FAILURE == XAxiVdma_DmaSetBufferAddr(&videoDMAController, XAXIVDMA_READ,
    		               myFrameBuffer.FrameStoreStartAddr)) {
    	 xil_printf("DMA Set Address Failed Failed\r\n");
     }
     // Print a sanity message if you get this far.
     xil_printf("\n\rWoohooz! I made it through initialization.\n\r");

     // Now, let's get ready to start displaying some stuff on the screen.
     // The variables framePointer and framePointer1 are just pointers to the base address
     // of frame 0 and frame 1.

    // Let's print out the alien as ASCII characters on the screen.
	// Each line of the alien is a 32-bit integer. We just need to strip the bits out and send
	// them to stdout.
	// MSB is the left-most pixel for the alien, so start from the MSB as we print from left to right.

    // This tells the HDMI controller the resolution of your display (there must be a better way to do this).
     XIo_Out32(XPAR_AXI_HDMI_0_BASEADDR, SCREEN_WIDTH*SCREEN_HEIGHT);

    // Start the DMA for the read channel only.
     if(XST_FAILURE == XAxiVdma_DmaStart(&videoDMAController, XAXIVDMA_READ)){
    	 xil_printf("DMA START FAILED\r\n");
     }
     int frameIndex = 0;
//     // We have two frames, let's park on frame 0. Use frameIndex to index them.
//     // Note that you have to start the DMA process before parking on a frame.
     if (XST_FAILURE == XAxiVdma_StartParking(&videoDMAController, frameIndex,  XAXIVDMA_READ)) {
    	 xil_printf("vdma parking failed\n\r");
     }


     ///////////////////////////////////
	 //CLEAR THE SCREEN//
     xil_printf("initialized!\n\r");
	int y, x = 0;
	unsigned int * frameP = (unsigned int *) FRAME_BUFFER_ADDR;
	for (y=0; y<SCREEN_HEIGHT; y++) {
		for (x = 0; x<SCREEN_WIDTH; x++) {
			frameP[(y)*SCREEN_WIDTH + (x)] = BLACK;
		}
	 }
     ///////////////////////////////////

     /////////////////////////////
     //initialize tank on screen//
     int draw = 1;
     tankPosX = 0;
     tankPosY = 414;
	 drawTank(tankPosX, tankPosY, draw);
	 /////////////////////////////

	 /////////////////////////////
	 //initialize bunkers on screen//
	 draw = 1;
	 int bunkerPosX = BUNKER_NUM_0_X;
	 int bunkerPosY = BUNKER_Y;
	 drawBunker(bunkerPosX, bunkerPosY, draw);
	 bunkerPosX = BUNKER_NUM_1_X;
	 drawBunker(bunkerPosX, bunkerPosY, draw);
	 bunkerPosX = BUNKER_NUM_2_X;
	 drawBunker(bunkerPosX, bunkerPosY, draw);
	 bunkerPosX = BUNKER_NUM_3_X;
	 drawBunker(bunkerPosX, bunkerPosY, draw);

	 /////////////////////////////
	 //initialize aliens alive array//
	 int i,j;
	 for(i=0; i<ALIENS_TALL; i++){
		 for(j=0; j<ALIENS_WIDE; j++){
			 aliens_alive[i][j] = 1;
		 }
	 }
	 int direction = LEFT;
	 /////////////////////////////
	 //initialize aliens on screen//
	 srand((unsigned)time(NULL));

	 int aliens_x = ALIENS_START_X;
	 int aliens_y = ALIENS_START_Y;
	 print_aliens(aliens_y, aliens_x, direction);
	 setvbuf(stdin,NULL,_IONBF,0);


     while (1) {
//    	 char c = getchar();
//    	 if(c == '6'){//key 6 so move right
//    		 if(tankPosX < SCREEN_WIDTH - TANK_WIDTH){
//    			 draw = 1;
//    			 tankPosX += 4;
//				 drawTank(tankPosX, tankPosY, draw);
//    		 }
//    	 }else if(c == '4'){//key 4 so move left
//    		 if(tankPosX > 0){
//    			 draw = 1;
//    			 tankPosX -= 4;
//				 drawTank(tankPosX, tankPosY, draw);
//    		 }
//		 }else if(c == '7'){//key 7 so erode bunker
//			 char bunkerNum = getchar(); //0 = 48, 1 = 49, 2 = 50, 3 = 51
//			 if(bunkerNum == BUNKER_0){
//				 int bunker0 = 0;
//				 erodeBunker(bunker0);
//			 }else if(bunkerNum == BUNKER_1){
//				 int bunker1 = 1;
//				 erodeBunker(bunker1);
//			 }else if(bunkerNum == BUNKER_2){
//				 int bunker2 = 2;
//				 erodeBunker(bunker2);
//			 }else if(bunkerNum == BUNKER_3){
//				 int bunker3 = 3;
//				 erodeBunker(bunker3);
//			 }
//		 }
//    	 else if(c=='8'){//update alien position if c='8'
//    		 if(direction == DOWN){	//if aliens have already gone down
//    			 if(aliens_x + first_row*(ALIEN_WIDTH+ALIEN_BUFFER) == 0){	//if aliens have hit the left edge of the screen
//    				 direction = RIGHT;
//    			 }
//    			 else{		//if aliens have hit the right edge of the screen
//    				 direction = LEFT;
//    			 }
//    		 }
//    		 else if(aliens_x + first_row*(ALIEN_WIDTH+ALIEN_BUFFER) == 0 || aliens_x == SCREEN_WIDTH - ALIENS_WIDE*(ALIEN_BUFFER+ALIEN_WIDTH)){	//if aliens hit the left or right edges
//    			 direction = DOWN;
//    		 }
//    		 if(direction == DOWN){
//    			 aliens_y = aliens_y + ALIEN_VERTICAL_MOVE + ALIEN_BUFFER;
//    		 }
//    		 else if(direction == LEFT){
//    			 aliens_x = aliens_x - ALIEN_HORIZ_MOVE;
//    		 }
//    		 else{			//if direction == RIGHT
//    			 aliens_x = aliens_x + ALIEN_HORIZ_MOVE;
//    		 }
//    		 print_aliens(aliens_y, aliens_x, direction);
//    	 }
//    	 else if(c=='5'){//key 5 so fire alien bullet
//    		 if(tankBulletCoordinates[1] <= 0){
//				 draw = 1;
//				 drawTankBullet(tankPosX + TANK_WIDTH/2 - 1, tankPosY - TANK_BULLET_HEIGHT, draw);
//				 tankBulletCoordinates[0] = tankPosX + TANK_WIDTH/2 - 1;
//				 tankBulletCoordinates[1] = tankPosY - TANK_BULLET_HEIGHT;
//			 }
//    	 }
//    	 else if(c=='3'){//key 3 so fire tank bullet
//    		 if(!exists_missile){
//    			 exists_missile = 1;
//				 draw = 1;
//				 int randNum = rand() % ALIENS_WIDE;
//				 int shoot_pos_x = aliens_x+randNum*(ALIEN_WIDTH+ALIEN_BUFFER)+(ALIEN_WIDTH/2)-2;
//				 int shoot_pos_y = aliens_y+ALIENS_TALL*(ALIEN_HEIGHT+ALIEN_BUFFER)-ALIEN_BUFFER;
//				 drawAlienMissile(shoot_pos_x, shoot_pos_y, draw);
//				 alienMissileCoordinates[0] = shoot_pos_x;
//				 alienMissileCoordinates[1] = shoot_pos_y;
//    		 }
//		 }
//    	 else if(c == '9'){//key 9 so move all bullets
//    		 moveBullets();
//    	 }
//    	 else if(c=='2'){//key 2
//    		 c = getchar();
//    		 char c2 = getchar();
//    		 int index = (((u_int)c)-'0')*10+((uint)c2-'0');	//combine 2 keypresses on keypad into 1 number
//    		 int alien_y_index = index/ALIENS_WIDE;
//    		 int alien_x_index = index%ALIENS_WIDE;
//    		 aliens_alive[alien_y_index][alien_x_index] = 0;
//    		 erase_alien(aliens_y+alien_y_index*(ALIEN_HEIGHT+ALIEN_BUFFER), aliens_x+alien_x_index*(ALIEN_WIDTH+ALIEN_BUFFER));
//    		 reevaluate_aliens();
//    	 }
     }
     cleanup_platform();

    return 0;
}
