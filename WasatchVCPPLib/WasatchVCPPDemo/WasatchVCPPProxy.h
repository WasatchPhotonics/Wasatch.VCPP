/**
    @file   WasatchVCPPProxy.h
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  interface of WasatchCPP::Proxy classes
    @note   Users can copy and import this file into their own Visual C++ solutions
*/
#pragma once

#include <vector>
#include <string>

namespace WasatchVCPP
{
    /**
        @brief provides object-oriented Driver and Spectrometer C++ classes on 
               the client (caller) side

        Okay, why do we need this namespace, and the classes within it?

        Well, we DON'T technically need it.  You can code against WasatchVCPPWrapper.h
        directly if you're fine talking over a C API.

        However, the WasatchCPPWrapper interface is somewhat clunky old-style C, and
        that's not the level of elegance and user-friendliness that I want to expose
        to our customers (or use myself for that matter).

        Therefore, this WasatchVCPP::Proxy namespace provides a handy object-oriented
        facade to the flatted C API the DLL exports in order to avoid ABI entanglements.

        @see README_ARCHITECTURE.md
    */
    namespace Proxy
    {
        //! A Proxy Spectrometer class providing an object-oriented / STL-based
        //! interface to customer code.
        class Spectrometer
        {
            public:
                Spectrometer(int specIndex);
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
                static bool setLogfile(const std::string& pathname);
                static int openAllSpectrometers();
                static bool closeAllSpectrometers();

                //! @warning the caller should not 'delete' or 'free' this pointer;
                //!          it will be released automatically by closeAllSpectrometers
                static Spectrometer* getSpectrometer(int index);

                static int numberOfSpectrometers;

            private:
                static std::vector<Spectrometer> spectrometers;
        };
    }
}

