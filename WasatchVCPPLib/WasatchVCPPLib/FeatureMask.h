/**
    @file   FeatureMask.h
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  interface of WasatchVCPP::FeatureMask
    @note   customers normally wouldn't access this file; use WasatchVCPP.h instead
*/

#pragma once

#include <cstdint>

namespace WasatchVCPP
{
    //! Internal class encapsulating a 16-bit set of boolean flags which indicate
    //! whether a given spectrometer has a particular feature or not.
    //! 
    //! Requires less storage than legacy hasCooling, hasLaser or hasBattery fields.
    class FeatureMask
    {
        public:
            enum Flags
            {
                FLAG_INVERT_X_AXIS = 0x0001, // 2^0 
                FLAG_BIN_2X2       = 0x0002, // 2^1
                FLAG_GEN15         = 0x0004, // 2^2
                FLAG_CUTOFF_FILTER = 0x0008, // 2^3
                FLAG_EVEN_ODD      = 0x0010  // 2^4
            };

            FeatureMask(uint16_t value = 0);

            uint16_t toUInt16();

            //! The orientations of the grating and detector in this spectrometer are 
            //! rotated such that spectra are read-out "red-to-blue" rather than the
            //! normal "blue-to-red".  Therefore, automatically reverse the spectrum
            //! array.  This EEPROM field ensures that, regardless of hardware 
            //! orientation or firmware, spectra is always reported to the user FROM
            //! WASATCH.NET in a consistent "blue-to-red" order (increasing 
            //! wavelengths).  This is the order intended and assumed by the factory-
            //! configured wavelength calibration.  
            //! 
            //! The user should not have to change behavior or process spectra 
            //! differently due to this value; its purpose is to communicate state 
            //! between the spectrometer and driver and ensure correct internal 
            //! processing within the driver.
            bool invertXAxis = false;

            //! Some 2D detectors use a Bayer filter in which pixel columns alternate
            //! between red and blue sensitivity (green is uniform throughout).  By 
            //! binning square blocks of 2x2 pixels (for any given detector position,
            //! this would include 1 blue, 1 red and 2 green), an even sensitivity is
            //! achieved across spectral range.
            //! 
            //! As "vertical binning" is normally performed within firmware, the only
            //! portion of this 2x2 binning which is performed within the software
            //! driver is the horizontal binning.  This is currently performed within
            //! Spectrometer.getSpectrumRaw.
            bool bin2x2 = false;

            //! Spectrometer has the new "Gen 1.5" OEM Accessory Connector 
            //! providing triggering, external lamp control, continuous strobe, 
            //! fan control, shutter control, load-switch reset, external power
            //! etc.
            bool gen15 = false;

            //! Spectrometer has a cutoff filter installed.
            bool cutoffFilterInstalled = false;

            //! InGaAs spectrometer has even/odd pixel gain/offset correction 
            //! implemented in the FPGA.
            bool hardwareEvenOdd = false;
    };
}
