#ifndef VIEW_LOGIC_H
#define VIEW_LOGIC_H

#include <windows.h>
#include <tchar.h>
#include <CommCtrl.h>
#include "controller.h"

HINSTANCE hInstance;

ATOM regClass(HINSTANCE hInstance, TCHAR * szAppName);
HWND winCreation(HINSTANCE hInstance, TCHAR * szAppName);

LRESULT CALLBACK winManager(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK winAboutDlg(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK winGameCreateDlg(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

int CloseServerMessageBox(HWND hWnd);

#endif // VIEW_LOGIC_H
