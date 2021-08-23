/** @file   demo-eeprom.cpp
*   @brief  example showing how to extract EEPROM fields (uses C++ Proxy API)
*/

#include <stdio.h>
#include <string>
#include <vector>

#include "WasatchVCPP.h"

using namespace WasatchVCPP::Proxy;

const int specIndex = 0;

//! @note this has been added to WasatchVCPP::Proxy::Spectrometer
std::vector<float> getWavecalCoeffs(Spectrometer* spec)
{
    std::vector<float> v;
    for (int i = 0; i < 5; i++)
        v.push_back(std::stof(spec->eepromFields["wavecalCoeffs[" + std::to_string(i) + "]"]));
    return v;
}

int main(int argc, char** argv)
{
    Driver driver;
    if (0 == driver.openAllSpectrometers())
    {
        printf("No spectrometer found.\n");
        exit(1);
    }

    Spectrometer* spec = driver.getSpectrometer(specIndex);
    printf("Connected to %s %s with %d pixels (%.2f, %.2f)\n",
        spec->model.c_str(), spec->serialNumber.c_str(), spec->pixels, spec->wavelengths[0], spec->wavelengths[spec->pixels - 1]);

    // print wavelength calibration coefficients
    auto coeffs = getWavecalCoeffs(spec);
    printf("Wavelength Coefficients:\n");
    for (int i = 0; i < coeffs.size(); i++)
        printf("  C%d = %le\n", i, coeffs[i]);

    // print all EEPROM fields
    printf("\nFull EEPROM:\n");
    for (auto it = spec->eepromFields.begin(); it != spec->eepromFields.end(); ++it)
        printf("  %30s: %s\n", it->first.c_str(), it->second.c_str());
}
