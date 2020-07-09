/**
    @file   Util.h
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  interface of WasatchVCPP::Util
*/

#include "pch.h"
#include "Util.h"

using std::string;
using std::set;

string WasatchVCPP::Util::sprintf(const char* fmt, ...)
{
    char str[1024];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(str, sizeof(str), fmt, args);
    va_end(args);

    return string(str);
}

string WasatchVCPP::Util::toHex(const std::vector<uint8_t>& data)
{
    string s = "0x";
    for (int i = 0; i < data.size(); i++)
        s += sprintf("%02x ", data[i]);

    return s;
}

