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

#define FRAME_BUFFER_ADDR 0xC0000000  // Starting location in DDR where we will store the images that we display.
#define MAX_SILLY_TIMER 10000000;

#define ALIEN_HEIGHT 16
#define NUM_ALIENS 11 * 32


#define WORD_WIDTH 32

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
     xil_printf("Woohoo! I made it through initialization.\n\r");
     // Now, let's get ready to start displaying some stuff on the screen.
     // The variables framePointer and framePointer1 are just pointers to the base address
     // of frame 0 and frame 1.
//     unsigned int * framePointer0 = (unsigned int *) FRAME_BUFFER_0_ADDR;
//     unsigned int * framePointer1 = ((unsigned int *) FRAME_BUFFER_0_ADDR) + 640*480;


    // Let's print out the alien as ASCII characters on the screen.
	// Each line of the alien is a 32-bit integer. We just need to strip the bits out and send
	// them to stdout.
	// MSB is the left-most pixel for the alien, so start from the MSB as we print from left to right.

     unsigned int * framePointer = (unsigned int *) FRAME_BUFFER_ADDR;

     int row, column;
     //int horiz, vert;row<ALIEN_HEIGHT
//     for (horiz = 0; horiz*WORD_WIDTH < 640; horiz++){
		for (row=0; row<ALIEN_HEIGHT; row++) {
			for (column = 0; column<640; column++) {
				if(column < NUM_ALIENS){
					if ((alien_top_out_12x8[row] & (1<<(WORD_WIDTH-1-column)))) {
						framePointer[row*640 + column] = 0x0000FF00;
					}
				} else {
					framePointer[row*640 + column] = 0x00000000;
				}
			}
		}
//		int screenHeight;
//		for(screenHeight = 16; screenHeight < 240; screenHeight ++){
//			for (column = 0; column<640; column++) {
//				framePointer[row*640 + column] = 0x00000000;
//			}
//		}
//	}


//     // Just paint some large red, green, blue, and white squares in different
//     // positions of the image for each frame in the buffer (framePointer0 and framePointer1).
//     int row=0, col=0;
//     for( row=0; row<480; row++) {
//    	 for(col=0; col<640; col++) {
//    	 if(row < 240) {
//    		 if(col<320) {
//    			 // upper left corner.
//    			 framePointer0[row*640 + col] = 0x00FF0000;  // frame 0 is red here.
//    			 framePointer1[row*640 + col] = 0x0000FF00;  // frame 1 is green here.
//    		 } else {
//    			 // upper right corner.
//    			 framePointer0[row*640 + col] = 0x000000FF;  // frame 0 is blue here.
//    			 framePointer1[row*640 + col] = 0x00FF0000;  // frame 1 is red here.
//    		 }
//    	 } else {
//    		 if(col<320) {
//    			 // lower left corner.
//    			 framePointer0[row*640 + col] = 0x0000FF00;  // frame 0 is green here.
//    			 framePointer1[row*640 + col] = 0x00FFFFFF;  // frame 1 is white here.
//    		 } else {
//    			 // lower right corner.
//    			 framePointer0[row*640 + col] = 0x00FFFFFF;  // frame 0 is white here.
//    			 framePointer1[row*640 + col] = 0x000000FF;  // frame 1 is blue here.
//    		 }
//    	 }
//       }
//     }
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
//     // Oscillate between frame 0 and frame 1.
//     int sillyTimer = MAX_SILLY_TIMER;  // Just a cheap delay between frames.

     while (1) {
//    	 while (sillyTimer) sillyTimer--;    // Decrement the timer.
//    	 sillyTimer = MAX_SILLY_TIMER;       // Reset the timer.
//         frameIndex = (frameIndex + 1) % 2;  // Alternate between frame 0 and frame 1.
//         if (XST_FAILURE == XAxiVdma_StartParking(&videoDMAController, frameIndex,  XAXIVDMA_READ)) {
//        	 xil_printf("vdma parking failed\n\r");
//         }
     }
     cleanup_platform();

    return 0;
}