![Visual Studio](https://github.com/WasatchPhotonics/Wasatch.VCPP/raw/master/screenshots/VisualStudio.png)

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

## Windows

- Visual Studio (tested with 2019 Community Edition)

This library is built atop the same libusb-win32 back-end used by ENLIGHTEN,
Wasatch.NET etc.  Therefore, if you have one of those installed, it should "just
work".  Until an installer is added to this project, it is recommended that users
install ENLIGHTEN first as the quickest way to install and configure the .inf
files to associate our USB VID and PID with the libusb-win32 low-level driver.

## Linux / POSIX

- see [Linux](README_LINUX.md)

# API

* [C API](https://wasatchphotonics.com/api/Wasatch.VCPP/_wasatch_v_c_p_p_8h.html#func-members)
* [C++ API](https://wasatchphotonics.com/api/Wasatch.VCPP/namespace_wasatch_v_c_p_p_1_1_proxy.html)

(both declared by [WasatchVCPP.h](https://wasatchphotonics.com/api/Wasatch.VCPP/_wasatch_v_c_p_p_8h.html))

# Architecture

All namespaces are prefixed with WasatchVCPP, meaning "Wasatch Photonics Visual C++".

This is the overall architecture as I envision it (including peer libraries
Wasatch.NET and Wasatch.PY for comparison).

                       _WasatchVCPP.cs__________
                      |                         |
     WasatchVCPPNet <--> Driver, Spectrometer   |              ___________      ____________      _____________
     (C# example)     |_________________________|             | ENLIGHTEN |<-->| Wasatch.PY |<-->|    pyusb    |
                                           ^                  |___________|    |____________|    |_____________|
                                           |                                                            ^
                       _WasatchVCPP.h______|____    _WasatchVCPP.dll/lib______________________          |
                      |   C API            v    |  |                                          |         |
     CustomerApp.c <---> wp_set_laser_enable() <----> WasatchVCPPWrapper.cpp (C API impl)     |   ______v_______      ______________
                      |                    ^    |  |  `--> WasatchVCPP::Driver                |  | libusb-win32 |    | spectrometer |
                      |  _C++ API__________|_   |  |  `--> WasatchVCPP::Spectrometer <---------->|  (libusb.h)  |<-->|  (ENG-0001)  |
                      | |   Proxy::Driver, | |  |  |       `--> WasatchVCPP::EEPROM (etc)     |  |______________|    |______________|
     WasatchVCPPDemo <----> Spectrometer <-' |  |  |            `--> WasatchVCPP::FeatureMask |         ^     \        /
     (C++ example)    | |____________________|  |  |      (actual library implementation)     |         |      `-.inf-'
                      |_________________________|  |__________________________________________|         |     (VID, PID)
                         (customer ABI)              (DLL ABI)                 _____________      ______v_______
                                                               C#, Delphi <-->| Wasatch.NET |<-->| LibUsbDotNet |
                                                               LabVIEW etc    |_____________|    |______________|

## Future Bindings

Some may wonder, if WasatchVCPPNet can call into WasatchVCPP, then could not 
Wasatch.NET do the same, making Wasatch.NET much smaller, and consolidating all 
business logic, opcodes, marshalling, endian fiddling etc into a single library?

And since Python can obviously call into C libraries, could not Wasatch.PY not
do the same, such that we only have one actual (C/C++) "application driver," 
wrapped by multiple lightweight language bindings?

Yes, we could do that.  And it's not out of bounds that we may someday do that.
But that's not currently in scope or required, and it's not what we're doing 
right now.

(Also, it can be useful to have "native" driver logic in C#, for instance to
access USB spectrometers from Xamarin on Android...a C library would actually
be less convenient for mobile platforms.  So there is that.)

# Contents

The WasatchVCPP distribution is structured as follows.  (Files ending in \*
are populated at build.)

- include/
    - WasatchVCPP.h (provides both C and C++ APIs)

- lib/
    - x86/
        - WasatchVCPP.dll*
        - WasatchVCPP.lib*
        - libusb0.dll*
    - x64/
        - WasatchVCPP.dll*
        - WasatchVCPP.lib*
        - libusb0.dll*

- bin/
    - WasatchVCPPDemo.exe\*
    - libusb0.dll\*

- dist/  (pre-compiled versions of libusb from the libusb-win32 distribution)
    - x86/
    - x64/

- libusb\_drivers/
    - The .inf and .cat files used to associate Wasatch Photonics USB 
      spectrometers with the libusb-win32 low-level driver.  This folder
      is an exact copy of that provided through Wasatch.NET:
    - https://github.com/WasatchPhotonics/Wasatch.NET/tree/master/libusb_drivers
    - To "install" these drivers with your spectrometer, follow the same
      procedure defined for Wasatch.NET:
    - https://github.com/WasatchPhotonics/Wasatch.NET#post-install-step-1-libusb-drivers

- WasatchVCPPLib/
    - WasatchVCPPLib/ 
        - sources to build WasatchVCPP.dll from Visual Studio
    - WasatchVCPPDemo/ 
        - Visual C++ GUI demo using WasatchVCPP.dll (no COM, .NET or managed memory)
    - Setup32/ 
    - Setup64/ 

- WasatchVCPPNet/
    - C# demo using WasatchVCPP.dll from managed .NET WinForms GUI

- WasatchCPPLib/ (future)
    - Makefile tree to build WasatchCPP.so or WasatchCPP.dylib for POSIX 
      environments using gcc / clang toolchain

# Usage

There are basically five ways you can use WasatchVCPP:

1. use the raw C functions (C API) to call precompiled WasatchVCPP.dll via C or C++
2. use WasatchVCPP::Proxy (C++ API) to call precompiled WasatchVCPP.dll via C++
3. compile WasatchVCPP.dll yourself, then call WasatchVCPP::Driver and
   WasatchVCPP::Spectrometer directly (having ensured perfect ABI alignment)
4. import WasatchVCPP files into your own project, then call them directly
5. make a copy of WasatchVCPPNet/WasatchVCPP.cs and call it from your C# program

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

## WasatchVCPPDemo

A simple Visual C++ GUI project, "WasatchVCPPDemo" is included in the solution
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

## WasatchVCPPNet

A second GUI demo is provided in the WasatchVCPPNet solution.  This is packaged
as a separate solution (as opposed to merely a project in the WasatchVCPPLib 
solution) to clearly demonstrate that only the WasatchVCPP.dll is being called,
and there are no "intra-package" linkage references or other shortcuts.

This GUI was written in C# for the simple reason that spectral graphing is 
trivial in .NET, and it was easier to call the WasatchVCPP C API from C# than to
import a graphing framework into Visual C++ :-(

# Changelog

- see [Changelog](README_CHANGELOG.md)

# References

libusb-win32
- https://sourceforge.net/p/libusb-win32/wiki/Home/
- https://sourceforge.net/p/libusb-win32/wiki/Examples/
- https://sourceforge.net/p/libusb-win32/wiki/Documentation/

![Ubuntu](https://github.com/WasatchPhotonics/Wasatch.VCPP/raw/master/screenshots/ubuntu.png)
