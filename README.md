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

* [C API](https://wasatchphotonics.com/api/Wasatch.VCPP/_wasatch_v_c_p_p_8h.html#func-members)
* [C++ API](https://wasatchphotonics.com/api/Wasatch.VCPP/namespace_wasatch_v_c_p_p_1_1_proxy.html)

(both declared by [WasatchVCPP.h](https://wasatchphotonics.com/api/Wasatch.VCPP/_wasatch_v_c_p_p_8h.html))

# Architecture

All namespaces are prefixed with WasatchVCPP, meaning "Wasatch Photonics Visual C++".

This is the overall architecture as I envision it (including peer libraries 
Wasatch.NET and Wasatch.PY for comparison).

                       _WasatchVCPP.cs_________
                      |                        |
     WasatchVCPPNet <--> Driver, Spectrometer  |                ___________      ____________      _____________   
     (C# example)     |________________________|               | ENLIGHTEN |<-->| Wasatch.PY |<-->|    pyusb    |
                                          ^                    |___________|    |____________|    |_____________|
                                          |                                                              ^
                       _WasatchVCPP.h_____v____      _WasatchVCPP.dll/lib______________________          |
                      |                        |    |                                          |         |
     CustomerApp.c <---> wp_foo() <---C API------+---> WasatchVCPPWrapper.cpp                  |   ______v_______      ______________
                      |                        | |  |  `--> WasatchVCPP::Driver                |  |              |    |    Wasatch   |
                      |  _WasatchVCPP::Proxy_  | |  |  `--> WasatchVCPP::Spectrometer <---------->| libusb-win32 |<-->| spectrometer |
                      | |   Driver           | | |  |       `--> WasatchVCPP::EEPROM (etc)     |  |______________|    |______________|
     WasatchVCPPDemo <----> Spectrometer <-------'  |            `--> WasatchVCPP::FeatureMask |         ^     \        /
     (C++ example)    | |____________________| |    |      (actual library implementation)     |         |      `-.inf-'
                      |________________________|    |__________________________________________|         |     (VID, PID)
                                                                                _____________      ______v_______
                                                                C#, Delphi <-->| Wasatch.NET |<-->| LibUsbDotNet |
                                                                LabVIEW etc    |_____________|    |______________|

# Contents

The WasatchVCPP distribution contains:

- include/
    - WasatchVCPP.h (provides both C and C++ APIs)

- lib/
    - x86/
        - WasatchVCPP.dll (pre-compiled binaries)
        - WasatchVCPP.lib
    - x64/
        - WasatchVCPP.dll
        - WasatchVCPP.lib

- WasatchVCPPLib/
    - WasatchVCPPLib/ 
        - sources to build WasatchVCPP.dll from Visual Studio
    - WasatchVCPPDemo/ 
        - Visual C++ GUI demo using WasatchVCPP.dll (no COM, .NET or managed memory)

- WasatchVCPPNet/
    - C# demo using WasatchVCPP.dll from managed .NET WinForms GUI

- WasatchCPPLib/ (future)
    - Makefile tree to build WasatchCPP.so or .dylib for POSIX environments using
      gcc / clang toolchain

# Usage

There are basically four ways you can use WasatchVCPP:

1. use the raw C functions (C API) to call precompiled WasatchVCPP.dll via C or C++
2. use WasatchVCPP::Proxy (C++ API) to call precompiled WasatchVCPP.dll via C++
3. compile WasatchVCPP.dll yourself, then call WasatchVCPP::Driver and
   WasatchVCPP::Spectrometer directly (having ensured perfect ABI alignment)
4. import WasatchVCPP files into your own project, then call them directly

## Discussion

The key takeaway for robust C++ DLL design is that DLLs should only export a
"flattened" C API, and not use C++ types, classes, templates, STL etc:

- https://stackoverflow.com/a/22797419/11615696

That requirement drove the creation of WasatchVCPP.h, which provides
a fully flattened, C-compatible interface.  However, it's not much fun calling C
libraries from C++, which led to the creation of WasatchVCPP::Proxy (also
defined in WasatchVCCP.h).

If you are calling the pre-compiled DLL and .lib provided in our distribution,
you can use either API defined in WasatchVCPP.h to access the DLL functions.

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
underlying C interface).

In short, this is a typical calling sequence:

    #include "WasatchCPP.h"
    
    // instantiate a Driver object
    WasatchVCPP::Proxy::Driver driver;
    
    // set where you want the logfile written (if any)
    driver.setLogfile(LOGFILE_PATH);
    
    // open all connected spectrometers
    int count = driver.openAllSpectrometers();
    
    // take a handle to the first spectrometer found on the bus
    WasatchVCPP::Proxy::Spectrometer* spectrometer = driver.getSpectrometer(0);
    
    // set integration time
    spectrometer->setIntegrationTimeMS(ms);
    
    // turn the laser on
    spectrometer->setLaserEnable(true);
    
    // take a spectrum
    vector<double> spectrum = spectrometer->getSpectrum();
    
    // turn the laser off
    spectrometer->setLaserEnable(false);
    
    // cleanup
    driver.closeAllSpectrometers();

You can see the above lines "in situ" when you grep the demo for "example":

    $ grep "example" WasatchVCPPLib/WasatchVCPPDemo/WasatchVCPPDemo.cpp

# References

libusb-win32
- https://sourceforge.net/p/libusb-win32/wiki/Home/
- https://sourceforge.net/p/libusb-win32/wiki/Examples/
- https://sourceforge.net/p/libusb-win32/wiki/Documentation/
