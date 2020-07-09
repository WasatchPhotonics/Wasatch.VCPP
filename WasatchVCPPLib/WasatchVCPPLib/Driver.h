/**
    @file   Driver.h
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  interface of WasatchCPP::Driver
*/

#pragma once

#ifdef WASATCHVCPPLIB_EXPORTS
#define WASATCHVCPPLIB_API __declspec(dllexport)
#else
#define WASATCHVCPPLIB_API __declspec(dllimport)
#endif

#include "libusb.h"

#include <string>
#include <vector>
#include <fstream>

namespace WasatchVCPP
{
    class Spectrometer;

    class Driver
    {
        public:
            static Driver* getInstance();

            int getNumberOfSpectrometers();
            int openAllSpectrometers();
            Spectrometer* getSpectrometer(int index);

            // logging
            bool setLogfile(const std::string& pathname);
            void log(const char* fmt, ...);

        private:
            static Driver* instance;
            Driver(); 

            std::ofstream logfile;
            std::vector<Spectrometer*> spectrometers;
    };
}
