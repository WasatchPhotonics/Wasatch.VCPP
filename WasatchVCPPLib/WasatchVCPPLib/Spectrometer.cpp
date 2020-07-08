#include "pch.h"
#include "Driver.h"
#include "Spectrometer.h"

using std::vector;

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

const int HOST_TO_DEVICE = 0x40;
const int DEVICE_TO_HOST = 0xC0;

unsigned long MAX_UINT24 = 16777216;

////////////////////////////////////////////////////////////////////////////////
// Lifecycle
////////////////////////////////////////////////////////////////////////////////

WasatchVCPP::Spectrometer::Spectrometer(usb_dev_handle* udev)
{
    this->udev = udev;
    driver = Driver::getInstance();
    driver->log("Spectrometer::ctor: instantiating");

    readEEPROM();

    ////////////////////////////////////////////////////////////////////////////
    // post-eeprom initialization
    ////////////////////////////////////////////////////////////////////////////

    pixels = eeprom.activePixelsHoriz;

    wavelengths.resize(pixels);
    for (int i = 0; i < pixels; i++)
        wavelengths[i] = eeprom.wavecalCoeffs[0] 
                       + eeprom.wavecalCoeffs[1] * i 
                       + eeprom.wavecalCoeffs[2] * i * i
                       + eeprom.wavecalCoeffs[3] * i * i * i
                       + eeprom.wavecalCoeffs[4] * i * i * i * i;

    if (eeprom.excitationNM > 0)
    {
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
}

bool WasatchVCPP::Spectrometer::close()
{
    usb_release_interface(udev, 0);
    usb_close(udev);
}

bool WasatchVCPP::Spectrometer::readEEPROM()
{
    vector<vector<uint8_t> > pages;
    for (int page = 0; page < EEPROM::MAX_PAGES; page++)
    {
        driver->log("reading EEPROM page %d", page);
        auto buf = getCmd(0x99, 0x01, page, EEPROM::PAGE_SIZE);
        pages.push_back(buf);
    }

    if (!eeprom.parse(pages))
    {
        driver->log("ERROR: unable to parse EEPROM");
        return false;
    }

    driver->log("EEPROM Contents:\n%s", eeprom.toString().c_str());
    
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

    driver->log("integrationTimeMS -> %lu", ms);

    return isSuccess(0xb2, result);
}

bool WasatchVCPP::Spectrometer::setLaserEnable(bool flag)
{
    int result = sendCmd(0xbe, flag ? 1 : 0);
    laserEnabled = flag;

    driver->log("laserEnable -> %d", flag);

    return isSuccess(0xbe, result);
}

////////////////////////////////////////////////////////////////////////////////
// Acquisition
////////////////////////////////////////////////////////////////////////////////

//! @todo support 2048-pixel detectors
std::vector<double> WasatchVCPP::Spectrometer::getSpectrum()
{
    vector<double> spectrum;

    driver->log("sending ACQUIRE");
    sendCmd(0xad);

    Sleep(integrationTimeMS);

    int ep = 0x82;
    int bytesExpected = pixels * 2;
    int totalBytesRead = 0;
    int bytesLeftToRead = bytesExpected;

    char* buf = (char*)malloc(bytesExpected);

    while (totalBytesRead < bytesExpected)
    {
        driver->log("attempting to read %d bytes from endpoint 0x%02x", bytesLeftToRead, ep);
        int bytesRead = usb_bulk_read(udev, ep, buf, bytesLeftToRead, timeoutMS);
        driver->log("read %d bytes from endpoint 0x%02x", bytesRead, ep);

        if (bytesRead <= 0)
        {
            driver->log("getSpectrum: error (bytesRead negative or zero), giving up");
            driver->log("getSpectrum: usb error: %s", usb_strerror());
            break;
        }

        if (bytesRead % 2 != 0)
        {
            driver->log("getSpectrum: error: read odd number of bytes (%d)", bytesRead);
            break;
        }

        for (int i = 0; i + 1 < bytesRead; i += 2)
        {
            unsigned short pixel = buf[i] | (buf[i + 1] << 8);
            spectrum.push_back(pixel);
        }

        totalBytesRead += bytesRead;
        bytesLeftToRead -= bytesRead;

        driver->log("getSpectrum: totalBytesRead %d, bytesLeftToRead %d", totalBytesRead, bytesLeftToRead);
    }

    if (buf != nullptr)
        free(buf);

    driver->log("getSpectrum: returning spectrum of %d pixels", spectrum.size());
    return spectrum;
}

////////////////////////////////////////////////////////////////////////////////
// Control Messages
////////////////////////////////////////////////////////////////////////////////

int WasatchVCPP::Spectrometer::sendCmd(int request, int value, int index, unsigned char* data, int len)
{
    int result = usb_control_msg(udev, HOST_TO_DEVICE, request, value, index, (char*)data, len, timeoutMS);
    return result;
}

int WasatchVCPP::Spectrometer::sendCmd(int request, int value, int index, vector<unsigned char> data)
{
    unsigned char* tmp = (unsigned char*)&data[0];
    return sendCmd(request, value, index, tmp, (int)data.size());
}

vector<unsigned char> WasatchVCPP::Spectrometer::getCmd(int request, int value, int index, int len)
{
    vector<unsigned char> retval;

    char* data = (char*)malloc(len);
    memset(data, 0, len);

    int result = usb_control_msg(udev, DEVICE_TO_HOST, request, value, index, data, len, timeoutMS);
    for (int i = 0; i < result; i++)
        retval.push_back(data[i]);

    free(data);
    return retval;
}

////////////////////////////////////////////////////////////////////////////////
// Utility
////////////////////////////////////////////////////////////////////////////////

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

