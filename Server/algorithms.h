#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "localStructs.h"

DWORD WINAPI PowerUpTimer(LPVOID tParam);
DWORD WINAPI PowerUps(LPVOID tParam);
DWORD WINAPI InvadersBomb(LPVOID tParam);
DWORD WINAPI RegPathInvaders(LPVOID tParam);
DWORD WINAPI RandPathInvaders(LPVOID tParam);
DWORD WINAPI ShipInstruction(LPVOID tParam);
DWORD WINAPI BombMovement(LPVOID tParam);
DWORD WINAPI ShotMovement(LPVOID tParam);

void PowerUpShip(Ship *ship, PowerUp *pUp, HANDLE mutex);
PowerUp GeneratePowerUp(int x_max, int duration);
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

int ValidateInvaderPosition(GameData *game, int x, int y, int index);
int GetRegularLargerXPosition(GameData *game);
int UpdateCoords(GameData * game, int *y);
int UpdateInvaderBombRate(int bombRate, Invader *invader);

#endif /* ALGORITHMS_H */