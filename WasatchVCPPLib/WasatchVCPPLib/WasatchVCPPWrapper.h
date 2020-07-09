/**
    @file   WasatchVCPPWrapper.h
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @note   Users can copy and import this file into their own Visual C++ solutions
            (optionally with WasatchVCPPProxy.h/cpp as well)

    This file defines the flattened C API that WasatchVCPP.dll exports.

    It uses legacy C native types (no STL templates or C++ language features)
    to avoid ABI problems when calling the DLL from code which was compiled in
    a different compiler (even a different version of Visual Studio).

    Calling code can use these functions directly, or they can use the higher-level
    WasatchVCPP::Proxy classes defined in WasatchVCPPProxy.h/cpp.
*/

#pragma once

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

extern "C"
{
    //! connects to all enumerated USB spectrometers 
    //! @returns number of spectrometers found
    DLL_API int wp_open_all_spectrometers();

    //! closes the specified spectrometer
    DLL_API int wp_close_spectrometer(int specIndex);

    //! @returns number of spectrometers previously opened
    DLL_API int wp_get_number_of_spectrometers();

    //! @returns the number of pixels in the selected spectrometer (negative on error)
    DLL_API int wp_get_pixels(int specIndex);

    //! Get the selected spectrometer's model.
    //!
    //! @param specIndex (Input) which spectrometer
    //! @param value (Output) pre-allocated buffer of 'len' bytes (33 recommended)
    //! @param len (Input) allocated length of 'value'
    //! @returns 0 on success, non-zero on error
    DLL_API int wp_get_model(int specIndex, char* value, int len);

    //! Get the selected spectrometer's serial number.
    //! @returns 0 on success, non-zero on error
    DLL_API int wp_get_serial_number(int specIndex, char* value, int len);

    //! Get the selected spectrometer's calibrated wavelength x-axis in nanometers
    //!
    //! @param specIndex (Input) which spectrometer
    //! @param wavelengths (Output) pre-allocated buffer of 'len' doubles 
    //! @param len (Input) allocated length of 'wavelengths' (should match 'pixels')
    //! @returns 0 on success, non-zero on error
    DLL_API int wp_get_wavelengths(int specIndex, double* wavelengths, int len);

    //! Get the selected spectrometer's calibrated x-axis in wavenumbers (1/cm)
    //!
    //! @param specIndex (Input) which spectrometer
    //! @param wavenumbers (Output) pre-allocated buffer of 'len' doubles 
    //! @param len (Input) allocated length of 'wavenumbers' (should match 'pixels')
    //! @returns 0 on success, non-zero on error (i.e., no excitation wavelength configured)
    DLL_API int wp_get_wavenumbers(int specIndex, double* wavenumbers, int len);

    //! Set the spectrometer's integration time in milliseconds
    //! @returns 0 on success, non-zero on error
    DLL_API int wp_set_integration_time_ms(int specIndex, unsigned long ms);

    //! Read one spectrum from the selected spectrometer
    //!
    //! This sends an "ACQUIRE" command, waits for "integration time"
    //! to pass, then performs a blocking read from the bulk endpoint.
    //!
    //! @param specIndex (Input) which spectrometer
    //! @param spectrum (Output) pre-allocated buffer of 'len' doubles 
    //! @param len (Input) allocated length of 'xAxis' (should match 'pixels')
    //! @returns 0 on success, non-zero on error
    DLL_API int wp_get_spectrum(int specIndex, double* spectrum, int len);

    //! Turns the laser on or off.
    //!
    //! @param specIndex (Input) which spectrometer
    //! @param value (Input) whether laser should be off (zero) or on (non-zero)
    //! @returns 0 on success, non-zero on error
    DLL_API int wp_set_laser_enable(int specIndex, int value);

    //! Sets a pathname for WasatchVCPP to write a debug logfile.
    //!
    //! @param pathname (Input) a valid pathname (need not exist, will be overwritten if found)
    //! @returns 0 on success, non-zero on error
    DLL_API int wp_set_logfile_path(const char* pathname);
}