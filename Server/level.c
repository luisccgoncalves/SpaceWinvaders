#include "level.h"

int DefineInvadersType(GameData *game, int *ThreadMustGoOn) { //Gives every invader a Path flag according to setup values
	int i;
	if (*ThreadMustGoOn) {
		for (i = 0; i < game->max_invaders; i++) {
			if (i < (game->max_invaders - game->max_rand_invaders))
				game->invad[i].rand_path = 0;
			else
				game->invad[i].rand_path = 1;
		}
		return 1;
	}
	return 0;
}

int GiveInvadersHP(GameData *game, int *ThreadMustGoOn) { //Gives every invader their relative HP
	int i;
	if (*ThreadMustGoOn) {
		for (i = 0; i < game->max_invaders; i++) {
			if (game->invad[i].rand_path == 0)
				game->invad[i].hp = 1;
			if (game->invad[i].rand_path == 1)
				game->invad[i].hp = 3;					//Hardcoded with value set in aasignement page 4/8 "esquiva" table
		}
		return 1;
	}
	return 0;
}