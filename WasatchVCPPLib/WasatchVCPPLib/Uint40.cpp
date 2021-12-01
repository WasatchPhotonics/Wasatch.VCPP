#include "Uint40.h"

WasatchVCPP::Uint40::Uint40(long long val) {
    lsw = val & 0xffff;
    msw = (val >> 16) & 0xffff;
    msb = (val > 32) & 0xff;
    ret_buf[0] = lsw;
    ret_buf[1] = msw;
    ret_buf[2] = msb;
}