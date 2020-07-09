/**
    @file   ParseData.h
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  interface of WasatchVCPP::ParseData
    @note   customers normally wouldn't access this file; use WasatchVCPPWrapper.h 
            or WasatchVCPP::Proxy instead
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace WasatchVCPP
{
    //! This class provides methods for reading and writing individual fields 
    //! within the EEPROM.  It is primarily used by EEPROM.parse() and EEPROM.write().
    class ParseData
    {
        public:
            static bool         toBool  (const std::vector<uint8_t>& buf, int index);
            static uint8_t      toUInt8 (const std::vector<uint8_t>& buf, int index);
            static int16_t      toInt16 (const std::vector<uint8_t>& buf, int index);
            static uint16_t     toUInt16(const std::vector<uint8_t>& buf, int index);
            static int32_t      toInt32 (const std::vector<uint8_t>& buf, int index);
            static uint32_t     toUInt32(const std::vector<uint8_t>& buf, int index);
            static float        toFloat (const std::vector<uint8_t>& buf, int index);
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

