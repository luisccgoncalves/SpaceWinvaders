#ifndef SERVER_DATA_H
#define SERVER_DATA_H

#include "score.h"
#include "algorithms.h"
#include "level.h"

DWORD WINAPI LaunchServer(SMCtrl *cThread);
DWORD WINAPI StartLobby(LPVOID tParam);
DWORD WINAPI StartGame(LPVOID tParam);
DWORD WINAPI GameTick(LPVOID tParam);

int loadShips(Ship *ship, Player *player);

#endif /* SERVER_DATA_H */
