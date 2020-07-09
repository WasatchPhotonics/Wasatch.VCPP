/**
    @file   Driver.h
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  interface of WasatchVCPP::Driver
    @note   customers normally wouldn't access this file; use WasatchVCPPWrapper.h 
            or WasatchVCPP::Proxy instead
*/

#pragma once

#ifdef WASATCHVCPPLIB_EXPORTS
#define WASATCHVCPPLIB_API __declspec(dllexport)
#else
#define WASATCHVCPPLIB_API __declspec(dllimport)
#endif

#include "libusb.h"
#include "Logger.h"

#include <string>
#include <vector>

//! encapsulates all classes of the WasatchVCPP USB spectrometer driver
namespace WasatchVCPP
{
    class Spectrometer;

    /**
        @brief  This is an internal class encapsulating state and control of all
                connected spectrometers.

        End-users / customers would not normally access this class directly; they 
        should interact with the library through WasatchVCPPWrapper.h or 
        WasatchVCPP::Proxy.
    */
    class Driver
    {
        public:
            static Driver* getInstance();

            int getNumberOfSpectrometers();
            int openAllSpectrometers();
            Spectrometer* getSpectrometer(int index);
            bool setLogfile(const std::string& pathname);

        private:
            static Driver* instance;
            Driver(); 

            std::vector<Spectrometer*> spectrometers;
            Logger logger;
    };
}
