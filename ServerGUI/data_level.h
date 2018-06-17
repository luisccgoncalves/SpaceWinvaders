#ifndef DATA_LEVEL_H
#define DATA_LEVEL_H

#include "data_structs.h"

int DefineInvadersType(GameData *game, int *ThreadMustGoOn);
int GiveInvadersHP(GameData *game, int *ThreadMustGoOn);
int PlaceDefenders(GameData *game, int *ThreadMustGoOn);
int OriginalPosition(GameData *game, int *ThreadMustGoOn);

#endif // DATA_LEVEL_H

