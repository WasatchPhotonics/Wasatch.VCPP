/**
    @file   FeatureMask.cpp
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  implementation of WasatchVCPP::FeatureMask
*/

#include "pch.h"
#include "FeatureMask.h"

WasatchVCPP::FeatureMask::FeatureMask(uint16_t value)
{
    invertXAxis = 0 != (value & FLAG_INVERT_X_AXIS);
    bin2x2      = 0 != (value & FLAG_BIN_2X2);
}

uint16_t WasatchVCPP::FeatureMask::toUInt16()
{
    uint16_t value = 0;
    if (invertXAxis) value |= FLAG_INVERT_X_AXIS;
    if (bin2x2)      value |= FLAG_BIN_2X2;
    return value;
}
