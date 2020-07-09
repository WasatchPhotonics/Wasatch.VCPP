/**
    @file   WasatchVCPPProxy.h
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @date   8-Jul-2020
    @brief  interface of WasatchCPPProxy
*/
#pragma once

#include <vector>
#include <string>

/**
    @brief namespace encapsulating WasatchVCPPProxy

    Okay, why do we need this namespace, and the classes within it?

    Because DLL boundaries strongly discourage use of any classes, objects, 
    templates (including STL) of any kind.  In fact, the guidance is basically
    to avoid C++ types altogether, and define C++ boundaries strictly using
    legacy C datatypes:

    https://stackoverflow.com/a/22797419/11615696

    That means the WasatchCPPWrapper interface is somewhat clunky old-style
    C, and that's not the level of elegance and user-friendliness that I want to
    expose to our customers.

    Therefore, this WasatchVCPPProxy namespace provides an encapsulated way
    to talk to our spectrometers using modern C++ and STL conventions.

    @see README.md#Architecture
*/
namespace WasatchVCPP
{
    class SpectrometerProxy
    {
        public:
            SpectrometerProxy(int specIndex);
            bool close();

            int specIndex;
            int pixels;
            std::string model;
            std::string serialNumber;

            std::vector<double> wavelengths;
            std::vector<double> wavenumbers;

            bool setIntegrationTimeMS(unsigned long ms);
            bool setLaserEnable(bool flag);
            std::vector<double> getSpectrum();

        private:
            double* spectrumBuf;
    };

    class Driver
    {
        public:
            static int openAllSpectrometers();
            static bool closeAllSpectrometers();
            static int numberOfSpectrometers;

            static SpectrometerProxy* getSpectrometer(int index);

            static bool setLogfile(const std::string& pathname);

        private:
            static std::vector<SpectrometerProxy> spectrometers;
    };
}

