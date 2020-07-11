/**
    @file   WasatchVCPP.h
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  Declaration of the flattened C API exported by WasatchVCPP.dll
    @note   Users can copy and import this file into their own Visual C++ solutions
            (optionally with WasatchVCPPProxy.h/cpp as well)

    It uses legacy C native types (no STL templates or C++ language features)
    to avoid ABI problems when calling the DLL from code which was compiled in
    a different compiler (even a different version of Visual Studio).

    Calling code can use these functions directly, or they can use the higher-level
    WasatchVCPP::Proxy classes defined in WasatchVCPPProxy.h/cpp.

    Although the majority of WasatchVCPP is written in C++ and uses a variety of
    internal classes, those classes and objects cannot be accessed through the 
    DLL ABI barrier.  This file is the only part of the DLL that customers can 
    actually "see" and "talk to."

    This file is still under development; many spectrometer functions have not 
    yet been added.

    @see README_BACKLOG.md for implementation status.
*/

#pragma once

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                   C API                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// This will ensure that when the LIBRARY is being built, it will EXPORT the
// DLL symbols, and when the library is being USED (this header is being 
// included into a customer application which is being compiled), the DLL
// symbols will be IMPORTED.
#ifdef WASATCHVCPPLIB_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

// all functions should return one of these codes unless indicated otherwise
#define WP_SUCCESS                     0
#define WP_ERROR                      -1
#define WP_ERROR_INVALID_SPECTROMETER -2
#define WP_ERROR_INSUFFICIENT_STORAGE -3
#define WP_ERROR_NO_LASER             -4

// Although we're using a C++ compiler (as the library is written in C++), we 
// want these function symbols to be compiled with C linkage (no C++ mangling). 
// This will ensure that the broadest range of customer languages, compilers and
// toolchains can link to the library.
extern "C"
{
    ////////////////////////////////////////////////////////////////////////////
    // Utility
    ////////////////////////////////////////////////////////////////////////////

    //! Sets a pathname for WasatchVCPP to write a debug logfile.
    //!
    //! @param pathname (Input) a valid pathname (need not exist, will be overwritten if found)
    //! @returns WP_SUCCESS or non-zero on error
    DLL_API int wp_set_logfile_path(const char* pathname);

    //! Obtains the version number of the WasatchVCPP library itself.
    //! @param value (Output) pre-allocated string to receive the value 
    //! @param len (Input) length of allocated buffer (16 recommended)
    //! @returns WP_SUCCESS or non-zero on error
    DLL_API int wp_get_library_version(char* value, int len);

    ////////////////////////////////////////////////////////////////////////////
    // Lifecycle
    ////////////////////////////////////////////////////////////////////////////

    //! Connects to and initializes all enumerated USB spectrometers.
    //!
    //! This is normally the first function called against the library.  It is 
    //! normally called only once per application session.  
    //!
    //! It does quite a lot of work: 
    //!
    //! - iterates over all connected USB busses
    //! - iterates over every USB device connected to those busses
    //! - finds any "valid Wasatch spectrometers" (supported VID and PID)
    //! - opens each spectrometer
    //! - "sets the configuration" and "claims the interface" 
    //! - instantiates an internal WasatchVCPP::Spectrometer object
    //! - reads and parses the EEPROM
    //! - applies any post-load configuration
    //!
    //! After calling this function, the driver is fully configured and ready
    //! to control all connected spectrometers.  
    //!
    //! @returns The number of spectrometers found.  Most other functions in this
    //!          namespace take an integral "spectrometer index" parameter.  That
    //!          "specIndex" relates directly to this "spectrometer count", as 
    //!          each spectrometer is tracked in a zero-indexed vector.  To be
    //!          clear, if you call wp_open_all_spectrometers() and receive a 
    //!          value of 3, it means you can then call the other library 
    //!          functions with specIndex values from 0-2.
    DLL_API int wp_open_all_spectrometers();

    //! Returns number of spectrometers previously opened.
    //!
    //! Assumes that wp_open_all_spectrometers has already been called.  Does not
    //! open or re-open anything; no state is changed. (convenience function)
    //! 
    //! @returns number of spectrometers 
    DLL_API int wp_get_number_of_spectrometers();

    //! Closes all connected spectrometers.
    //! @returns WP_SUCCESS or non-zero on error
    DLL_API int wp_close_all_spectrometers();

    //! Closes the specified spectrometer.
    //! @param specIndex (Input) which spectrometer
    //! @returns WP_SUCCESS or non-zero on error
    DLL_API int wp_close_spectrometer(int specIndex);

    ////////////////////////////////////////////////////////////////////////////
    // EEPROM 
    ////////////////////////////////////////////////////////////////////////////

    //! This is provided so the caller can correctly size the 'names' and 
    //! 'values' arrays for a call to wp_get_eeprom().
    //!
    //! @returns how many EEPROM fields are available (negative on error)
    DLL_API int wp_get_eeprom_field_count(int specIndex);

    //! Read a table of all EEPROM fields, as strings.
    //!
    //! This is provided as a fast-and-simple way to expose the entire EEPROM to
    //! the caller in one function call.  It would be nice if we could return the
    //! actual EEPROM object with all the fields parsed into native types, but 
    //! you can't send objects across the ABI.  It would be nice if we could send 
    //! a std::map of name-value pairs, but you can't send templates across the 
    //! ABI.  So we get...this.
    //!
    //! Note this doesn't actually copy any data...all it does is copy the 
    //! current pointer location of each EEPROM field name and stringified
    //! value from the EEPROM::stringified map.  Currently, there is no 
    //! use-case where those strings are changed after the spectrometer is
    //! instantiated, partly because we're not yet supporting "EEPROM write".
    //!
    //! Therefore these character pointers SHOULD be valid until the 
    //! spectrometers are closed and their EEPROM objects destroyed.  However,
    //! it would probably be a good idea for calling code to make copies of
    //! these values if it wants to persist them.  (This is what 
    //! WasatchVCPP::Proxy does, instantiating each field into a new 
    //! map<string, string> immediately after calling this function.)
    //!
    //! A "full" API implementation could probably include native-type accessors
    //! (and settors!) for every field in the EEPROM; I've already added such
    //! "convenience accessors" for pixels, model and serialNumber due to their
    //! relative importance when testing spectroscopy applications.  However, 
    //! that's a lot of typing and testing, and this will do for now.
    //!
    //! @param specIndex (Input) which spectrometer
    //! @param names (Output) a pre-allocated array of character pointers to hold field names
    //! @param values (Output) a pre-allocated array of character pointers to hold field values
    //! @param len (Input) number of elements in names and values arrays
    //!
    //! @returns WP_SUCCESS or non-zero on error
    DLL_API int wp_get_eeprom(int specIndex, const char** names, const char** values, int len);

    //! Read one stringified EEPROM field by name.
    //!
    //! If you don't want to call wp_get_eeprom and only want one or two fields
    //! and you already know their names, this can be easier than reading the
    //! whole table.
    //!
    //! @param specIndex (Input) which spectrometer
    //! @param name (Input) case-insensitive name of the desired EEPROM field
    //! @param value (Output) a pre-allocated character array to hold the value
    //! @param len (Input) length of pre-allocated array
    //! @returns WP_SUCCESS or non-zero on error
    DLL_API int wp_get_eeprom_field(int specIndex, const char* name, char* value, int len);

    ////////////////////////////////////////////////////////////////////////////
    // Convenience accessors
    ////////////////////////////////////////////////////////////////////////////

    //! Returns the number of pixels in the selected spectrometer.o
    //!
    //! This is critical for correctly sizing arrays sent to other functions like
    //! get_spectrum or get_wavelengths.  
    //!
    //! @note convenience function around eepromFields["activePixelsHoriz"]
    //! @param specIndex (Input) which spectrometer
    //! @returns the number of pixels (negative on error)
    DLL_API int wp_get_pixels(int specIndex);

    //! Get the selected spectrometer's model.
    //!
    //! @note convenience function around eepromFields["model"]
    //! @param specIndex (Input) which spectrometer
    //! @param value (Output) pre-allocated buffer of 'len' bytes (33 recommended)
    //! @param len (Input) allocated length of 'value'
    //! @returns WP_SUCCESS or non-zero on error
    DLL_API int wp_get_model(int specIndex, char* value, int len);

    //! Get the selected spectrometer's serial number.
    //!
    //! @note convenience function around eepromFields["serialNumber"]
    //! @param value (Output) pre-allocated buffer of 'len' bytes (33 recommended)
    //! @param len (Input) allocated length of 'value'
    //! @returns WP_SUCCESS or non-zero on error
    DLL_API int wp_get_serial_number(int specIndex, char* value, int len);

    //! Get the selected spectrometer's calibrated wavelength x-axis in nanometers
    //!
    //! @param specIndex (Input) which spectrometer
    //! @param wavelengths (Output) pre-allocated buffer of 'len' doubles 
    //! @param len (Input) allocated length of 'wavelengths' (should match 'pixels')
    //! @returns WP_SUCCESS or non-zero on error
    DLL_API int wp_get_wavelengths(int specIndex, double* wavelengths, int len);

    //! Get the selected spectrometer's calibrated x-axis in wavenumbers (1/cm)
    //!
    //! @param specIndex (Input) which spectrometer
    //! @param wavenumbers (Output) pre-allocated buffer of 'len' doubles 
    //! @param len (Input) allocated length of 'wavenumbers' (should match 'pixels')
    //! @returns WP_SUCCESS or non-zero on error (e.g., no configured excitation)
    DLL_API int wp_get_wavenumbers(int specIndex, double* wavenumbers, int len);

    ////////////////////////////////////////////////////////////////////////////
    // Acquisition
    ////////////////////////////////////////////////////////////////////////////

    //! Read one spectrum from the selected spectrometer
    //!
    //! This sends an "ACQUIRE" command, waits for "integration time"
    //! to pass, then performs a blocking read from the bulk endpoint.
    //!
    //! @param specIndex (Input) which spectrometer
    //! @param spectrum (Output) pre-allocated buffer of 'len' doubles 
    //! @param len (Input) allocated length of 'xAxis' (should match 'pixels')
    //! @returns WP_SUCCESS or non-zero on error
    DLL_API int wp_get_spectrum(int specIndex, double* spectrum, int len);

    ////////////////////////////////////////////////////////////////////////////
    // Opcodes
    ////////////////////////////////////////////////////////////////////////////

    //! Set the spectrometer's integration time in milliseconds
    //!
    //! @param specIndex (Input) which spectrometer
    //! @returns WP_SUCCESS or non-zero on error
    DLL_API int wp_set_integration_time_ms(int specIndex, unsigned long ms);

    //! Turns the laser on or off.
    //!
    //! @param specIndex (Input) which spectrometer
    //! @param value (Input) whether laser should be off (zero) or on (non-zero)
    //! @returns WP_SUCCESS or non-zero on error
    DLL_API int wp_set_laser_enable(int specIndex, int value);

    //! Set detector gain.
    //!
    //! This should not be done casually by the user; detector gain is normally
    //! optimized for a given detector series, and rarely varies significantly
    //! from component to component.  In the event that detector gain calibration
    //! is required, it is done at the factory, and the calibrated value is
    //! stored in the EEPROM ("detectorGain").
    //!
    //! There is a lot which can be said about this attribute, which may be
    //! fleshed-out here at a later date.  The internal data format is somewhat
    //! odd, essentially a signed bfloat16 (see WasatchNET documentation on
    //! FunkyFloat).  For historical reasons, many spectrometers are tuned to
    //! a default gain of 1.9 (0x01e6).  It is a valid question why the 
    //! calibrated gain is stored on the spectrometer (in the EEPROM) yet needs
    //! to be applied from software over USB.  
    //!
    //! @warning don't call this function unless you really know what you're 
    //!          doing; experimentation is unlikely to improve measurement quality
    //!
    //! @param specIndex (Input) which spectrometer
    //! @param value (Input) desired gain (e.g. 1.9)
    //! @returns WP_SUCCESS or non-zero on error
    DLL_API int wp_set_detector_gain(int specIndex, float value);

    //! On InGaAs spectrometers, configures the gain for odd-numbered pixels.
    //!
    //! @par Even vs Odd Pixels
    //!
    //! Hamamatsu InGaAs detectors use two interleaved arrays of photodiodes,
    //! where one array controls even-numbered pixels and the other array 
    //! controls the odd-numbered.  This means that with regard to gain and noise
    //! characteristics, all even pixels are electrically related, and all odd
    //! pixels are electrically related, but the two sets may behave differently
    //! from one another.  Therefore, NIR spectrometers support independent gain
    //! and offset calibration for the two pixels sets.
    //!
    //! @see wp_set_detector_gain for information about detector gain itself
    //! @param specIndex (Input) which spectrometer
    //! @param value (Input) desired gain (positive or negative)
    //! @returns WP_SUCCESS or non-zero on error (such as use with silicon detectors)
    DLL_API int wp_set_detector_gain_odd(int specIndex, float value);
    
    //! Set detector offset.
    //! 
    //! @see set_detector_offset for discussion on detector offset
    //! @see set_detector_gain_odd for discussion on even-vs-odd pixels
    //! @param specIndex (Input) which spectrometer
    //! @param value (Input) desired offset (positive or negative)
    //! @returns WP_SUCCESS or non-zero on error (e.g. silicon detector)
    DLL_API int wp_set_detector_offset(int specIndex, int16_t value);

    //! On InGaAs spectrometers, configures the offset for odd-numbered pixels.
    //! 
    //! @see set_detector_offset for discussion on detector offset
    //! @see set_detector_gain_odd for discussion on even-vs-odd pixels
    //! @param specIndex (Input) which spectrometer
    //! @param value (Input) desired offset (positive or negative)
    //! @returns WP_SUCCESS or non-zero on error (e.g. silicon detector)
    DLL_API int wp_set_detector_offset_odd(int specIndex, int16_t value);

    //! Turn the detector TEC on or off.
    //!
    //! @note  if TEC setpoint has not yet been set, will automatically set to 
    //!        EEPROM configured minimum
    //! @param specIndex (Input) which spectrometer
    //! @param value (Input) zero for off, non-zero for on
    //! @returns WP_SUCCESS or non-zero on error (e.g. uncooled spectrometer)
    DLL_API int wp_set_tec_enable(int specIndex, int value);

    //! Set the detector TEC setpoint.
    //!
    //! @param specIndex (Input) which spectrometer
    //! @param value (Input) desired temperature in degrees Celsius
    //! @returns WP_SUCCESS or non-zero on error (e.g. uncooled spectrometer)
    DLL_API int wp_set_detector_tec_setpoint_deg_c(int specIndex, int value);

    //! Enable or disable "high gain" mode on InGaAs detectors.
    //!
    //! @note this is enabled by default on suitable detectors
    //! @param specIndex (Input) which spectrometer
    //! @param value (Input) zero to disable, non-zero to enable
    //! @returns WP_SUCCESS or non-zero on error (e.g. silicon detector)
    DLL_API int wp_set_high_gain_mode(int specIndex, int value);

    //! Get the firmware version of the microcontroller (FX2 or ARM).
    //!
    //! @param specIndex (Input) which spectrometer
    //! @param value (Output) pre-allocated character array 
    //! @param len (Input) allocated size (should be 16+)
    //! @returns WP_SUCCESS or non-zero on error
    DLL_API int wp_get_firmware_version(int specIndex, char* value, int len);

    //! Get the version of the FPGA.
    //! 
    //! @param specIndex (Input) which spectrometer
    //! @param value (Output) pre-allocated character array 
    //! @param len (Input) allocated size (should be 16+)
    //! @returns WP_SUCCESS or non-zero on error
    DLL_API int wp_get_fpga_version(int specIndex, char* value, int len);

    //! Get the detector temperature.
    //!
    //! @param specIndex (Input) which spectrometer
    //! @returns -999 on error, else detector temperature in degrees Celsius
    DLL_API float wp_get_detector_temperature_deg_c(int specIndex);
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                  C++ API                                   //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// We only want the following classes declared (and defined, since they're 
// inline) if this file is being #included from "calling customer code" on the
// client side.  Therefore, we wrap it in the following #ifndef, which prevents
// the block being processed if we're actually compiling the WasatchVCPP DLL
// itself.
//
// It is important to prevent these from being compiled into the DLL, because
// otherwise the customer could end up with two "copies" of the same classes
// (one compiled into the DLL, the other instanted into their own application
// when they include this header).  That could create linkage collisions down
// the road.  It MIGHT not be an issue, as long as we don't "dllexport" these
// declarations (and we're not), but I'd rather not take the chance.
//
// Another reason why we don't want these classes compiled into the DLL is
// because, for ABI boundary reasons, they'd be unusable to the customer
// that way.  To be useable by the customer application, C++ types and STL 
// containers must be instantiated and compiled directly into the client
// executable.
// 
// See README_ARCHITECTURE.md for more.

#ifndef WASATCHVCPPLIB_EXPORTS

#include <vector>
#include <string>
#include <map>

namespace WasatchVCPP
{
    /**
        @brief provides object-oriented Driver and Spectrometer C++ classes on 
               the client (caller) side

        The C API is somewhat clunky and cumbersome to use, and that's not the 
        level of elegance and user-friendliness that I want to expose to our 
        customers (or use myself for that matter).

        Therefore, this WasatchVCPP::Proxy namespace provides a handy object-
        oriented facade to the flatted C API the DLL exports in order to avoid 
        ABI entanglements.

        @see README_ARCHITECTURE.md
    */
    namespace Proxy
    {
        ////////////////////////////////////////////////////////////////////////
        //
        //                        Spectrometer Proxy
        //
        ////////////////////////////////////////////////////////////////////////

        //! A proxy customer-facing class providing an object-oriented / STL-based
        //! interface to command and control an individual Spectrometer.
        class Spectrometer
        {
            ////////////////////////////////////////////////////////////////////
            // Public methods
            ////////////////////////////////////////////////////////////////////
            public:

                //! instantiated by WasatchVCPP::Proxy::openAllSpectrometers
                Spectrometer(int specIndex)
                {
                    this->specIndex = specIndex;

                    readEEPROMFields();

                    pixels = wp_get_pixels(specIndex); // or eepromFields["activePixelsHoriz"]
                    if (pixels <= 0)
                        return;

                    // pre-allocate a buffer for reading spectra
                    spectrumBuf = (double*)malloc(pixels * sizeof(double));

                    model = eepromFields["model"];
                    serialNumber = eepromFields["serialNumber"];

                    wavelengths.resize(pixels);
                    wp_get_wavelengths(specIndex, &wavelengths[0], pixels);

                    excitationNM = (float)atof(eepromFields["excitationNM"].c_str());
                    if (excitationNM > 0)
                    {
                        wavenumbers.resize(pixels);
                        wp_get_wavenumbers(specIndex, &wavenumbers[0], pixels);
                    }
                }

                //! release resources associated with this spectrometer
                //! @returns true on success
                bool close()
                {
                    if (specIndex >= 0)
                    {
                        wp_close_spectrometer(specIndex);
                        specIndex = -1;
                    }

                    if (spectrumBuf != nullptr)
                    {
                        free(spectrumBuf);
                        spectrumBuf = nullptr;
                    }

                    return true;
                }

            ////////////////////////////////////////////////////////////////////
            // Public attributes
            ////////////////////////////////////////////////////////////////////
            public:
                int specIndex;              //!< index of this spectrometer
                int pixels;                 //!< number of pixels
                std::string model;          //!< model name
                std::string serialNumber;   //!< serial number

                //! a dictionary of EEPROM name-value pairs rendered as strings
                std::map<std::string, std::string> eepromFields;

                std::vector<double> wavelengths;    //!< expanded wavecal in nm
                std::vector<double> wavenumbers;    //!< expanded wavecal in 1/cm (Raman-only)
                float excitationNM;                 //!< configured laser excitation wavelength (Raman-only)

            ////////////////////////////////////////////////////////////////////
            // Public methods
            ////////////////////////////////////////////////////////////////////
            public:
                //! set integration time
                //! @param ms (Input) time in milliseconds
                //! @return true on success
                bool setIntegrationTimeMS(unsigned long ms)
                { return wp_set_integration_time_ms(specIndex, ms); }

                //! set laser firing state
                //! @param flag (Input) desired state (true for firing, false for off)
                //! @return true on successful communication (does not represent firing state)
                bool setLaserEnable(bool flag)
                { return wp_set_laser_enable(specIndex, flag); }

                //! Retrieve one spectrum from the spectrometer.
                //!
                //! Sends an ACQUIRE command, then enters BLOCKING read on bulk endpoint.
                //! Demarshalls retreived little-endian pixel values.  Applies minimal
                //! post-processing (see WasatchVCPP::Spectrometer::getSpectrum for details).
                //!
                //! @returns spectrum as vector of doubles
                std::vector<double> getSpectrum()
                {
                    std::vector<double> result;
                    if (spectrumBuf != nullptr)
                    {
                        if (WP_SUCCESS == wp_get_spectrum(specIndex, spectrumBuf, pixels))
                        {
                            // This seems to work, but doing manually for now:
                            // result = vector<double>(spectrumBuf, spectrumBuf + pixels); 
                            for (int i = 0; i < pixels; i++)
                                result.push_back(spectrumBuf[i]);
                        }
                    }
                    return result;
                }

                //! @see wp_set_detector_gain
                bool setDetectorGain(float value)
                { return WP_SUCCESS == wp_set_detector_gain(specIndex, value); }

                //! @see wp_set_detector_gain_odd
                bool setDetectorGainOdd(float value)
                { return WP_SUCCESS == wp_set_detector_gain_odd(specIndex, value); }

                //! @see wp_set_detector_offset
                bool setDetectorOffset(int16_t value)
                { return WP_SUCCESS == wp_set_detector_offset(specIndex, value); }

                //! @see wp_set_detector_offset_odd
                bool setDetectorOffsetOdd(int16_t value)
                { return WP_SUCCESS == wp_set_detector_offset_odd(specIndex, value); }

                //! @see wp_set_tec_enable
                bool setTECEnable(bool flag)
                { return WP_SUCCESS == wp_set_tec_enable(specIndex, flag ? 1 : 0); }

                //! @see wp_set_detector_tec_setpoint_deg_c
                bool setDetectorTECSetpointDegC(int value)
                { return WP_SUCCESS == wp_set_detector_tec_setpoint_deg_C(specIndex, value); }

                //! @see wp_set_high_gain_mode
                bool setHighGainMode(bool flag)
                { return WP_SUCCESS == wp_set_high_gain_mode(specIndex, flag ? 1 : 0); }

                //! @see wp_get_firmware_version
                std::string getFirmwareVersion()
                {
                    char buf[16];
                    wp_get_firmware_version(specIndex, buf, sizeof(buf));
                    return string(buf);
                }

                //! @see wp_get_fpga_version
                std::string getFPGAVersion()
                {
                    char buf[16];
                    wp_get_fpga_version(specIndex, buf, sizeof(buf));
                    return string(buf);
                }

                //! @see wp_get_detector_temperature_deg_c
                float getDetectorTemperatureDegC()
                { return wp_get_detector_temperature_deg_c(specIndex); }

            private:
                bool readEEPROMFields()
                {
                    int count = wp_get_eeprom_field_count(specIndex);
                    if (count <= 0)
                        return false;

                    const char** names  = (const char**)malloc(count * sizeof(const char*));
                    const char** values = (const char**)malloc(count * sizeof(const char*));

                    if (WP_SUCCESS != wp_get_eeprom(specIndex, names, values, count))
                    {
                        free(names);
                        free(values);
                        return false;
                    }

                    for (int i = 0; i < count; i++)
                        eepromFields.insert(std::make_pair(std::string(names[i]), std::string(values[i])));

                    free(names);
                    free(values);
                    return true;
                }
                double* spectrumBuf;
        };

        ////////////////////////////////////////////////////////////////////////
        // 
        //                               Proxy Driver
        //
        ////////////////////////////////////////////////////////////////////////

        //! A proxy customer-facing class providing an object-oriented / STL-based
        //! interface to command and control the WasatchVCPP library.
        class Driver
        {
            ////////////////////////////////////////////////////////////////////
            // Public methods
            ////////////////////////////////////////////////////////////////////
            public:
                //! Instantiate a Proxy::Driver
                Driver() {}

                //! Enable WasatchVCPP internal debug logging, and direct to a 
                //! textfile.
                //!
                //! @returns true on success
                bool setLogfile(const std::string& pathname)
                { 
                    return WP_SUCCESS == wp_set_logfile_path(pathname.c_str()); 
                }

                //! @returns version number of the WasatchVCPP library itself
                std::string getLibraryVersion()
                {
                    char buf[16];
                    wp_get_library_version(specIndex, buf, sizeof(buf));
                    return string(buf);
                }

                //! Open and initialize all connected Wasatch Photonics spectrometers.
                //!
                //! Must be called before getSpectrometer().
                //!
                //! @returns number of spectrometers found (also sets numberOfSpectrometers)
                int openAllSpectrometers()
                {
                    spectrometers.clear();

                    numberOfSpectrometers = wp_open_all_spectrometers();
                    if (numberOfSpectrometers <= 0)
                        return 0;

                    for (int i = 0; i < numberOfSpectrometers; i++)
                        spectrometers.push_back(Spectrometer(i));

                    return numberOfSpectrometers;
                }

                //! Retrieve a handle to one Spectrometer.
                //! 
                //! @peram specIndex (Input) which spectrometer (less than numberOfSpectrometers)
                //! @warning the caller should not 'delete' or 'free' this pointer;
                //!          it will be released automatically by closeAllSpectrometers
                //! @returns handle to Proxy::Spectrometer
                Spectrometer* getSpectrometer(int index)
                {
                    if (index >= spectrometers.size())
                        return nullptr;

                    return &spectrometers[index];
                }

                //! Close all spectrometers; call at application shutdown.
                //! @returns true on success
                bool closeAllSpectrometers()
                {
                    for (int i = 0; i < numberOfSpectrometers; i++)
                        spectrometers[i].close();
                    spectrometers.clear();
                    return true;
                }

            ////////////////////////////////////////////////////////////////////
            // Public attributes
            ////////////////////////////////////////////////////////////////////
            public:                    

                //! number of spectrometers found (set by openAllSpectrometers)
                int numberOfSpectrometers;

            ////////////////////////////////////////////////////////////////////
            // Private attributes
            ////////////////////////////////////////////////////////////////////
            private:
                std::vector<Spectrometer> spectrometers;
        };
    }
}
#endif