/**
    @file   EEPROM.h
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  interface of WasatchVCPP::EEPROM
    @note   customers normally wouldn't access this file; use WasatchVCPP.h instead
*/

#pragma once

#include "FeatureMask.h"
#include "Logger.h"

#include <cstdint>
#include <string>
#include <vector>
#include <set>
#include <map>

namespace WasatchVCPP
{
    //! Internal class encapsulating the EEPROM and its fields.
    class EEPROM
    {
        public:

            ////////////////////////////////////////////////////////////////////
            // Datatypes
            ////////////////////////////////////////////////////////////////////

            enum class Subformats
            {
                SUBFORMAT_USER_DATA = 0,
                SUBFORMAT_RAMAN_INTENSITY_CALIBRATION = 1,
                SUBFORMAT_SPLINE_WAVECAL = 2,
                SUBFORMAT_COUNT = 3
            };

            ////////////////////////////////////////////////////////////////////
            // Constants
            ////////////////////////////////////////////////////////////////////

            static const int MAX_PAGES = 8;
            static const int PAGE_SIZE = 64;

            ////////////////////////////////////////////////////////////////////
            // Methods
            ////////////////////////////////////////////////////////////////////

            EEPROM(Logger& logger);

            bool parse(const std::vector<std::vector<uint8_t> >& pages);

            void stringifyAll();
            void stringify(const std::string& name, const std::string& value);
            bool hasLaserPowerCalibration(void);
            float laserPowermWToPercent(float mW);
            ////////////////////////////////////////////////////////////////////
            // Attributes
            ////////////////////////////////////////////////////////////////////

            Logger& logger;
            std::map<std::string, std::string> stringified;
            std::vector<std::vector<uint8_t> > pages;

            ////////////////////////////////////////////////////////////////////
            // EEPROM fields
            ////////////////////////////////////////////////////////////////////

            uint8_t format = 0;

            std::string model;
            std::string serialNumber;
            uint32_t baudRate = 0;
            bool hasCooling = false;
            bool hasBattery = false;
            bool hasLaser = false;
            float excitationNM = 0;
            uint16_t slitSizeUM = 0;

            uint16_t startupIntegrationTimeMS = 0;
            int16_t startupDetectorTemperatureDegC = 0;
            uint8_t startupTriggeringMode = 0;
            float detectorGain = 0;
            int16_t detectorOffset = 0;
            float detectorGainOdd = 0;
            int16_t detectorOffsetOdd = 0;

            float wavecalCoeffs[5] = { 0 };
            float degCToDACCoeffs[3] = { 0 };
            int16_t detectorTempMax = 0;
            int16_t detectorTempMin = 0;
            float adcToDegCCoeffs[3] = { 0 };
            int16_t thermistorResistanceAt298K = 0;
            int16_t thermistorBeta = 0;
            std::string calibrationDate;
            std::string calibrationBy;

            std::string detectorName;
            uint16_t activePixelsHoriz = 0;
            uint16_t activePixelsVert = 0;
            uint16_t minIntegrationTimeMS = 0;
            uint16_t maxIntegrationTimeMS = 0;
            uint16_t actualPixelsHoriz = 0;
            uint16_t ROIHorizStart = 0;
            uint16_t ROIHorizEnd = 0;
            uint16_t ROIVertRegionStart[3] = { 0 };
            uint16_t ROIVertRegionEnd[3] = { 0 };
            float linearityCoeffs[5] = { 0 };

            float laserPowerCoeffs[4] = { 0 };
            float maxLaserPowerMW = 0;
            float minLaserPowerMW = 0;

            std::vector<uint8_t> userData;
            std::string userText;

            std::set<int16_t> badPixels;
            std::string productConfiguration;

            uint8_t intensityCorrectionOrder = 0;
            std::vector<float> intensityCorrectionCoeffs;
            
            float avgResolution = 0;

            Subformats subformat;

            FeatureMask featureMask;
    };
}

