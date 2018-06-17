#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "localStructs.h"

DWORD WINAPI PowerUpTimer(LPVOID tParam);
DWORD WINAPI PowerUps(LPVOID tParam);
DWORD WINAPI RegPathInvaders(LPVOID tParam);
DWORD WINAPI RandPathInvaders(LPVOID tParam);
DWORD WINAPI PacketListener(LPVOID tParam);
DWORD WINAPI BombMovement(LPVOID tParam);
DWORD WINAPI ShotMovement(LPVOID tParam);

int markPlayerReady(ClientMoves *ps);
int handShakeClient(ClientMoves *ps);
void PowerUpShip(GameData *game, Ship *ship, PowerUp *pUp, HANDLE mutex);
PowerUp GeneratePowerUp(int x_max, int duration);
int DefenderShipsCollision(GameData *game, int x, int y, int index);
int UpdateLocalShip(ClientMoves *move);
void GetDrunk(ClientMoves *moves);
int InstantiateGame(GameData *game);

int ShipCollision(GameData *game, Ship *ship, HANDLE mhStructSync);
int ShotCollision(GameData *game, ShipShot *shot);
int InvaderCollision(GameData *game, Invader *invader);
int BombCollision(GameData *game, InvaderBomb *bomb);;

int PowerUpCollision(GameData * game, PowerUp *pUp, HANDLE mhStructSync);
int ShipPowerUpCollision(GameData * game, Ship * ship, PowerUp *pUp, HANDLE mhStructSync);
int DamageShip(Ship *in);
int DamageInvader(Invader *in);

int ResetShip(Ship *in);
int ResetShot(ShipShot *in);
int ResetInvader(Invader *in);
int ResetBomb(InvaderBomb *in);

int ValidateInvaderPosition(GameData *game, int x, int y, int index);
int GetRegularLargerXPosition(GameData *game);
int UpdateCoords(GameData * game, int *y);
int BombLauncher(BombMoves *bombMoves);

#endif /* ALGORITHMS_H */
