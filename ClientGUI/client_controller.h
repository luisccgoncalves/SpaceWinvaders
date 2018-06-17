#ifndef CLIENT_CONTROLLER_H
#define CLIENT_CONTROLLER_H

#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include "resource.h"
#include "..\DLL\dll.h"

int loadTop10(HWND hDlg);
int validateLoginValues(HWND hDlg);

#endif	//CLIENT_CONTROLLER_H
