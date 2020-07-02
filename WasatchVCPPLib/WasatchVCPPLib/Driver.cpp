#include "pch.h"
#include "Driver.h"

WasatchVCPP::Driver::Driver()
{
}

int WasatchVCPP::Driver::add()
{
    return 1;
}

// Static C-type function calls
int WasatchVCPP::add()
{
    return 0;
}