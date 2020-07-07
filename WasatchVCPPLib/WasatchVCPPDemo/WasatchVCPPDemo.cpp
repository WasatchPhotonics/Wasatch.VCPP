#include "framework.h"
#include "WasatchVCPPDemo.h"
#include "Driver.h"
#include "Spectrometer.h"

#include <stdio.h>

#include <string>
#include <vector>

#define MAX_LOADSTRING 100

using std::string;
using std::vector;

////////////////////////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////////////////////////

HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

HWND hTextbox;
string logBuffer;
const int MAX_LOG_LEN = 16 * 1024;

WasatchVCPP::Driver* driver = nullptr;
WasatchVCPP::Spectrometer* spectrometer = nullptr;

////////////////////////////////////////////////////////////////////////////////
// Forward declarations (move to Demo.h)
////////////////////////////////////////////////////////////////////////////////

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// spectrometer
void doConnect();
void doSetIntegrationTime();
void doAcquire();

// util
int __cdecl log(const char* format, ...);
void flushLog();

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

    log("getting WasatchVCPP::Driver instance");
    driver = WasatchVCPP::Driver::getInstance();
    driver->setLogBuffer(logBuffer);

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WASATCHVCPPDEMO));

    log("entering message loop");
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    log("exited message loop");

    return (int) msg.wParam;
}

////////////////////////////////////////////////////////////////////////////////
// WinForms Implementation
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
                switch (wmId)
                {
                    case IDM_ABOUT: DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About); break;
                    case IDM_EXIT: DestroyWindow(hWnd); break;
                    case ID_SPECTROMETER_CONNECT: doConnect(); break;
                    case ID_SPECTROMETER_SETINTEGRATIONTIME: doSetIntegrationTime(); break;
                    case ID_SPECTROMETER_ACQUIRE: doAcquire(); break;
                    default: return DefWindowProc(hWnd, message, wParam, lParam);
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

// Message handler for about box
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

////////////////////////////////////////////////////////////////////////////////
// Spectrometer Callbacks
////////////////////////////////////////////////////////////////////////////////

void doConnect() 
{ 
    int count = driver->openAllSpectrometers();
    if (count > 0)
    {
        log("found %d connected spectrometers", count);
        spectrometer = driver->getSpectrometer(0);
    }
    else
    {
        log("no spectrometers found");
        spectrometer = nullptr;
    }
}

void doSetIntegrationTime()
{
    if (spectrometer == nullptr)
        return;

    spectrometer->setIntegrationTimeMS(100);
    flushLog();
}

void doAcquire()
{
    if (spectrometer == nullptr)
        return;

    vector<double> spectrum = spectrometer->getSpectrum();
    if (spectrum.size() < 5)
    {
        log("doAcquire: ERROR: failed to read spectrum");
        return;
    }

    log("doAcquire: read spectrum of %d pixels: %.2f, %.2f, %.2f, %.2f, %.2f ...",
        spectrum.size(), spectrum[0], spectrum[1], spectrum[2], spectrum[3], spectrum[4]);
}

////////////////////////////////////////////////////////////////////////////////
// Utilities
////////////////////////////////////////////////////////////////////////////////

// https://stackoverflow.com/a/30887925/11615696
int __cdecl log(const char *format, ...)
{
    char str[1024];
    va_list argptr;
    va_start(argptr, format);
    int len = vsnprintf(str, sizeof(str), format, argptr);
    va_end(argptr);

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

    flushLog();

    return len;
}

void flushLog()
{
    SetWindowTextA(hTextbox, logBuffer.c_str());
}
