/**
    @file   ParseData.h
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  interface of WasatchVCPP::ParseData
    @note   customers normally wouldn't access this file; use WasatchVCPP.h instead
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace WasatchVCPP
{
    //! Internal class providing methods for reading and writing individual fields 
    //! within the EEPROM.  
    //! 
    //! It is used by EEPROM.parse() and EEPROM.write() (generally with non-zero values 
    //! for index).  However, this class is also used to demarsharl little-endian gettors
    //! from WasatchVCPP::Spectrometer.
    //! 
    //! @note all serialized data is presumed little-endian unless specified otherwise
    class ParseData
    {
        public:
            static bool         toBool  (const std::vector<uint8_t>& buf, int index = 0);
            static uint8_t      toUInt8 (const std::vector<uint8_t>& buf, int index = 0);
            static int16_t      toInt16 (const std::vector<uint8_t>& buf, int index = 0, bool bigEndian = false);
            static uint16_t     toUInt16(const std::vector<uint8_t>& buf, int index = 0, bool bigEndian = false);
            static uint32_t     toUInt24(const std::vector<uint8_t>& buf, int index = 0, bool bigEndian = false);
            static int32_t      toInt32 (const std::vector<uint8_t>& buf, int index = 0, bool bigEndian = false);
            static uint32_t     toUInt32(const std::vector<uint8_t>& buf, int index = 0, bool bigEndian = false);
            static uint64_t     toUInt40(const std::vector<uint8_t>& buf, int index = 0, bool bigEndian = false);
            static float        toFloat (const std::vector<uint8_t>& buf, int index = 0);
            static std::string  toString(const std::vector<uint8_t>& buf, int index = 0, int len = 0);

            static bool writeBool   (bool               value, std::vector<uint8_t>& buf, int index);
            static bool writeUInt8  (uint8_t            value, std::vector<uint8_t>& buf, int index);
            static bool writeUInt16 (uint16_t           value, std::vector<uint8_t>& buf, int index);
            static bool writeInt16  (int16_t            value, std::vector<uint8_t>& buf, int index);
            static bool writeUInt32 (uint32_t           value, std::vector<uint8_t>& buf, int index);
            static bool writeInt32  (int32_t            value, std::vector<uint8_t>& buf, int index);
            static bool writeFloat  (float              value, std::vector<uint8_t>& buf, int index);
            static bool writeString (const std::string& value, std::vector<uint8_t>& buf, int index, int maxLen);
    };
}

