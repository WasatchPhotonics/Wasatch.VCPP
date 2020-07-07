#include "pch.h"

#include "Driver.h"

#include <stdio.h>

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

bool WasatchVCPP::Driver::connect()
{
    this->log("connect: calling usb_init");
    usb_init();

    auto bus_result = usb_find_busses();
    this->log("usb_find_busses = %d", bus_result);

    auto dev_result = usb_find_devices();
    this->log("usb_find_devices = %d", dev_result);

    return true;
}

usb_dev_handle* WasatchVCPP::Driver::openDevice(int vid, int pid)
{
    struct usb_bus* bus;
    struct usb_device* dev;
    struct usb_dev_handle* udev;

    for (bus = usb_get_busses(); bus; bus = bus->next)
    {
        for (dev = bus->devices; dev; dev = dev->next)
        {
            if (dev->descriptor.idVendor == vid && dev->descriptor.idProduct == pid)
            {
                if ((udev = usb_open(dev)))
                {
                    if (dev->descriptor.bNumConfigurations)
                    {
                        // if (usb_find_interface(&dev->config[0], interfaceNumber, altInterfaceNumber, NULL) != NULL)
                        return udev;
                    }

                    usb_close(udev);
                }
            }
        }
    }
    return NULL;
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
    OutputDebugStringA("\n");
}

////////////////////////////////////////////////////////////////////////////////
// Static C-type function calls
////////////////////////////////////////////////////////////////////////////////

bool WasatchVCPP::connect() 
{ 
    WasatchVCPP::Driver* driver = WasatchVCPP::Driver::getInstance();
    return driver->connect();
}

usb_dev_handle* WasatchVCPP::openDevice(int vid, int pid) { return NULL; }