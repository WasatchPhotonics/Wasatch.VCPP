/**
    @file   WasatchVCPPProxy.cpp
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  implementation of WasatchCPP::Proxy classes
    @note   Users can copy and import this file into their own Visual C++ solutions
*/

#include "WasatchVCPPProxy.h"

#include "WasatchVCPPWrapper.h"

using std::vector;
using std::string;

int WasatchVCPP::Proxy::Driver::numberOfSpectrometers = 0;
vector<WasatchVCPP::Proxy::Spectrometer> WasatchVCPP::Proxy::Driver::spectrometers;

////////////////////////////////////////////////////////////////////////////////
// 
//                               Proxy Driver
//
////////////////////////////////////////////////////////////////////////////////

int WasatchVCPP::Proxy::Driver::openAllSpectrometers()
{
    spectrometers.clear();

    numberOfSpectrometers = wp_open_all_spectrometers();
    if (numberOfSpectrometers <= 0)
        return 0;

    for (int i = 0; i < numberOfSpectrometers; i++)
        spectrometers.push_back(Spectrometer(i));

    return numberOfSpectrometers;
}

WasatchVCPP::Proxy::Spectrometer* WasatchVCPP::Proxy::Driver::getSpectrometer(int index)
{
    if (index >= spectrometers.size())
        return nullptr;

    return &spectrometers[index];
}

bool WasatchVCPP::Proxy::Driver::closeAllSpectrometers()
{
    for (int i = 0; i < numberOfSpectrometers; i++)
        spectrometers[i].close();
    spectrometers.clear();
    return true;
}

bool WasatchVCPP::Proxy::Driver::setLogfile(const string& pathname)
{ 
    return WP_SUCCESS == wp_set_logfile_path(pathname.c_str()); 
}

////////////////////////////////////////////////////////////////////////////////
//
//                           Proxy Spectrometer 
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Lifecycle
////////////////////////////////////////////////////////////////////////////////

WasatchVCPP::Proxy::Spectrometer::Spectrometer(int specIndex)
{
    this->specIndex = specIndex;

    pixels = wp_get_pixels(specIndex);
    if (pixels <= 0)
        return;

    spectrumBuf = (double*)malloc(pixels * sizeof(double));

    char tmp[33];
    if (WP_SUCCESS == wp_get_serial_number(specIndex, tmp, sizeof(tmp)))
        serialNumber.assign(tmp);

    if (WP_SUCCESS == wp_get_model(specIndex, tmp, sizeof(tmp)))
        model.assign(tmp);

    readEEPROMFields();
}

bool WasatchVCPP::Proxy::Spectrometer::readEEPROMFields()
{
    int count = wp_get_eeprom_field_count(specIndex);
    if (count <= 0)
        return false;

    const char** names  = (const char**)malloc(count * sizeof(const char*));
    const char** values = (const char**)malloc(count * sizeof(const char*));

    auto ok = WP_SUCCESS == wp_get_eeprom(specIndex, names, values, count);

    free(names);
    free(values);

    if (!ok)
        return false;

    for (int i = 0; i < count; i++)
        eepromFields.insert(make_pair(string(names[i]), string(values[i])));

    return true;
}

bool WasatchVCPP::Proxy::Spectrometer::close()
{
    if (specIndex >= 0)
    {
        wp_close_spectrometer(specIndex);
        specIndex = -1;
    }

    if (spectrumBuf != nullptr)
    {
        free(spectrumBuf);
        spectrumBuf = nullptr;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Acquisition
////////////////////////////////////////////////////////////////////////////////

vector<double> WasatchVCPP::Proxy::Spectrometer::getSpectrum()
{
    vector<double> result;
    if (spectrumBuf != nullptr)
        if (WP_SUCCESS == wp_get_spectrum(specIndex, spectrumBuf, pixels))
            result = vector<double>(spectrumBuf, spectrumBuf + pixels * sizeof(*spectrumBuf));
    return result;
}

////////////////////////////////////////////////////////////////////////////////
// simple pass-throughs
////////////////////////////////////////////////////////////////////////////////

bool WasatchVCPP::Proxy::Spectrometer::setIntegrationTimeMS(unsigned long ms)
{ return wp_set_integration_time_ms(specIndex, ms); }

bool WasatchVCPP::Proxy::Spectrometer::setLaserEnable(bool flag)
{ return wp_set_laser_enable(specIndex, flag); }
