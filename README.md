# Overview

Unmanaged Visual C++ driver for Wasatch Photonics spectrometers which does not 
utilize .NET or COM.

# API

* [C API](https://wasatchphotonics.com/api/Wasatch.VCPP/_wasatch_v_c_p_p_wrapper_8h.html)
* [C++ API](https://wasatchphotonics.com/api/Wasatch.VCPP/namespace_wasatch_v_c_p_p_1_1_proxy.html)

# Architecture

All namespaces are prefixed with WasatchVCPP, meaning "Wasatch Photonics Visual C++".

This is the overall architecture as I envision it:

                       _WasatchVCPP::Proxy_    _WasatchVCPP.dll/lib_______________________________________   
                      |   Driver           |  |                                                           |
    WasatchVCPPDemo <---> Spectrometer <----->| WasatchVCPPWrapper.h (exports)                            |
    (customer code)   |____________________|  |          ^                                                |
                                              |----------|------------------------------------------------|
                                              |          v                 .--> WasatchVCPP::Driver       |
                                              | WasatchVCPPWrapper.cpp <--:                               |
                                              |                            `--> WasatchVCPP::Spectrometer |
                                              |                                 '--> WasatchVCPP::EEPROM  |
                                              |___________________________________________________________|
# Contents

The WasatchVCPP distribution contains:

- WasatchVCPP.dll (compiled driver)
- WasatchVCPP.lib (needed to link with DLL)
- WasatchVCPPWrapper.h (call via this...)
- WasatchVCPPProxy.h   (...or this)
- WasatchVCPPProxy.cpp

# Usage

There are basically four ways you can use WasatchVCPP:

1. use WasatchVCPPWrapper.h (C API) to call precompiled WasatchVCPP.dll via C or C++
2. use WasatchVCPP::Proxy (C++ API) to call precompiled WasatchVCPP.dll via C++
3. compile WasatchVCPP.dll yourself, then call WasatchVCPP classes directly 
   (or use either of the above methods)
4. import WasatchVCPP classes into your own project, then call them directly 
   (or use any of the above methods)

## Discussion

The key takeaway for robust C++ DLL design is that DLLs should only export a 
"flattened" C API, and not use C++ types, classes, templates, STL etc:

- https://stackoverflow.com/a/22797419/11615696

That requirement drove the creation of WasatchVCPPWrapper.h, which provides
a fully flattened, C-compatible interface.  However, it's not much fun calling C 
libraries from C++, which led to the creation of WasatchVCPPProxy.

If you are calling the pre-compiled DLL and .lib provided in our distribution,
you can use either WasatchVCPPWrapper.h or WasatchVCPPProxy to access the DLL
functions.  The difference is that WasatchVCPPWrapper.h provides a very raw
"C" API using only legacy C datatypes.  In contrast, WasatchVCPPProxy wraps
that "C" API on the caller side, exposing a more-modern encapsulated C++ 
interface using STL containers.  Either interface should yield equivalent
results, as indeed the WasatchVCPPProxy itself calls WasatchVCPPWrapper.h
for all operations.

Alternately, since the driver itself is open-source, you may choose to simply 
compile the DLL yourself, such that it will be fully "ABI-compatible" with your
application code, so that you can instantiate and call the WasatchVCPP::Driver
and WasatchVCPP::Spectrometer classes directly.  As long as the entire source
of your project and dependent libraries are compiled with the same Visual Studio
version etc, this should work fine.

As a second alternetive, you can always skip the DLL entirely and simply import
the WasatchVCPP sources directly into your project, similarly allowing you to call
WasatchVCPP::Driver and WasatchVCPP::Spectrometer classes directly.  This should
likewise work fine, noting that it may complicate updates to newer releases of
WasatchVCPP.

# Backlog

- when logging EEPROM fields, change to "register" ala EnlightenMobile
- add get\_eeprom\_field(str, \*str)
- add get\_driver\_version(\*str)

## Merge WasatchVCPPWrapper and WasatchVCPPProxy?

It seems possible to physically merge the contents of WasatchVCPPProxy.h/cpp into 
WasatchVCPPWrapper.h, such that there's only one header file to include regardless
of whether you decide to use the C or C++ API.  All of WasatchVCPPProxy.cpp could
be made into an "inline" class declaration.  As this would be compiled by the
customer code (as a header file), there would be no ABI issues such as occur with
the .cpp compiled into the DLL.  

We probably would need to use #ifdef to prevent the WasatchVCPPProxy class from 
being compiled into the DLL when building a release, so that there aren't 
"duplicate implementations" (possibly with conflicting ABI) between the customer's
compiled application and the pre-built DLL.

# References

libusb-win32
- https://sourceforge.net/p/libusb-win32/wiki/Documentation/
- https://sourceforge.net/p/libusb-win32/wiki/Examples/
- https://sourceforge.net/p/libusb-win32/wiki/Home/

misc (developmental notes)
- https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/how-to-write-a-windows-desktop-app-that-communicates-with-a-usb-device
- https://github.com/libusb/libusb.github.com/issues/1
- https://github.com/libusb/libusb/issues/419
- https://github.com/libusb/libusb/wiki/Windows
- https://github.com/libusb/libusb/wiki/Windows#How_to_use_libusb_on_Windows
- https://social.msdn.microsoft.com/Forums/sharepoint/en-US/3b083822-120d-41ab-b73b-6428f8ac7927/lnk2019-unresolved-external-symbol-error-in-libusbwin32-test-program?forum=vclanguage
- https://stackoverflow.com/questions/22529097/how-to-add-libusb-in-microsoft-visual-studio-2013
- https://stackoverflow.com/questions/24054531/use-libusb-in-visual-studio-2013
- https://stackoverflow.com/questions/26064090/how-to-import-libusb-dll
- https://www.beyondlogic.org/usbnutshell/usb1.shtml
- https://www.microchip.com/forums/m437429.aspx#437498
- https://www.programmersought.com/article/6407765724/
- https://www.youtube.com/watch?v=pxEMIOASLkg
