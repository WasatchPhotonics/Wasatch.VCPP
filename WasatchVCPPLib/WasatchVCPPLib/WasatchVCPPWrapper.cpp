/**
    @file   WasatchVCPPWrapper.cpp
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  Implementation of the flattened C API exported by WasatchVCPP.dll

    This file implements the "C" interface to the WasatchVCPP library and DLL.
    Customers would not normally have this file in their distribution; they
    should be fine with just the header (WasatchVCPPWrapper.h).

    This file is the one and only place where WasatchVCPP::Driver is actually
    instantiated (via its Singleton).

    This file is the one place where the "C" API calls into the C++ library
    internals; "above" this file is only C, while "below" this file is only
    C++.
*/

#include "pch.h"
#include "WasatchVCPPWrapper.h"

#include <string>
#include <map>

#include "Util.h"
#include "Driver.h"
#include "Spectrometer.h"

using WasatchVCPP::Util;
using WasatchVCPP::Driver;
using WasatchVCPP::Spectrometer;

using std::string;
using std::map;

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
    for (int i = 0; i < s.size(); i++)
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

int wp_close_all_spectrometers()
{
    for (int i = 0; i < driver->getNumberOfSpectrometers(); i++)
        wp_close_spectrometer(i);
}

int wp_close_spectrometer(int specIndex)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    return spec->close();
}

////////////////////////////////////////////////////////////////////////////////
// Gettors
////////////////////////////////////////////////////////////////////////////////

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

    for (int i = 0; i < spec->wavelengths.size(); i++)
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

    for (int i = 0; i < spec->wavenumbers.size(); i++)
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
        return WP_ERROR_INVALID_SPECTROMETER;

    auto intensities = spec->getSpectrum();
    if (intensities.empty())
        return WP_ERROR;

    for (int i = 0; i < intensities.size(); i++)
        if (i < len)
            spectrum[i] = intensities[i];
        else
            return WP_ERROR_INSUFFICIENT_STORAGE;

    return WP_SUCCESS;
}

int wp_get_eeprom_field_count(int specIndex)
{
    auto spec = driver->getSpectrometer(specIndex);
    if (spec == nullptr)
        return WP_ERROR_INVALID_SPECTROMETER;

    return (int)spec->eeprom.stringified.size();
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

    const map<string, string>& entries = spec->eeprom.stringified;
    for (map<string, string>::const_iterator i = entries.begin(); i != entries.end(); i++)
    {
        const string& name = i->first;
        const string& value = i->second;

        if (lc == Util::toLower(name))
        {
            strncpy_s(valueOut, len, value.c_str(), value.size());
            return WP_SUCCESS;
        }
    }

    // field was not found
    return WP_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
// Settors
////////////////////////////////////////////////////////////////////////////////

int wp_set_logfile_path(const char* pathname)
{
    if (!driver->setLogfile(pathname))
        return WP_ERROR;

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
