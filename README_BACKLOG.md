# Backlog

# Feature Matrix

Per the "Design Remit" described in README.md:

## Supported

(done)
- readEEPROM
- getSpectrum
- setIntegrationTimeMS
- setLaserEnable
- getLibraryVersion
- setDetectorGain
- setDetectorGainOdd
- setDetectorOffset
- setDetectorOffsetOdd
- setTECEnable
- setDetectorTECSetpointDegC
- setHighGainMode
- getFirmwareVersion
- getFPGAVersion
- getDetectorTemperatureRaw
- getDetectorTemperatureDegC

(not done)
- auto-apply integration time at open
- support invertXAxis
- support bin2x2
- 2048-pixel support
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
- read laser TEC temperature (degC) (doesn't work well, regardless)
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
- add WasatchCPP (no "V") Makefile tree for POSIX / GCC / CLANG targets

## libusb-win32 deprecation

At some point we may need to move from the legacy libusb-win32 to the newer
libusbk backend.  That should be achievable without too much heartache, but
at the moment libusb-win32 is what our standard .inf files (and ENLIGHTEN)
use, and it still seems to work for all our use-cases, so this has not been
prioritized.
