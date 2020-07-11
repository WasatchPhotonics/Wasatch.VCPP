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
            bool isMicro();

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
            bool setDetectorTECEnable(bool flag);
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
            bool detectorTECSetpointHasBeenSet;

            bool readEEPROM();
            int generateTimeoutMS();

            // control messages
            int sendCmd(uint8_t bRequest, uint16_t wValue = 0, uint16_t wIndex = 0, uint8_t* data = NULL, int len = 0);
            int sendData(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, std::vector<uint8_t> data);
            std::vector<uint8_t> getCmd(uint8_t bRequest, int len, uint16_t wIndex=0, int fullLen=0);
            std::vector<uint8_t> getCmd2(uint16_t wValue, int len, uint16_t wIndex=0, int fullLen=0);
            std::vector<uint8_t> getCmdReal(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, int len, int fullLen);

            // utility
            bool isSuccess(unsigned char opcode, int result);
            unsigned long clamp(unsigned long value, unsigned long min, unsigned long max);
    };
}

