#ifndef VIEW_CLIENT_H
#define VIEW_CLIENT_H

#include "controller_client.h"

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

int paintMap(HDC hdc);

#endif // VIEW_CLIENT_H