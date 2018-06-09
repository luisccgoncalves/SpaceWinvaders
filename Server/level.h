#ifndef LEVEL_H
#define LEVEL_H

#include "localStructs.h"

int DefineInvadersType(GameData *game, int *ThreadMustGoOn);
int GiveInvadersHP(GameData *game, int *ThreadMustGoOn);
int PlaceDefenders(GameData *game, int *ThreadMustGoOn);

#endif // LEVEL_H
