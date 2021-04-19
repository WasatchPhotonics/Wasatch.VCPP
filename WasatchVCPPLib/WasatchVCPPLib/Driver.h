/**
    @file   Driver.h
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  interface of WasatchVCPP::Driver
    @note   customers normally wouldn't access this file; use WasatchVCPP.h instead
*/

#pragma once

#ifdef USE_LIBUSB_WIN32
#include "libusb.h"
#else
//#include <libusb-1_0.h>
#include <libusb.h>
#endif

#include "Logger.h"

#include <string>
#include <mutex>
#include <map>

//! Namespace encapsulating the internal implementation of WasatchVCPP; customers
//! would not normally access these classes or objects directly.
namespace WasatchVCPP
{
    class Spectrometer;

    /**
        @brief  This is an internal class encapsulating state and control of all
                connected spectrometers.

        End-users / customers would not normally access this class directly; they 
        should interact with the library through WasatchVCPPWrapper.h or 
        WasatchVCPP::Proxy.

        @par Thread-Safety

        It is expected that this library will be called from multi-threaded code,
        and therefore should support (or at least "fail gracefully") concurrent
        access.

        In general, this means that no more than one concurrent acquisition is
        supported at a time (via Spectrometer::mutAcquisition), and that only one
        control message may be exchanged over endpoint 0 at any given time (per
        Spectrometer).

        There are no specific locks in place, presently, to preclude things like:

        - changing integration time during acquisition (in fact, cancelOperation
          will do precisely this)
        - changing laser state during acquisition
    */
    class Driver
    {
        public:

            //! This is where the "master version number" is stored for the
            //! library.  It's not in WasatchVCPP.h because that file will
            //! often be customer-writeable...what we really want to know is
            //! what version of the LIBRARY was compiled into the DLL, not
            //! what the "current" customer API is.
            //!
            //! @note this value is checked by scripts/deploy against the given 
            //!       release tag
            const std::string libraryVersion = "1.0.5";

            static Driver* getInstance();

            int getNumberOfSpectrometers();
            int openAllSpectrometers();
            bool closeAllSpectrometers();

            Spectrometer* getSpectrometer(int index);
            bool removeSpectrometer(int index);

            std::string getLibraryVersion();

            Logger logger;

        private:
            static std::mutex mutDriver;        //!< synchronize singleton 
            static std::mutex mutSpectrometers; //!< synchronize spectrometers map
            static Driver* instance;

            Driver(); 

            std::map<int, Spectrometer*> spectrometers;
    };
}
