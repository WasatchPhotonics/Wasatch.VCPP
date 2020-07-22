/**
    @file   Driver.h
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  interface of WasatchVCPP::Driver
    @note   customers normally wouldn't access this file; use WasatchVCPP.h instead
*/

#pragma once

#include "libusb.h"
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

        - changing integration time during acquisition
        - changing laser state during acquisition
    */
    class Driver
    {
        public:
            const std::string libraryVersion = "1.0.0";

            static Driver* getInstance();

            int getNumberOfSpectrometers();
            int openAllSpectrometers();
            bool closeAllSpectrometers();

            Spectrometer* getSpectrometer(int index);
            bool removeSpectrometer(int index);

            std::string getLibraryVersion();

            Logger logger;

        private:
            static Driver* instance;
            Driver(); 

            std::map<int, Spectrometer*> spectrometers;

            std::mutex mutLifecycle;
    };
}
