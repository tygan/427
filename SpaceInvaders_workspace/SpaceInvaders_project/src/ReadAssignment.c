/*
 * ReadAssignment.c
 *
 *  Created on: Sep 28, 2015
 *      Author: superman
 */
#include <stdio.h>
//#include "platform.h"

int input=10;
int my_array[2048];
int my_init_array[] ={
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
};

int factorial (int n) {
  if (n==1)
	  return 1;
  else
	  return n * factorial(n-1);
}

int main() {
	xil_printf("123456789 123456789 123456789 123456789 ");
	factorial(input);
	return 0;
}
