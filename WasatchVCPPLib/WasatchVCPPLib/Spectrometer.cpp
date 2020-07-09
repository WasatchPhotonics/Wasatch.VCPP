/**
    @file   Spectrometer.cpp
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  implementation of WasatchVCPP::Spectrometer
*/

#include "pch.h"
#include "Spectrometer.h"
#include "Util.h"

#include <algorithm>

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

    //! @todo apply gain/offset from EEPROM to FPGA
    //! @todo apply vertical ROI from EEPROM to FPGA, if isMicro()

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
        auto buf = getCmd(0xff, 0x01, page, EEPROM::PAGE_SIZE);
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

    char* buf = (char*)malloc(bytesExpected);

    while (totalBytesRead < bytesExpected)
    {
        logger.debug("attempting to read %d bytes from endpoint 0x%02x with timeout %dms", 
            bytesLeftToRead, ep, timeoutMS);
        int bytesRead = usb_bulk_read(udev, ep, buf, bytesLeftToRead, timeoutMS);
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
            unsigned short pixel = buf[i] | (buf[i + 1] << 8);
            spectrum.push_back(pixel);
        }

        totalBytesRead += bytesRead;
        bytesLeftToRead -= bytesRead;

        logger.debug("getSpectrum: totalBytesRead %d, bytesLeftToRead %d", totalBytesRead, bytesLeftToRead);
    }

    if (buf != nullptr)
        free(buf);

    logger.debug("getSpectrum: returning spectrum of %d pixels", spectrum.size());
    return spectrum;
}

////////////////////////////////////////////////////////////////////////////////
// Control Messages
////////////////////////////////////////////////////////////////////////////////

int WasatchVCPP::Spectrometer::sendCmd(int request, int value, int index, unsigned char* data, int len)
{
    unsigned char buf[MIN_ARM_LEN] = { 0 };
    if (data == nullptr && isARM())
    {
        data = buf;
        len = sizeof(buf);
    }

    int timeoutMS = 1000;
    logger.debug("sendCmd(request 0x%02x, value 0x%04x, index 0x%04x, len %d, timeout %dms", 
        request, value, index, len, timeoutMS);
    int result = usb_control_msg(udev, HOST_TO_DEVICE, request, value, index, (char*)data, len, timeoutMS);
    return result;
}

int WasatchVCPP::Spectrometer::sendCmd(int request, int value, int index, vector<unsigned char> data)
{
    unsigned char* tmp = (unsigned char*)&data[0];
    return sendCmd(request, value, index, tmp, (int)data.size());
}

vector<uint8_t> WasatchVCPP::Spectrometer::getCmd(int request, int value, int index, int len)
{
    vector<uint8_t> retval;

    if (isARM())
        len = max(MIN_ARM_LEN, len);

    char* data = (char*)malloc(len);
    memset(data, 0, len);

    int timeoutMS = 1000;
    logger.debug("getCmd(request 0x%02x, value 0x%04x, index 0x%04x, len %d, timeout %dms", 
        request, value, index, len, timeoutMS);
    int result = usb_control_msg(udev, DEVICE_TO_HOST, request, value, index, data, len, timeoutMS);
    for (int i = 0; i < result; i++)
        retval.push_back(data[i]);

    free(data);
    return retval;
}

////////////////////////////////////////////////////////////////////////////////
// Utility
////////////////////////////////////////////////////////////////////////////////

bool WasatchVCPP::Spectrometer::isARM() { return pid == 0x4000; }

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

