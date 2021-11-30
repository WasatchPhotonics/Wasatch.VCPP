/**
    @file   Spectrometer.h
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  interface of WasatchVCPP::Spectrometer
    @note   customers normally wouldn't access this file; use WasatchVCPP.h instead
*/

#pragma once

#ifdef USE_LIBUSB_WIN32
#include "libusb.h"
#define WPVCPP_UDEV_TYPE usb_dev_handle
#else
//#include <libusb-1_0.h>
#include <libusb.h>
#define WPVCPP_UDEV_TYPE libusb_device_handle
#endif

#include "EEPROM.h"
#include "Logger.h"

#include <vector>
#include <mutex>

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
            //! @todo move to ErrorCodes.h so Driver.cpp can use
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

            Spectrometer(WPVCPP_UDEV_TYPE* udev, int pid, int index, Logger& logger);
            ~Spectrometer();

            bool close();

            EEPROM eeprom;
            Driver* driver = nullptr;     // still needed?

            // public metadata
            int pid = 0;
            int index = -1;
            std::vector<double> wavelengths;
            std::vector<double> wavenumbers;
            bool isARM();
            bool isInGaAs();
            bool isMicro();

            //! Other Wasatch drivers don't really have this concept...basically,
            //! all blocking reads to bulk endpoints (spectral reads) wait no
            //! longer than this, and loop over multiple reads if necessary, to
            //! fulfill the given integration time (plus unknown trigger wait).
            //! Keeping this value small and controllable is key to being able to
            //! interrupt and cancel long integrations (5-10min) within a 
            //! reasonable response time.
            //!
            //! @todo we probably need to remove this variable altogether when we 
            //! implement "interruptable acquisitions" in the FPGA.
            int maxTimeoutMS = 1000;

            // cached properties
            int pixels = 0;
            std::string firmwareVersion;
            std::string fpgaVersion;
            int integrationTimeMS = 1;
            bool laserEnabled = false;
            bool laserPowerHighResolution = false;
            bool laserPowerRequireModulation = false;
            bool modEnabled = false;
            float laserPowerMw = 0.0;
            float modWidthus = 0.0;
            float modPeriodus = 0.0;
            float laserPowerPerc = 0.0;
            float lastAppliedLaserPower = 0.0;
            float nextAppliedLaserPower = 0.0;
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
            bool setLaserPowerPerc(float percent);
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
            bool setLaserPowerPercImmediate(float value);
            bool setModEnable(bool flag);
            bool setModPeriodus(int us);
            bool setModWidthus(int us);
            int getModEnabled(void);
            bool getModWidthus(void);
            bool getModPeriodus(void);
            bool setLaserPowermW(float mW_in);
            uint16_t* to40bit(long long val);
            std::vector<uint8_t> getCmd(uint8_t bRequest, int len, uint16_t wIndex=0, int fullLen=0);

            // acquisition
            std::vector<double> getSpectrum();
            bool cancelOperation(bool blocking);

        ////////////////////////////////////////////////////////////////////////
        // Private attributes
        ////////////////////////////////////////////////////////////////////////
        private:
            WPVCPP_UDEV_TYPE* udev = nullptr;

            std::vector<uint8_t> endpoints;
            std::vector<uint8_t> bufSubspectrum; 
            int pixelsPerEndpoint = 0;

            bool detectorTECSetpointHasBeenSet = false;
            bool acquiring = false;
            bool operationCancelled = false;
            int cancelledIntegrationTimeMS = 0;
            bool lastAcquisitionWasCancelled = false;

            std::mutex mutAcquisition;
            std::mutex mutComm;

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
            uint16_t serializeGain(float value);
            float deserializeGain(const std::vector<uint8_t>& data);
            inline unsigned long clamp(unsigned long value, unsigned long min, unsigned long max);
            bool lockComm();
            void unlockComm();
    };
}
