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

//int DefenderShipsCollision(GameData *game, int x, int y, int index) {
//	int i;
//
//	for (i = 0; i < game->num_players; i++) {
//		if (i != index) {
//			if (game->ship[i].lives >= 0 && game->ship[i].x == x && game->ship[i].y == y) {
//				return 0;
//			}
//		}
//	}
//	return 1;
//}
//
//int UpdateLocalShip(ClientMoves *move) {
//
//	DWORD			tShotLauncherID;
//	HANDLE			htShotLauncher;
//
//	int xTemp, yTemp;
//	int index = move->localPacket.owner;
//
//	xTemp = move->game->ship[index].x;
//	yTemp = move->game->ship[index].y;
//
//	//validate action
//	switch (move->localPacket.instruction) {
//	case 0:
//		if (move->game->ship[index].x < (move->game->xsize - 1) && move->game->ship[index].lives >= 0)
//			if (DefenderShipsCollision(move->game, ++xTemp, yTemp, index)) {
//				move->game->ship[index].x++;
//			}
//		break;
//	case 1:
//		if (move->game->ship[index].y<(move->game->ysize - 1) && move->game->ship[index].lives >= 0)
//			if (DefenderShipsCollision(move->game, xTemp, ++yTemp, index)) {
//				move->game->ship[index].y++;
//			}
//		break;
//	case 2:
//		if (move->game->ship[index].x>0 && move->game->ship[index].lives >= 0)
//			if (DefenderShipsCollision(move->game, --xTemp, yTemp, index)) {
//				move->game->ship[index].x--;
//			}
//		break;
//	case 3:
//		if (move->game->ship[index].y>(move->game->ysize - (move->game->ysize*0.2)) && move->game->ship[index].lives >= 0)
//			if (DefenderShipsCollision(move->game, xTemp, --yTemp, index)) {
//				move->game->ship[index].y--;
//			}
//		break;
//	case 4:
//
//		htShotLauncher = CreateThread(
//			NULL,										//Thread security attributes
//			0,											//Stack size
//			ShotMovement,								//Thread function name
//			move,										//Thread parameter struct
//			0,											//Creation flags
//			&tShotLauncherID);							//gets thread ID 
//		if (htShotLauncher == NULL) {
//			_tprintf(TEXT("[Error] Creating thread htShotLauncher (%d) at server\n"), GetLastError());
//			return -1;
//		}
//
//		break;
//	default:
//		break;
//	}
//
//	return 0;
//}