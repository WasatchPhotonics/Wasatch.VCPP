#pragma once
#include <mutex>

namespace WasatchVCPP
{
    class Uint40 {
    public:
        uint16_t ret_buf[3];
        uint16_t lsw;
        uint16_t msw;
        uint16_t msb;
        Uint40(long long val);
    };
}