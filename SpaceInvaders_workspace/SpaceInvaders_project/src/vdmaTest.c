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

#include <stdio.h>
#include "platform.h"
#include "xparameters.h"
#include "xaxivdma.h"
#include "xio.h"
#include "time.h"
#include "unistd.h"
#include "shapes.h"
#define DEBUG
void print(char *str);

#define FRAME_BUFFER_ADDR 0xC1000000  // Starting location in DDR where we will store the images that we display.
#define MAX_SILLY_TIMER 10000000;

#define ALIEN_HEIGHT 16
#define ALIEN_WIDTH 24

#define TANK_WIDTH 30
#define TANK_HEIGHT 16

#define BUNKER_WIDTH 48
#define BUNKER_HEIGHT 36

#define BUNKER_DAMAGE_HEIGHT 12
#define BUNKER_DAMAGE_WIDTH 12

#define X_DAMAGE 16
#define Y_DAMAGE 3
#define NUM_ALIENS 11 * 32
//0 = 48, 1 = 49, 2 = 50, 3 = 51
#define BUNKER_0 48
#define BUNKER_1 49
#define BUNKER_2 50
#define BUNKER_3 51

#define BUNKER_NUM_0_X 72
#define BUNKER_NUM_1_X 216
#define BUNKER_NUM_2_X 360
#define BUNKER_NUM_3_X 504
#define BUNKER_Y 300

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 640
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

#define WORD_WIDTH 32

int aliens_in = 1;
int aliens_alive[ALIENS_TALL][ALIENS_WIDE];
int bunkerHealth[Y_DAMAGE][X_DAMAGE] = {{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
										{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
										{3,0,0,3,3,0,0,3,3,0,0,3,3,0,0,3}};

void print_aliens(int corner_top, int corner_left, int direction){
	unsigned int * framePointer = (unsigned int *) FRAME_BUFFER_ADDR;
	int i;

	aliens_in = !aliens_in;		//toggle aliens from out to in
	if(direction == DOWN){
		int i,j;
		for(i=corner_top - (ALIEN_HEIGHT + ALIEN_BUFFER); i<corner_top; i++){
			for(j=corner_left; j<corner_left + ALIENS_WIDE*(ALIEN_BUFFER+ALIEN_WIDTH); j++){
				framePointer[i*640 + j] = 0x00000000;
			}
		}
	}
	if(direction == RIGHT){
		int i,j;
		for(i=corner_top; i<corner_top+ALIENS_TALL*(ALIEN_HEIGHT+ALIEN_BUFFER); i++){
			for(j=corner_left-ALIEN_BUFFER; j<corner_left; j++){
				framePointer[i*640 + j] = 0x00000000;
			}
		}
	}
	for(i=0; i<ALIENS_TALL; i++){
		int j;
		for(j=0; j<ALIENS_WIDE; j++){
			int inner_row, inner_column;
			for (inner_row=0; inner_row<ALIEN_HEIGHT+ALIEN_BUFFER; inner_row++) {
				for (inner_column = 0; inner_column<ALIEN_WIDTH+ALIEN_BUFFER; inner_column++) {
					if(aliens_alive[i][j]){
						if(inner_row<ALIEN_HEIGHT && inner_column<ALIEN_WIDTH){
							if(i==0){
								if(aliens_in){
									if ((alien_top_in_24x16[inner_row] & (1<<(ALIEN_WIDTH-1-inner_column)))) {
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*640 + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = 0x00FFFFFF;
									}else{
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*640 + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = 0x00000000;
									}
								}
								else{
									if ((alien_top_out_24x16[inner_row] & (1<<(ALIEN_WIDTH-1-inner_column)))) {
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*640 + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = 0x00FFFFFF;
									}else{
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*640 + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = 0x00000000;
									}
								}
							}
							else if(i==1 || i==2){
								if(aliens_in){
									if ((alien_middle_in_24x16[inner_row] & (1<<(ALIEN_WIDTH-1-inner_column)))) {
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*640 + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = 0x00FFFFFF;
									}else{
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*640 + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = 0x00000000;
									}
								}
								else{
									if ((alien_middle_out_24x16[inner_row] & (1<<(ALIEN_WIDTH-1-inner_column)))) {
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*640 + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = 0x00FFFFFF;
									}else{
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*640 + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = 0x00000000;
									}
								}
							}
							else if(i==3 || i==4){
								if(aliens_in){
									if ((alien_bottom_in_24x16[inner_row] & (1<<(ALIEN_WIDTH-1-inner_column)))) {
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*640 + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = 0x00FFFFFF;
									}else{
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*640 + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = 0x00000000;
									}
								}
								else{
									if ((alien_bottom_out_24x16[inner_row] & (1<<(ALIEN_WIDTH-1-inner_column)))) {
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*640 + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = 0x00FFFFFF;
									}else{
										framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*640 + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = 0x00000000;
									}
								}
							}
						}
						else{
							framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*640 + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = 0x00000000;
						}
					}
					else{
						framePointer[(inner_row + i*(ALIEN_HEIGHT+ALIEN_BUFFER) + corner_top)*640 + (inner_column + j*(ALIEN_WIDTH+ALIEN_BUFFER) + corner_left)] = 0x00000000;
					}
				}
			}
		}
	}
}

void erase_alien(int corner_top, int corner_left){
	unsigned int * framePointer = (unsigned int *) FRAME_BUFFER_ADDR;
	int row,column;
	for (row=corner_top; row<corner_top+ALIEN_HEIGHT; row++) {
			for (column = corner_left; column<corner_left+ALIEN_WIDTH; column++) {
				framePointer[row*640 + column] = 0x00000000;
			}
	}
}

void drawTank(left_corner, top_corner, draw){
	int color;
	if(draw == 1){
		color = 0x0000FF00;
	}else{
		color = 0x00000000;
	}
	unsigned int * framePointer = (unsigned int *) FRAME_BUFFER_ADDR;
	int row, column;
	for (row=top_corner; row<top_corner+TANK_HEIGHT; row++) {
	    for (column = left_corner; column<left_corner+TANK_WIDTH; column++) {
			if ((tank_30x16[row-top_corner] & (1<<(TANK_WIDTH-1-(column-left_corner))))) {
				framePointer[(row)*640 + (column)] = color;
			}else{
				framePointer[(row)*640 + (column)] = 0x00000000;
			}
		}
	 }
}
void drawBunkerDamage(x_pos, y_pos, damageType){
	unsigned int * framePointer = (unsigned int *) FRAME_BUFFER_ADDR;
	int row, column;
	for (row=y_pos; row<y_pos+BUNKER_DAMAGE_HEIGHT; row++) {
	    for (column = x_pos; column<x_pos+BUNKER_DAMAGE_WIDTH; column++) {
	    	if(damageType == 3){
	    		if ((bunkerDamage2_12x12[row-y_pos] & (1<<(BUNKER_DAMAGE_WIDTH-1-(column-x_pos))))) {
					framePointer[(row)*640 + (column)] = 0x0000FF00;
				}else{
					framePointer[(row)*640 + (column)] = 0x00000000;
				}
	    	}else if(damageType == 2){
	    		if ((bunkerDamage1_12x12[row-y_pos] & (1<<(BUNKER_DAMAGE_WIDTH-1-(column-x_pos))))) {
					framePointer[(row)*640 + (column)] = 0x0000FF00;
				}else{
					framePointer[(row)*640 + (column)] = 0x00000000;
				}
	    	}else if(damageType == 1){
	    		if ((bunkerDamage0_12x12[row-y_pos] & (1<<(BUNKER_DAMAGE_WIDTH-1-(column-x_pos))))) {
					framePointer[(row)*640 + (column)] = 0x0000FF00;
				}else{
					framePointer[(row)*640 + (column)] = 0x00000000;
				}
	    	}else if(damageType == 0){
				framePointer[(row)*640 + (column)] = 0x00000000;
	    	}

		}
	 }
}
void erodeBunker(bunkerNumber){//drawBunkerDamage(bunkerx+(16*xBunkerDamage), bunkery, damageType);
	//#define BUNKER_NUM_0_X 72
	//#define BUNKER_NUM_1_X 216
	//#define BUNKER_NUM_2_X 360
	//#define BUNKER_NUM_3_X 504
	//#define BUNKER_Y 300
	//This will get the current damage of the place you are going to erode.
	int bunkPosY;
	int bunkDamage;
	int bunkYOffset = 0;

	int randNum = rand() % 4;
	xil_printf("%d",randNum);
	int bunkPosX = (bunkerNumber * 4) + randNum;

	if(bunkerHealth[0][bunkPosX] == -1){
		if(bunkerHealth[1][bunkPosX] == -1){
			if(bunkerHealth[2][bunkPosX] == -1) {
				//do nothing
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
	xil_printf("going to draw bunker damage\n\r");
	xil_printf("%d\n\r",bunkPosX);
	xil_printf("%d\n\r",bunkPosY);
	xil_printf("%d\n\r",bunkDamage);
	drawBunkerDamage(bunkPosX, bunkPosY, bunkDamage);
}


void drawBunker(left_corner, top_corner, draw){
	int color;
	if(draw == 1){
		color = 0x0000FF00;
	}else{
		color = 0x00000000;
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
				framePointer[(row)*640 + (column)] = color;
			}else{
				framePointer[(row)*640 + (column)] = 0x00000000;
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

int main()
{
	init_platform();                   // Necessary for all programs.
	int Status;                        // Keep track of success/failure of system function calls.
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
    myFrameBuffer.VertSizeInput = 480;    // 480 vertical pixels.
    myFrameBuffer.HoriSizeInput = 640*4;  // 640 horizontal (32-bit pixels).
    myFrameBuffer.Stride = 640*4;         // Dont' worry about the rest of the values.
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
     xil_printf("Woohooz! I made it through initialization.\n\r");

     // Now, let's get ready to start displaying some stuff on the screen.
     // The variables framePointer and framePointer1 are just pointers to the base address
     // of frame 0 and frame 1.
//     unsigned int * framePointer0 = (unsigned int *) FRAME_BUFFER_0_ADDR;
//     unsigned int * framePointer1 = ((unsigned int *) FRAME_BUFFER_0_ADDR) + 640*480;


    // Let's print out the alien as ASCII characters on the screen.
	// Each line of the alien is a 32-bit integer. We just need to strip the bits out and send
	// them to stdout.
	// MSB is the left-most pixel for the alien, so start from the MSB as we print from left to right.

//     unsigned int * framePointer = (unsigned int *) FRAME_BUFFER_ADDR;


//     // This tells the HDMI controller the resolution of your display (there must be a better way to do this).
     XIo_Out32(XPAR_AXI_HDMI_0_BASEADDR, 640*480);
//
//     // Start the DMA for the read channel only.
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
     xil_printf("initialized!");
	int y, x = 0;
	unsigned int * frameP = (unsigned int *) FRAME_BUFFER_ADDR;
	for (y=0; y<SCREEN_HEIGHT; y++) {
		for (x = 0; x<SCREEN_WIDTH; x++) {
			frameP[(y)*640 + (x)] = 0x00000000;
		}
	 }
     ///////////////////////////////////

     /////////////////////////////
     //initialize tank on screen//
     int draw = 1;
     int tankPosX = 0;
     int tankPosY = 414;
	 drawTank(tankPosX, tankPosY, draw);
	 /////////////////////////////

	 /////////////////////////////
	 //initialize bunkers on screen//
	 draw = 1;
	 int bunkerPosX = 72;
	 int bunkerPosY = 300;
	 drawBunker(bunkerPosX, bunkerPosY, draw);
	 bunkerPosX = 216;
	 drawBunker(bunkerPosX, bunkerPosY, draw);
	 bunkerPosX = 360;
	 drawBunker(bunkerPosX, bunkerPosY, draw);
	 bunkerPosX = 504;
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
    	 char c = getchar();
    	 xil_printf("%d",(u_int)c);
    	 if((u_int)c == 54){//key 6 so move right
    		 if(tankPosX < SCREEN_WIDTH - TANK_WIDTH){
    			 draw = 0;
    			 drawTank(tankPosX, tankPosY, draw);
    			 draw = 1;
    			 tankPosX += 3;
				 drawTank(tankPosX, tankPosY, draw);
    		 }
    	 }else if((u_int)c == 52){//key 4 so move left
    		 if(tankPosX > 0){
    			 draw = 0;
    			 drawTank(tankPosX, tankPosY, draw);
    			 draw = 1;
    			 tankPosX -= 3;
				 drawTank(tankPosX, tankPosY, draw);
    		 }
		 }else if((u_int)c == 55){//key 7 so erode bunker
			 char bunkerNum = getchar(); //0 = 48, 1 = 49, 2 = 50, 3 = 51
			 if(bunkerNum == BUNKER_0){
				 int bunker0 = 0;
				 erodeBunker(bunker0);
			 }else if(bunkerNum == BUNKER_1){
				 int bunker1 = 1;
				 erodeBunker(bunker1);
			 }else if(bunkerNum == BUNKER_2){
				 int bunker2 = 2;
				 erodeBunker(bunker2);
			 }else if(bunkerNum == BUNKER_3){
				 int bunker3 = 3;
				 erodeBunker(bunker3);
			 }
		 }
    	 else if((uint)c==56){//update alien position if c='8'
    		 if(direction == DOWN){	//if aliens have already gone down
    			 if(aliens_x == 0){	//if aliens have hit the left edge of the screen
    				 direction = RIGHT;
    			 }
    			 else{		//if aliens have hit the right edge of the screen
    				 direction = LEFT;
    			 }
    		 }
    		 else if(aliens_x == 0 || aliens_x == SCREEN_WIDTH - ALIENS_WIDE*(ALIEN_BUFFER+ALIEN_WIDTH)){	//if aliens hit the left or right edges
    			 direction = DOWN;
    		 }
    		 if(direction == DOWN){
    			 aliens_y = aliens_y + ALIEN_VERTICAL_MOVE + ALIEN_BUFFER;
    		 }
    		 else if(direction == LEFT){
    			 aliens_x = aliens_x - ALIEN_HORIZ_MOVE;
    		 }
    		 else{			//if direction == RIGHT
    			 aliens_x = aliens_x + ALIEN_HORIZ_MOVE;
    		 }
    		 print_aliens(aliens_y, aliens_x, direction);
    	 }
    	 else if((uint)c==50){
    		 c = getchar();
    		 char c2 = getchar();
    		 int index = (((u_int)c)-48)*10+((uint)c2-48);
    		 xil_printf("%d",index);
    		 int alien_y_index = index/ALIENS_WIDE;
    		 int alien_x_index = index%ALIENS_WIDE;
    		 aliens_alive[alien_y_index][alien_x_index] = 0;
    		 erase_alien(aliens_y+alien_y_index*(ALIEN_HEIGHT+ALIEN_BUFFER), aliens_x+alien_x_index*(ALIEN_WIDTH+ALIEN_BUFFER));
    	 }
     }
     cleanup_platform();

    return 0;
}
