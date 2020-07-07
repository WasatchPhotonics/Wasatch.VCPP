#include "pch.h"

#include "Driver.h"

#include <stdio.h>

WasatchVCPP::Driver::Driver()
{
}

bool WasatchVCPP::Driver::init()
{
    usb_init();

    auto bus_result = usb_find_busses();
    printf("usb_find_busses = %d\n", bus_result);

    auto dev_result = usb_find_devices();
    printf("usb_find_devices = %d\n", dev_result);

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

int WasatchVCPP::Driver::add()
{
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Static C-type function calls
////////////////////////////////////////////////////////////////////////////////

usb_dev_handle* WasatchVCPP::openDevice(int vid, int pid) { return NULL; }
bool WasatchVCPP::init() { return false; }
int WasatchVCPP::add() { return 0; }