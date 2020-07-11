/**
    @file   Spectrometer.h
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  interface of WasatchVCPP::Spectrometer
    @note   customers normally wouldn't access this file; use WasatchVCPP.h instead
*/

#pragma once

#include "libusb.h"
#include "EEPROM.h"
#include "Logger.h"

#include <vector>

namespace WasatchVCPP
{
    //! Internal class encapsulating state and control of one spectrometer.
    class Spectrometer
    {
        public:
            Spectrometer(usb_dev_handle* udev, int pid, Logger& logger);
            bool close();

            EEPROM eeprom;

            // convenience attributes
            int pixels = 1024;
            std::vector<double> wavelengths;
            std::vector<double> wavenumbers;
            std::string firmwareVersion;
            std::string fpgaVersion;
            bool isARM();
            bool isInGaAs();

            // cached properties
            int integrationTimeMS;
            bool laserEnabled;

            // opcodes
            bool setIntegrationTimeMS(unsigned long ms);
            bool setLaserEnable(bool flag);
            bool setDetectorGain(float value);
            bool setDetectorGainOdd(float value);
            bool setDetectorOffset(int16_t value);
            bool setDetectorOffsetOdd(int16_t value);
            bool setTECEnable(bool flag);
            bool setDetectorTECSetpointDegC(int value);
            bool setHighGainMode(bool flag);
            std::string getFirmwareVersion();
            std::string getFPGAVersion();
            int32_t getDetectorTemperatureRaw(); 
            float getDetectorTemperatureDegC();

            // acquisition
            std::vector<double> getSpectrum();

        private:
            Logger& logger;
            usb_dev_handle* udev;
            int pid;

            bool readEEPROM();
            int generateTimeoutMS();

            // control messages
            int sendCmd(int request, int value = 0, int index = 0, unsigned char* data = NULL, int len = 0);
            int sendCmd(int request, int value, int index, std::vector<unsigned char> data);
            std::vector<unsigned char> getCmd(int request, int value, int index, int len);

            // utility
            bool isSuccess(unsigned char opcode, int result);
            unsigned long clamp(unsigned long value, unsigned long min, unsigned long max);
    };
}

