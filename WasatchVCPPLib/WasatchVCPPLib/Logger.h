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

