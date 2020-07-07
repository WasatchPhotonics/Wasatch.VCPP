#pragma once

#ifdef WASATCHVCPPLIB_EXPORTS
#define WASATCHVCPPLIB_API __declspec(dllexport)
#else
#define WASATCHVCPPLIB_API __declspec(dllimport)
#endif

#include "libusb.h"

#include <string>
#include <vector>

namespace WasatchVCPP
{
    class Spectrometer;

    class Driver
    {
        public:
            WASATCHVCPPLIB_API static Driver* getInstance();

            WASATCHVCPPLIB_API int getNumberOfSpectrometers();
            WASATCHVCPPLIB_API int openAllSpectrometers();
            WASATCHVCPPLIB_API Spectrometer* getSpectrometer(int index);

            WASATCHVCPPLIB_API void log(const char* fmt, ...);

            WASATCHVCPPLIB_API void setLogBuffer(std::string& buf);

        private:
            static Driver* instance;

            Driver(); 

            std::string* logBuffer;

            // std::vector<usb_dev_handle*> devices;
            std::vector<Spectrometer*> spectrometers;
    };

    WASATCHVCPPLIB_API int open_all_spectrometers();
}
