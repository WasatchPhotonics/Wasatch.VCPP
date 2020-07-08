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
            static std::string join(const std::set<int16_t>& values);

            template<typename T> static std::string join(const T& values)
            {
                std::string s;
                typename T::const_iterator iter = values.begin(); 
                while (iter != values.end())
                {
                    if (s.size() == 0)
                        s += sprintf("%g", *iter);
                    else
                        s += sprintf(", %g", *iter);
                    iter++;
                }
                return s;
            }
    };
}

