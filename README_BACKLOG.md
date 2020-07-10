# Backlog

# Feature Matrix

Per the "Design Remit" described in README.md:

## Supported

(done)
- read EEPROM
- get spectrum
- set integration time
- dis/enable laser

(not done)
- auto-apply offset/gain at open
- auto-apply integration time at open
- auto-enable TEC and setpoint at open
- support gain/offset through API
- support TEC control through API
- read firmware version
- read FPGA version
- read detector TEC temperature (degC)
- 2048-pixel support
- high-gain mode (InGaAs)
- set laser power (%)
- hardware triggering (ARM)
- basic gettors: isLaserEnabled, integrationTimeMS, triggerSource etc
- Raman Intensity Calibration (ROI / vignetting?)
- expose logLevel
- installer (including .inf files, etc)

## Undecided 

- write EEPROM 
- bad pixel correction
- internal scan averaging
- read laser TEC temperature (degC)
- ramanMicro features (battery, laser watchdog, vertical ROI etc)

## Not supported (pending request / use-case)

- set laser power (mW)
- dark correction
- boxcar averaging
- transmission / reflectance or absorbance
- selectable ADC
- set TEC setpoint
- set DFU mode
- reset FPGA
- actual frame count
- area scan 
- threshold sensing
- configurable throwaways
- rigorous thread safety
- internal threading / callbacks / inversion of control

# Architectural Backlog

- add "Release" target
- change WasatchVCPP::Proxy into a header-only class
- move WasatchVCPPWrapper.h and WasatchVCPPProxy.h to distribution /include
- add WasatchCPP (no "V") Makefile tree for POSIX / GCC / CLANG targets

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

## libusb-win32 deprecation

At some point we may need to move from the legacy libusb-win32 to the newer
libusbk backend.  That should be achievable without too much heartache, but
at the moment libusb-win32 is what our standard .inf files (and ENLIGHTEN)
use, and it still seems to work for all our use-cases, so this has not been
prioritized.
