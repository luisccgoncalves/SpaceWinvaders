#ifndef CONTROLLER_SERVER_H
#define CONTROLLER_SERVER_H

//#include <windows.h>
//#include <tchar.h>
#include "resource.h"
#include "data_server.h"

int startServer();
int startGameProcedure();
int getGDataGameConfigured();
int gameRunning();

int setCreateGameDlgValues(HWND hDlg);
int validateCreateGameDlgValues(HWND hDlg);
int centerDialogWnd(HWND hDlg);

int sendCreateGameValuesToServer(HWND hDlg);
int getPlayersReady();

#endif // CONTROLLER_SERVER_H