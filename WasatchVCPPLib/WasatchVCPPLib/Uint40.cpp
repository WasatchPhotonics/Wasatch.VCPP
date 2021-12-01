#include "pch.h"
#include "Uint40.h"

WasatchVCPP::Uint40::Uint40(long long val) 
{
	const long long max = (((long long)1) << 40) - 1;
	if (val > max)
		return;
	LSW  = (uint16_t)(val & 0xffff);         // least-significant word
	MidW = (uint16_t)((val >> 16) & 0xffff); // next-least significant word
	MSB  = (uint8_t)(val >> 32);             // most-significant byte
}