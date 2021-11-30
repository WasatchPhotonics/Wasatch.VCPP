/**
    @file   EEPROM.cpp
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  implementation of WasatchVCPP::EEPROM
    @note   customers normally wouldn't access this file; use WasatchVCPP.h instead
*/

#include "pch.h"
#include "EEPROM.h"
#include "ParseData.h"
#include "Util.h"

using std::vector;
using std::string;
using std::set;

WasatchVCPP::EEPROM::EEPROM(Logger& logger)
    : logger(logger)
{
}

bool WasatchVCPP::EEPROM::parse(const vector<vector<uint8_t> >& pages_in)
{
    // cache so caller can retrieve if desired
    pages = pages_in;

    format = ParseData::toUInt8(pages[0], 63);

    model = ParseData::toString(pages[0], 0, 16);
    serialNumber = ParseData::toString(pages[0], 16, 16);
    baudRate = ParseData::toUInt32(pages[0], 32);
    hasCooling = ParseData::toBool(pages[0], 36);
    hasBattery = ParseData::toBool(pages[0], 37);
    hasLaser = ParseData::toBool(pages[0], 38);
    excitationNM = ParseData::toUInt16(pages[0], 39); // for old formats, first read this as excitation
    slitSizeUM = ParseData::toUInt16(pages[0], 41);

    startupIntegrationTimeMS = ParseData::toUInt16(pages[0], 43);
    startupDetectorTemperatureDegC = ParseData::toInt16(pages[0], 45);
    startupTriggeringMode = ParseData::toUInt8(pages[0], 47);
    detectorGain = ParseData::toFloat(pages[0], 48); 
    detectorOffset = ParseData::toInt16(pages[0], 52); 
    detectorGainOdd = ParseData::toFloat(pages[0], 54); 
    detectorOffsetOdd = ParseData::toInt16(pages[0], 58);

    wavecalCoeffs[0] = ParseData::toFloat(pages[1], 0);
    wavecalCoeffs[1] = ParseData::toFloat(pages[1], 4);
    wavecalCoeffs[2] = ParseData::toFloat(pages[1], 8);
    wavecalCoeffs[3] = ParseData::toFloat(pages[1], 12);
    degCToDACCoeffs[0] = ParseData::toFloat(pages[1], 16);
    degCToDACCoeffs[1] = ParseData::toFloat(pages[1], 20);
    degCToDACCoeffs[2] = ParseData::toFloat(pages[1], 24);
    detectorTempMax = ParseData::toInt16(pages[1], 28);
    detectorTempMin = ParseData::toInt16(pages[1], 30);
    adcToDegCCoeffs[0] = ParseData::toFloat(pages[1], 32);
    adcToDegCCoeffs[1] = ParseData::toFloat(pages[1], 36);
    adcToDegCCoeffs[2] = ParseData::toFloat(pages[1], 40);
    thermistorResistanceAt298K = ParseData::toInt16(pages[1], 44);
    thermistorBeta = ParseData::toInt16(pages[1], 46);
    calibrationDate = ParseData::toString(pages[1], 48, 12);
    calibrationBy = ParseData::toString(pages[1], 60, 3);

    detectorName = ParseData::toString(pages[2], 0, 16);
    activePixelsHoriz = ParseData::toUInt16(pages[2], 16); // note: byte 18 unused
    activePixelsVert = ParseData::toUInt16(pages[2], 19);
    minIntegrationTimeMS = ParseData::toUInt16(pages[2], 21); // will overwrite if 
    maxIntegrationTimeMS = ParseData::toUInt16(pages[2], 23); //   format >= 5
    actualPixelsHoriz = ParseData::toUInt16(pages[2], 25);
    ROIHorizStart = ParseData::toUInt16(pages[2], 27);
    ROIHorizEnd = ParseData::toUInt16(pages[2], 29);
    ROIVertRegionStart[0] = ParseData::toUInt16(pages[2], 31);
    ROIVertRegionEnd[0] = ParseData::toUInt16(pages[2], 33);
    ROIVertRegionStart[1] = ParseData::toUInt16(pages[2], 35);
    ROIVertRegionEnd[1] = ParseData::toUInt16(pages[2], 37);
    ROIVertRegionStart[2] = ParseData::toUInt16(pages[2], 39);
    ROIVertRegionEnd[2] = ParseData::toUInt16(pages[2], 41);
    linearityCoeffs[0] = ParseData::toFloat(pages[2], 43);
    linearityCoeffs[1] = ParseData::toFloat(pages[2], 47);
    linearityCoeffs[2] = ParseData::toFloat(pages[2], 51);
    linearityCoeffs[3] = ParseData::toFloat(pages[2], 55);
    linearityCoeffs[4] = ParseData::toFloat(pages[2], 59);

    // deviceLifetimeOperationMinutes = ParseData::toInt32(pages[3], 0);
    // laserLifetimeOperationMinutes = ParseData::toInt32(pages[3], 4);
    // laserTemperatureMax  = ParseData::toInt16(pages[3], 8);
    // laserTemperatureMin  = ParseData::toInt16(pages[3], 10);

    laserPowerCoeffs[0] = ParseData::toFloat(pages[3], 12);
    laserPowerCoeffs[1] = ParseData::toFloat(pages[3], 16);
    laserPowerCoeffs[2] = ParseData::toFloat(pages[3], 20);
    laserPowerCoeffs[3] = ParseData::toFloat(pages[3], 24);
    maxLaserPowerMW = ParseData::toFloat(pages[3], 28);
    minLaserPowerMW = ParseData::toFloat(pages[3], 32);

    // correct laser excitation across formats
    if (format >= 4)
        excitationNM = ParseData::toFloat(pages[3], 36);

    if (format >= 5)
    {
        minIntegrationTimeMS = ParseData::toUInt32(pages[3], 40);
        maxIntegrationTimeMS = ParseData::toUInt32(pages[3], 44);
    }

    userData = pages[4]; // technically, on format < 4, should only be 63 bytes

    badPixels.clear();
    for (int i = 0; i < 15; i++)
    {
        auto pixel = ParseData::toInt16(pages[5], i * 2);
        if (pixel >= 0)
            badPixels.insert(pixel); 
    }

    if (format >= 5)
        productConfiguration = ParseData::toString(pages[5], 30, 16);
    else
        productConfiguration.clear();

    // Raman Intensity Calibration (SRM)
    intensityCorrectionOrder = 0;
    intensityCorrectionCoeffs.clear();
    if (format >= 6)
    {
        intensityCorrectionOrder = ParseData::toUInt8(pages[6], 0);
        auto numCoeffs = intensityCorrectionOrder + 1;
        if (numCoeffs > 8)
            numCoeffs = 0;
        for (int i = 0; i < numCoeffs; ++i)
            intensityCorrectionCoeffs.push_back(ParseData::toFloat(pages[6], 1 + 4 * i));
    }

    avgResolution = format >= 7 ? ParseData::toFloat(pages[3], 48) : 0.f;

    if (format >= 8)
    {
        wavecalCoeffs[4] = ParseData::toFloat(pages[2], 21);
        subformat = (Subformats) ParseData::toUInt8(pages[5], 63);
        if (subformat == Subformats::SUBFORMAT_USER_DATA)
        {
            intensityCorrectionOrder = 0;
            intensityCorrectionCoeffs.clear();
        }
    }
    else
    {
        if (format >= 6)
            subformat = Subformats::SUBFORMAT_RAMAN_INTENSITY_CALIBRATION;
        else
            subformat = Subformats::SUBFORMAT_USER_DATA;
    }

    if (format >= 9)
        featureMask = FeatureMask(ParseData::toUInt16(pages[0], 39));

    // ensure startupTemperature within bounds
    if (startupDetectorTemperatureDegC < detectorTempMin)
        startupDetectorTemperatureDegC = detectorTempMin;
    if (startupDetectorTemperatureDegC > detectorTempMax)
        startupDetectorTemperatureDegC = detectorTempMax;

    // log what we've read, while adding to the string map 
    stringifyAll();

    return true;
}

inline const char* toBool(bool b) { return b ? "true" : "false"; }

void WasatchVCPP::EEPROM::stringify(const string& name, const string& value)
{
    stringified.insert(make_pair(name, value));
    logger.debug("EEPROM: %30s = %s", name.c_str(), value.c_str());
}

bool WasatchVCPP::EEPROM::hasLaserPowerCalibration(void) {
    if (maxLaserPowerMW <= 0) {
        return false;
    }
    int coeffsLength = sizeof(laserPowerCoeffs) / sizeof(laserPowerCoeffs[0]);
    for (int i = 0; i < coeffsLength; i++) {
        if (isnan(laserPowerCoeffs[i])) {
            return false;
        }
    }
    bool allDefault = true;
    for (int i = 0; i < coeffsLength; i++) {
        if (i == 0 && laserPowerCoeffs[i] != 1.0) {
            allDefault = false;
        }
        else if (laserPowerCoeffs[i] != 0.0) {
            allDefault = false;
        }
    }
    if (allDefault) {
        return false;
    }
    bool allConst = true;
    for (int i = 0; i < coeffsLength; i++) {
        if (laserPowerCoeffs[0] != laserPowerCoeffs[i]) {
            allConst = false;
        }
    }
    if (allConst) {
        return false;
    }
    return true;
}

float WasatchVCPP::EEPROM::laserPowermWToPercent(float mW) {
    if (!hasLaserPowerCalibration()) {
        return 0.0;
    }
    float perc = laserPowerCoeffs[0] \
        + laserPowerCoeffs[1] * mW \
        + laserPowerCoeffs[2] * mW * mW \
        + laserPowerCoeffs[3] * mW * mW * mW;

    return perc;
}

void WasatchVCPP::EEPROM::stringifyAll()
{
    stringified.clear();

    stringify("format", Util::sprintf("%d", format));
    stringify("model", model);
    stringify("serialNumber", serialNumber);
    stringify("baudRate", Util::sprintf("%d", baudRate));
    stringify("hasCooling", toBool(hasCooling));
    stringify("hasBattery", toBool(hasBattery));
    stringify("hasLaser", toBool(hasLaser));
    stringify("bin2x2", toBool(featureMask.bin2x2));
    stringify("invertXAxis", toBool(featureMask.invertXAxis));
    stringify("gen15", toBool(featureMask.gen15));
    stringify("cutoffFilterInstalled", toBool(featureMask.cutoffFilterInstalled));
    stringify("hardwareEvenOdd", toBool(featureMask.hardwareEvenOdd));
    stringify("excitationNM", Util::sprintf("%.3f", excitationNM));
    stringify("slitSizeUM", Util::sprintf("%u", slitSizeUM));

    stringify("startupIntegrationTimeMS", Util::sprintf("%u", startupIntegrationTimeMS));
    stringify("startupDetectorTemperatureDegC", Util::sprintf("%d", startupDetectorTemperatureDegC));
    stringify("startupTriggeringMode", Util::sprintf("%u", startupTriggeringMode));
    stringify("detectorGain", Util::sprintf("%.2f", detectorGain));
    stringify("detectorOffset", Util::sprintf("%d", detectorOffset));
    stringify("detectorGainOdd", Util::sprintf("%.2f", detectorGainOdd));
    stringify("detectorOffsetOdd", Util::sprintf("%d", detectorOffsetOdd));
                                                       
    stringify("detectorTempMax", Util::sprintf("%d", detectorTempMax));
    stringify("detectorTempMin", Util::sprintf("%d", detectorTempMin));
    stringify("thermistorResistanceAt298K", Util::sprintf("%d", thermistorResistanceAt298K));
    stringify("thermistorBeta", Util::sprintf("%d", thermistorBeta));
    stringify("calibrationDate", calibrationDate);
    stringify("calibrationBy", calibrationBy);

    stringify("detectorName", detectorName);
    stringify("activePixelsHoriz", Util::sprintf("%u", activePixelsHoriz));
    stringify("activePixelsVert", Util::sprintf("%u", activePixelsVert));
    stringify("minIntegrationTimeMS", Util::sprintf("%u", minIntegrationTimeMS));
    stringify("maxIntegrationTimeMS", Util::sprintf("%u", maxIntegrationTimeMS));
    stringify("actualPixelsHoriz", Util::sprintf("%u", actualPixelsHoriz));
    stringify("ROIHorizStart", Util::sprintf("%u", ROIHorizStart));
    stringify("ROIHorizEnd", Util::sprintf("%u", ROIHorizEnd));
    stringify("maxLaserPowerMW", Util::sprintf("%g", maxLaserPowerMW));
    stringify("minLaserPowerMW", Util::sprintf("%g", minLaserPowerMW));

    stringify("userData", Util::toHex(userData)); // should be about 195 characters
    stringify("userText", userText);

    stringify("badPixels", Util::join(badPixels, "%d"));
    stringify("productConfiguration", productConfiguration);
    stringify("avgResolution", Util::sprintf("%.2f", avgResolution));

    stringify("subformat", Util::sprintf("%u", subformat));

    stringify("intensityCorrectionOrder", Util::sprintf("%u", intensityCorrectionOrder));
    stringify("intensityCorrectionCoeffs", Util::join(intensityCorrectionCoeffs, "%g"));

    for (int i = 0; i < 5; i++)
        stringify(Util::sprintf("wavecalCoeffs[%d]", i), Util::sprintf("%g", wavecalCoeffs[i]));
    for (int i = 0; i < 3; i++)
        stringify(Util::sprintf("degCToDACCoeffs[%d]", i), Util::sprintf("%g", degCToDACCoeffs[i]));
    for (int i = 0; i < 3; i++)
        stringify(Util::sprintf("adcToDegCCoeffs[%d]", i), Util::sprintf("%g", adcToDegCCoeffs[i]));
    for (int i = 0; i < 3; i++)
        stringify(Util::sprintf("ROIVertRegion[%d]", i), Util::sprintf("(%u, %u)", ROIVertRegionStart[i], ROIVertRegionEnd[i]));
    for (int i = 0; i < 5; i++)
        stringify(Util::sprintf("linearityCoeffs[%d]", i), Util::sprintf("%g", linearityCoeffs[i]));
    for (int i = 0; i < 4; i++)
        stringify(Util::sprintf("laserPowerCoeffs[%d]", i), Util::sprintf("%g", laserPowerCoeffs[i]));
}
