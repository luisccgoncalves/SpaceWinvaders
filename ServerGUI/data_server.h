#ifndef DATA_SERVER_H
#define DATA_SERVER_H

#include "data_score.h"
#include "data_algorithms.h"
#include "data_level.h"

DWORD WINAPI LaunchServer(SMCtrl *cThread);
DWORD WINAPI StartLobby(LPVOID tParam);
DWORD WINAPI StartGame(LPVOID tParam);
DWORD WINAPI GameTick(LPVOID tParam);

int loadShips(Ship *ship, Player *player);

#endif /* DATA_SERVER_H */
