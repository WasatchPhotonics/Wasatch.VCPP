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
    char* buf = (char*)malloc(bytesExpected);
    int totalBytesRead = 0;

    int bytesLeftToRead = bytesExpected;
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
    return sendCmd(request, value, index, tmp, data.size());
}

vector<unsigned char> WasatchVCPP::Spectrometer::getCmd(int request, int value, int index, int len, int timeout)
{
    vector<unsigned char> retval;

    char* data = (char*)malloc(len);
    memset(data, 0, len);

    int result = usb_control_msg(udev, DEVICE_TO_HOST, request, value, index, data, len, timeout);
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

