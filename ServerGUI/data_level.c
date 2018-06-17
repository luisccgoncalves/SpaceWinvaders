#include "data_level.h"

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

int PlaceDefenders(GameData *game, int *ThreadMustGoOn) { //Places defenders relatively to play area size
	int i;
	int slot = game->xsize / game->num_players;

	if (*ThreadMustGoOn) {
		for (i = 0; i < game->num_players; i++) {

			game->ship[i].x = (slot*i) + (slot / 2);
			game->ship[i].y = game->ysize - 1;
		}

		return 1;
	}
	return 0;
}

int OriginalPosition(GameData *game, int *ThreadMustGoOn) {
	int i;
	int regularsByRow = ((game->xsize / 4)); //Half of screen, half of positions on screen
	int regInvaders = (game->max_invaders - game->max_rand_invaders);
	int randomByRow = ((game->xsize / 8));
	int randInvaders = (game->max_invaders - regInvaders);

	if (*ThreadMustGoOn) {
		for (i = 0; i < regInvaders; i++) {

			game->invad[i].x = game->invad[i].x_init = (i % regularsByRow) * 2;
			game->invad[i].y = game->invad[i].y_init = (i / regularsByRow) * 2;

		}

		for (i = 0; i < randInvaders; i++) {

			game->invad[i + regInvaders].x = (game->xsize / 2) + (i % randomByRow) * 4;
			game->invad[i + regInvaders].x_init = game->invad[i + regInvaders].x;

			game->invad[i + regInvaders].y = ((i / randomByRow) * 4) + 2;
			game->invad[i + regInvaders].y_init = game->invad[i + regInvaders].y;

			game->invad[i + regInvaders].direction = RandomValue(3);

		}
		return 1;
	}
	return 0;
}