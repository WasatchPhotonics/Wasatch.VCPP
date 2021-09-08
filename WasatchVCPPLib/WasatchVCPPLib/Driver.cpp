/**
    @file   Driver.cpp
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  implementation of WasatchVCPP::Driver
    @note   customers normally wouldn't access this file; use WasatchVCPP.h instead
*/
#include "pch.h"

#include "Driver.h"
#include "Spectrometer.h"
#include "Util.h"

#include <stdio.h>
#include <string>
#include <iostream>

using std::mutex;
using std::vector;
using std::string;
using std::make_pair;

////////////////////////////////////////////////////////////////////////////////
// Singleton
////////////////////////////////////////////////////////////////////////////////

WasatchVCPP::Driver* WasatchVCPP::Driver::instance = nullptr;

mutex WasatchVCPP::Driver::mutDriver;
mutex WasatchVCPP::Driver::mutSpectrometers;

WasatchVCPP::Driver* WasatchVCPP::Driver::getInstance()
{
    mutDriver.lock();
    if (instance == nullptr)
        instance = new WasatchVCPP::Driver();
    mutDriver.unlock();
    return instance;
}

void WasatchVCPP::Driver::destroy()
{
    mutDriver.lock();
    if (instance != nullptr)
    {
        instance->closeAllSpectrometers();
        delete instance;
        instance = nullptr;
    }
    mutDriver.unlock();
#ifndef USE_LIBUSB_WIN32
    libusb_exit(nullptr);
#endif
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
    logger.info("Driver::openAllSpectrometers");

    mutSpectrometers.lock();
    if (!spectrometers.empty())
    {
        logger.error("Driver::openAllSpectrometers: please call closeAllSpectrometers before re-calling");
        mutSpectrometers.unlock();
        return -1;
    }

    spectrometers.clear();

#ifdef USE_LIBUSB_WIN32
    usb_init();
    usb_find_busses();
    usb_find_devices();

    for (struct usb_bus* bus = usb_get_busses(); bus; bus = bus->next)
    {
        logger.debug("traversing bus %lu (%s)", bus->location, bus->dirname);
        for (struct usb_device* dev = bus->devices; dev; dev = dev->next)
        {
            logger.debug("discovered 0x%04x:0x%04x", dev->descriptor.idVendor, dev->descriptor.idProduct);

            if (dev->descriptor.idVendor == 0x24aa)
            {
                unsigned pid = dev->descriptor.idProduct;
                if (pid == 0x1000 || pid == 0x2000 || pid == 0x4000)
                {
                    logger.debug("opening device");
                    struct usb_dev_handle* udev = usb_open(dev);
                    if (udev != nullptr)
                    {
                        if (dev->descriptor.bNumConfigurations)
                        {
                            int configResult = usb_set_configuration(udev, 1);
                            if (configResult != 0)
                            {
                                logger.error("error setting configuration 1 (result %d): %s", 
                                    configResult, usb_strerror());
                                usb_close(udev);
                                continue;
                            }

                            int claimResult = usb_claim_interface(udev, 0);
                            if (claimResult != 0)
                            {
                                logger.error("error claiming interface 0 (result %d): %s", 
                                    claimResult, usb_strerror());
                                usb_close(udev);
                                continue;
                            }

                            int index = (int)spectrometers.size();
                            auto spec = new Spectrometer(udev, pid, index, logger);
                            logger.debug("adding Spectrometer as index %d", index);

                            spectrometers.insert(make_pair(index, spec));
                        }
                        else
                        {
                            usb_close(udev);
                        }
                    }
                    else
                    {
                        logger.error("open failed");
                    }
                }
            }
        }
    }
#else
    libusb_device **devs;
    int r = libusb_init(nullptr);

    if (r < 0) 
    {
        logger.error("Failed to init USB");
        return -1;
    }

    ssize_t cnt = libusb_get_device_list(nullptr, &devs);

    if (cnt < 0){
        logger.debug("Failed to get USB device list");
        libusb_exit(nullptr);
        return int(cnt);
    }

    libusb_device *dev;
    int i = 0;

    while ((dev = devs[i++]) != nullptr)
    {
        struct libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(dev, &desc);
        if (r < 0) {
            logger.debug("Failed to get device descriptor");
            return -1;
        }

        logger.debug("discovered 0x%04x:0x%04x", desc.idVendor, desc.idProduct);

        if(desc.idVendor == 0x24aa)
        {
            unsigned pid = desc.idProduct;
            if (pid == 0x1000 || pid == 0x2000 || pid == 0x4000)
            {
                logger.debug("opening device");

                libusb_device_handle *udev;
                int openResult = libusb_open(dev, &udev);

                if(udev != nullptr)
                {
                    if (desc.bNumConfigurations)
                    {
                        int configResult = libusb_set_configuration(udev, 1);
                        if (configResult != 0)
                        {
                            logger.debug("USB config error: %d", configResult);
                            libusb_close(udev);
                            continue;
                        }

                        int claimResult = libusb_claim_interface(udev, 0);
                        if (claimResult != 0)
                        {
                            logger.debug("USB claim error: %d", claimResult);
                            libusb_close(udev);
                            continue;
                        }

                        int index = (int)spectrometers.size();
                        auto spec = new Spectrometer(udev, pid, index, logger);
                        logger.debug("adding Spectrometer as index %d", index);

                        spectrometers.insert(make_pair(index, spec));
                    }
                    else
                    {
                        libusb_close(udev);
                    }
                }
                else
                {
                    logger.error("open failed");
                }
            }
        }
    }
    libusb_free_device_list(devs, 1);
#endif

    mutSpectrometers.unlock();

    logger.info("Driver::openAllSpectrometers: done");
    return (int)spectrometers.size();
}

WasatchVCPP::Spectrometer* WasatchVCPP::Driver::getSpectrometer(int index)
{
    Spectrometer* retval = nullptr;

    mutSpectrometers.lock();
    auto iter = spectrometers.find(index);
    if (iter != spectrometers.end())
        retval = iter->second;
    else
        logger.error("Driver::getSpectrometer(%d) not found", index);
    mutSpectrometers.unlock();

    return retval;
}

bool WasatchVCPP::Driver::removeSpectrometer(int index)
{
    logger.info("Driver::removeSpectrometer(%d)", index);

    auto spec = getSpectrometer(index);
    if (spec == nullptr)
        return false;
    
    mutSpectrometers.lock();

    // release resources
    bool ok = spec->close();
    delete spec;
    spec = nullptr;

    spectrometers.erase(index);
    mutSpectrometers.unlock();
    return ok;
}

bool WasatchVCPP::Driver::closeAllSpectrometers()
{
    logger.info("Driver::closeAllSpectrometers");

    mutSpectrometers.lock();
    vector<int> indices;
    for (auto i = spectrometers.begin(); i != spectrometers.end(); i++)
        indices.push_back(i->first);
    mutSpectrometers.unlock();

    for (auto index : indices)
        removeSpectrometer(index);

    return true;
}

string WasatchVCPP::Driver::getLibraryVersion() { return libraryVersion; }

