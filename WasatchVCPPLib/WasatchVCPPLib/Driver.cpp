#include "pch.h"

#include "Driver.h"
#include "Spectrometer.h"

#include <stdio.h>
#include <string>
#include <iostream>

using std::string;

////////////////////////////////////////////////////////////////////////////////
// Class Attributes
////////////////////////////////////////////////////////////////////////////////

WasatchVCPP::Driver* WasatchVCPP::Driver::instance = NULL;

////////////////////////////////////////////////////////////////////////////////
// Class Methods
////////////////////////////////////////////////////////////////////////////////
WasatchVCPP::Driver* WasatchVCPP::Driver::getInstance()
{
    if (instance == NULL)
        instance = new WasatchVCPP::Driver();
    return instance;
}

////////////////////////////////////////////////////////////////////////////////
// Lifecycle
////////////////////////////////////////////////////////////////////////////////

WasatchVCPP::Driver::Driver()
{
}

int WasatchVCPP::Driver::getNumberOfSpectrometers() { return (int)spectrometers.size(); }

int WasatchVCPP::Driver::openAllSpectrometers()
{
    this->log("openAllSpectrometers: calling usb_init");
    usb_init();
    usb_find_busses();
    usb_find_devices();

    spectrometers.clear();
    for (struct usb_bus* bus = usb_get_busses(); bus; bus = bus->next)
    {
        log("openAllSpectrometers: exploring bus %lu (%s)", bus->location, bus->dirname);
        for (struct usb_device* dev = bus->devices; dev; dev = dev->next)
        {
            log("openAllSpectrometers: discovered device VID 0x%04x, PID 0x%04x",
                dev->descriptor.idVendor,
                dev->descriptor.idProduct);

            if (dev->descriptor.idVendor == 0x24aa)
            {
                unsigned pid = dev->descriptor.idProduct;
                if (pid == 0x1000 || pid == 0x2000 || pid == 0x4000)
                {
                    log("openAllSpectrometers: trying to open device");
                    struct usb_dev_handle* udev = usb_open(dev);
                    if (udev != nullptr)
                    {
                        log("openAllSpectrometers: open succeeded");
                        if (dev->descriptor.bNumConfigurations)
                        {
                            log("openAllSpectrometers: setting configuration");
                            int configResult = usb_set_configuration(udev, 1);
                            if (configResult != 0)
                            {
                                log("openAllSpectrometers: error setting configuration 1 (result %d): %s", 
                                    configResult, usb_strerror());
                                usb_close(udev);
                                continue;
                            }

                            log("openAllSpectrometers: claiming interface");
                            int claimResult = usb_claim_interface(udev, 0);
                            if (claimResult != 0)
                            {
                                log("openAllSpectrometers: error claiming interface 0 (result %d): %s", 
                                    claimResult, usb_strerror());
                                usb_close(udev);
                                continue;
                            }

                            log("openAllSpectrometers: instantiating Spectrometer");
                            Spectrometer* spec = new Spectrometer(udev);
                            if (spec != nullptr)
                            { 
                                log("openAllSpectrometers: keeping Spectrometer");
                                spectrometers.push_back(spec);
                            }
                            else
                            {
                                log("openAllSpectrometers: Spectrometer instantiation failed");
                            }
                        }
                        else
                        {
                            usb_close(udev);
                        }
                    }
                    else
                    {
                        log("openAllSpectrometers: open failed");
                    }
                }
            }
        }
    }
    return (int)spectrometers.size();
}

WasatchVCPP::Spectrometer* WasatchVCPP::Driver::getSpectrometer(int index)
{
    if (index >= 0 && index < spectrometers.size())
        return spectrometers[index];
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// Logging
////////////////////////////////////////////////////////////////////////////////

bool WasatchVCPP::Driver::setLogfile(const string& pathname)
{
    logfile.open(pathname);
    return logfile.is_open();
}

// https://stackoverflow.com/a/30887925/11615696
void WasatchVCPP::Driver::log(const char *format, ...)
{
    char str[1024];
    va_list argptr;
    va_start(argptr, format);
    int len = vsnprintf(str, sizeof(str), format, argptr);
    va_end(argptr);
    OutputDebugStringA(str);
    OutputDebugStringA("\r\n");

    if (logfile.is_open())
    {
        logfile << str << "\r\n";
        logfile.flush();
    }
}
