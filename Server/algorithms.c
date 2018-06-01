#include "algorithms.h"



DWORD WINAPI InvadersBomb(LPVOID tParam) {

	int * ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	GameData *baseGame = &((SMCtrl *)tParam)->localGameData;

	DWORD			tBombLauncherID;
	HANDLE			htBombLauncher[MAX_BOMBS];  //### THIS NEEDS A CONSTANTE VALUE

	for (int i = 0; i < baseGame->max_bombs; i++) {


		htBombLauncher[i] = CreateThread(
			NULL,										//Thread security attributes
			0,											//Stack size
			BombMovement,								//Thread function name
			tParam,										//Thread parameter struct
			0,											//Creation flags
			&tBombLauncherID);							//gets thread ID to close it afterwards
		if (htBombLauncher[i] == NULL) {
			_tprintf(TEXT("[Error] Creating thread htBombLauncher[%d] (%d) at server\n"),i, GetLastError());
			return -1;
		}


	}
	WaitForMultipleObjects(MAX_BOMBS, htBombLauncher,TRUE, INFINITE);
	return 0;
}

DWORD WINAPI BombMovement(LPVOID tParam) {

	int * ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	GameData *baseGame = &((SMCtrl *)tParam)->localGameData;

	int invPosition = -1;
	int bombNum = -1;
	int num = -1;


	while (*ThreadMustGoOn) {

		num = RandomValue(10);
		Sleep(500 * (num+1));

		for (int i = 0; i < baseGame->max_bombs; i++) {						//cicle to check if there is available slots to fire a bomb
			if (!baseGame->bomb[i].fired) {
				bombNum = i;
				break;
			}
		}

		do {																//find a random invader to send the bomb from
			invPosition = RandomValue(baseGame->max_invaders);
		} while (baseGame->invad[invPosition].hp == 0);

		if (bombNum > -1) {

			baseGame->bomb[bombNum].x = baseGame->invad[invPosition].x;		//give invaders coords to bomb
			baseGame->bomb[bombNum].y = baseGame->invad[invPosition].y;
			baseGame->bomb[bombNum].fired = 1;								//update bomb status

			while (*ThreadMustGoOn && baseGame->bomb[bombNum].fired/*&&bombColDetect(&bomb,tParam)*/) {
				if (baseGame->bomb[bombNum].y < baseGame->ysize-1) {		//if bomb has not reached the end of the play area
					baseGame->bomb[bombNum].y++;							//update it's position, an wait for next tick 

					BombCollision(baseGame, &baseGame->bomb[bombNum]);
					
					Sleep(((baseGame->invaders_bombs_speed) / 5) * (*ThreadMustGoOn));
				}
				else {														//reset bomb to out of screen
					ResetBomb(&baseGame->bomb[bombNum]);					
				}
			}
		}
	}
	return 0;
}

DWORD WINAPI RegPathInvaders(LPVOID tParam) {

	int * ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	GameData *baseGame = &((SMCtrl *)tParam)->localGameData;
	//GameData *baseGame = ((SMCtrl *)tParam)->pSMemGameData;
	HANDLE		*mhStructSync = ((SMCtrl *)tParam)->mhStructSync;

	int i, j;
	int sidestep = 4;
	int totalsteps = (baseGame->ysize - (baseGame->max_invaders/ INVADER_BY_ROW)) * sidestep;
	int regInvaderNr = (baseGame->max_invaders - baseGame->max_rand_invaders);


	while (*ThreadMustGoOn) {						//Thread main loop

		for (i = 0; (i < totalsteps) && *ThreadMustGoOn; i++) {

			WaitForSingleObject(mhStructSync, INFINITE);

			for (j = 0; (j < regInvaderNr) && *ThreadMustGoOn; j++) {

				baseGame->invad[j].y = (i / sidestep) + baseGame->invad[j].y_init;				//Invader goes down after n sidesteps

				if ((i % (sidestep * 2)) < sidestep)
					baseGame->invad[j].x = (i % (sidestep * 2)) + baseGame->invad[j].x_init;		//Invader goes right
				else if ((i % (sidestep * 2)) > sidestep)
					baseGame->invad[j].x--;													//Invader goes left
				InvaderCollision(baseGame, &baseGame->invad[j]);
			}

			ReleaseMutex(mhStructSync);

			Sleep(baseGame->invaders_speed*(*ThreadMustGoOn));
		}
	}

	return 0;
}

DWORD WINAPI RandPathInvaders(LPVOID tParam) {

	int * ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	GameData *baseGame = &((SMCtrl *)tParam)->localGameData;
	HANDLE		*mhStructSync = ((SMCtrl *)tParam)->mhStructSync;

	int i, xTemp, yTemp;
	int startIndex = baseGame->max_invaders - baseGame->max_rand_invaders;
	int maxXpos = baseGame->xsize - 1;
	int maxYpos = (int)((baseGame->ysize - 1)*0.8);

	while (*ThreadMustGoOn) {						//Thread main loop

		WaitForSingleObject(mhStructSync, INFINITE);

		for (i = startIndex; (i < baseGame->max_invaders) && *ThreadMustGoOn; i++) {
			do {
				xTemp = baseGame->invad[i].x;
				yTemp = baseGame->invad[i].y;

				switch (RandomValue(4)) {
				case 0:
					if (xTemp > 0)
						xTemp--;
					else
						xTemp = 1;
					break;
				case 1:
					if (xTemp < maxXpos)
						xTemp++;
					else
						xTemp = maxXpos - 1;
					break;
				case 2:
					if (yTemp > 0)
						yTemp--;
					else
						yTemp = 1;
					break;
				case 3:
					if (yTemp < maxYpos)
						yTemp++;
					else
						yTemp = maxYpos - 1;
					break;
				}
			} while (ValidateInvaderPosition(baseGame, xTemp, yTemp));

			baseGame->invad[i].x = xTemp;
			baseGame->invad[i].y = yTemp;

			InvaderCollision(baseGame, &baseGame->invad[i]);
		}
		ReleaseMutex(mhStructSync);
		Sleep((baseGame->invaders_speed / 4)*(*ThreadMustGoOn));
	}

	return 0;
}

DWORD WINAPI ShipInstruction(LPVOID tParam) {
	SMCtrl		*cThread = (SMCtrl*)tParam;

	ClientMoves move;
	move.game = &cThread->localGameData;
	move.TheadmustGoOn = &cThread->ThreadMustGoOn;

	int	nextOut = 0;


	while (cThread->ThreadMustGoOn) {

		//Consume item from buffer
		move.localPacket = consumePacket(cThread, &nextOut);  //Problem here: No exit condition

		WaitForSingleObject(cThread->mhStructSync, INFINITE);

		UpdateLocalShip(&move);
		ShipCollision(move.game, &move.game->ship[move.localPacket.owner]);

		ReleaseMutex(cThread->mhStructSync);

	}

	return 0;
}

DWORD WINAPI ShotMovement(LPVOID tParam) {

	int * ThreadMustGoOn = ((ClientMoves *)tParam)->TheadmustGoOn;
	GameData *baseGame = ((ClientMoves *)tParam)->game;
	int owner = ((ClientMoves*)tParam)->localPacket.owner;

	int shotNum = -1;
		
		for (int i = 0; i < MAX_SHOTS; i++) {						//cicle to check if there is available slots to fire a shot
			if (!baseGame->shot[i].fired) {
				shotNum = i;
				break;
			}
		}

		if (shotNum > -1) {

			baseGame->shot[shotNum].x = baseGame->ship[owner].x;
			baseGame->shot[shotNum].y = baseGame->ship[owner].y;
			baseGame->shot[shotNum].fired = 1;

			while (*ThreadMustGoOn && baseGame->shot[shotNum].fired/*&&bombColDetect(&bomb,tParam)*/) {

				if (baseGame->shot[shotNum].y > 0) {						//if bomb has not reached the end of the play area
					baseGame->shot[shotNum].y--;							//update it's position, an wait for next tick 
					ShotCollision(baseGame, &baseGame->shot[shotNum]);
					Sleep(((baseGame->invaders_bombs_speed / 4)) * (*ThreadMustGoOn));

				}
				else {														//reset bomb to out of screen
					ResetShot(&baseGame->shot[shotNum]);
				}

			}
		}

	return 0;
}

int UpdateLocalShip(ClientMoves *move) {

	DWORD			tShotLauncherID;
	HANDLE			htShotLauncher;  //### THIS NEEDS A CONSTANTE VALUE

	//validate action
	switch (move->localPacket.instruction) {
	case 0:
		if (move->game->ship[move->localPacket.owner].x < (move->game->xsize - 1) && move->game->ship[move->localPacket.owner].lives >= 0)
			move->game->ship[move->localPacket.owner].x++;
		//if (ShipCollision(move->game, &move->game->ship[move->localPacket.owner])) {
		//	DamageShip(&move->game->ship[move->localPacket.owner]);
		//}
		break;
	case 1:
		if (move->game->ship[move->localPacket.owner].y<(move->game->ysize - 1) && move->game->ship[move->localPacket.owner].lives >= 0)
			move->game->ship[move->localPacket.owner].y++;
		//if (ShipCollision(move->game, &move->game->ship[move->localPacket.owner])) {
		//	DamageShip(&move->game->ship[move->localPacket.owner]);
		//}
		break;
	case 2:
		if (move->game->ship[move->localPacket.owner].x>0 && move->game->ship[move->localPacket.owner].lives >= 0)
			move->game->ship[move->localPacket.owner].x--;
		//if (ShipCollision(move->game, &move->game->ship[move->localPacket.owner])) {
		//	DamageShip(&move->game->ship[move->localPacket.owner]);
		//}
		break;
	case 3:
		if (move->game->ship[move->localPacket.owner].y>(move->game->ysize - (move->game->ysize*0.2)) && move->game->ship[move->localPacket.owner].lives >= 0)
			move->game->ship[move->localPacket.owner].y--;
		//if (ShipCollision(move->game, &move->game->ship[move->localPacket.owner])) {
		//	DamageShip(&move->game->ship[move->localPacket.owner]);
		//}
		break;
	case 4:
		
		htShotLauncher = CreateThread(
			NULL,										//Thread security attributes
			0,											//Stack size
			ShotMovement,								//Thread function name
			move,										//Thread parameter struct
			0,											//Creation flags
			&tShotLauncherID);							//gets thread ID to close it afterwards
		if (htShotLauncher == NULL) {
			_tprintf(TEXT("[Error] Creating thread htShotLauncher (%d) at server\n"), GetLastError());
			return -1;
		}

		break;
	default:
		break;
	}

	return 0;
}

int InstantiateGame(GameData *game) {
	int i;
	/*
	this is for filling the game,
	that then will be altered later
	...
	another function?
	*/

	game->xsize = XSIZE;
	game->ysize = YSIZE;

	game->invaders_bombs_speed = INVADER_SPEED;		// not correct
	game->invaders_speed = INVADER_SPEED;		// Base speed for invader
	game->max_bombs = MAX_BOMBS;			// Base max num of bombs at same time
	game->max_invaders = MAX_INVADER;		// Base num of invaders in the field
	game->max_rand_invaders = RAND_INVADER;		// Base num of invaders in the field
	game->num_players = MAX_PLAYERS;		// Base num of players
	game->power_up_speed = INVADER_SPEED;		// Base speed for power up
	game->ship_shot_speed = INVADER_SPEED;		// Base speed for defender ship

	for (i = 0; i < MAX_BOMBS; i++) {			//Instantiates all bombs outside of game and updates the status
		ResetBomb(&game->bomb[i]);
	}

	for (i = 0; i < MAX_SHOTS; i++) {				//Instantiates all shots outside of game and updates the status
		ResetShot(&game->shot[i]);
	}

	for (i = 0; i < MAX_PLAYERS; i++) {
		game->ship[i].drunk = 0;
		game->ship[i].laser_shots = 0;
		game->ship[i].lives = 1;
		game->ship[i].x = 0;
		game->ship[i].y = 0;
		//...
	}

	return 0;
}

int ShipCollision(GameData *game, Ship *ship) {
	int i = 0;
	for (i = 0; i < game->max_bombs; i++) {
		if (game->bomb[i].x == ship->x && game->bomb[i].y == ship->y && game->bomb[i].fired) {
			ResetBomb(&game->bomb[i]);
			DamageShip(ship);
			return 1;
		}
	}
	for (i = 0; i < game->max_invaders; i++) {
		if (game->invad[i].x == ship->x && game->invad[i].y == ship->y && game->invad[i].hp > 0) {
			DamageInvader(&game->invad[i]);
			DamageShip(ship);
			return 1;
		}
	}

	//if (game->pUp.x == ship->x && game->pUp.y == ship->y && game->pUp.fired == 1) {
	//	//Update game status? like lauch a thread reset after a sleep?
	//DamageShip(ship);
	//	return 1;
	//}
	return 0;
}

int ShotCollision(GameData *game, ShipShot *shot) {
	int i = 0;
	for (i = 0; i < game->max_invaders; i++) {
		if (game->invad[i].x == shot->x && game->invad[i].y == shot->y && game->invad[i].hp > 0) {
			DamageInvader(&game->invad[i]);
			ResetShot(shot);
			return 1;
		}
	}
	for (i = 0; i < game->max_bombs; i++) {
		if (game->bomb[i].x == shot->x && game->bomb[i].y == shot->y && game->bomb[i].fired) {
			ResetBomb(&game->bomb[i]);
			ResetShot(shot);
			return 1;
		}
	}

	return 0;
}

int InvaderCollision(GameData * game, Invader * invader)
{
	int i = 0;
	for (i = 0; i < game->num_players; i++) {
		if (game->ship[i].x == invader->x && game->ship[i].y == invader->y && game->ship[i].lives >= 0) {
			DamageShip(&game->ship[i]);
			DamageInvader(invader);
			return 1;
		}
	}
	for (i = 0; i < MAX_SHOTS; i++) {
		if (game->shot[i].x == invader->x && game->shot[i].y == invader->y && game->shot[i].fired) {
			ResetShot(&game->shot[i]);
			DamageInvader(invader);
			return 1;
		}
	}
}

int BombCollision(GameData * game, InvaderBomb * bomb)
{
	int i = 0;
	for (i = 0; i < game->num_players; i++) {
		if (game->ship[i].x == bomb->x && game->ship[i].y == bomb->y && game->ship[i].lives >= 0) {
			DamageShip(&game->ship[i]);
			ResetBomb(bomb);
			return 1;
		}
	}
	for (i = 0; i < MAX_SHOTS; i++) {
		if (game->shot[i].x == bomb->x && game->shot[i].y == bomb->y && game->shot[i].fired) {
			ResetShot(&game->shot[i]);
			ResetBomb(bomb);
			return 1;
		}
	}
}



int DamageShip(Ship *in) {
	in->lives--;
	if (in->lives< 0) {
		ResetShip(in);
		return 1;
	}
		return 0;
}



int DamageInvader(Invader *in) {
	in->hp--;
	if (in->hp < 0) {
		ResetInvader(in);
		return 1;
	}
		return 0;
}





int FullCollision(GameData *game) {		//dele this
	int i = 0;
	
	for (i = 0; i < MAX_PLAYERS; i++) {
		if (ShipCollision(game, &game->ship[i])) {
		//	DamageShip(&game->ship[i]);
			return 1;
		}
	}

	for (i = 0; i < MAX_SHOTS; i++) {
		if (ShotCollision(game, &game->shot[i])) {
			ResetShot(&game->shot[i]);
		}
	}

	return 0;
}


int ResetShip(Ship *in) {
	in->lives = -1;
	in->x = -1;
	in->y = -1;
	return 1;
}

int ResetShot(ShipShot *in) {

	in->fired = 0;
	in->x = -1;
	in->y = -1;
	return 1;
}
int ResetInvader(Invader *in) {

	in->hp = 0;
	in->x = -1;
	in->y = -1;
	return 1;
}

int ResetBomb(InvaderBomb *in) {

	in->fired = 0;
	in->x = -1;
	in->y = -1;
	return 1;
}

int ValidateInvaderPosition(GameData * game, int x, int y)
{
	int i;
	for (i = 0; i < game->max_invaders; i++) {
		if (game->invad[i].x == x && game->invad[i].y == y && game->invad[i].hp > 0) {
			return 1;
		}
	}
	return 0;
}






