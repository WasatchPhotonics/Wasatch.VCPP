#include "pch.h"
#include "WasatchVCPPWrapper.h"

#include <string>

#include "Driver.h"
#include "Spectrometer.h"

using std::string;
using WasatchVCPP::Driver;
using WasatchVCPP::Spectrometer;

////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////

Driver* driver = Driver::getInstance();

////////////////////////////////////////////////////////////////////////////////
// helper functions
////////////////////////////////////////////////////////////////////////////////

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
    for (int i = 0; i < intensities.size(); i++)
        if (i < len)
            spectrum[i] = intensities[i];
        else
            return WP_ERROR_INSUFFICIENT_STORAGE;

    return WP_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// Settors
////////////////////////////////////////////////////////////////////////////////

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

int wp_set_logfile_path(const char* pathname)
{
    if (!driver->setLogfile(pathname))
        return WP_ERROR;

    return WP_SUCCESS;
}
