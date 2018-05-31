#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "localStructs.h"

DWORD WINAPI InvadersBomb(LPVOID tParam);
DWORD WINAPI RegPathInvaders(LPVOID tParam);
DWORD WINAPI RandPathInvaders(LPVOID tParam);
DWORD WINAPI ShipInstruction(LPVOID tParam);
DWORD WINAPI BombMovement(LPVOID tParam);
DWORD WINAPI ShotMovement(LPVOID tParam);

int UpdateLocalShip(ClientMoves *move);
int InstantiateGame(GameData *game);
int ShotCollision(GameData *game, ShipShot *shot);
int FullCollision(GameData *game);

int DamageShip(Ship *in);
int ResetShip(Ship *in);
int DamageInvader(Invader *in);
int ResetInvader(Invader *in);
int ResetShot(ShipShot *in);
int ResetBomb(InvaderBomb *in);



#endif /* ALGORITHMS_H */