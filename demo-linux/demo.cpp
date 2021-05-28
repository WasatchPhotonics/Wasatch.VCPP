#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <vector>
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

////////////////////////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////////////////////////

int logLevel = WP_LOG_LEVEL_DEBUG;
int count = 5;
int pixels;
char serialNumber[STR_LEN];
char model[STR_LEN];
map<string, string> eeprom;
vector<double> wavelengths;
bool ramanModeEnabled = false;

////////////////////////////////////////////////////////////////////////////////
// Functional Implementation
////////////////////////////////////////////////////////////////////////////////

void loadWavelengths()
{
    wavelengths.clear();
    double values[pixels];
    if (WP_SUCCESS == wp_get_wavelengths(specIndex, values, pixels))
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

bool init()
{
    char libraryVersion[STR_LEN];
    wp_get_library_version(libraryVersion, STR_LEN);
    printf("Wasatch.VCPP Demo (%s)\n", libraryVersion);

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

    printf("Found %s %s with %d pixels (%.2lf, %.2lf)\n", model, serialNumber, pixels, 
        wavelengths[0], wavelengths[pixels-1]);
    
    loadEEPROM();

    printf("EEPROM:\n");
    for (map<string, string>::const_iterator i = eeprom.begin(); i != eeprom.end(); i++)
    {
        const string& name = i->first;
        const string& value = i->second;
        printf("  %30s: %s\n", name.c_str(), value.c_str());
    }

    if (ramanModeEnabled)
    {

    }

    return true;
}

void ramanOperation() 
{
	double dark_spectrum[pixels];
	double corrected_spectrum[pixels];
	double initial_spectrum[pixels];
	int i;
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
	usleep(3000000);
	wp_set_laser_enable(specIndex, 1);
	usleep(1000);
	wp_get_spectrum(specIndex, initial_spectrum, pixels);
	wp_set_laser_enable(specIndex, 0);
	usleep(1000);
	for (i = 0; i < pixels; i++)
	{
		corrected_spectrum[i] = -1*(initial_spectrum[i] - dark_spectrum[i]);
	}
	if (wp_has_srm_calibration(specIndex))
	{
		printf("srm calibration present");
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

void demo()
{
    for (int i = 0; i < count; i++)
    {
        double spectrum[pixels];
        if (WP_SUCCESS == wp_get_spectrum(specIndex, spectrum, pixels))
        {
            printf("Spectrum %3d of %3d:", i + 1, count);
            for (int i = 0; i < 10; i++)
                printf(" %.2lf%s", spectrum[i], i + 1 < 10 ? "," : " ...\n");
        }
        else
        {
            printf("ERROR: failed getting spectrum\n");
            exit(-1);
        }
    }

    writeToEEPROM();
}

void usage()
{
    printf("Usage: $ demo-linux [--count n] [--log-level DEBUG|INFO|ERROR|NEVER] [--integration-time-ms n] [--raman-mode]\n");
    exit(1);
}

void parseArgs(int argc, char** argv)
{
    for (int i = 1; i < argc; i++)
    {
        printf("current arg is %s\n", argv[i]);
        if (!strcmp(argv[i], "--integration-time-ms")) 
        {
            if (i + 1 < argc)
            {
                int ms = atoi(argv[++i]);
                wp_set_integration_time_ms(specIndex,ms);
            }
            else
            {
                printf("not enough args according to integration time\n");
                usage();
            }
        }
        else if (!strcmp(argv[i], "--raman-mode"))
        {
            ramanModeEnabled = true;
        }
        else if (!strcmp(argv[i], "--count"))
        {
            if (i + 1 < argc)
                count = atoi(argv[++i]); 
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
        return -1;
    demo();

    wp_close_all_spectrometers();
    return 0;
}
