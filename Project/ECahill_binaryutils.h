/* Erin Cahill
*  August 30, 2022
*  Project 0 - Bitwise ops and C practice
*/

#pragma once
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#ifndef var
#define var

void setbit(uint32_t* addr, uint8_t whichbit);

void clearbit(uint32_t* addr, uint8_t whichbit);

void setbits(uint32_t* addr, uint32_t bitmask);

void clearbits(uint32_t* addr, uint32_t bitmask);

void display_binary(int num);

#endif

