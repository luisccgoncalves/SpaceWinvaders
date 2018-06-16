#ifndef CLIENT_VIEW_LOGIC_H
#define CLIENT_VIEW_LOGIC_H

#include "client_controller.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Login(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Logout(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    HighScores(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ConfigureKeys(HWND, UINT, WPARAM, LPARAM);

#endif // CLIENT_VIEW_LOGIC_H