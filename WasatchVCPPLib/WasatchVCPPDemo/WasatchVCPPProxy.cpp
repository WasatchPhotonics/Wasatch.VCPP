/**
    @file   WasatchVCPPProxy.cpp
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @date   8-Jul-2020
    @brief  implementation of WasatchCPPProxy
*/

#include "WasatchVCPPProxy.h"

#include "WasatchVCPPWrapper.h"

using std::vector;
using std::string;

int WasatchVCPP::Driver::numberOfSpectrometers = 0;
vector<WasatchVCPP::SpectrometerProxy> WasatchVCPP::Driver::spectrometers;

int WasatchVCPP::Driver::openAllSpectrometers()
{
    spectrometers.clear();

    numberOfSpectrometers = wp_open_all_spectrometers();
    if (numberOfSpectrometers <= 0)
        return 0;

    for (int i = 0; i < numberOfSpectrometers; i++)
        spectrometers.push_back(SpectrometerProxy(i));

    return numberOfSpectrometers;
}

WasatchVCPP::SpectrometerProxy* WasatchVCPP::Driver::getSpectrometer(int index)
{
    if (index >= spectrometers.size())
        return nullptr;

    return &spectrometers[index];
}

bool WasatchVCPP::Driver::closeAllSpectrometers()
{
    for (int i = 0; i < numberOfSpectrometers; i++)
        spectrometers[i].close();
    spectrometers.clear();
    return true;
}

bool WasatchVCPP::Driver::setLogfile(const string& pathname)
{ 
    return WP_SUCCESS == wp_set_logfile_path(pathname.c_str()); 
}

////////////////////////////////////////////////////////////////////////////////
//
//                           Spectrometer Proxy
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Lifecycle
////////////////////////////////////////////////////////////////////////////////

WasatchVCPP::SpectrometerProxy::SpectrometerProxy(int specIndex)
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
}

bool WasatchVCPP::SpectrometerProxy::close()
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

vector<double> WasatchVCPP::SpectrometerProxy::getSpectrum()
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

bool WasatchVCPP::SpectrometerProxy::setIntegrationTimeMS(unsigned long ms)
{ return wp_set_integration_time_ms(specIndex, ms); }

bool WasatchVCPP::SpectrometerProxy::setLaserEnable(bool flag)
{ return wp_set_laser_enable(specIndex, flag); }
