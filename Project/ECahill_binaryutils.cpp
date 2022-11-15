//#include <stdint.h>
//#include <stdio.h>
#include "ECahill_binaryutils.h"

void setbit(uint32_t* addr, uint8_t whichbit) {		//Sets a specific bit (whichbit) in a 32-bit number
	*addr = *addr | (1 << whichbit);
}

void clearbit(uint32_t* addr, uint8_t whichbit) {	//Clears a specific bit (whichbit) in a 32-bit number
	*addr = *addr & ~(1 << whichbit);
}

void setbits(uint32_t* addr, uint32_t bitmask) {	//Sets multiple bits given a bitmask
	int n = 0;
	while (n < 33) {
		*addr = *addr | bitmask;
		n++;
	}
}

void clearbits(uint32_t* addr, uint32_t bitmask) {	//Clears multiple bits given a bitmask
	int n = 0;
	while (n < 33) {
		*addr = *addr & ~(bitmask);
		n++;
	}
}

void display_binary(int num) {						//Displays a given integer in binary
	int n = 0;
	int bin[32];
	int i;
	//printf("%i\n", num);

	for (i = 0; num > 0; i++) {
		bin[i] = num % 2;
		num = num / 2;
		n++;
	}

	while (n < 33) {
		printf("0");
		n++;
	}

	for (i = i - 1; i >= 0; i--) {
		printf("%d", bin[i]);
	}
	printf("\n");
}