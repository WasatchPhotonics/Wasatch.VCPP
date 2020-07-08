#pragma once

#include "libusb.h"
#include "EEPROM.h"

#include <vector>

namespace WasatchVCPP
{
    class Driver;

    class Spectrometer
    {
        public:
            Spectrometer(usb_dev_handle* udev);
            bool close();

            EEPROM eeprom;

            // convenience attributes
            int pixels = 1024;
            int timeoutMS = 1000;
            std::vector<double> wavelengths;
            std::vector<double> wavenumbers;

            // cached properties
            int integrationTimeMS;
            bool laserEnabled;

            // opcodes
            bool setIntegrationTimeMS(unsigned long ms);
            bool setLaserEnable(bool flag);

            // acquisition
            std::vector<double> getSpectrum();

        private:
            usb_dev_handle* udev;
            Driver* driver;

            bool readEEPROM();

            // control messages
            int sendCmd(int request, int value = 0, int index = 0, unsigned char* data = NULL, int len = 0);
            int sendCmd(int request, int value, int index, std::vector<unsigned char> data);
            std::vector<unsigned char> getCmd(int request, int value, int index, int len);

            // utility
            bool isSuccess(unsigned char opcode, int result);
            unsigned long clamp(unsigned long value, unsigned long min, unsigned long max);
    };
}

