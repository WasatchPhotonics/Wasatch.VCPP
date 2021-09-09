/**
    @file   WasatchVCPPWrapper.cpp
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  Implementation of the flattened C API exported by WasatchVCPP.dll
    @note   customers normally wouldn't access this file; use WasatchVCPP.h instead

    This file implements the "C" interface to the WasatchVCPP library and DLL
    which is defined in WasatchCPP.h.

    This file is the one and only place where the WasatchVCPP::Driver Singleton
    is actually instantiated (meaning it probably doesn't actually have to be
    a Singleton...)

    @todo   rename to WasatchVCPP.cpp, as the impl to WasatchVCPP.h?
*/

#include "pch.h"
#include "WasatchVCPP.h"

#include <vector>
#include <string>
#include <map>

#include <string.h> // Linux memset

#include "Util.h"
#include "Logger.h"
#include "Driver.h"
#include "Spectrometer.h"

using WasatchVCPP::Util;
using WasatchVCPP::Driver;
using WasatchVCPP::Spectrometer;
using WasatchVCPP::Logger;

using std::string;
using std::vector;
using std::map;
using std::min;

////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////

Driver* driver = Driver::getInstance();

////////////////////////////////////////////////////////////////////////////////
// helper functions
////////////////////////////////////////////////////////////////////////////////

//! copy a std::string to a C string
//!
//! @param s (Input) a populated std::string
//! @param buf (Output) the destination where the string is to be copied
//! @param len (Input) allocated size of 'buf'
//! @returns WP_SUCCESS or WP_ERROR_INSUFFICIENT_STORAGE
int exportString(string s, char* buf, int len)
{
    memset(buf, 0, len);
    for (int i = 0; i < (int)s.size(); i++)
        if (i < len)
            buf[i] = s[i];
        else
            return WP_ERROR_INSUFFICIENT_STORAGE;

    return WP_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// Lifecycle
////////////////////////////////////////////////////////////////////////////////

int wp_open_all_spectrometers()
{
    return driver->openAllSpectrometers();
}

int wp_close_spectrometer(int specIndex)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    return driver->removeSpectrometer(specIndex) ? WP_SUCCESS : WP_ERROR;
}

int wp_close_all_spectrometers()
{
    return driver->closeAllSpectrometers() ? WP_SUCCESS : WP_ERROR;
}

void wp_destroy_driver()
{
    driver->destroy();
}

////////////////////////////////////////////////////////////////////////////////
// Gettors
////////////////////////////////////////////////////////////////////////////////

int wp_get_library_version(char* value, int len)
{ 
    return exportString(driver->libraryVersion, value, len); 
}

int wp_get_number_of_spectrometers()
{
    return driver->getNumberOfSpectrometers();
}

int wp_get_pixels(int specIndex)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;
    return spec->pixels;
}

int wp_get_model(int specIndex, char* value, int len)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;
    return exportString(spec->eeprom.model, value, len);
}

int wp_get_serial_number(int specIndex, char* value, int len)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;
    return exportString(spec->eeprom.serialNumber, value, len);
}

int wp_get_wavelengths(int specIndex, double* wavelengths, int len)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    for (int i = 0; i < (int)spec->wavelengths.size(); i++)
        if (i < len)
            wavelengths[i] = spec->wavelengths[i];
        else
            return WP_ERROR_INSUFFICIENT_STORAGE;

    return WP_SUCCESS;
}

int wp_get_wavenumbers(int specIndex, double* wavenumbers, int len)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    if (spec->eeprom.excitationNM <= 0)
        return WP_ERROR_NO_LASER;

    for (int i = 0; i < (int)spec->wavenumbers.size(); i++)
        if (i < len)
            wavenumbers[i] = spec->wavenumbers[i];
        else
            return WP_ERROR_INSUFFICIENT_STORAGE;

    return WP_SUCCESS;
}

int wp_get_spectrum(int specIndex, double* spectrum, int len)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
    {
        driver->logger.error("wp_get_spectrum: invalid specIndex %d", specIndex);
        return WP_ERROR_INVALID_SPECTROMETER;
    }

    auto intensities = spec->getSpectrum();
    if (intensities.empty())
    {
        driver->logger.error("wp_get_spectrum: error generating spectrum");
        return WP_ERROR;
    }

    if (len < (int)intensities.size())
    {
        driver->logger.error("wp_get_spectrum: insufficient storage");
        return WP_ERROR_INSUFFICIENT_STORAGE;
    }

    for (int i = 0; i < (int)intensities.size(); i++)
        spectrum[i] = intensities[i];

    return WP_SUCCESS;
}

int wp_get_eeprom_field_count(int specIndex)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    return (int)spec->eeprom.stringified.size();
}

int wp_get_eeprom_field_name(int specIndex, int index, char* value, int len)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    if (index < 0 || index >= (int)spec->eeprom.stringified.size())
        return WP_ERROR; // invalid index

    int count = 0;
    for (auto i = spec->eeprom.stringified.begin(); i != spec->eeprom.stringified.end(); i++, count++)
        if (count == index)
            return exportString(i->first, value, len);

    driver->logger.error("wp_get_eeprom_field_name: impossible? (index = %d)", index);
    return WP_ERROR;
}

int wp_get_eeprom(int specIndex, const char** names, const char** values, int len)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;
    
    int count = 0;
    const map<string, string>& entries = spec->eeprom.stringified;
    for (map<string, string>::const_iterator i = entries.begin(); i != entries.end(); i++, count++)
    {
        if (count >= len)
            return WP_ERROR_INSUFFICIENT_STORAGE;

        const string& name = i->first;
        const string& value = i->second;

        names[count] = name.c_str();
        values[count] = value.c_str();
    }

    return WP_SUCCESS;
}

int wp_get_eeprom_field(int specIndex, const char* name, char* valueOut, int len)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    string lc = Util::toLower(name);

    /*
    if (lc == "userdata")
    {
        int size = min(len, 64);
        memcpy(valueOut, &(spec->eeprom.userData[0]), size);
        return WP_SUCCESS;
    }
    */

    const map<string, string>& entries = spec->eeprom.stringified;
    for (map<string, string>::const_iterator i = entries.begin(); i != entries.end(); i++)
    {
        const string& name = i->first;
        const string& value = i->second;

        if (lc == Util::toLower(name))
        {
            // if the full string (plus null) doesn't fit, just say so
            if (len < (int)value.size() + 1)
                return WP_ERROR_INSUFFICIENT_STORAGE;

#if _WINDOWS
            strncpy_s(valueOut, len, value.c_str(), value.size());
#else
            strncpy(valueOut, value.c_str(), min(len, (int)value.size()));
#endif

            valueOut[len - 1] = 0; // no matter what, null-terminate the output string
            return WP_SUCCESS;
        }
    }

    // field was not found
    return WP_ERROR;
}

int wp_get_eeprom_page(int specIndex, int page, uint8_t* buf, int len)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    if (page < 0 || page > WasatchVCPP::EEPROM::MAX_PAGES)
        return WP_ERROR;

    const vector<uint8_t>& data = spec->eeprom.pages[page];
    if (len < (int)data.size())
        return WP_ERROR_INSUFFICIENT_STORAGE;

#if _WINDOWS
    memcpy_s(buf, len, &(data[0]), data.size());
#else
    memcpy(buf, &(data[0]), min(len, (int)data.size()));
#endif

    return WP_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// Settors
////////////////////////////////////////////////////////////////////////////////

int wp_set_logfile_path(const char* pathname, int len)
{
    string s;
    for (int i = 0; i < len && pathname[i]; i++)
        s += pathname[i];

    if (!driver->logger.setLogfile(s))
        return WP_ERROR;

    return WP_SUCCESS;
}

int wp_set_log_level(int level)
{
    Logger::Levels lvl = (Logger::Levels)level;
    if (lvl < Logger::Levels::LOG_LEVEL_DEBUG || 
        lvl > Logger::Levels::LOG_LEVEL_NEVER)
        return WP_ERROR;

    driver->logger.level = lvl;
    return WP_SUCCESS;
}

int wp_log_debug(const char* msg, int len)
{ 
    if (driver->logger.level > Logger::Levels::LOG_LEVEL_DEBUG)
        return WP_ERROR;

    string s;
    for (int i = 0; i < len && msg[i]; i++)
        s += msg[i];
    driver->logger.debug("%s", s.c_str()); 
    return WP_SUCCESS;
}

int wp_set_integration_time_ms(int specIndex, unsigned long ms)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    if (!spec->setIntegrationTimeMS(ms))
        return WP_ERROR;

    return WP_SUCCESS;
}

int wp_set_laser_enable(int specIndex, int value)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    if (!spec->setLaserEnable(value))
        return WP_ERROR;

    return WP_SUCCESS;
}

int wp_set_detector_gain(int specIndex, float value)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    if (!spec->setDetectorGain(value))
        return WP_ERROR;

    return WP_SUCCESS;
}

int wp_set_detector_gain_odd(int specIndex, float value)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    if (!spec->setDetectorGainOdd(value))
        return WP_ERROR;

    return WP_SUCCESS;
}

int wp_set_detector_offset(int specIndex, int value)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    if (!spec->setDetectorOffset(value))
        return WP_ERROR;

    return WP_SUCCESS;
}

int wp_set_detector_offset_odd(int specIndex, int value)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    if (!spec->setDetectorOffsetOdd(value))
        return WP_ERROR;

    return WP_SUCCESS;
}

int wp_set_detector_tec_enable(int specIndex, int value)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    if (!spec->setDetectorTECEnable(value != 0))
        return WP_ERROR;

    return WP_SUCCESS;
}

int wp_set_detector_tec_setpoint_deg_c(int specIndex, int value)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    if (!spec->setDetectorTECSetpointDegC(value))
        return WP_ERROR;

    return WP_SUCCESS;
}

int wp_set_high_gain_mode_enable(int specIndex, int value)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    if (!spec->setHighGainModeEnable(value != 0))
        return WP_ERROR;

    return WP_SUCCESS;
}

int wp_get_firmware_version(int specIndex, char* value, int len)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    auto s = spec->getFirmwareVersion();
    if (s.empty())
        return WP_ERROR;

    return exportString(s, value, len);
}

int wp_get_fpga_version(int specIndex, char* value, int len)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    auto s = spec->getFPGAVersion();
    if (s.empty()) 
        return WP_ERROR;

    return exportString(s, value, len);
}

float wp_get_detector_temperature_deg_c(int specIndex)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_TEMPERATURE;

    return spec->getDetectorTemperatureDegC();
}

long wp_get_integration_time_ms(int specIndex)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    return spec->getIntegrationTimeMS();
}

int wp_get_laser_enable(int specIndex)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    return spec->getLaserEnable() ? 1 : 0;
}

float wp_get_detector_gain(int specIndex)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    return spec->getDetectorGain();
}

float wp_get_detector_gain_odd(int specIndex)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    return spec->getDetectorGainOdd();
}

int wp_get_detector_offset(int specIndex)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    return spec->getDetectorOffset();
}

int wp_get_detector_offset_odd(int specIndex)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    return spec->getDetectorOffsetOdd();
}

int wp_get_detector_tec_enable(int specIndex)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    return spec->getDetectorTECEnable() ? 1 : 0;
}

int wp_get_detector_tec_setpoint_deg_c(int specIndex)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    return spec->getDetectorTECSetpointDegC();
}

int wp_get_high_gain_mode_enable(int specIndex)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    return spec->getHighGainModeEnable() ? 1 : 0;
}

int wp_cancel_operation(int specIndex, int blocking)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    return spec->cancelOperation(blocking != 0) ? WP_SUCCESS : WP_ERROR;
}

int wp_set_max_timeout_ms(int specIndex, int maxTimeoutMS)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    spec->maxTimeoutMS = maxTimeoutMS;
    return WP_SUCCESS;
}

int wp_get_max_timeout_ms(int specIndex)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    return spec->maxTimeoutMS;
}

int wp_send_control_msg(int specIndex, unsigned char bRequest, unsigned int wValue,
    unsigned int wIndex, unsigned char* data, int len)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    return spec->sendCmd(bRequest, wValue, wIndex, data, len);
}

int wp_read_control_msg(int specIndex, unsigned char bRequest, unsigned int wIndex,
    unsigned char* data, int len)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    vector<uint8_t> response = spec->getCmd(bRequest, len, wIndex);

    // could use memcpy
    for (int i = 0; i < len; i++)
        if (i < (int)response.size())
            data[i] = response[i];
        else
            return WP_ERROR_INSUFFICIENT_STORAGE;

    return (int)response.size();
}
