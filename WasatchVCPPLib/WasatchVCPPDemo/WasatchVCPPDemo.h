#pragma once

#include "resource.h"

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// spectrometer
void doConnect();
void doSetIntegrationTime();
void doAcquire();

// util
void log(const char* format, ...);
