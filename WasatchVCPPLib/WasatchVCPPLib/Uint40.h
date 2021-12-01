#pragma once
#include <mutex>

namespace WasatchVCPP
{
    class Uint40 {
    public:
        uint16_t LSW;
        uint16_t MidW;
        uint8_t MSB;
        uint8_t buf[3];
		Uint40(long long val);
    };
}