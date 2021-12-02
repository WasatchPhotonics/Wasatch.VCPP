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

#include <stdarg.h> 

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
    if (level > Levels::LOG_LEVEL_DEBUG)
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
    if (level > Levels::LOG_LEVEL_INFO)
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
    if (level > Levels::LOG_LEVEL_ERROR)
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

#if _WINDOWS
    OutputDebugStringA(line.c_str());
#endif

    if (logfile.is_open())
    {
        logfile << line;
        logfile.flush();
    }
}
