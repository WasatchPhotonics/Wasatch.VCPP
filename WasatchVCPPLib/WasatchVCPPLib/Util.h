/**
    @file   Util.h
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  interface of WasatchVCPP::Util
    @note   customers normally wouldn't access this file; use WasatchVCPP.h instead
*/

#pragma once

#include <string>
#include <vector>
#include <set>

namespace WasatchVCPP
{
    //! Internal class providing static utility functions for strings and whatnot.
    //!
    //! Some of these methods are defined to wrap platform-specific implementation.
    class Util
    {
        public:
            static std::string sprintf(const char* fmt, ...);
            static std::string toHex(const std::vector<uint8_t>& data);
            static std::string toHex(const uint8_t* data, int len);
            static std::string toLower(const std::string& s);
            static std::string timestamp();
            static void sleepMS(int ms);

            //! Joins an iterable containter to a delimited string.
            //!
            //! @param values (Input) some STL collection (e.g. a set<int> or vector<double>)
            //! @param fmt (Input) printf-style format code for collection elements (e.g. "%d" or "%.2f")
            //! @param delim (Input) delimiter (i.e. ", " [default])
            //! @returns string("1, 2, 3") or string("1.11, 2.22, 3.33") etc
            template<typename T> 
            static std::string join(const T& values, const char* fmt, const char* delim = ", ")
            {
                std::string fmtDelim = sprintf("%%s%s", fmt);
                std::string s;
                for (typename T::const_iterator i = values.begin(); i != values.end(); i++)
                    if (s.size() == 0)
                        s += sprintf(fmt, *i);
                    else
                        s += sprintf(fmtDelim.c_str(), delim, *i);
                return s;
            }
    };
}

