#ifndef VIEW_SERVER_H
#define VIEW_SERVER_H

#include "controller_server.h"

HINSTANCE hInstance;

ATOM regClass(HINSTANCE hInstance, TCHAR * szAppName);
HWND winCreation(HINSTANCE hInstance, TCHAR * szAppName);

LRESULT CALLBACK winManager(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK winAboutDlg(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK winGameCreateDlg(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

#endif // VIEW_SERVER_H
