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

int ShipCollision(GameData *game, Ship *ship);
int ShotCollision(GameData *game, ShipShot *shot);
int InvaderCollision(GameData *game, Invader *invader);
int BombCollision(GameData *game, InvaderBomb *bomb);;

int DamageShip(Ship *in);
int DamageInvader(Invader *in);

int ResetShip(Ship *in);
int ResetShot(ShipShot *in);
int ResetInvader(Invader *in);
int ResetBomb(InvaderBomb *in);

int ValidateInvaderPosition(GameData *game, int x, int y);



#endif /* ALGORITHMS_H */