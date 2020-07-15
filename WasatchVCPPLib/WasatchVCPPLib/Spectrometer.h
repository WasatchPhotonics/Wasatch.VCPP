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
    class Driver;

    //! Internal class encapsulating state and control of one spectrometer.
    //!
    //! All "Hungarian notation" parameter names (bRequest, wValue etc) are taken 
    //! from public USB specifications to avoid confusion.
    //!
    //! @see https://www.beyondlogic.org/usbnutshell/usb6.shtml
    class Spectrometer
    {
        public:
            //! keep synchronized with WasatchVCPP.h WP_ERROR_*
            enum ErrorCodes 
            {
                Success             =  0,
                Error               = -1,
                InvalidSpectrometer = -2,
                InsufficientStorage = -3,
                NoLaser             = -4,
                NotInGaAs           = -5,
                InvalidGain         = -256,
                InvalidTemperature  = -999,
                InvalidOffset       = -32768 
            };

            Spectrometer(usb_dev_handle* udev, int pid, Logger& logger);
            bool close();

            EEPROM eeprom;
            Driver* driver;

            // public metadata
            std::vector<double> wavelengths;
            std::vector<double> wavenumbers;
            bool isARM();
            bool isInGaAs();
            bool isMicro();
            int maxTimeoutMS = 1000;
            bool operationCancelled;

            // cached properties
            int pixels = 1024;
            std::string firmwareVersion;
            std::string fpgaVersion;
            int integrationTimeMS;
            bool laserEnabled;
            int detectorTECSetointDegC = ErrorCodes::InvalidTemperature;

            // opcodes
            bool setIntegrationTimeMS(unsigned long ms);
            bool setLaserEnable(bool flag);
            bool setDetectorGain(float value);
            bool setDetectorGainOdd(float value);
            bool setDetectorOffset(int16_t value);
            bool setDetectorOffsetOdd(int16_t value);
            bool setDetectorTECEnable(bool flag);
            bool setDetectorTECSetpointDegC(int value);
            bool setHighGainModeEnable(bool flag);
            std::string getFirmwareVersion();
            std::string getFPGAVersion();
            int32_t getDetectorTemperatureRaw(); 
            float getDetectorTemperatureDegC();
            unsigned long getIntegrationTimeMS();
            bool getLaserEnable();
            float getDetectorGain();
            float getDetectorGainOdd();
            int getDetectorOffset();
            int getDetectorOffsetOdd();
            bool getDetectorTECEnable();
            int getDetectorTECSetpointDegC();
            bool getHighGainModeEnable();

            // public to support wp_send/read_control_msg()
            int sendCmd(uint8_t bRequest, uint16_t wValue = 0, uint16_t wIndex = 0, uint8_t* data = NULL, int len = 0);
            std::vector<uint8_t> getCmd(uint8_t bRequest, int len, uint16_t wIndex=0, int fullLen=0);

            // acquisition
            std::vector<double> getSpectrum();

        ////////////////////////////////////////////////////////////////////////
        // Private attributes
        ////////////////////////////////////////////////////////////////////////
        private:
            usb_dev_handle* udev;
            int pid;

            std::vector<uint8_t> endpoints;
            int pixelsPerEndpoint;
            uint8_t* bufSubspectrum;

            bool detectorTECSetpointHasBeenSet;

            Logger& logger;

        ////////////////////////////////////////////////////////////////////////
        // Private methods
        ////////////////////////////////////////////////////////////////////////
        private:
            // initialization
            bool readEEPROM();

            // acquisition 
            std::vector<uint16_t> getSubspectrum(uint8_t ep, long allocatedMS);
            long generateTotalWaitMS();

            // control messages
            int sendCmd(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, std::vector<uint8_t> data);
            std::vector<uint8_t> getCmd2(uint16_t wValue, int len, uint16_t wIndex=0, int fullLen=0);
            std::vector<uint8_t> getCmdReal(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, int len, int fullLen);

            // utility
            bool isSuccess(unsigned char opcode, int result);
            unsigned long clamp(unsigned long value, unsigned long min, unsigned long max);
            float deserializeGain(const std::vector<uint8_t>& data);

    };
}

