# Backlog

Tracking for features planned for implementation, and explicit listing of 
features considered _and rejected_ for implementation (at least at present).

# Feature Matrix

Per the "Design Remit" described in README.md:

## Supported

(done)
- see WasatchVCPP.h and README_CHANGELOG.md

(not done)
- WasatchVCPPNet demo (started)
- added .inf files to installer
- re-entrant thread-safety

## Not supported (pending customer use-case)

- ARM features
    - hardware triggering
- laser power
    - modulated (%)
    - calibrated (mW)
- ramanMicro 
    - battery
    - laser watchdog
    - vertical ROI
- spectral processing
    - bad pixel correction
    - scan averaging
    - boxcar averaging
    - dark correction
    - transmission / reflectance or absorbance
    - Raman Intensity Calibration (ROI / vignetting?)
    - peakfinding
- manufacturing features
    - write EEPROM 
    - set TEC setpoint
    - set DFU mode
    - reset FPGA
    - area scan
- rare features
    - actual frame count
    - threshold sensing
    - configurable throwaways
    - selectable ADC
    - read laser TEC temperature (degC) (doesn't work well, regardless)
- advanced driver features
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
