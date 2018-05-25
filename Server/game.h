#ifndef GAME_H
#define GAME_H

#include "algorithms.h"
//#include "localStructs.h"

DWORD WINAPI StartGame(LPVOID tParam);
DWORD WINAPI GameTick(LPVOID tParam);
int UpdateLocalShip(GameData *game, Packet *localpacket);

#endif // GAME_H