/**
    @file   Spectrometer.cpp
    @author Mark Zieg <mzieg@wasatchphotonics.com>
    @brief  implementation of WasatchVCPP::Spectrometer
    @see    WasatchVCPPWrapper.h for list of missing features
    @note   customers normally wouldn't access this file; use WasatchVCPP.h instead
*/

#include "pch.h"
#include "Driver.h"
#include "Spectrometer.h"
#include "ParseData.h"
#include "Uint40.h"
#include "Util.h"

#include <math.h>
#include <stdio.h>

#include <algorithm>
#include <chrono>

using std::string;
using std::vector;
using std::max;
using std::min;

#ifdef _WIN32
#pragma warning(disable : 26451) // using operator on 4-byte value then casting result to 8-byte value
#endif

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

const int HOST_TO_DEVICE = 0x40;
const int DEVICE_TO_HOST = 0xC0;

const int MIN_ARM_LEN = 8;

unsigned long MAX_UINT24 = 16777216;

////////////////////////////////////////////////////////////////////////////////
// Lifecycle
////////////////////////////////////////////////////////////////////////////////

WasatchVCPP::Spectrometer::Spectrometer(WPVCPP_UDEV_TYPE* udev, int pid, int index, Logger& logger)
    : udev(udev), pid(pid), index(index), logger(logger), eeprom(logger)
{

    logger.debug("Spectrometer::ctor: instantiating index %d (pid 0x%04x)", index, pid);

    driver = Driver::getInstance();

    // read firmware versions first (useful for debugging, validates FPGA comms)
    fpgaVersion = getFPGAVersion();
    firmwareVersion = getFirmwareVersion();
    logger.debug("firmware %s, FPGA %s", firmwareVersion.c_str(), fpgaVersion.c_str());

    ////////////////////////////////////////////////////////////////////////////
    // EEPROM
    ////////////////////////////////////////////////////////////////////////////

    readEEPROM();

    ////////////////////////////////////////////////////////////////////////////
    // post-eeprom initialization
    ////////////////////////////////////////////////////////////////////////////

    pixels = eeprom.activePixelsHoriz;

    wavelengths.resize(pixels);
    for (int i = 0; i < pixels; i++)
        wavelengths[i] = eeprom.wavecalCoeffs[0] 
                       + eeprom.wavecalCoeffs[1] * i 
                       + eeprom.wavecalCoeffs[2] * i * i
                       + eeprom.wavecalCoeffs[3] * i * i * i
                       + eeprom.wavecalCoeffs[4] * i * i * i * i;

    if (eeprom.excitationNM > 0)
    {
        const double nmToCm = 1.0 / 1e7;
        const double laserCm = 1.0 / (eeprom.excitationNM * nmToCm);

        wavenumbers.resize(pixels);
        for (int i = 0; i < pixels; i++)
            if (wavelengths[i] != 0)
                wavenumbers[i] = laserCm - (1.0 / (wavelengths[i] * nmToCm));
            else
                wavenumbers[i] = 0;
    }
    else
        wavenumbers.resize(0);

    // apply configured gain/offset from EEPROM to FPGA
    setDetectorGain     (eeprom.detectorGain);
    setDetectorGainOdd  (eeprom.detectorGainOdd);
    setDetectorOffset   (eeprom.detectorOffset);
    setDetectorOffsetOdd(eeprom.detectorOffsetOdd);

    // initialize integration time
    const int MAX_SENSIBLE_STARTUP_INTEGRATION_TIME_MS = 5000;
    if (eeprom.startupIntegrationTimeMS >= eeprom.minIntegrationTimeMS &&
        eeprom.startupIntegrationTimeMS <= eeprom.maxIntegrationTimeMS &&
        eeprom.startupIntegrationTimeMS <= MAX_SENSIBLE_STARTUP_INTEGRATION_TIME_MS)
        setIntegrationTimeMS(eeprom.startupIntegrationTimeMS);
    else if (eeprom.minIntegrationTimeMS <= MAX_SENSIBLE_STARTUP_INTEGRATION_TIME_MS)
        setIntegrationTimeMS(eeprom.minIntegrationTimeMS);

    // initialize TEC and setpoint
    if (eeprom.hasCooling)
    {
        setDetectorTECSetpointDegC(eeprom.startupDetectorTemperatureDegC);
        setDetectorTECEnable(true);
    }

    // initialize micro models
    if (isMicro())
    {
        // @todo initVerticalROI();
    }

    // initialize acquisition parameters
    endpoints.push_back(0x82);
    pixelsPerEndpoint = pixels;
    if (pixels == 2048 && !isARM())
    {
        endpoints.push_back(0x86);
        pixelsPerEndpoint = 1024;
    }
    bufSubspectrum.resize(pixelsPerEndpoint * 2);

    if (isInGaAs())
        setHighGainModeEnable(true);

    logger.debug("Spectrometer::ctor: done");
}

WasatchVCPP::Spectrometer::~Spectrometer()
{
    logger.info("WasatchVCPP::Spectrometer::dtor");
    close();
}

bool WasatchVCPP::Spectrometer::close()
{
    logger.info("Spectrometer::close");
    if (udev != nullptr)
    {
#if USE_LIBUSB_WIN32
        usb_release_interface(udev, 0);
        usb_close(udev);
#else
        logger.info("Spectrometer::close releasing interface on Linux");
        libusb_release_interface(udev, 0);
        libusb_close(udev);
#endif
        udev = nullptr;
    }
    logger.info("Spectrometer::close: end");
    return true;
}

bool WasatchVCPP::Spectrometer::readEEPROM()
{
    vector<vector<uint8_t> > pages;
    for (int page = 0; page < EEPROM::MAX_PAGES; page++)
    {
        auto buf = getCmd2(0x01, EEPROM::PAGE_SIZE, page);
        pages.push_back(buf);
        logger.debug("EEPROM page %d: %s", page, Util::toHex(buf).c_str());
    }

    if (!eeprom.parse(pages))
    {
        logger.error("Spectrometer::readEEPROM: unable to parse EEPROM");
        return false;
    }
    if (eeprom.has_srm()) 
    {
        srm_in_EEPROM = true;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Opcodes
////////////////////////////////////////////////////////////////////////////////

bool WasatchVCPP::Spectrometer::setIntegrationTimeMS(unsigned long ms)
{
    ms = clamp(ms, 1, MAX_UINT24 - 1);

    unsigned short lsw = ms & 0xffff;
    unsigned short msw = (ms >> 16) & 0x00ff;

    auto bytesWritten = sendCmd(0xb2, lsw, msw);

    integrationTimeMS = ms;
    logger.debug("integrationTimeMS -> %lu", ms);
    return bytesWritten >= 0;
}

bool WasatchVCPP::Spectrometer::setLaserPowerPerc(float percent)
{
    if (!eeprom.hasLaser)
    {
        logger.error("Unable to control laser. EEPROM reports no laser.");
        return false;
    }

    float value = float(max(0.f, min(100.f, percent)));
    logger.debug("set_laser_power_perc: range (0, 100), requested %.2f, applying %.2f", percent, value);

    // If zero, disable laser
    if (value <= 0)
        return setLaserEnable(false);

    // If full power (and allowed), disable modulation and exit
    if (value >= 100) {

        if (laserPowerRequireModulation) 
        {
            logger.debug("100%% power requested, yet laser modulation required, so not disabling modulation");
        }
        else 
        {
            logger.debug("Turning off laser modulation (full power)");
            setModEnable(false);
            return true;
        }
    }

    int period_us = laserPowerHighResolution ? 1000 : 100;
    int width_us = int((value * period_us / 100.0)); // note that value is in range (0, 100) not (0, 1)

    // pulse width can't be longer than period, or shorter than 1us
    width_us = max(1, min(width_us, period_us));

    logger.debug("Writing values of period %d and width %d", period_us, width_us);
    
    // Change the pulse period. Note that we're not parsing into 40-bit
    // because we currently enforce periods of either 100us or 1000us
    // (both fitting well within uint16)
    logger.debug("setting mod period");
    bool result = setModPeriodus(period_us);
	if (!result) 
    {
        logger.error("Hardware Failure to send laser mod. pulse period");
        return false;
	}

    // Set the pulse width to the 0 - 100 percentage of power
    logger.debug("setting mod width");
    result = setModWidthus(width_us);
	if (!result) 
    {
        logger.error("Hardware Failure to send pulse width");
        return false;
	}

    // Enable modulation
    logger.debug("enabling mod");
    result = setModEnable(true);
	if (!result) 
    {
		logger.error("Hardware Failure to send laser modulation");
		return false;
	}

    return result;
}

bool WasatchVCPP::Spectrometer::setLaserPowermW(float mW_in) {
    if (!eeprom.hasLaserPowerCalibration()) {
        logger.error("EEPROM doesn't have laser power calibration");
        return false;
    }
    float mW = min(eeprom.maxLaserPowerMW, max(eeprom.minLaserPowerMW, mW_in));

    float perc = eeprom.laserPowermWToPercent(mW);
	logger.debug("set_laser_power_mW: range (%.2f, %.2f), requested %.2f, approved %.2f, percent = %.2f",
            eeprom.minLaserPowerMW,
            eeprom.maxLaserPowerMW,
            mW_in,
            mW,
            perc);
    return setLaserPowerPerc(perc);
}

bool WasatchVCPP::Spectrometer::setModEnable(bool flag) {
    modEnabled = flag;
    int value = flag ? 1 : 0;
    sendCmd(0xbd, value);
    return true;
}

// currently for debugging
int WasatchVCPP::Spectrometer::getModEnabled(void){
    vector<uint8_t> getRes = getCmd(0xe3, 1);
    logger.debug("got modulation status response is");
    for (auto i : getRes)
        logger.debug("%d", i);
    return getRes[0];
}

// currently for debugging
uint64_t WasatchVCPP::Spectrometer::getModPeriodus(void) {
    vector<uint8_t> getRes = getCmd(0xcb, 5);
    logger.debug("got modulation period. response is");
    for (auto i : getRes)
        logger.debug("%d", i);
    uint64_t modPeriod = getRes[4]
						| getRes[3] >> 8
						| getRes[2] >> 16
						| getRes[1] >> 24
						| (((uint64_t)getRes[0]) >> 32);
    return modPeriod;
}

bool WasatchVCPP::Spectrometer::setModPeriodus(int us) {
    uint16_t lsw;
    uint16_t msw;
    //uint16_t* bit_buf;
    Uint40 bit_buf = Uint40(us);
    //bit_buf = to40bit(us);
    lsw = bit_buf.LSW;
    msw = bit_buf.MidW;
    uint8_t buf[8] = { (uint8_t)bit_buf.MSB, 0, 0, 0, 0, 0, 0, 0 };
    auto bytesWritten = sendCmd(0xc7, lsw, msw, buf, sizeof(buf)/sizeof(buf[0]));
    return bytesWritten >= 0;
}

bool  WasatchVCPP::Spectrometer::setModWidthus(int us) {
    Uint40 bit_buf = Uint40(us);
    uint16_t lsw = bit_buf.LSW;
    uint16_t msw = bit_buf.MidW;
    uint8_t buf[8] = { (uint8_t)bit_buf.MSB, 0, 0, 0, 0, 0, 0, 0 };
    auto bytesWritten = sendCmd(0xdb, lsw, msw, buf, sizeof(buf)/sizeof(buf[0]));
    return bytesWritten >= 0;
}

bool WasatchVCPP::Spectrometer::setLaserEnable(bool flag)
{
    auto bytesWritten = sendCmd(0xbe, flag ? 1 : 0);
    laserEnabled = flag;
    logger.debug("laserEnable -> %d (bytesWritten %d)", flag, bytesWritten);
    return bytesWritten >= 0;
}

uint16_t WasatchVCPP::Spectrometer::serializeGain(float value)
{
    uint8_t msb = (int)value & 0xff;
    uint8_t lsb = ((uint16_t)((value - msb) * 256.0 + 0.5)) & 0xff;
    uint16_t word = (msb << 8) | lsb;
    return word;
}

bool WasatchVCPP::Spectrometer::setDetectorGain(float value)
{
    const uint8_t op = 0xb7;
    if (value < 0 || value >= 256)
        return false;

    uint16_t word = serializeGain(value);

    auto bytesWritten = sendCmd(op, word);
    logger.debug("detectorGain -> 0x%04x (%.2f)", word, value);
    return bytesWritten >= 0;
}

bool WasatchVCPP::Spectrometer::setDetectorGainOdd(float value)
{
    const uint8_t op = 0x9d;
    if (value < 0 || value >= 256)
        return false;

    uint16_t word = serializeGain(value);

    auto bytesWritten = sendCmd(op, word);
    logger.debug("detectorGainOdd -> 0x%04x (%.2f)", word, value);
    return bytesWritten >= 0;
}

bool WasatchVCPP::Spectrometer::setDetectorOffset(int16_t value)
{
    const uint8_t op = 0xb6;
    uint16_t word = *((uint16_t*) &value); // send original signed int16 bit pattern
    auto bytesWritten = sendCmd(op, word);
    logger.debug("detectorOffset -> 0x%04x (%d)", word, value);
    return bytesWritten >= 0;
}

bool WasatchVCPP::Spectrometer::setDetectorOffsetOdd(int16_t value)
{
    const uint8_t op = 0x9c;
    uint16_t word = *((uint16_t*) &value);
    auto bytesWritten = sendCmd(op, word);
    logger.debug("detectorOffsetOdd -> 0x%04x (%d)", word, value);
    return bytesWritten >= 0;
}

bool WasatchVCPP::Spectrometer::setDetectorTECEnable(bool flag)
{
    const uint8_t op = 0xd6;
    if (!eeprom.hasCooling)
        return false;

    if (!detectorTECSetpointHasBeenSet)
    {
        logger.debug("defaulting TEC setpoint to min");
        setDetectorTECSetpointDegC(eeprom.detectorTempMin);
    }

    auto bytesWritten = sendCmd(op, flag ? 1 : 0);
    logger.debug("detectorTECEnable -> %s", flag ? "on" : "off");
    return bytesWritten >= 0;
}

bool WasatchVCPP::Spectrometer::setDetectorTECSetpointDegC(int degC)
{
    const uint8_t op = 0xd8;
    if (!eeprom.hasCooling)
        return false;

    if (degC < eeprom.detectorTempMin || degC > eeprom.detectorTempMax)
        return false;

    float dac = eeprom.degCToDACCoeffs[0]
              + eeprom.degCToDACCoeffs[1] * degC
              + eeprom.degCToDACCoeffs[2] * degC * degC;

    // clamp and round to 12-bit DAC value
    if (dac < 0)
        dac = 0;
    if (dac > 0xfff)
        dac = 0xfff;

    uint16_t word = ((uint16_t)(dac + 0.5)) & 0xfff;
    auto bytesWritten = sendCmd(op, word);

    logger.debug("detectorTECSetpointDegC -> 0x%04x (%d)", word, degC);

    detectorTECSetpointHasBeenSet = true;
    detectorTECSetointDegC = degC;

    return bytesWritten >= 0;
}

bool WasatchVCPP::Spectrometer::setHighGainModeEnable(bool flag)
{
    const uint8_t op = 0xeb;
    if (!isInGaAs())
        return false;

    // no idea why this is required, but ENLIGHTEN does it; presumably
    // some code was copy-pasted in FW and it thinks it's dealing with
    // a uint40 like used in laser modulation commands
    vector<uint8_t> junk(8); 

    auto bytesWritten = sendCmd(op, flag ? 1 : 0, 0, junk);

    logger.debug("highGainModeEnable -> %s", flag ? "on" : "off");

    return bytesWritten >= 0;
}

string WasatchVCPP::Spectrometer::getFirmwareVersion()
{
    string s = "ERROR";
    auto data = getCmd(0xc0, 4);
    if (data.size() >= 4)
        s = Util::sprintf("%d.%d.%d.%d", data[3], data[2], data[1], data[0]);

    logger.debug("firmwareVersion <- %s", s.c_str());
    return s;
}

string WasatchVCPP::Spectrometer::getFPGAVersion()
{
    auto data = getCmd(0xb4, 7);
    string s;
    for ( auto c : data )
        if (0x20 <= c && c <= 0x7f) // visible ASCII
            s += (char)c;

    logger.debug("fpgaVersion <- %s", s.c_str());

    return s;
}

//! @returns negative on error, else valid uint16_t
int32_t WasatchVCPP::Spectrometer::getDetectorTemperatureRaw()
{
    if (!eeprom.hasCooling)
        return -1;

    const uint8_t op = 0xd7;
    auto data = getCmd(op, 2);
    if (data.size() < 2)
    {
        logger.error("getDetectorTemperatureRaw: data = %s", Util::toHex(data).c_str());
        return -1;
    }
        
    uint16_t raw = (data[0] << 8) | data[1]; // MSB-LSB

    logger.error("getDetectorTemperatureRaw <- 0x%04x", raw);

    return raw;
}

float WasatchVCPP::Spectrometer::getDetectorTemperatureDegC()
{
    int32_t rawOrError = getDetectorTemperatureRaw();
    if (rawOrError < 0)
        return -999;

    uint16_t raw = (uint16_t) rawOrError;
    float degC = eeprom.adcToDegCCoeffs[0]
               + eeprom.adcToDegCCoeffs[1] * raw
               + eeprom.adcToDegCCoeffs[2] * raw * raw;

    logger.debug("detectorTemperatureDegC <- %.2f (0x%04x raw)", degC, raw);
    return degC;
}

////////////////////////////////////////////////////////////////////////////////
// Acquisition
////////////////////////////////////////////////////////////////////////////////

//! Attempt to cancel an ongoing integration.
//! 
//! @warning this function will not work correctly without custom firmware
bool WasatchVCPP::Spectrometer::cancelOperation(bool blocking)
{
    if (!acquiring)
        return false;

    // This will cause this class's bulk endpoint read "retry loop" to stop 
    // cycling, at least within maxTimeoutMS.  However, this doesn't actually 
    // change anything inside the hardware spectrometer.
    operationCancelled = true;

    // To actually cause the spectrometer to abruptly end the current acquisition
    // before the original scheduled "end-of-integration time," we need to reduce
    // the current integration time.  With appropriate FPGA FW, this will cause
    // the current acquisition to "end immediately" (read-out the sensor and push
    // the abbreviated intensities to the bulk endpoint, where we may or may not
    // bother to actually read them).
    cancelledIntegrationTimeMS = integrationTimeMS;
    setIntegrationTimeMS(eeprom.minIntegrationTimeMS);

    // The question now is when to restore the original integration time.  My 
    // current approach is to set a new flag, lastAcquisitionWasCancelled, which
    // will cause the NEXT acquisition to start by re-setting the originally-
    // configured integration time.
    lastAcquisitionWasCancelled = true;

    // if requested, block until current bulk read times-out
    if (blocking)
    {
        // check at least 1Hz 
        int sleepMS = min(maxTimeoutMS, 1000); 
        while (acquiring)
        {
            logger.debug("cancelOperation: blocking while acquiring");
            Util::sleepMS(sleepMS);
        }
    }
    return true;
}

//! Determine how long we should wait for an acquisition to return the spectrum.
//!
//! Note that this is the "full period" we should wait, which may end up being
//! implemented through a series of shorter waits, allowing length operations to
//! be cancelled.
long WasatchVCPP::Spectrometer::generateTotalWaitMS()
{
    int numberOfSpectrometers = driver->getNumberOfSpectrometers();
    return 100 * numberOfSpectrometers 
         + 2 * integrationTimeMS 
         + 500;
}

std::vector<double> WasatchVCPP::Spectrometer::getSpectrum()
{
    mutAcquisition.lock();
    logger.debug("getSpectrum started on %", eeprom.serialNumber.c_str());

    // perform clean-up from cancelled operation, if any
    if (lastAcquisitionWasCancelled)
    {
        setIntegrationTimeMS(cancelledIntegrationTimeMS);
        lastAcquisitionWasCancelled = false;
    }

    vector<double> spectrum;
    if (acquiring)
    {
        // just in case
        logger.error("Spectrometer %s already acquiring", eeprom.serialNumber.c_str());
        mutAcquisition.unlock();
        return spectrum;
    }

    operationCancelled = false;
    acquiring = true;

    // send software trigger
    logger.debug("sending ACQUIRE");
    sendCmd(0xad);

    // how long we'll wait for the FIRST subspectrum
    int subspectrumTimeoutMS = generateTotalWaitMS();

    for (auto ep : endpoints)
    {
        auto subspectrum = getSubspectrum(ep, subspectrumTimeoutMS);
        if (subspectrum.size() != pixelsPerEndpoint)
        {
            if (operationCancelled)
                logger.debug("getSpectrum: operation cancelled");
            else
                logger.error("failed reading subspectrum (%d of %d pixels read)", 
                    subspectrum.size(), pixelsPerEndpoint);
            operationCancelled = false;
            acquiring = false;
            mutAcquisition.unlock();
            return vector<double>();
        }

        for (auto word : subspectrum)
            spectrum.push_back(word);

        // subspectra from subsequent endpoints should be nearly instantaneous 
        // (USB comms only)
        subspectrumTimeoutMS = 100 * driver->getNumberOfSpectrometers();
    }

    ////////////////////////////////////////////////////////////////////////////
    // post-processing
    ////////////////////////////////////////////////////////////////////////////

    // stomp first pixel -- only required if start-of-frame marker enabled
    // spectrum[0] = spectrum[1];

    if (eeprom.featureMask.invertXAxis)
        std::reverse(spectrum.begin(), spectrum.end());

    correctBadPixels(spectrum);

    if (eeprom.featureMask.bin2x2)
        spectrum = bin2x2(spectrum);

    logger.debug("getSpectrum: returning spectrum of %d pixels", spectrum.size());
    acquiring = false;
    mutAcquisition.unlock();
    return spectrum;
}

//! @param allocatedMS (Input) total time allocated in milliseconds (wall-clock)
//! @returns either a populated subspectrum of exactly 'pixelsPerEndpoint' 
//!          deserialized pixels, or an empty vector on error
std::vector<uint16_t> WasatchVCPP::Spectrometer::getSubspectrum(uint8_t ep, long allocatedMS)
{
    //! @see https://sourceforge.net/p/libusb-win32/code/HEAD/tree/trunk/libusb/src/windows.c#l493
    //! @see https://sourceforge.net/p/libusb-win32/code/HEAD/tree/trunk/libusb/src/error.h#l41
    const int LIBUSB_WIN32_ERROR_TIMEOUT = -116;
    const int LIBUSB_ERROR_TIMEOUT = -7;

    vector<uint16_t> subspectrum;

    int bytesExpected = (int)bufSubspectrum.size();
    int bytesLeftToRead = bytesExpected;
    int totalBytesRead = 0;

    // break the total allotted time (e.g. 5min) into shorter reads (e.g. 1sec),
    // between which we can check for external cancellation events
    
    // of the total time allocated for this read, how much is left?
    long remainingMS = allocatedMS;

    // what is the size of the individual reads we're going to perform (allowing
    // interruptions between, but not during each)?
    long periodMS = allocatedMS; // min(allocatedMS, (long)maxTimeoutMS);

    // what is the WALLCLOCK elapsed time we've spent so far on this venture?
    long elapsedMS = 0;

    // iterate over multiple reads until we have all this subspectrum's pixels,
    // or we run out of time
    while (totalBytesRead < bytesExpected)
    {
        int timeoutMS = min(periodMS, remainingMS);
        auto timeReadStart = std::chrono::high_resolution_clock::now();

        logger.debug("attempting to read %d bytes from endpoint 0x%02x with timeout %dms", 
            bytesLeftToRead, ep, timeoutMS);

#if USE_LIBUSB_WIN32
        int result = 0;
        int bytesRead = usb_bulk_read(udev, ep, (char*)&bufSubspectrum[0], bytesLeftToRead, timeoutMS);
#else
        int bytesRead = 0;
        int result = libusb_bulk_transfer(udev, ep, (unsigned char*)&bufSubspectrum[0], bytesLeftToRead, &bytesRead, timeoutMS);
#endif

        logger.debug("read %d bytes from endpoint 0x%02x (result %d)", bytesRead, ep, result);

        // update timing
        auto timeReadEnd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedThisRead = timeReadEnd - timeReadStart;
        long elapsedThisReadMS = (long)(elapsedThisRead.count() * 1000 + 0.5);
        elapsedMS += elapsedThisReadMS;
        remainingMS -= elapsedThisReadMS;

        // have we been cancelled?
        if (operationCancelled)
        {
            logger.error("getSubspectrum: cancellation detected");
            return vector<uint16_t>();
        }

        // did an error occur?
        if (bytesRead <= 0)
        {
            // was it a timeout?
            if (bytesRead == LIBUSB_WIN32_ERROR_TIMEOUT || result == LIBUSB_ERROR_TIMEOUT)
            {
                // do we still have time to spend on this?
                if (remainingMS > 0)
                {
                    logger.debug("getSubspectrum: still waiting after timeout (allocated %ldms, period %dms, elapsed %ldms, remaining %ldms",
                        allocatedMS, periodMS, elapsedMS, remainingMS);
                    continue;
                }
            }

            // either it wasn't a timeout, or we're out of time
            logger.error("getSubspectrum: bytesRead negative or zero, giving up (allocated %ldms, period %dms, elapsed %ldms, remaining %ldms (%s)", 
                allocatedMS, periodMS, elapsedMS, remainingMS, 
#if USE_LIBUSB_WIN32
                usb_strerror()
#else
                libusb_strerror(libusb_error(result))
#endif
            );
            return vector<uint16_t>();
        }

        // doesn't seem worth supporting this case; doubt it occurs
        if (bytesRead % 2 != 0)
        {
            logger.error("getSubspectrum: read odd number of bytes (%d)", bytesRead);
            return vector<uint16_t>();
        }

        ////////////////////////////////////////////////////////////////////////
        // we received aligned data, so demarshall
        ////////////////////////////////////////////////////////////////////////

        // demarshal little-endian pixels
        for (int i = 0; i + 1 < bytesRead; i += 2)
            subspectrum.push_back(bufSubspectrum[i] | (bufSubspectrum[i + 1] << 8));

        totalBytesRead += bytesRead;
        bytesLeftToRead -= bytesRead;

        if (bytesLeftToRead != 0)
            logger.debug("getSubspectrum: totalBytesRead %d, bytesLeftToRead %d", 
                totalBytesRead, bytesLeftToRead);
    }

    return subspectrum;
}

unsigned long WasatchVCPP::Spectrometer::getIntegrationTimeMS()
{ return ParseData::toUInt24(getCmd(0xbf, 3, 0, 6)); }

bool WasatchVCPP::Spectrometer::getLaserEnable()
{
    if (!eeprom.hasLaser)
        return false;

    auto data = getCmd(0xe2, 1);
    if (data.size() < 1)
    {
        logger.error("getLaserEnable: no response");
        return false;
    }

    return data[0] != 0;
}

float WasatchVCPP::Spectrometer::deserializeGain(const vector<uint8_t>& data)
{
    if (data.size() != 2)
    {
        logger.error("deserializeGain: invalid data %s", Util::toHex(data).c_str());
        return ErrorCodes::InvalidGain;
    }

    // note that we SEND gain big-endian, but we READ it little-endian :-(
    const uint8_t& lsb = data[0];
    const uint8_t& msb = data[1];

    return msb + (lsb / 256.f);
}

//! @returns ErrorCodes::InvalidGain on error
float WasatchVCPP::Spectrometer::getDetectorGain()
{ return deserializeGain(getCmd(0xc5, 2)); }

//! @returns ErrorCodes::InvalidGain on error
float WasatchVCPP::Spectrometer::getDetectorGainOdd()
{ return isInGaAs() ? deserializeGain(getCmd(0x9f, 2)) : ErrorCodes::NotInGaAs; }

int WasatchVCPP::Spectrometer::getDetectorOffset()
{ return ParseData::toInt16(getCmd(0xc4, 2)); }

int WasatchVCPP::Spectrometer::getDetectorOffsetOdd()
{ return isInGaAs() ? ParseData::toInt16(getCmd(0x9e, 2)) : ErrorCodes::InvalidOffset; }

bool WasatchVCPP::Spectrometer::getDetectorTECEnable()
{ return eeprom.hasCooling ? ParseData::toBool(getCmd(0xda, 1)) : false; }

int WasatchVCPP::Spectrometer::getDetectorTECSetpointDegC()
{ return eeprom.hasCooling ? detectorTECSetointDegC : ErrorCodes::InvalidTemperature; }

bool WasatchVCPP::Spectrometer::getHighGainModeEnable()
{ return isInGaAs() ? ParseData::toBool(getCmd(0xec, 1)) : false; }

////////////////////////////////////////////////////////////////////////////////
// Post-Processing
////////////////////////////////////////////////////////////////////////////////

//! Averages over bad pixels in-place.
void WasatchVCPP::Spectrometer::correctBadPixels(std::vector<double> spectrum)
{
    int pixels = (int)spectrum.size();
    for (int i = 0; i < eeprom.badPixelsVector.size(); i++)
    {
        auto badPix = eeprom.badPixelsVector[i];

        if (badPix < 0)
            continue;

        if (badPix == 0)
        {
            // handle left edge
            auto nextGood = badPix + 1;
            while (eeprom.badPixelsSet.count(nextGood) && nextGood < pixels)
            {
                nextGood++;
                i++;
            }
            if (nextGood < pixels)
                for (int j = 0; j < nextGood; j++)
                    spectrum[j] = spectrum[nextGood];
        }
        else
        {
            // find previous good pixel
            auto prevGood = badPix - 1;
            while (eeprom.badPixelsSet.count(prevGood) && prevGood >= 0)
                prevGood -= 1;

            if (prevGood >= 0) 
            {
                // find next good pixel
                auto nextGood = badPix + 1;
                while (eeprom.badPixelsSet.count(nextGood) && nextGood < pixels)
                {
                    nextGood++;
                    i++;
                }

                if (nextGood < pixels)
                {
                    // draw a line between prevGood and nextGood intensity
                    float deltaIntensity = spectrum[nextGood] - spectrum[prevGood];
                    int rangePix = nextGood - prevGood;
                    float intensityPerPix = deltaIntensity / rangePix;
                    for (int j = 0; j < rangePix - 1; j++)
                        spectrum[prevGood + j + 1] = spectrum[prevGood] + intensityPerPix * (j + 1);
                }
                else
                {
                    // we ran off the high end, so copy-right
                    for (int j = badPix; j < pixels; j++)
                        spectrum[j] = spectrum[prevGood];
                }
            }
        }
    }
}

//! perform 2x2 binning for Bayer filters
std::vector<double> WasatchVCPP::Spectrometer::bin2x2(const std::vector<double> spectrum)
{
    vector<double> binned;
    int pixels = (int)spectrum.size();
    for (int i = 0; i < pixels - 1; i++)
        binned.push_back((spectrum[i] + spectrum[i + 1]) / 2.0);
    binned.push_back(spectrum[pixels - 1]);

    return binned;
}

////////////////////////////////////////////////////////////////////////////////
// Control Messages
////////////////////////////////////////////////////////////////////////////////

//! Write data to the spectrometer.
//!
//! @param bRequest (Input) the opcode being sent
//! @param wValue (Input) in general, the primary argument being sent; can also
//!        represent the "actual opcode" when bRequest is 0xff, indicating a so-
//!        called "second-tier command"
//! @param wIndex (Input) in general, a secondary argument being sent; in the 
//!        case of "longer" primary arguments (larger than 16 bits), may
//!        represent an extra 8 or 16 bits of the primary argument; or in the 
//!        case of "second-tier commands", may represent the "effective primary
//!        argument" (as the "effective opcode" is being sent as wValue)
//! @param data (Input) additional payload being sent along with the opcode and
//!        parameters; in the case of opcodes taking truly long arguments (larger
//!        than 32 bits, such as the uint40 values used for laser modulation),
//!        can provide additional bits of precision atop wValue and wIndex
//! @param len (Input) number of bytes provided in data
//! @returns number of bytes written (not really indicative of success/failure).
//!
//! @note I *think* this actually indicates the number of DATA bytes 
//!       transferred, and is not actually intended to include the "control
//!       packet" itself; therefore, bytes written will be ZERO for most 
//!       successful setters, and negative for comms failures.
int WasatchVCPP::Spectrometer::sendCmd(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint8_t* data, int len)
{
    // ARM firmware expects all commands to include at least 8 payload bytes
    uint8_t buf[MIN_ARM_LEN] = { 0 };
    if (data == nullptr && isARM())
    {
        data = buf;
        len = sizeof(buf);
    }

    string dataStr;
    if (len > 0)
        dataStr = Util::sprintf(" (data: %s)", Util::toHex(data, len).c_str());

    if (!lockComm())
        return -1;

#if USE_LIBUSB_WIN32
    int bytesWritten = usb_control_msg        (udev, HOST_TO_DEVICE, bRequest, wValue, wIndex, (char*)data, len, maxTimeoutMS);
#else
    int bytesWritten = libusb_control_transfer(udev, HOST_TO_DEVICE, bRequest, wValue, wIndex,        data, len, maxTimeoutMS);
#endif

    unlockComm();

    logger.debug("sendCmd(bRequest 0x%02x, wValue 0x%04x, wIndex 0x%04x, len %d, timeout %dms)%s (wrote %d bytes)", 
        bRequest, wValue, wIndex, len, maxTimeoutMS, dataStr.c_str(), bytesWritten);
    return bytesWritten;
}

//! Convenience wrapper over sendCmd if payload is in a vector.
int WasatchVCPP::Spectrometer::sendCmd(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, vector<uint8_t> data)
{
    return sendCmd(bRequest, wValue, wIndex, &data[0], (int)data.size());
}

//! This is the standard "getter opcode" function.  It doesn't take a "wValue" 
//! parameter because getters usually don't have one.  (The main exception to 
//! that rule is so-called "2nd-tier" gettors, for which getCmd2 is provided.)
//!
//! @param request (Input) the opcode
//! @param len (Input) how many bytes the caller actually NEEDS back
//! @param fullLen (Input) how many bytes we think the spectrometer will actually
//!        send, with padding
//! @returns vector of just the 'len' needed bytes
vector<uint8_t> WasatchVCPP::Spectrometer::getCmd(uint8_t bRequest, int len, uint16_t wIndex, int fullLen)
{
    const uint16_t wValue = 0;
    logger.debug("relaying getCmd(bRequest 0x%02x, len %d, wIndex 0x%04x, fullLen %d)",
        bRequest, len, wIndex, fullLen);
    return getCmdReal(bRequest, wValue, wIndex, len, fullLen);
}

//! This supports so-called "2nd-tier" gettors, all of which use opcode 0xff and
//! are instead distinguished by their 'value' parameter.
//!
//! @param value (Input) which 2nd-tier spectrometer attribute is being read
//! @param len (Input) how many bytes the caller actually NEEDS in response
//! @param fullLen (Input) how many bytes we think the spectrometer will send, 
//!        with padding
//! @returns vector of just the 'len' bytes requested
vector<uint8_t> WasatchVCPP::Spectrometer::getCmd2(uint16_t wValue, int len, uint16_t wIndex, int fullLen)
{
    const uint8_t bRequest = 0xff;
    logger.debug("relaying getCmd2(wValue 0x%04x, len %d, wIndex 0x%04x, fullLen %d)",
        wValue, len, wIndex, fullLen);
    return getCmdReal(bRequest, wValue, wIndex, len, fullLen);
}

//! The actual "gettor" function, typically called through either the getCmd or 
//! getCmd2 facades.
//!
//! All "Hungarian notation" parameter names (bRequest, wValue etc) are taken from
//! public USB specifications to reduce confusion.  (It is worth remembering that
//! bRequest is a byte, wValue and wIndex are 16-bit words, etc.)
//! 
//! @see getCmd
//! @see getCmd2
//! @see https://www.beyondlogic.org/usbnutshell/usb6.shtml
//!
//! @param bRequest (Input) the accessor opcode to read
//! @param wValue   (Input) the primary argument to the opcode (or "specific opcode"
//!                         if bRequest is 0xff to indicate a "second-tier" opcode)
//! @param wIndex   (Input) the secondary argument to the opcode; or additional bytes
//!                         of precision for long arguments; or the primary argument
//!                         to second-tier opcodes.
//! @param len      (Input) how many bytes we NEED BACK in the response from this 
//!                         function
//! @param fullLen  (Input) how many bytes we ACTUALLY EXPECT BACK over USB
//! @returns the 'len' requested bytes from the USB response (empty on error)
vector<uint8_t> WasatchVCPP::Spectrometer::getCmdReal(
        uint8_t bRequest, 
        uint16_t wValue, 
        uint16_t wIndex, 
        int len, 
        int fullLen)
{
    // this is what we'll return
    vector<uint8_t> retval;

    // ARM firmware expects all commands to provide at least 8 payload bytes
    int bytesToRead = max(len, fullLen);
    if (isARM())
        bytesToRead = max(MIN_ARM_LEN, bytesToRead);

    // this is our temporary (often somewhat larger) buffer
    vector<uint8_t> data(bytesToRead); 

    if (!lockComm())
        return retval;

    logger.debug("getCmdReal(bRequest 0x%02x, wValue 0x%04x, wIndex 0x%04x, len %d, timeout %dms)", 
        bRequest, wValue, wIndex, bytesToRead, maxTimeoutMS);

#if USE_LIBUSB_WIN32
    int bytesRead = usb_control_msg        (udev, DEVICE_TO_HOST, bRequest, wValue, wIndex, (char*)&data[0], (int)data.size(), maxTimeoutMS);
#else
    int bytesRead = libusb_control_transfer(udev, DEVICE_TO_HOST, bRequest, wValue, wIndex,        &data[0], (int)data.size(), maxTimeoutMS);
#endif

    unlockComm();

    logger.debug("getCmdReal(0x%02x): read %d bytes: %s", bRequest, bytesRead, Util::toHex(data).c_str());

    if (bytesRead < 0)
    {
        logger.error("getCmdReal(0x%02x): no data", bRequest);
        return retval;
    }
    else if (bytesRead < len) 
    {
        logger.error("getCmdReal: incomplete response (%d bytes read, %d needed, %d expected)", 
            bytesRead, len, bytesToRead);
        return retval;
    }

    retval.resize(len);
    for (int i = 0; i < len; i++)
        retval[i] = data[i];

    return retval;
}

////////////////////////////////////////////////////////////////////////////////
// Utility
////////////////////////////////////////////////////////////////////////////////

bool WasatchVCPP::Spectrometer::isARM() { return pid == 0x4000; }

//! @todo support ARM-based InGaAs
bool WasatchVCPP::Spectrometer::isInGaAs() { return pid == 0x2000; }

bool WasatchVCPP::Spectrometer::isMicro()
{ return isARM() && Util::toLower(eeprom.detectorName).find("imx") != string::npos; }

//! @todo use PID to determine appropriate result code by platform
//! @warning Right now, we literally aren't reading the single-byte result code
//!          returned to the control endpoint following a sendCmd, so I don't
//!          even know where that status code is going.  I ASSUME that it simply
//!          gets "overwritten" by the next call to getCmdReal?
bool WasatchVCPP::Spectrometer::isSuccess(unsigned char opcode, int result)
{ return true; }

//! Clamps the value between min and max.  Would make a good M4 macro.
//! @todo move to Util
unsigned long WasatchVCPP::Spectrometer::clamp(unsigned long value, unsigned long min, unsigned long max)
{
    if (value < min)
        return min;
    else if (value > max)
        return max;
    return value;
}

bool WasatchVCPP::Spectrometer::lockComm()
{
    mutComm.lock();
    return true;
}

#ifdef _WIN32
#pragma warning(disable : 26110) // caller failing to hold lock before calling unlock
#endif
void WasatchVCPP::Spectrometer::unlockComm()
{
    mutComm.unlock();
}
