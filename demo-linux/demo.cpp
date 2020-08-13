#include <stdio.h>
#include <string.h>

#include <vector>
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

int pixels;
char serialNumber[STR_LEN];
char model[STR_LEN];
map<string, string> eeprom;
vector<double> wavelengths;

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

bool init()
{
    char libraryVersion[STR_LEN];
    wp_get_library_version(libraryVersion, STR_LEN);
    printf("Wasatch.VCPP Demo (%s)\n", libraryVersion);

    wp_set_log_level(WP_LOG_LEVEL_DEBUG);
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

    return true;
}

int demo()
{
    const int max = 5;
    for (int i = 0; i < max; i++)
    {
        double spectrum[pixels];
        if (WP_SUCCESS == wp_get_spectrum(specIndex, spectrum, pixels))
        {
            printf("Spectrum %d of %d:", i + 1, max);
            for (int i = 0; i < 10; i++)
                printf(" %.2lf%s", spectrum[i], i + 1 < 10 ? "," : " ...\n");
        }
        else
        {
            printf("ERROR: failed getting spectrum\n");
            break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// main()
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    if (!init())
        return -1;
    demo();
    return 0;
}
