/**
    @file   WasatchVCPPProxy.cpp
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @date   8-Jul-2020
    @brief  implementation of WasatchCPPProxy
*/

#include "WasatchVCPPProxy.h"

#include "WasatchVCPPWrapper.h"

using std::vector;

int WasatchVCPP::numberOfSpectrometers = 0;
vector<WasatchVCPP::SpectrometerProxy> spectrometers;

bool WasatchVCPP::connect()
{
    spectrometers.clear();

    numberOfSpectrometers = wp_open_all_spectrometers();
    if (numberOfSpectrometers <= 0)
        return false;

    for (int i = 0; i < numberOfSpectrometers; i++)
        spectrometers.push_back(SpectrometerProxy(i));
}

bool WasatchVCPP::close()
{
    for (int i = 0; i < numberOfSpectrometers; i++)
        spectrometers[i].close();
    spectrometers.clear()
}

////////////////////////////////////////////////////////////////////////////////
//
//                           Spectrometer Proxy
//
////////////////////////////////////////////////////////////////////////////////

WasatchVCPP::SpectrometerProxy::SpectrometerProxy(int specIndex)
{
    this->specIndex = specIndex;

    pixels = wp_get_pixels(specIndex);
    spectrumBuf = (double*)malloc(pixels * sizeof(double));

    char tmp[33];
    if (WP_SUCCESS == wp_get_serial_number(specIndex, tmp, sizeof(tmp)))
        serialNumber.assign(tmp);

    if (WP_SUCCESS == wp_get_model(specIndex, tmp, sizeof(tmp)))
        model.assign(tmp);
}

void WasatchVCPP::SpectrometerProxy::close()
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
}
