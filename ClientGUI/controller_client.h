#ifndef CONTROLLER_CLIENT_H
#define CONTROLLER_CLIENT_H

#include "data_client.h"

int startClient();
int LogPlayer();

int loadTop10(HWND hDlg);
int validateLoginValues(HWND hDlg, BOOL remote);
int centerDialogWnd(HWND hDlg);

#endif	//CONTROLLER_CLIENT_H
