#pragma once

#include "libusb.h"

#include <vector>

#ifdef WASATCHVCPPLIB_EXPORTS
#define WASATCHVCPPLIB_API __declspec(dllexport)
#else
#define WASATCHVCPPLIB_API __declspec(dllimport)
#endif


namespace WasatchVCPP
{
    class Driver;

    class Spectrometer
    {
        public:
            Spectrometer(usb_dev_handle* udev);

            int pixels = 1024;
            int timeoutMS = 1000;
            int integrationTimeMS = 0;

            // opcodes
            WASATCHVCPPLIB_API bool setIntegrationTimeMS(unsigned long ms);

            // acquisition
            WASATCHVCPPLIB_API std::vector<double> getSpectrum();

        private:
            usb_dev_handle* udev;
            Driver* driver;

            // control messages
            int sendCmd(int request, int value = 0, int index = 0, unsigned char* data = NULL, int len = 0);
            int sendCmd(int request, int value, int index, std::vector<unsigned char> data);
            std::vector<unsigned char> getCmd(int request, int value, int index, int len, int timeout);

            // utility
            bool isSuccess(unsigned char opcode, int result);
            unsigned long clamp(unsigned long value, unsigned long min, unsigned long max);
    };
}

