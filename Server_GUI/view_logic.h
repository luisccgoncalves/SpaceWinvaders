#ifndef VIEW_LOGIC_H
#define VIEW_LOGIC_H

#include <windows.h>
#include <tchar.h>

ATOM regClass(HINSTANCE hInstance, TCHAR * szAppName);
HWND winCreation(HINSTANCE hInstance, TCHAR * szAppName);
LRESULT CALLBACK winManager(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
int dialog(HINSTANCE hInstance);

#endif // VIEW_LOGIC_H


