/**
    @file   WasatchVCPPWrapper.h
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

//! This will ensure that when the LIBRARY is being built, it will EXPORT the
//! DLL symbols, and when the library is being USED (this header is being 
//! included into a customer application which is being compiled), the DLL
//! symbols will be IMPORTED.
#ifdef WASATCHVCPPLIB_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

//! all functions should return one of these codes unless indicated otherwise
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
    //! Sets a pathname for WasatchVCPP to write a debug logfile.
    //!
    //! @param pathname (Input) a valid pathname (need not exist, will be overwritten if found)
    //! @returns WP_SUCCESS or non-zero on error
    DLL_API int wp_set_logfile_path(const char* pathname);

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
    //!
    //! @returns The number of spectrometers found.  Most other functions in this
    //!          namespace take an integral "spectrometer index" parameter.  That
    //!          "specIndex" relates directly to this "spectrometer count", as 
    //!          each spectrometer is tracked in a zero-indexed vector.  To be
    //!          clear, if you call wp_open_all_spectrometers() and receive a 
    //!          value of 3, it means you can then call the other library 
    //!          functions with specIndex values from 0-2.
    DLL_API int wp_open_all_spectrometers();

    //! Closes all connected spectrometers.
    //! @returns WP_SUCCESS or non-zero on error
    DLL_API int wp_close_all_spectrometers();

    //! Closes the specified spectrometer.
    //! @param specIndex (Input) which spectrometer
    //! @returns WP_SUCCESS or non-zero on error
    DLL_API int wp_close_spectrometer(int specIndex);

    //! Returns number of spectrometers previously opened.
    //!
    //! Assumes that wp_open_all_spectrometers has already been called.  Does not
    //! open or re-open anything; no state is changed. (convenience function)
    //! 
    //! @returns number of spectrometers 
    DLL_API int wp_get_number_of_spectrometers();

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

    //! Set the spectrometer's integration time in milliseconds
    //!
    //! @param specIndex (Input) which spectrometer
    //! @returns WP_SUCCESS or non-zero on error
    DLL_API int wp_set_integration_time_ms(int specIndex, unsigned long ms);

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

    //! Turns the laser on or off.
    //!
    //! @param specIndex (Input) which spectrometer
    //! @param value (Input) whether laser should be off (zero) or on (non-zero)
    //! @returns WP_SUCCESS or non-zero on error
    DLL_API int wp_set_laser_enable(int specIndex, int value);

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
}
