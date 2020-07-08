#include "pch.h"
#include "Util.h"

using std::string;
using std::set;

string WasatchVCPP::Util::sprintf(const char* fmt, ...)
{
    char str[1024];
    va_list argptr;
    va_start(argptr, fmt);
    int len = vsnprintf(str, sizeof(str), fmt, argptr);
    va_end(argptr);

    return string(str);
}

string WasatchVCPP::Util::toHex(const std::vector<uint8_t>& data)
{
    string s = "0x";
    for (int i = 0; i < data.size(); i++)
        s += sprintf("%02x ", data[i]);

    return s;
}

