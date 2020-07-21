/**
    @file   Logger.h
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  interface of WasatchVCPP::Logger
    @note   customers normally wouldn't access this file; use WasatchVCPP.h instead
*/

#pragma once

#include <fstream>
#include <string>
#include <vector>

namespace WasatchVCPP
{
    //! Internal logger (outputs to textfile if configured).
    class Logger
    {
        public:
            //! keep synchronized with like-named macros in WasatchVCPP.h
            enum Levels 
            { 
                LOG_LEVEL_DEBUG = 0, 
                LOG_LEVEL_INFO  = 1, 
                LOG_LEVEL_ERROR = 2, 
                LOG_LEVEL_NEVER = 3
            };

            Levels level = LOG_LEVEL_DEBUG;

            void debug(const char* fmt, ...);
            void info(const char* fmt, ...);
            void error(const char* fmt, ...);

            void hexdump(const std::string& prefix, const uint8_t* buf, int len);
            void hexdump(const std::string& prefix, const std::vector<uint8_t>& data);

            bool setLogfile(const std::string& pathname);

        private:
            void output(const std::string& lvlName, const std::string& msg);
            std::ofstream logfile;
    };
}

