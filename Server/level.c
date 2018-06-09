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

int PlaceDefenders(GameData *game, int *ThreadMustGoOn) { //Places defenders relatively to play area size
	int i;
	int slot = game->xsize / game->num_players;

	if (*ThreadMustGoOn) {
		for (i = 0; i < game->num_players; i++) {

			game->ship[i].x = (slot*i) + (slot/2);
			game->ship[i].y = game->ysize-1;
		}

		return 1;
	}
	return 0;
}

int OriginalPosition(GameData *game, int *ThreadMustGoOn) {
	int i;
	int regularsByRow = ((game->xsize / 4)); //Half of screen, half of positions on screen
	int randomByRow	= ((game->xsize / 4));
	int randInvaders = (game->max_invaders - game->max_rand_invaders);
	
	if (*ThreadMustGoOn) {
		for (i = 0; i < game->max_invaders; i++) {
			if (!(game->invad[i].rand_path)) {			//If regular path

				game->invad[i].x = game->invad[i].x_init = (i % regularsByRow) * 2;
				game->invad[i].y = game->invad[i].y_init = (i / regularsByRow) * 2;
			}
			else {
				game->invad[i].x = game->invad[i].x_init = (game->xsize / 2) + (i % randomByRow) * 4;
				game->invad[i].y = game->invad[i].y_init = ((i-randInvaders) / randomByRow) * 2;
				game->invad[i].direction = -1;

				//do {
				//	game->invad[i].x = game->invad[i].x_init = RandomValue((game->xsize / 2)) + (game->xsize / 3);
				//	game->invad[i].y = game->invad[i].y_init = RandomValue(game->ysize);
				//	game->invad[i].direction = RandomValue(3);
				//} while (ValidateInvaderPosition(game, game->invad[i].x, game->invad[i].y, i));
			}
		}
		return 1;
	}
	return 0;
}