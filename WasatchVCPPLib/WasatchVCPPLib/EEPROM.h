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

            enum Subformats
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
            std::string toString() const;

            void stringifyAll();
            void stringify(const std::string& name, const std::string& value);

            ////////////////////////////////////////////////////////////////////
            // Attributes
            ////////////////////////////////////////////////////////////////////

            Logger& logger;
            std::map<std::string, std::string> stringified;

            ////////////////////////////////////////////////////////////////////
            // EEPROM fields
            ////////////////////////////////////////////////////////////////////

            uint8_t format;

            std::string model;
            std::string serialNumber;
            uint32_t baudRate;
            bool hasCooling;
            bool hasBattery;
            bool hasLaser;
            float excitationNM;
            uint16_t slitSizeUM;

            uint16_t startupIntegrationTimeMS;
            int16_t startupDetectorTemperatureDegC;
            uint8_t startupTriggeringMode;
            float detectorGain;
            int16_t detectorOffset;
            float detectorGainOdd;
            int16_t detectorOffsetOdd;

            float wavecalCoeffs[5];
            float degCToDACCoeffs[3];
            int16_t detectorTempMax;
            int16_t detectorTempMin;
            float adcToDegCCoeffs[3];
            int16_t thermistorResistanceAt298K;
            int16_t thermistorBeta;
            std::string calibrationDate;
            std::string calibrationBy;

            std::string detectorName;
            uint16_t activePixelsHoriz;
            uint16_t activePixelsVert;
            uint16_t minIntegrationTimeMS;
            uint16_t maxIntegrationTimeMS;
            uint16_t actualPixelsHoriz;
            uint16_t ROIHorizStart;
            uint16_t ROIHorizEnd;
            uint16_t ROIVertRegionStart[3];
            uint16_t ROIVertRegionEnd[3];
            float linearityCoeffs[5];

            float laserPowerCoeffs[4];
            float maxLaserPowerMW;
            float minLaserPowerMW;

            std::vector<uint8_t> userData;
            std::string userText;

            std::set<int16_t> badPixels;
            std::string productConfiguration;

            uint8_t intensityCorrectionOrder;
            std::vector<float> intensityCorrectionCoeffs;
            
            float avgResolution;

            Subformats subformat;

            FeatureMask featureMask;
    };
}

