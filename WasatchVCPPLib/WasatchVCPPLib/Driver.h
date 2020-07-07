#pragma once

#ifdef WASATCHVCPPLIB_EXPORTS
#define WASATCHVCPPLIB_API __declspec(dllexport)
#else
#define WASATCHVCPPLIB_API __declspec(dllimport)
#endif

#include "libusb.h"

namespace WasatchVCPP
{
    class Driver
    {
    public:
        WASATCHVCPPLIB_API Driver();

        bool init();
        int add();
        usb_dev_handle* openDevice(int vid, int pid);
    };

    WASATCHVCPPLIB_API bool init();
    WASATCHVCPPLIB_API usb_dev_handle* openDevice(int vid, int pid);
    WASATCHVCPPLIB_API int add();
}

class Driver
{
};

