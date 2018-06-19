#ifndef CONTROLLER_CLIENT_H
#define CONTROLLER_CLIENT_H

#include "data_client.h"

int startClient();
int LogPlayer(HWND hDlg, BOOL remote);

int loadTop10(HWND hDlg);
int validateLoginValues(HWND hDlg, BOOL remote);
int centerDialogWnd(HWND hDlg);

int ConnectGame();
int PlayerLogged();
int PlayerIsGameBound();

DWORD WINAPI UpdateView(LPVOID tParam);

int paintMap(HDC hdc);
int SendKey(int num);
int GetScore();
int GetLives();

#endif	//CONTROLLER_CLIENT_H
