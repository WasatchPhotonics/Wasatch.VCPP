/**
    @file   Spectrometer.cpp
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  implementation of WasatchVCPP::Spectrometer
    @see    WasatchVCPPWrapper.h for list of missing features
    @note   customers normally wouldn't access this file; use WasatchVCPP.h instead
*/

#include "pch.h"
#include "Spectrometer.h"
#include "Util.h"

#include <algorithm>

using std::string;
using std::vector;
using std::max;

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

const int HOST_TO_DEVICE = 0x40;
const int DEVICE_TO_HOST = 0xC0;

const int MIN_ARM_LEN = 8;

unsigned long MAX_UINT24 = 16777216;

////////////////////////////////////////////////////////////////////////////////
// Lifecycle
////////////////////////////////////////////////////////////////////////////////

WasatchVCPP::Spectrometer::Spectrometer(usb_dev_handle* udev, int pid, Logger& logger)
    : udev(udev), pid(pid), logger(logger), eeprom(logger)
{

    logger.debug("Spectrometer::ctor: instantiating");

    // read firmware versions first (useful for debugging, validates FPGA comms)
    firmwareVersion = getFirmwareVersion();
    fpgaVersion = getFPGAVersion();
    logger.debug("firmware %s, FPGA %s", firmwareVersion.c_str(), fpgaVersion.c_str());

    ////////////////////////////////////////////////////////////////////////////
    // EEPROM
    ////////////////////////////////////////////////////////////////////////////

    readEEPROM();

    ////////////////////////////////////////////////////////////////////////////
    // post-eeprom initialization
    ////////////////////////////////////////////////////////////////////////////

    pixels = eeprom.activePixelsHoriz;

    logger.debug("Spectrometer::ctor: expanding wavecal");
    wavelengths.resize(pixels);
    for (int i = 0; i < pixels; i++)
        wavelengths[i] = eeprom.wavecalCoeffs[0] 
                       + eeprom.wavecalCoeffs[1] * i 
                       + eeprom.wavecalCoeffs[2] * i * i
                       + eeprom.wavecalCoeffs[3] * i * i * i
                       + eeprom.wavecalCoeffs[4] * i * i * i * i;

    if (eeprom.excitationNM > 0)
    {
        logger.debug("Spectrometer::ctor: expanding wavenumbers");
        const double nmToCm = 1.0 / 1e7;
        const double laserCm = 1.0 / (eeprom.excitationNM * nmToCm);

        wavenumbers.resize(pixels);
        for (int i = 0; i < pixels; i++)
            if (wavelengths[i] != 0)
                wavenumbers[i] = laserCm - (1.0 / (wavelengths[i] * nmToCm));
            else
                wavenumbers[i] = 0;
    }
    else
        wavenumbers.resize(0);

    // apply configured gain/offset from EEPROM to FPGA
    setDetectorGain     (eeprom.detectorGain);
    setDetectorGainOdd  (eeprom.detectorGainOdd);
    setDetectorOffset   (eeprom.detectorOffset);
    setDetectorOffsetOdd(eeprom.detectorOffsetOdd);

    // initialize TEC and setpoint
    if (eeprom.hasCooling)
        setDetectorTECEnable(true);

    // initialize micro models
    if (isMicro())
    {
        // @todo initVerticalROI();
    }

    logger.debug("Spectrometer::ctor: done");
}

bool WasatchVCPP::Spectrometer::close()
{
    usb_release_interface(udev, 0);
    usb_close(udev);
    return true;
}

bool WasatchVCPP::Spectrometer::readEEPROM()
{
    vector<vector<uint8_t> > pages;
    for (int page = 0; page < EEPROM::MAX_PAGES; page++)
    {
        auto buf = getCmd2(0x01, EEPROM::PAGE_SIZE, page);
        pages.push_back(buf);
        logger.debug("EEPROM page %d: %s", page, Util::toHex(buf).c_str());
    }

    if (!eeprom.parse(pages))
    {
        logger.error("unable to parse EEPROM");
        return false;
    }

    logger.debug("Spectrometer::readEEPROM done");
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Opcodes
////////////////////////////////////////////////////////////////////////////////

bool WasatchVCPP::Spectrometer::setIntegrationTimeMS(unsigned long ms)
{
    ms = clamp(ms, 1, MAX_UINT24 - 1);

    unsigned short lsw = ms & 0xffff;
    unsigned short msw = (ms >> 16) & 0x00ff;

    int result = sendCmd(0xb2, lsw, msw);

    integrationTimeMS = ms;
    logger.debug("integrationTimeMS -> %lu", ms);
    return isSuccess(0xb2, result);
}

bool WasatchVCPP::Spectrometer::setLaserEnable(bool flag)
{
    int result = sendCmd(0xbe, flag ? 1 : 0);
    laserEnabled = flag;
    logger.debug("laserEnable -> %d", flag);
    return isSuccess(0xbe, result);
}

bool WasatchVCPP::Spectrometer::setDetectorGain(float value)
{
    const uint8_t op = 0xb7;
    if (value < 0 || value >= 256)
        return false;

    uint8_t msb = (int)value & 0xff;
    uint8_t lsb = (int)((value - msb) * 256) & 0xff;
    uint16_t word = (msb << 8) | lsb;

    int result = sendCmd(op, word);
    logger.debug("detectorGain -> 0x%04x (%.2f)", word, value);
    return isSuccess(op, result);
}

bool WasatchVCPP::Spectrometer::setDetectorGainOdd(float value)
{
    const uint8_t op = 0x9d;
    if (value < 0 || value >= 256)
        return false;

    uint8_t msb = (int)value & 0xff;
    uint8_t lsb = (int)((value - msb) * 256) & 0xff;
    uint16_t word = (msb << 8) | lsb;

    int result = sendCmd(op, word);
    logger.debug("detectorGainOdd -> 0x%04x (%.2f)", word, value);
    return isSuccess(op, result);
}

bool WasatchVCPP::Spectrometer::setDetectorOffset(int16_t value)
{
    const uint8_t op = 0xb6;
    uint16_t word = *((uint16_t*) &value); // send original signed int16 bit pattern
    int result = sendCmd(op, word);
    logger.debug("detectorOffset -> 0x%04x (%d)", word, value);
    return isSuccess(op, result);
}

bool WasatchVCPP::Spectrometer::setDetectorOffsetOdd(int16_t value)
{
    const uint8_t op = 0x9c;
    uint16_t word = *((uint16_t*) &value);
    int result = sendCmd(op, word);
    logger.debug("detectorOffsetOdd -> 0x%04x (%d)", word, value);
    return isSuccess(op, result);
}

bool WasatchVCPP::Spectrometer::setDetectorTECEnable(bool flag)
{
    const uint8_t op = 0xd6;
    if (!eeprom.hasCooling)
        return false;

    if (!detectorTECSetpointHasBeenSet)
    {
        logger.debug("defaulting TEC setpoint to min");
        setDetectorTECSetpointDegC(eeprom.detectorTempMin);
    }

    int result = sendCmd(op, flag ? 1 : 0);
    logger.debug("detectorTECEnable -> %s", flag ? "on" : "off");
    return isSuccess(op, result);
}

bool WasatchVCPP::Spectrometer::setDetectorTECSetpointDegC(int degC)
{
    const uint8_t op = 0xd8;
    if (!eeprom.hasCooling)
        return false;

    if (degC < eeprom.detectorTempMin || degC > eeprom.detectorTempMax)
        return false;

    float dac = eeprom.degCToDACCoeffs[0]
              + eeprom.degCToDACCoeffs[1] * degC
              + eeprom.degCToDACCoeffs[2] * degC * degC;

    // clamp and round to 12-bit DAC value
    if (dac < 0)
        dac = 0;
    if (dac > 0xfff)
        dac = 0xfff;
    uint16_t word = ((uint16_t)(dac + 0.5)) & 0xfff;

    int result = sendCmd(op, word);
    detectorTECSetpointHasBeenSet = true;

    return isSuccess(op, result);
}

//! @warning may need to send 8-byte buffer?
bool WasatchVCPP::Spectrometer::setHighGainMode(bool flag)
{
    const uint8_t op = 0xeb;
    if (!isInGaAs())
        return false;
    int result = sendCmd(op, flag ? 1 : 0);
    return isSuccess(op, result);
}

string WasatchVCPP::Spectrometer::getFirmwareVersion()
{
    string s;
    const uint8_t op = 0xc0;
    auto data = getCmd(op, 4);
    if (data.size() >= 4)
        s = Util::sprintf("%d.%d.%d.%d", data[3], data[2], data[1], data[0]);
    return s;
}

string WasatchVCPP::Spectrometer::getFPGAVersion()
{
    const uint8_t op = 0xb4;
    auto data = getCmd(op, 7);
    string s;
    for ( auto c : data )
        if (0x20 <= c && c <= 0x7f) // visible ASCII
            s += (char)c;
    return s;
}

//! @returns negative on error, else valid uint16_t
int32_t WasatchVCPP::Spectrometer::getDetectorTemperatureRaw()
{
    const uint8_t op = 0xd7;
    auto data = getCmd(op, 2);
    if (data.size() < 2)
    {
        logger.error("getDetectorTemperatureRaw: data = %s", Util::toHex(data));
        return -1;
    }
        
    uint16_t raw = (data[0] << 8) | data[1]; // MSB-LSB
    return raw;
}

float WasatchVCPP::Spectrometer::getDetectorTemperatureDegC()
{
    int32_t rawOrError = getDetectorTemperatureRaw();
    if (rawOrError < 0)
        return -999;

    uint16_t raw = (uint16_t) rawOrError;
    float degC = eeprom.adcToDegCCoeffs[0]
               + eeprom.adcToDegCCoeffs[1] * raw
               + eeprom.adcToDegCCoeffs[2] * raw * raw;

    logger.debug("detectorTemperatureDegC = %.2f (0x%04x raw)", degC, raw);
    return degC;
}

////////////////////////////////////////////////////////////////////////////////
// Acquisition
////////////////////////////////////////////////////////////////////////////////

int WasatchVCPP::Spectrometer::generateTimeoutMS()
{
    return 2 * integrationTimeMS + 2000;
}

//! @todo support 2048-pixel detectors
std::vector<double> WasatchVCPP::Spectrometer::getSpectrum()
{
    vector<double> spectrum;

    logger.debug("sending ACQUIRE");
    sendCmd(0xad);

    // logger.debug("sleeping for %lums", integrationTimeMS);
    // Sleep(integrationTimeMS);

    int ep = 0x82;
    int bytesExpected = pixels * 2;
    int totalBytesRead = 0;
    int bytesLeftToRead = bytesExpected;
    int timeoutMS = generateTimeoutMS();

    //! @todo make this an instance attribute
    uint8_t* buf = (uint8_t*)malloc(bytesExpected);

    while (totalBytesRead < bytesExpected)
    {
        logger.debug("attempting to read %d bytes from endpoint 0x%02x with timeout %dms", 
            bytesLeftToRead, ep, timeoutMS);
        int bytesRead = usb_bulk_read(udev, ep, (char*)buf, bytesLeftToRead, timeoutMS);
        logger.debug("read %d bytes from endpoint 0x%02x", bytesRead, ep);

        if (bytesRead <= 0)
        {
            logger.error("getSpectrum: bytesRead negative or zero, giving up (%s)", usb_strerror());
            break;
        }

        if (bytesRead % 2 != 0)
        {
            logger.error("getSpectrum: read odd number of bytes (%d)", bytesRead);
            break;
        }

        for (int i = 0; i + 1 < bytesRead; i += 2)
        {
            uint16_t pixel = buf[i] | (buf[i + 1] << 8);
            spectrum.push_back(pixel);
        }

        totalBytesRead += bytesRead;
        bytesLeftToRead -= bytesRead;

        logger.debug("getSpectrum: totalBytesRead %d, bytesLeftToRead %d", totalBytesRead, bytesLeftToRead);
    }

    if (buf != nullptr)
        free(buf);

    if (spectrum.empty())
    {
        logger.error("getSpectrum: returning empty spectrum");
        return spectrum;
    }

    ////////////////////////////////////////////////////////////////////////////
    // post-processing
    ////////////////////////////////////////////////////////////////////////////

    // stomp first pixel
    spectrum[0] = spectrum[1];

    logger.debug("getSpectrum: returning spectrum of %d pixels", spectrum.size());
    return spectrum;
}

////////////////////////////////////////////////////////////////////////////////
// Control Messages
////////////////////////////////////////////////////////////////////////////////

//! Send data to the spectrometer.
//!
//! All "Hungarian notation" parameter names (bRequest, wValue etc) are taken from
//! public USB specifications to avoid confusion.
//! 
//! @see https://www.beyondlogic.org/usbnutshell/usb6.shtml
int WasatchVCPP::Spectrometer::sendCmd(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint8_t* data, int len)
{
    unsigned char buf[MIN_ARM_LEN] = { 0 };
    if (data == nullptr && isARM())
    {
        data = buf;
        len = sizeof(buf);
    }

    int timeoutMS = 1000;
    logger.debug("sendCmd(bRequest 0x%02x, wValue 0x%04x, wIndex 0x%04x, len %d, timeout %dms)", 
        bRequest, wValue, wIndex, len, timeoutMS);
    int result = usb_control_msg(udev, HOST_TO_DEVICE, bRequest, wValue, wIndex, (char*)data, len, timeoutMS);
    return result;
}

//! I don't actually remember the use-case for this function
int WasatchVCPP::Spectrometer::sendData(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, vector<uint8_t> data)
{
    uint8_t* tmp = (uint8_t*)&data[0];
    return sendCmd(bRequest, wValue, wIndex, tmp, (int)data.size());
}

//! This is the standard "getter opcode" function.  It doesn't take a "wValue" 
//! parameter because getters usually don't have one.  (The main exception to 
//! that rule is so-called "2nd-tier" gettors, for which getCmd2 is provided.)
//!
//! @param request (Input) the opcode
//! @param len (Input) how many bytes the caller actually NEEDS back
//! @param fullLen (Input) how many bytes we think the spectrometer will actually
//!        send, with padding
//! @returns vector of just the 'len' needed bytes
vector<uint8_t> WasatchVCPP::Spectrometer::getCmd(uint8_t bRequest, int len, uint16_t wIndex, int fullLen)
{
    const uint16_t wValue = 0;
    return getCmdReal(bRequest, wValue, wIndex, len, fullLen);
}

//! This supports so-called "2nd-tier" gettors, all of which use opcode 0xff and
//! are instead distinguished by their 'value' parameter.
//!
//! @param value (Input) which 2nd-tier spectrometer attribute is being read
//! @param len (Input) how many bytes the caller actually NEEDS in response
//! @param fullLen (Input) how many bytes we think the spectrometer will send, 
//!        with padding
//! @returns vector of just the 'len' bytes requested
vector<uint8_t> WasatchVCPP::Spectrometer::getCmd2(uint16_t wValue, int len, uint16_t wIndex, int fullLen)
{
    const uint8_t bRequest = 0xff;
    return getCmdReal(bRequest, wValue, wIndex, len, fullLen);
}

//! The actual "gettor" function, typically called through either the getCmd or 
//! getCmd2 facades.
//!
//! All "Hungarian notation" parameter names (bRequest, wValue etc) are taken from
//! public USB specifications to avoid confusion.
//! 
//! @see getCmd
//! @see getCmd2
//! @see https://www.beyondlogic.org/usbnutshell/usb6.shtml
vector<uint8_t> WasatchVCPP::Spectrometer::getCmdReal(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, int len, int fullLen)
{
    int bytesToRead = max(len, fullLen);
    if (isARM())
        bytesToRead = max(MIN_ARM_LEN, bytesToRead);

    vector<uint8_t> retval;

    char* data = (char*)malloc(bytesToRead);
    memset(data, 0, bytesToRead);

    int timeoutMS = 1000;
    logger.debug("calling getCmdReal(bRequest 0x%02x, wValue 0x%04x, wIndex 0x%04x, len %d, timeout %dms)", 
        bRequest, wValue, wIndex, bytesToRead, timeoutMS);
    int bytesRead = usb_control_msg(udev, DEVICE_TO_HOST, bRequest, wValue, wIndex, data, bytesToRead, timeoutMS);

    if (bytesRead < 0)
    {
        logger.error("getCmd2 read error (bRequest 0x%02x)", bRequest);
        return retval;
    }
    else if (bytesRead < len) 
    {
        logger.error("getCmd2 read incomplete response (%d bytes read, %d needed, %d expected)", 
            bytesRead, len, bytesToRead);
        return retval;
    }

    for (int i = 0; i < len; i++)
        retval.push_back(data[i]);

    free(data);
    return retval;
}

////////////////////////////////////////////////////////////////////////////////
// Utility
////////////////////////////////////////////////////////////////////////////////

bool WasatchVCPP::Spectrometer::isARM() { return pid == 0x4000; }

bool WasatchVCPP::Spectrometer::isInGaAs() { return pid == 0x2000; }

bool WasatchVCPP::Spectrometer::isMicro()
{
    return isARM() && Util::toLower(eeprom.detectorName).find("IMX") != string::npos;
}

//! @todo use PID to determine appropriate result code by platform
bool WasatchVCPP::Spectrometer::isSuccess(unsigned char opcode, int result)
{
    return true;
}

unsigned long WasatchVCPP::Spectrometer::clamp(unsigned long value, unsigned long min, unsigned long max)
{
    if (value < min)
        return min;
    else if (value > max)
        return max;
    return value;
}

