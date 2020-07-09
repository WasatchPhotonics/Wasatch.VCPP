/**
    @file   Util.h
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  interface of WasatchVCPP::Util
*/

#pragma once

#include <string>
#include <vector>
#include <set>

namespace WasatchVCPP
{
    class Util
    {
        public:
            static std::string sprintf(const char* fmt, ...);
            static std::string toHex(const std::vector<uint8_t>& data);
            static std::string timestamp();

            template<typename T> static std::string join(const T& values, const char* delim = ", ")
            {
                std::string s;
                for (typename T::const_iterator i = values.begin(); i != values.end(); i++)
                    if (s.size() == 0)
                        s += sprintf("%g", *i);
                    else
                        s += sprintf("%s%g", delim, *i);
                return s;
            }
    };
}

