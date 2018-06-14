#ifndef VIEWLOGIC_H
#define VIEWLOGIC_H

#include <windows.h>
#include <tchar.h>

ATOM regClass(HINSTANCE hInstance, TCHAR * szAppName);
HWND winCreation(HINSTANCE hInstance, TCHAR * szAppName);
LRESULT CALLBACK winManager(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

#endif // VIEWLOGIC_H

