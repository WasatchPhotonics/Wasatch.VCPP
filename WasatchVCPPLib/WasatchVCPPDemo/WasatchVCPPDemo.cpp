/**
    @file   WasatchVCPPDemo.cpp
    @author Mark Zieg <mzieg@wasatchphotonics.com>

    This file demonstrates how the WasatchVCPPProxy class can be used to control
    Wasatch Photonics spectrometers from Visual C++.

    This is a simple GUI created from a Visual C++ "Desktop Application" template.
    It has a few menu-bar options (configured using the "Resource View"), and a 
    scrolling log event viewer.  It doesn't provide an on-screen chart because
    I haven't figured out a lightweight way to do that from Visual C++ without
    adding a dependency on Qt or equivalent.  A graphing demo is provided via 
    the standalone WasatchVCPPNet C# GUI.

    @todo add some kind of charting to graph spectra

    @see README.md
*/
#include "framework.h"
#include "InputBox.h"
#include "WasatchVCPPDemo.h"

#include "WasatchVCPP.h"

#include <stdio.h>

#include <fstream>
#include <string>
#include <vector>
#include <map>

#define MAX_LOADSTRING 100
#define LOGFILE_PATH "wasatch_vcpp.log"

using std::string;
using std::vector;
using std::map;

////////////////////////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////////////////////////

HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// for the event log
HWND hTextbox;
string logBuffer;
const int MAX_LOG_LEN = 16 * 1024; 

WasatchVCPP::Proxy::Driver driver; // example
WasatchVCPP::Proxy::Spectrometer* spectrometer = nullptr; // example
std::ofstream outfile;

int maxSpectra = 1;

////////////////////////////////////////////////////////////////////////////////
// Utilities
////////////////////////////////////////////////////////////////////////////////

//! A simple log function which updates the scrolling Textbox on the GUI.
//! @see https://stackoverflow.com/a/30887925/11615696
void log(const char *fmt, ...)
{
    char str[1024];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(str, sizeof(str), fmt, args);
    va_end(args);

    // output the string to the Visual Studio output window
    OutputDebugStringA(str);
    OutputDebugStringA("\r\n");

    // if the string would overfill the on-screen event log buffer, toss the first half of the buffer
    if (logBuffer.size() + 2 > MAX_LOG_LEN)
    {
        int bytesToDelete = sizeof(logBuffer) / 2;
        logBuffer.assign(logBuffer.substr(bytesToDelete, logBuffer.size() - bytesToDelete));
    }

    // append the new message to the (possibly shrunken) buffer, followed by a linefeed
    if (logBuffer.size() + len + 2 < MAX_LOG_LEN)
    {
        logBuffer.append(str);
        logBuffer.append("\r\n");
    }

    // update the Textbox
    SetWindowTextA(hTextbox, logBuffer.c_str());
}

////////////////////////////////////////////////////////////////////////////////
// Spectrometer Callbacks (from the "Spectrometer" menu)
////////////////////////////////////////////////////////////////////////////////

void doConnect() 
{ 
    log("searching for spectrometers");
    int count = driver.openAllSpectrometers(); // example
    if (count <= 0)
    {
        log("no spectrometers found");
        spectrometer = nullptr;
        return;
    }
    log("found %d connected spectrometers", count);

    spectrometer = driver.getSpectrometer(0); // example
    if (spectrometer->wavelengths.empty())
        log("connected to %s %s with %d pixels (no wavecal)",
            spectrometer->model.c_str(),
            spectrometer->serialNumber.c_str(),
            spectrometer->pixels);
    else
        log("connected to %s %s with %d pixels (%.2f, %.2fnm)",
            spectrometer->model.c_str(),
            spectrometer->serialNumber.c_str(),
            spectrometer->pixels,
            spectrometer->wavelengths[0],
            spectrometer->wavelengths[((int)spectrometer->pixels) - 1]);
}

void doSetIntegrationTime()
{
    if (spectrometer == nullptr)
        return;
    int ms = atoi(InputBox("Enter integration time (ms)").c_str());
    spectrometer->setIntegrationTimeMS(ms); // example
    log("integration time -> %dms", ms);
}

void doSetMaxSpectra()
{
    maxSpectra = atoi(InputBox("Enter max spectra").c_str());
    log("maxSpectra -> %d", maxSpectra);
}

void doSetLaserEnable()
{
    if (spectrometer == nullptr)
        return;
    string response = InputBox("Fire laser (Y/N)");
    bool enabled = response.size() > 0 && (response[0] == 'y' || response[0] == 'Y');
    spectrometer->setLaserEnable(enabled); // example
    log("laser -> %s", enabled ? "on" : "off");
}

void doSetLaserPower()
{
    if (spectrometer == nullptr)
        return;
    spectrometer->setLaserPowerPercImmediate(50.0);
}

void doFullLaserPower()
{
    if (spectrometer == nullptr)
        return;
    //spectrometer->setLaserPowerPercImmediate(50.0);
}


void doGetDetectorTemperatureDegC()
{
    if (spectrometer == nullptr)
        return;
    auto degC = spectrometer->getDetectorTemperatureDegC(); // example
    log("detector temperature -> %.2f degC", degC);
}

void doReadEEPROM()
{
    if (spectrometer == nullptr)
        return;

    log("EEPROM fields:");
    for (map<string, string>::const_iterator i = spectrometer->eepromFields.begin();
        i != spectrometer->eepromFields.end(); i++)
        log("  %s = %s", i->first.c_str(), i->second.c_str());
}

void doAcquire()
{
    if (spectrometer == nullptr)
        return;

    for (int i = 0; i < maxSpectra; i++)
    {
        vector<double> spectrum = spectrometer->getSpectrum(); // example
        if (spectrum.empty())
        {
            log("doAcquire: ERROR: failed to read spectrum");
            return;
        }

        if (maxSpectra < 100 || ((i + 1) % 10 == 0))
        {
            log("doAcquire: read spectrum %d of %d: %u pixels: %.2f, %.2f, %.2f, %.2f, %.2f ...",
                i + 1, maxSpectra, spectrum.size(), spectrum[0], spectrum[1], spectrum[2], spectrum[3], spectrum[4]);
            // @todo redraw window on long loops
        }

        // if we've opened an outfile, append the spectrum (row-ordered)
        if (outfile.is_open())
        {
            for (auto y : spectrum)
                outfile << y << ", ";
            outfile << "\n";
        }
    }
}

void doSetOutfile()
{
    string filename = InputBox("Enter filename", "Set Outfile", "wasatch_vcpp.csv");
    outfile.open(filename);
    log("Spectra will be appended to %s", filename.c_str());

    // if we've already connected to a spectrometer, initialize the file by writing the wavelength axis
    if (spectrometer != nullptr)
    {
        for (auto nm : spectrometer->wavelengths)
            outfile << nm << ", ";
        outfile << "\n";
    }
}

////////////////////////////////////////////////////////////////////////////////
// main()
////////////////////////////////////////////////////////////////////////////////

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR    lpCmdLine,
                      _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WASATCHVCPPDEMO, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
        return FALSE;

    log("driver logging to %s", LOGFILE_PATH);
    driver.setLogfile(LOGFILE_PATH); // example

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WASATCHVCPPDEMO));

    // this is the GUI "event loop" where the UI thread will spin while
    // processing GUI events (menu selections etc) in background threads
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    driver.destroy();

    // after exiting event loop, application will terminate
    return (int) msg.wParam;
}

////////////////////////////////////////////////////////////////////////////////
// GUI Implementation
////////////////////////////////////////////////////////////////////////////////

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WASATCHVCPPDEMO));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WASATCHVCPPDEMO);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

// create and display the main program window
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
      return FALSE;

   // https://stackoverflow.com/a/15447035/11615696
   hTextbox = CreateWindowExW(NULL, L"Edit", L"EventLog", 
       WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
       10, 10, 600, 400, hWnd, nullptr, hInst, nullptr);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   log("WasatchVCPPDemo starting");

   return TRUE;
}

//  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_COMMAND:
            {
                int wmId = LOWORD(wParam);
                log("calling message with id %d", wmId);
                switch (wmId)
                {
                    case IDM_ABOUT: 
                        DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About); 
                        break;
                    case IDM_EXIT: 
                        DestroyWindow(hWnd); 
                        break;

                    // add new menu options here
                    // (physically add them to the menu by double-clicking 
                    //  WasatchVCPPDemo.rc -> Menu -> IDC_WASATCHVCPPDEMO)
                    case ID_SPECTROMETER_CONNECT: 
                        doConnect(); 
                        break;
                    case ID_SPECTROMETER_SETINTEGRATIONTIME: 
                        doSetIntegrationTime(); 
                        break;
                    case ID_SPECTROMETER_ACQUIRE: 
                        doAcquire(); 
                        break;
                    case ID_SPECTROMETER_READEEPROM:
                        doReadEEPROM();
                        break;
                    case ID_SPECTROMETER_SETLASERENABLE:
                        doSetLaserEnable();
                        break;
                    case ID_SPECTROMETER_GETDETECTORTEMPERATUREDEGC:
                        doGetDetectorTemperatureDegC();
                        break;
                    case ID_SPECTROMETER_SETOUTFILE:
                        doSetOutfile();
                        break;
                    case ID_SPECTROMETER_MAXSPECTRA:
                        doSetMaxSpectra();
                        break;
                    case ID_SPECTROMETER_SETLASERPOWER:
                        doSetLaserPower();
                        break;
                    case ID_SPECTROMETER_LASERFULLPOWER:
                        doFullLaserPower();
                        break;
                    default: 
                        return DefWindowProc(hWnd, message, wParam, lParam);
                }
            }
            break;
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hWnd, &ps);
                // Add any drawing code that uses hdc here...
                EndPaint(hWnd, &ps);
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for About box
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        case WM_INITDIALOG: return (INT_PTR)TRUE;
        case WM_COMMAND:
            // whether the user clicked Ok or Cancel, just close the dialog
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
}

