#include "BWilichowski_binaryutils.h";

void setbit(uint32_t* addr, uint8_t whichbit)
{
	//shift to the desired bit and OR it with the addr
	uint32_t ans = 0;

	uint32_t temp = 1 << whichbit;
	ans = *addr | temp;
	*addr = ans; 
}

void clearbit(uint32_t* addr, uint8_t whichbit)
{
	// set the desired bit (position of which bit) to zero
	uint32_t ans = 0;

	uint32_t temp = 1 << whichbit;
	ans = *addr & ~temp;
	*addr = ans;
	
}

void setbits(uint32_t* addr, uint32_t bitmask)
{
	*addr = *addr | bitmask; //Set only bits defined in the bitmask
}

void clearbits(uint32_t* addr, uint32_t bitmask)
{
	*addr = *addr & ~bitmask; // Clear bits defined in the bitmask
}

void display_binary(uint32_t num)
{
	uint32_t bit = 2147483648; //In order to comapre bit values, I created a 32 bit variable with only bit 31
	int i = 0;

	while (i < 32) //while loop to go through all 32 bits
	{ 
		if ((bit & num) == bit) //compare the 1 bit location to the num input
		{ 
			printf("1"); //if the current location has a 1, print 1
		}
		else {
			printf("0");// else, print 0
		}
		bit = bit >> 1; //shift over to the next bit location to compare
		i++; //increment i by 1
	}
}