/**
    @file   Logger.h
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  interface of WasatchVCPP::Logger
    @note   customers normally wouldn't access this file; use WasatchVCPPWrapper.h 
            or WasatchVCPP::Proxy instead
*/

#pragma once

#include <string>
#include <fstream>

namespace WasatchVCPP
{
    //! A simple logger for now.
    class Logger
    {
        enum Levels { LOG_DEBUG, LOG_INFO, LOG_ERROR };

        public:
            Levels level = LOG_DEBUG;

            void debug(const char* fmt, ...);
            void info(const char* fmt, ...);
            void error(const char* fmt, ...);

            bool setLogfile(const std::string& pathname);

        private:
            void output(const std::string& lvlName, const std::string& msg);
            std::ofstream logfile;
    };
}
