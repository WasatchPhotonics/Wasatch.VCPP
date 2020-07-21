/**
    @file   ParseData.cpp
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  implementation of WasatchVCPP::ParseData
    @note   customers normally wouldn't access this file; use WasatchVCPP.h instead
*/

#include "pch.h"
#include "ParseData.h"

using std::string;
using std::vector;

////////////////////////////////////////////////////////////////////////////////
// Parsing EEPROM (convert buffer to native types)
////////////////////////////////////////////////////////////////////////////////

bool WasatchVCPP::ParseData::toBool(const vector<uint8_t>& buf, int index)
{
    if (index >= (int)buf.size())
        return false;

    return buf[index] != 0;
}

uint8_t WasatchVCPP::ParseData::toUInt8(const vector<uint8_t>& buf, int index)
{
    if (index >= (int)buf.size())
        return 0;

    return buf[index];
}

//! assumes little endian
int16_t WasatchVCPP::ParseData::toInt16(const vector<uint8_t>& buf, int index, bool bigEndian)
{
    uint16_t raw = toUInt16(buf, index, bigEndian);
    return *((int16_t*)&raw);
}

//! assumes little endian
uint16_t WasatchVCPP::ParseData::toUInt16(const vector<uint8_t>& buf, int index, bool bigEndian)
{
    if (index + 1 >= (int)buf.size())
        return 0;

    uint16_t raw = bigEndian ? ((buf[index] << 8) | buf[index + 1])
                             : ((buf[index + 1] << 8) | buf[index]);
    return raw;
}

//! integration time is a 24-bit value
uint32_t WasatchVCPP::ParseData::toUInt24(const vector<uint8_t>& buf, int index, bool bigEndian)
{
    if (index + 2 >= (int)buf.size())
        return 0;

    uint32_t raw = bigEndian ? ((buf[index + 2]) | (buf[index + 1] << 8) | (buf[index] << 16))
                             : ((buf[index + 2] << 16) | (buf[index + 1] <<  8) | (buf[index]));
    return raw;
}

//! assumes little endian
int32_t WasatchVCPP::ParseData::toInt32(const vector<uint8_t>& buf, int index, bool bigEndian)
{
    uint32_t raw = toUInt32(buf, index, bigEndian);
    return *((int32_t*)&raw);
}

//! assumes little endian
uint32_t WasatchVCPP::ParseData::toUInt32(const vector<uint8_t>& buf, int index, bool bigEndian)
{
    if (index + 3 >= (int)buf.size())
        return 0;

    uint32_t raw;
    if (bigEndian)
        raw = (buf[index + 3]) 
            | (buf[index + 2] <<  8)
            | (buf[index + 1] << 16)
            | (buf[index + 0] << 24);
    else
        raw = (buf[index + 3] << 24) 
            | (buf[index + 2] << 16)
            | (buf[index + 1] <<  8)
            | (buf[index + 0]);
    return raw;
}

//! used for laser modulation
uint64_t WasatchVCPP::ParseData::toUInt40(const vector<uint8_t>& buf, int index, bool bigEndian)
{
    if (index + 4 >= (int)buf.size())
        return 0;

    uint64_t raw = 0;
    if (bigEndian)
        raw = (((uint64_t)buf[index + 0]) << 32)
            |            (buf[index + 1]  << 24)
            |            (buf[index + 2]  << 16)
            |            (buf[index + 3]  << 8)
            |            (buf[index + 4]);
    else
        raw = (((uint64_t)buf[index + 4]) << 32)
            |            (buf[index + 3]  << 24)
            |            (buf[index + 2]  << 16)
            |            (buf[index + 1]  << 8)
            |            (buf[index + 0]);
    return raw;
}


float WasatchVCPP::ParseData::toFloat (const vector<uint8_t>& buf, int index)
{
    uint32_t raw = toUInt32(buf, index);
    float f = *((float*)&raw);
    if (isnan(f))
        f = 0;
    return f;
}

string WasatchVCPP::ParseData::toString(const vector<uint8_t>& buf, int index, int len)
{
    if (len == 0)
        len = (int)buf.size();

    std::string s;
    for (int i = 0; i < len; i++)
        if (index + i < (int)buf.size())
            if (buf[index + 1] == 0)
                break;
            else
                s += (char)buf[index + i];
    return s;
}

////////////////////////////////////////////////////////////////////////////////
// Writing EEPROM (convert native types to buffer)
////////////////////////////////////////////////////////////////////////////////

bool WasatchVCPP::ParseData::writeBool(bool value, vector<uint8_t>& buf, int index)
{
    if (index >= (int)buf.size())
        return false;

    buf[index] = value ? 1 : 0;
    return true;
}

bool WasatchVCPP::ParseData::writeUInt8(uint8_t value, vector<uint8_t>& buf, int index)
{
    if (index >= (int)buf.size())
        return false;

    buf[index] = value;
    return true;
}

bool WasatchVCPP::ParseData::writeUInt16(uint16_t value, vector<uint8_t>& buf, int index)
{
    if (index + 1 >= (int)buf.size())
        return false;

    buf[index + 0] = (value     ) & 0xff;
    buf[index + 1] = (value >> 8) & 0xff;

    return true;
}

bool WasatchVCPP::ParseData::writeInt16(int16_t value, vector<uint8_t>& buf, int index)
{
    uint16_t raw = *((uint16_t*)&value);
    return writeUInt16(raw, buf, index);
}

bool WasatchVCPP::ParseData::writeUInt32(uint32_t value, vector<uint8_t>& buf, int index)
{
    if (index + 3 >= (int)buf.size())
        return false;

    buf[index + 0] = (value      ) & 0xff;
    buf[index + 1] = (value <<  8) & 0xff;
    buf[index + 2] = (value << 16) & 0xff;
    buf[index + 3] = (value << 24) & 0xff;

    return true;
}

bool WasatchVCPP::ParseData::writeInt32(int32_t value, vector<uint8_t>& buf, int index)
{
    uint32_t raw = *((uint32_t*)&value);
    return writeUInt32(raw, buf, index);
}

bool WasatchVCPP::ParseData::writeFloat(float value, vector<uint8_t>& buf, int index)
{
    uint32_t raw = *((uint32_t*)&value);
    return writeUInt32(raw, buf, index);
}

//! null-pad any remaining space within "maxLen" which is not covered by "value",
//! up to the limit of the buffer
bool WasatchVCPP::ParseData::writeString(const string& value, vector<uint8_t>& buf, int index, int maxLen)
{
    if (value.size() == 0)
    {
        // handle empty strings (write NULLs)
        for (int i = 0; i < maxLen; i++)
            if (index + i < (int)buf.size())
                buf[index + i] = 0;
            else
                break;
    }
    else
    {
        for (int i = 0; i < maxLen; i++)
            if (index + i < (int)buf.size())
            {
                if (i < (int)value.size())
                    buf[index + i] = (uint8_t)value[i];
                else
                    buf[index + i] = 0;
            }
            else
                break;
    }
    return true;
}
