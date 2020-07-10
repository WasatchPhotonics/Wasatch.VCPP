# Overview

Unmanaged Visual C++ driver for Wasatch Photonics spectrometers which does not 
utilize .NET or COM.

# Design Remit

All drivers must make a decision as to what is within their remit and where to 
"draw the line" on functionality.  If you implement too many features under the hood,
you risk "bloatware"; if you don't provide enough automation, you scare off 
would-be spectroscopic innovators who aren't saavy with bitwise manipulation.

This driver exists within a software catalog which already includes a 
fully-featured driver suite (Wasatch.NET) supporting high-level languages like
C#, Visual Basic, MATLAB and LabVIEW, as well as an eclectic but functional 
Python package (Wasatch.PY) capable of supporting GUI applications like ENLIGHTEN.

Therefore, as open-source reference implementations have already been provided
for every feature available through our USB API (see Wasatch Photonics ENG-0001),
it is the intent to make the C/C++ driver as small and lightweight as possible.
Although every _spectrometer_ feature will be available, not every _spectroscopic
technique_ will be automated.

This boundary remains malleable and may be adjusted per customer request and
arising use-cases.

It is also noteworthy that the driver is maintained as an open-source project
on GitHub, so if users wish to add new features themselves, they are encouraged to 
fork and submit pull requests :-)

* see [Backlog](README_BACKLOG.md) for a list of the features intended, rejected,
and undecided for inclusion.

# Dependencies

- Visual Studio (tested with 2019 Community Edition)

This library is built atop the same libusb-win32 back-end used by ENLIGHTEN, 
Wasatch.NET etc.  Therefore, if you have one of those installed, it should "just 
work".  Until an installer is added to this project, it is recommended that users
install ENLIGHTEN first as the quickest way to install and configure the .inf
files to associate our USB VID and PID with the libusb-win32 low-level driver.

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

- WasatchVCPPWrapper.h (call via this...)
- WasatchVCPPProxy.h   (...or this)
- WasatchVCPPProxy.cpp

When the WasatchVCPPLib solution is compiled in Visual Studio, it generates:

- WasatchVCPP.dll (compiled driver)
- WasatchVCPP.lib (needed to link with DLL)

At a later point, the three .h/.cpp files mentioned above may be moved to 
WasatchVCPP/include (out of the WasatchVCPPLib folder), so it is clear that they
are "user-copyable" files.

And I should probably eliminate WasatchVCPPProxy.cpp altogether, and make
WasatchVCPP::Proxy a "header-only" class.

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

# Examples

A simple Visual C++ GUI project, "WastchVCPPDemo" is included in the solution
so you can see how these functions can be called through the WasatchVCPP::Proxy
C++ interface (whose source code itself provides an example to using the 
WasatchVCPPWrapper.h C interface).

In short, this is a typical calling sequence:

    #include "WasatchCPPProxy.h"
    
    // set where you want the logfile written (if any)
    WasatchVCPP::Proxy::Driver::setLogfile(LOGFILE_PATH); 

    // open all connected spectrometers
    int count = WasatchVCPP::Proxy::Driver::openAllSpectrometers(); 

    // take a handle to the first spectrometer found on the bus
    WasatchVCPP::Proxy::Spectrometer* spectrometer = WasatchVCPP::Proxy::Driver::getSpectrometer(0); 
    
    // set integration time
    spectrometer->setIntegrationTimeMS(ms); 

    // turn the laser on
    spectrometer->setLaserEnable(true); 

    // take a spectrum
    vector<double> spectrum = spectrometer->getSpectrum(); 

    // turn the laser off
    spectrometer->setLaserEnable(false); 
    
    // cleanup
    WasatchVCPP::Proxy::Driver::closeAllSpectrometers(); 

You can see the above lines "in situ" when you grep the demo for "example":

    $ grep "example" WasatchVCPPLib/WasatchVCPPDemo/WasatchVCPPDemo.cpp

# References

libusb-win32
- https://sourceforge.net/p/libusb-win32/wiki/Home/
- https://sourceforge.net/p/libusb-win32/wiki/Examples/
- https://sourceforge.net/p/libusb-win32/wiki/Documentation/
