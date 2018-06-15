#ifndef GAME_H
#define GAME_H

#include "algorithms.h"
#include "level.h"
//#include "localStructs.h"

DWORD WINAPI StartLobby(LPVOID tParam);
DWORD WINAPI StartGame(LPVOID tParam);
DWORD WINAPI GameTick(LPVOID tParam);


#endif // GAME_H