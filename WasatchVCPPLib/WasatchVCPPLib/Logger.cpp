/**
    @file   Logger.cpp
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  implementation of WasatchVCPP::Logger
    @note   customers normally wouldn't access this file; use WasatchVCPP.h instead

    @see usb_set_debug(int level)
*/

#include "pch.h"
#include "Logger.h"
#include "Util.h"

using std::string;
using std::vector;

#define BUF_SIZE 256

bool WasatchVCPP::Logger::setLogfile(const string& pathname)
{
    logfile.open(pathname);
    return logfile.is_open();
}

void WasatchVCPP::Logger::debug(const char* fmt, ...)
{
    if (level > LOG_LEVEL_DEBUG)
        return;

    char buf[BUF_SIZE];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    output("DEBUG", buf);
}

void WasatchVCPP::Logger::info(const char* fmt, ...)
{
    if (level > LOG_LEVEL_INFO)
        return;

    char buf[BUF_SIZE];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    output("INFO", buf);
}

void WasatchVCPP::Logger::error(const char* fmt, ...)
{
    if (level > LOG_LEVEL_ERROR)
        return;

    char buf[BUF_SIZE];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    output("ERROR", buf);
}

void WasatchVCPP::Logger::output(const string& lvlName, const string& msg)
{
    string line = Util::sprintf("%s [%s] %s\r\n", Util::timestamp().c_str(), lvlName.c_str(), msg.c_str());
    OutputDebugStringA(line.c_str());

    if (logfile.is_open())
    {
        logfile << line;
        logfile.flush();
    }
}

void WasatchVCPP::Logger::hexdump(const string& prefix, const vector<uint8_t>& data)
{
    if (level > LOG_LEVEL_DEBUG)
        return;

    string line = prefix + ": 0x";
    for (int i = 0; i < data.size(); i++)
        line += Util::sprintf("%02x ", data[i]);
    debug("%s", line.c_str());
}

void WasatchVCPP::Logger::hexdump(const string& prefix, const uint8_t* buf, int len)
{
    if (level > LOG_LEVEL_DEBUG)
        return;

    string line = prefix + ": 0x";
    for (int i = 0; i < len; i++)
        line += Util::sprintf("%02x ", buf[i]);
    debug("%s", line.c_str());
}

