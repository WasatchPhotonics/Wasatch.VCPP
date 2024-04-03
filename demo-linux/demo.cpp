#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <map>
#include <ctime>
#include <sstream>
#include <cstring>

#include "WasatchVCPP.h"

using std::vector;
using std::string;
using std::map;

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

const int specIndex = 0;
const int STR_LEN = 33;
const int LASER_WARMUP_SEC = 10;
const int PAGE_SIZE = 64;

////////////////////////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////////////////////////

int logLevel = WP_LOG_LEVEL_DEBUG;
int integrationTimeMS = 100;
int count = 5;
int pixels;
char serialNumber[STR_LEN];
char model[STR_LEN];
bool testLaser = false;
map<string, string> eeprom;
bool ramanModeEnabled = false;
bool tabs = false;
bool EEPROMedit = false;
bool integrationTimeEdit = false;
vector<float> wavelengths;
vector<float> wavenumbers;
unsigned long delay_us = 0;
int throwaways = 0;

////////////////////////////////////////////////////////////////////////////////
// Utility
////////////////////////////////////////////////////////////////////////////////

void pause(const string& msg)
{
    string prompt = "Press <return> to continue...";
    if (msg.size() > 0)
        prompt = msg;
    printf("\n%s", msg.c_str());
    string junk;
    std::getline(std::cin, junk);
}

string timestamp()
{
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buf[100] = {0}; // currently using 19 bytes
    std::strftime(buf, sizeof(buf), "%F %T", std::localtime(&now));
    return buf;
}

////////////////////////////////////////////////////////////////////////////////
// Functional Implementation
////////////////////////////////////////////////////////////////////////////////

void loadWavelengths()
{
    wavelengths.clear();
    float values[pixels];
    if (WP_SUCCESS == wp_get_wavelengths_float(specIndex, values, pixels))
        for (int i = 0; i < pixels; i++)
            wavelengths.push_back(values[i]);
}

void loadWavenumbers()
{
    float laserWavelength = atof(eeprom["excitationNM"].c_str());
    if (laserWavelength > 0)
        for (int i = 0; i < pixels; i++)
            wavenumbers.push_back(1e7/laserWavelength - 1e7/wavelengths[i]);
}

void loadEEPROM()
{
    int eeprom_count = wp_get_eeprom_field_count(specIndex);
    if (eeprom_count <= 0)
        return;

    const char** names  = (const char**)malloc(eeprom_count * sizeof(const char*));
    const char** values = (const char**)malloc(eeprom_count * sizeof(const char*));

    if (WP_SUCCESS != wp_get_eeprom(specIndex, names, values, eeprom_count))
    {
        free(names);
        free(values);
        return;
    }

    for (int i = 0; i < eeprom_count; i++)
        eeprom.insert(std::make_pair(string(names[i]), string(values[i])));

    free(names);
    free(values);
}

string bufToString(unsigned char* buf, int len)
{
    std::string s;
    for (int i = 0; i < len; i++)
    {
        char c = (char)buf[i];    
        if (c == 0)
            break;
        else if (isprint(c))
            s += (char)buf[i];
        else
            s += '.';
    }
    return s;
}

// This function writes a demo string to page 4 of the EEPROM ("User Data")
void writeToEEPROM()
{
    const int USER_DATA = 4;

    // read the current page contents
    unsigned char buf[PAGE_SIZE];
    wp_get_eeprom_page(specIndex, USER_DATA, buf, PAGE_SIZE);
    string orig = bufToString(buf, PAGE_SIZE);
    printf("Original contents of EEPROM page %d: [%s]\n", USER_DATA, orig.c_str());

    // generate new page contents 
    string writeString = "Wasatch.VCPP demo at " + timestamp();

    // write the EEPROM page
    printf("Overwriting page contents with: [%s]\n", writeString.c_str());
    strncpy((char*)buf, writeString.c_str(), PAGE_SIZE);
    auto result = wp_write_eeprom_page(specIndex, USER_DATA, buf, PAGE_SIZE);
    if (WP_SUCCESS != result)
    {
        printf("ERROR *** failed to write EEPROM (result %d)\n", result);
        return;
    }
    printf("write successful");
}

void perform_throwaways()
{
    double junk[pixels];
    for (int i = 0; i < throwaways; i++)
    {
        if (WP_SUCCESS == wp_get_spectrum(specIndex, junk, pixels))
        {
            printf("took throwaway %d of %d...\n", i + 1, throwaways);
        }
        else 
        {
            printf("ERROR *** failed to take throwaway %d of %d\n", i + 1, throwaways);
            return;
        }
    }
}

void performRamanReading() 
{
    perform_throwaways();

    double dark[pixels];
    printf("taking dark spectrum\n");
    if (WP_SUCCESS == wp_get_spectrum(specIndex, dark, pixels))
    {
        printf("Obtained dark spectrum\n");
    }
    else 
    {
        printf("ERROR *** failed to obtain dark spectrum\n");
        return;
    }

    pause("Press <return> to enable laser...");
    wp_set_laser_enable(specIndex, 1);

    printf("Waiting %d sec for laser to warm-up\n", LASER_WARMUP_SEC);
    usleep(LASER_WARMUP_SEC * 1000000);

    perform_throwaways();

    double raw[pixels];
    printf("taking raw spectrum\n");
    wp_get_spectrum(specIndex, raw, pixels);

    printf("disabling laser\n");
    wp_set_laser_enable(specIndex, 0);

    double darkCorrected[pixels];
    printf("performing dark correction\n");
    for (int i = 0; i < pixels; i++)
        darkCorrected[i] = raw[i] - dark[i];

    double srmCorrected[pixels];
    memset(srmCorrected, 0, pixels * sizeof(srmCorrected[0]));
    auto result = wp_has_srm_calibration(specIndex);
    if (WP_SUCCESS == result)
    {
        int start = atoi(eeprom["ROIHorizStart"].c_str());
        int end = atoi(eeprom["ROIHorizEnd"].c_str());
        int ROILen = wp_get_cropped_spectrum_length(specIndex);

        printf("found Raman Intensity Calibration from pixels %d to %d (%d total)\n",
            start, end, ROILen);
        if (ROILen != end - start + 1) 
            printf("WARNING: ROILen %d != end %d - start %d\n", ROILen, end, start);

        if (ROILen > 0)
        {
            // we would normally want to cache the factors rather than re-generate 
            // them for each spectrum, but this is just a demo
            double factors[ROILen];
            result = wp_get_raman_intensity_factors(specIndex, factors, ROILen);
            if (WP_SUCCESS == result)
            {
                printf("read %d Raman Intensity Correction factors\n", ROILen);

                printf("Applying Raman Intensity Correction\n");
                memcpy(srmCorrected, darkCorrected, pixels * sizeof(darkCorrected[0]));
                result = wp_apply_raman_intensity_factors(
                    specIndex,      // specIndex
                    srmCorrected,   // spectrum to be corrected in-place
                    pixels,         // spectrum_len
                    factors,        // generated factors
                    ROILen,         // number of factors
                    start,          // start of horizontal ROI
                    end);           // end of horizontal ROI
                if (WP_SUCCESS == result)
                    printf("Raman Intensity Correction successfully applied\n");
                else
                    printf("ERROR *** failed to apply Raman Intensity Correction (result %d)\n", result);
            }
            else
                printf("ERROR *** failed to read Raman Intensity Correction factors (result %d)\n", result);
        }
        else
            printf("ERROR *** ROILen %d\n", ROILen);
    }
    else
        printf("skipping Raman Intensity Correction as no Raman Intensity Calibration found (result %d)\n", result);

    if (tabs)
    {
        printf("\npixel\twavelength\twavenumber\tdark\traw\tdarkCorrected\tsrmCorrected\n");
        for (int i = 0; i < pixels; i++)
            printf("%d\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%.2lf\n", 
                i, wavelengths[i], wavenumbers[i], dark[i], raw[i], darkCorrected[i], srmCorrected[i]);
    }
    else
    {
        printf("\npixel, wavelength, wavenumber,     dark,      raw, darkCorrected, srmCorrected\n");
        for (int i = 0; i < pixels; i++)
            printf("%5d, %10.2lf, %10.2lf, %8.2lf, %8.2lf, %13.2lf, %12.2lf\n", 
                i, wavelengths[i], wavenumbers[i], dark[i], raw[i], darkCorrected[i], srmCorrected[i]);
    }
}

bool init()
{
    char libraryVersion[STR_LEN];
    wp_get_library_version(libraryVersion, STR_LEN);
    printf("Wasatch.VCPP Demo (%s)\n", libraryVersion);
    wp_set_log_level(logLevel);
    const char* logfile = "demo.log";
    wp_set_logfile_path(logfile, strlen(logfile));

    wp_set_driver_delay_us(delay_us);

    int numberOfSpectrometers = wp_open_all_spectrometers();
    printf("%d spectrometers found\n", numberOfSpectrometers);
    if (numberOfSpectrometers < 1)
        return false;

    pixels = wp_get_pixels(specIndex);
    wp_get_serial_number(specIndex, serialNumber, STR_LEN);
    wp_get_model(specIndex, model, STR_LEN);
    loadWavelengths();

    printf("Found %s %s with %d pixels (%.2f, %.2f)\n", model, serialNumber, pixels, 
        wavelengths[0], wavelengths[pixels-1]);
    
    loadEEPROM();

    printf("EEPROM:\n");
    for (map<string, string>::const_iterator i = eeprom.begin(); i != eeprom.end(); i++)
    {
        const string& name = i->first;
        const string& value = i->second;
        printf("  %30s: %s\n", name.c_str(), value.c_str());
    }

    loadWavenumbers();

    if (integrationTimeEdit) {
        printf("Setting integration time to %dms\n", integrationTimeMS);
        wp_set_integration_time_ms(specIndex, integrationTimeMS);
    }

    return true;
}

void demo()
{
    ////////////////////////////////////////////////////////////////////////////
    // read the requested number of spectra (even for Raman mode, do this to warm-up the sensor)
    ////////////////////////////////////////////////////////////////////////////
    for (int i = 0; i < count; i++)
    {
        float spectrum[pixels];
        if (WP_SUCCESS == wp_get_spectrum_float(specIndex, spectrum, pixels))
        {
            auto now = timestamp();
            printf("%s Spectrum %5d of %5d:", now.c_str(), i + 1, count);
            for (int i = 0; i < 10; i++)
                printf(" %.2f%s", spectrum[i], i + 1 < 10 ? "," : " ...\n");
        }
        else
        {
            printf("ERROR: failed getting spectrum\n");
            exit(-1);
        }
    }

    if (ramanModeEnabled)
    {
        performRamanReading();
    }

    if (EEPROMedit) 
        writeToEEPROM();

    ////////////////////////////////////////////////////////////////////////////
    // test result codes in the event of communication failures
    ////////////////////////////////////////////////////////////////////////////

    if (testLaser)
    {
        pause("Press <return> to enable laser...");
        auto result = wp_set_laser_enable(specIndex, 1);
        if (WP_SUCCESS == result)
            printf("successfully enabled laser\n");
        else
            printf("ERROR: unable to enable laser (%d)\n", result);
        auto degC = wp_get_detector_temperature_deg_c(specIndex);
        printf("detector temperature %.2f degC\n", degC);

        printf("\n<<< Disconnect spectrometer to test comms >>>\n");

        pause("Press <return> to disable laser...");
        result = wp_set_laser_enable(specIndex, 0);
        if (WP_SUCCESS == result)
            printf("successfully disabled laser\n");
        else
            printf("ERROR: unable to disable laser (%d)\n", result);
        degC = wp_get_detector_temperature_deg_c(specIndex);
        printf("detector temperature %.2f degC\n", degC);
    }
}

void usage()
{
    printf("Usage: $ demo [--count n] [--integration-time-ms] [--laser] [--raman-mode]\n"
           "              [--log-level DEBUG|INFO|ERROR|NEVER] [--write-eeprom]\n"
           "              [--delay-us delay_microsec] [--throwaways n]\n");
    exit(1);
}

void parseArgs(int argc, char** argv)
{
    for (int i = 1; i < argc; i++)
    {
        printf("current arg is %s\n", argv[i]);
        if (!strcmp(argv[i], "--raman-mode"))
        {
            ramanModeEnabled = true;
        }
        else if (!strcmp(argv[i], "--count"))
        {
            if (i + 1 < argc)
            {
                count = atoi(argv[++i]);
            }
            else
                usage();
        }
        else if (!strcmp(argv[i], "--laser"))
        {
            testLaser = true;
        }
        else if (!strcmp(argv[i], "--integration-time-ms"))
        {
            if (i + 1 < argc) {
                int ms = atoi(argv[++i]);
                integrationTimeMS = ms;
                integrationTimeEdit = true;
            }
            else
                usage();
        }
        else if (!strcmp(argv[i], "--throwaways"))
        {
            if (i + 1 < argc) 
                throwaways = atoi(argv[++i]);
            else
                usage();
        }
        else if (!strcmp(argv[i], "--delay-us"))
        {
            if (i + 1 < argc) 
                delay_us = strtoul(argv[++i], nullptr, 10);
            else
                usage();
        }
        else if (!strcmp(argv[i], "--log-level"))
        {
            if (i + 1 < argc)
            {
                const char* level = argv[++i]; 
                     if (!strcasecmp(level, "DEBUG")) logLevel = WP_LOG_LEVEL_DEBUG;
                else if (!strcasecmp(level, "INFO" )) logLevel = WP_LOG_LEVEL_INFO;
                else if (!strcasecmp(level, "ERROR")) logLevel = WP_LOG_LEVEL_ERROR;
                else if (!strcasecmp(level, "NEVER")) logLevel = WP_LOG_LEVEL_NEVER;
                else usage();
            }
            else
                usage();
        }
        else if (!strcmp(argv[i], "--write-eeprom"))
        {
            EEPROMedit = true;
        }
        else if (!strcmp(argv[i], "--tabs"))
        {
            tabs = true;
        }
        else
        {
            printf("did not match with any of the args\n");
            usage();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// main()
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    parseArgs(argc, argv);
    if (!init())
    {
        wp_destroy_driver();
        return -1;
    }
    demo();

    wp_close_all_spectrometers();
    wp_destroy_driver();
    return 0;
}
