// Author: Ben Wilichowski
// Date Created: 8/30/2022
// Last modified: 9/4/2022
// Code is designed to change specifically defined bits using function calls to craft a desired 32 bit number
#ifndef VARNAME
#define VARNAME
#include <stdint.h>
#include <stdio.h> 
#include <math.h>

//Declare all required functions
void setbit(uint32_t* addr, uint8_t whichbit);
void clearbit(uint32_t* addr, uint8_t whichbit);
void setbits(uint32_t* addr, uint32_t bitmask);
void clearbits(uint32_t* addr, uint32_t bitmask);
void display_binary(uint32_t num);

#endif 

