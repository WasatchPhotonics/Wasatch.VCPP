#include <stdio.h>
#include <string.h>

#include "WasatchVCPP.h"

const int specIndex = 0;
const int STR_LEN = 33;

bool init()
{
    char libraryVersion[STR_LEN];
    wp_get_library_version(libraryVersion, STR_LEN);
    printf("Wasatch.VCPP Demo (%s)\n", libraryVersion);

    printf("configuring logging\n");
    wp_set_log_level(WP_LOG_LEVEL_DEBUG);
    const char* logfile = "demo.log";
    wp_set_logfile_path(logfile, strlen(logfile));

    printf("calling wp_open_all_spectrometers\n");
    int numberOfSpectrometers = wp_open_all_spectrometers();
    printf("%d spectrometers found\n", numberOfSpectrometers);
    if (numberOfSpectrometers < 1)
        return false;

    return true;
}

void dumpSettings()
{
    char serialNumber[STR_LEN];
    char model[STR_LEN];

    int pixels = wp_get_pixels(specIndex);
    wp_get_serial_number(specIndex, serialNumber, STR_LEN);
    wp_get_model(specIndex, model, STR_LEN);
    printf("Found %s %s with %d pixels\n", model, serialNumber, pixels);
}

int demo()
{
    dumpSettings();
}

int main(int argc, char** argv)
{
    if (!init())
        return -1;

    demo();

    return 0;
}
