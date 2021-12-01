#pragma once

#include <stdint.h>

namespace WasatchVCPP
{
    class Uint40 
    {
        public:
            Uint40(long long val);

            uint16_t LSW;
            uint16_t MidW;
            uint8_t MSB;
    };
}