#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <map>

#include "WasatchVCPP.h"

using std::vector;
using std::string;
using std::map;

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

const int specIndex = 0;
const int STR_LEN = 33;

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
<<<<<<< Updated upstream
vector<float> wavelengths;

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
    char buf[100] = {0};
    std::strftime(buf, sizeof(buf), "%F %T", std::localtime(&now));
    return buf;
}
=======
vector<double> wavelengths;
bool ramanModeEnabled = false;
int measurement_integration_time;
>>>>>>> Stashed changes

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

void loadEEPROM()
{
    int count = wp_get_eeprom_field_count(specIndex);
    if (count <= 0)
        return;

    const char** names  = (const char**)malloc(count * sizeof(const char*));
    const char** values = (const char**)malloc(count * sizeof(const char*));

    if (WP_SUCCESS != wp_get_eeprom(specIndex, names, values, count))
    {
        free(names);
        free(values);
        return;
    }

    for (int i = 0; i < count; i++)
        eeprom.insert(std::make_pair(string(names[i]), string(values[i])));

    free(names);
    free(values);
}

<<<<<<< Updated upstream
=======
int writeToEEPROM()
{
    unsigned char buf[64];
    string writeString;
    wp_get_eeprom_page(specIndex, 4, buf, 64);
    printf("The value at page 4 is %s\n", buf);
    std::stringstream s;
    s << "Test string. ";
    s << time(NULL);
    writeString = s.str();
    strcpy((char*)buf, writeString.c_str());
    printf("wrote string to buffer, buffer is now %s\n", buf);
    int i = wp_write_eeprom_page(specIndex, 4, buf, 64);
    strcpy((char*)buf, "");
    printf("wrote the page to the eeprom and buffer now has value [%s]\n", buf);
    wp_get_eeprom_page(specIndex, 4, buf, 64);
    printf("The value at page 4 after edit is %s and the write operation was %d\n", buf, i);
    return 0;
}

void performRamanReading() 
{
	double dark_spectrum[pixels];
	double corrected_spectrum[pixels];
	double initial_spectrum[pixels];
	int i;
    wp_set_integration_time_ms(specIndex,measurement_integration_time);
	string stringSpec;
	if (WP_SUCCESS == wp_get_spectrum(specIndex, dark_spectrum, pixels))
	{
		printf("Obtained a dark spectra\n");
	}
	else {
		printf("Did not obtain a dark spectra\n");
	}
	printf("Took dark spectrum.\n\n");
	printf("About to enable laser\n");
	wp_set_laser_enable(specIndex, 1);
	usleep(10000000);
	usleep(1000);
	wp_get_spectrum(specIndex, initial_spectrum, pixels);
	wp_set_laser_enable(specIndex, 0);
	usleep(1000);
	for (i = 0; i < pixels; i++)
	{
		initial_spectrum[i] = initial_spectrum[i] - dark_spectrum[i];
        corrected_spectrum[i] = initial_spectrum[i];
	}
	if (wp_has_srm_calibration(specIndex))
	{
		printf("srm calibration present\n");
		int ROILen;
		ROILen = wp_get_vignetted_spectrum_length(specIndex);
		double factors[ROILen];
		wp_get_raman_intensity_factors(specIndex, factors, ROILen);
		wp_apply_raman_intensity_factors(specIndex, corrected_spectrum, pixels, factors, ROILen, 0, pixels);
	}
	printf("Took corrected_spectrum\n");
	printf("dark,initial,corrected\n");
	for (i = 0; i < pixels; i++)
	{
		printf("%.2lf,%.2lf,%.2lf\n ", dark_spectrum[i],initial_spectrum[i],corrected_spectrum[i]);
	}
	printf(" ... \n");
}

>>>>>>> Stashed changes
bool init()
{
    char libraryVersion[STR_LEN];
    wp_get_library_version(libraryVersion, STR_LEN);
    printf("Wasatch.VCPP Demo (%s)\n", libraryVersion);
    wp_set_integration_time_ms(specIndex,measurement_integration_time);
    wp_set_log_level(logLevel);
    const char* logfile = "demo.log";
    wp_set_logfile_path(logfile, strlen(logfile));

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

    wp_set_integration_time_ms(specIndex, integrationTimeMS);

    return true;
}

void demo()
{
    ////////////////////////////////////////////////////////////////////////////
    // read the requested number of spectra
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
    printf("Usage: $ demo-linux [--integration-time-ms n] [--count n] [--laser] [--log-level DEBUG|INFO|ERROR|NEVER]\n");
    exit(1);
}

void parseArgs(int argc, char** argv)
{
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "--count"))
        {
            if (i + 1 < argc)
<<<<<<< Updated upstream
                count = atoi(argv[++i]); 
=======
            {
                int ms = atoi(argv[++i]);
                measurement_integration_time = ms;
                
            }
>>>>>>> Stashed changes
            else
                usage();
        }
        else if (!strcmp(argv[i], "--laser"))
        {
            testLaser = true;
        }
        else if (!strcmp(argv[i], "--integration-time-ms"))
        {
            if (i + 1 < argc)
                integrationTimeMS = atoi(argv[++i]); 
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
        else
        {
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
