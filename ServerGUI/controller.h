#ifndef CONTROLLER_H
#define CONTROLLER_H

//#include <windows.h>
//#include <tchar.h>
#include "resource.h"
#include "data_server.h"

int startServer();

int setCreateGameDlgValues(HWND hDlg);
int validateCreateGameDlgValues(HWND hDlg);
int centerDialogWnd(HWND hDlg);

int sendCreateGameValuesToServer(HWND hDlg);

#endif // CONTROLLER_H