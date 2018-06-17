#ifndef DATA_CLIENT_H
#define DATA_CLIENT_H

#include "data_structs.h"


DWORD WINAPI LaunchClient(ThreadCtrl *cThread);


//void printGame(GameData msg);
int readPipeMsg(HANDLE hPipe, HANDLE readReady, GameData * msg);
int writePipeMsg(HANDLE hPipe, HANDLE writeReady, Packet msg);
DWORD WINAPI ReadGame(LPVOID tParam);
DWORD WINAPI GetKey(LPVOID tParam);
int StartPipeListener(HANDLE *hPipe);
int createProdConsEvents(ThreadCtrl * ps);
int markPlayerReady(ThreadCtrl * ps, Packet token);
Packet handShakeServer(ThreadCtrl * ps);

#endif /* DATA_CLIENT_H */
