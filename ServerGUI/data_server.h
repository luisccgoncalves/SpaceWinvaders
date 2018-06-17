#ifndef DATA_SERVER_H
#define DATA_SERVER_H

#include "data_score.h"
#include "data_algorithms.h"
#include "data_level.h"

DWORD WINAPI LaunchServer(SMCtrl *cThread);
DWORD WINAPI StartGame(LPVOID tParam);
DWORD WINAPI GameTick(LPVOID tParam);

int StartLobby(SMCtrl * cThread);
int loadShips(Ship *ship, Player *player);

int updateConfigGameValues(ConfigurableVars vars, GameData *game, HANDLE mhStructSync);

#endif /* DATA_SERVER_H */
