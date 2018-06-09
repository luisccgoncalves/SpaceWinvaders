#include "algorithms.h"

DWORD WINAPI PowerUpTimer(LPVOID tParam) {

	PUpTimer	timerStr = *(PUpTimer*)tParam;

	for (int i = 0; i < 10 && (timerStr.ship->lives>=0); i++) {
		Sleep(timerStr.pUp.duration / 10);
	}

	WaitForSingleObject(timerStr.mhStructSync, INFINITE);
	switch (timerStr.pUp.type) {
	case 0:
		timerStr.ship->shield--;
		break;
	case 1:
		timerStr.ship->drunk--;
		break;
	case 2:
		timerStr.ship->turbo--;
		break;
	case 3:
		timerStr.ship->laser_shots--;
		break;
	default:
		break;
	}
	ReleaseMutex(timerStr.mhStructSync);

	return 0;
}

void PowerUpShip(Ship *ship, PowerUp *pUp, HANDLE mutex) {

	HANDLE		htPowerUpTimer;
	PUpTimer	tParam;

	tParam.mhStructSync = mutex;
	tParam.ship = ship;
	tParam.pUp = *pUp;


	switch (pUp->type) {
	case 0:
		ship->shield++;
		break;
	case 1:
		ship->drunk++;
		break;
	case 2:
		ship->turbo++;
		break;
	case 3:
		ship->laser_shots++;
		break;
	default:
		break;
	}
	
	//Launches a thread with the powerUp timeout
	htPowerUpTimer = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size
		PowerUpTimer,								//Thread function name
		(LPVOID)&tParam,								//Thread parameter struct
		0,											//Creation flags
		NULL);										//gets thread ID 
	if (htPowerUpTimer == NULL) {
		_tprintf(TEXT("[Error] Creating powerUpTimer thread (%d)\n"), GetLastError());
	}
}

PowerUp GeneratePowerUp(int x_max, int duration) {
	
	PowerUp pUp;

	pUp.x = RandomValue(x_max);		//Choose a random x position to drop
	pUp.y = 0;

	pUp.fired = 0;
	pUp.type = RandomValue(3);		//Random type between 0 and 3

	pUp.duration = duration;
	return pUp;
}

DWORD WINAPI PowerUps(LPVOID tParam) {

	SMCtrl	*cThread = (SMCtrl*)tParam;
	int		*ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;

	while (*ThreadMustGoOn) {

		//Sleep((10000 + RandomValue(10000))*(*ThreadMustGoOn));
		for (int i = 0; i < 20 && *ThreadMustGoOn; i++) {
			if (i > 10)
				if (RandomValue(2))
					break;
			Sleep(1000);
		}

		WaitForSingleObject(cThread->mhStructSync, INFINITE);
		cThread->localGameData.pUp = GeneratePowerUp(cThread->localGameData.xsize, cThread->localGameData.pup_duration);
		ReleaseMutex(cThread->mhStructSync);

		//Flash the powerUp 5x before dropping it
		for (int i = 0; i < 5 && *ThreadMustGoOn; i++) {

			Sleep(500 * (*ThreadMustGoOn));

			WaitForSingleObject(cThread->mhStructSync, INFINITE);
			cThread->localGameData.pUp.fired ^= 1;		//Bit shift blinker
			ReleaseMutex(cThread->mhStructSync);
		}

		Sleep(1000 * (*ThreadMustGoOn)); //Waits 1 sec

		//Drops the powerUp
		for (int i = 0; i < cThread->localGameData.ysize && cThread->localGameData.pUp.fired && *ThreadMustGoOn; i++) {

			WaitForSingleObject(cThread->mhStructSync,INFINITE);
			cThread->localGameData.pUp.y = i;			//Drops powerUP one place


			//Tests for collisions
  			if (cThread->localGameData.pUp.y>(cThread->localGameData.ysize*0.2)) {
				for (int j = 0; j < cThread->localGameData.num_players && cThread->localGameData.pUp.fired; j++) {
					if (cThread->localGameData.pUp.x == cThread->localGameData.ship[j].x &&
						cThread->localGameData.pUp.y == cThread->localGameData.ship[j].y) {

						PowerUpShip(&cThread->localGameData.ship[j], 
							&cThread->localGameData.pUp, 
							cThread->mhStructSync);

						cThread->localGameData.pUp.fired = 0;
					}
				}
			}

			ReleaseMutex(cThread->mhStructSync);
			Sleep(cThread->localGameData.projectiles_speed*(*ThreadMustGoOn));		//Pratical assignment pdf, page 3, 4th rule
		}

		cThread->localGameData.pUp.fired = 0;

	}

	return 0;
}

//DWORD WINAPI InvadersBomb(LPVOID tParam) {
//
//	GameData	*baseGame = ((BombMoves *)tParam)->game;
//	Invader		*invader = ((BombMoves*)tParam)->invader;
//
//	DWORD		tBombLauncherID;
//	HANDLE		htBombLauncher[MAX_BOMBS];  
//
//	int			i;
//	if (invader->bombRateCounter = 1) {
//		for (i = 0; i < baseGame->max_bombs; i++) {
//			htBombLauncher[i] = CreateThread(
//				NULL,										//Thread security attributes
//				0,											//Stack size
//				BombMovement,								//Thread function name
//				tParam,										//Thread parameter struct
//				0,											//Creation flags
//				&tBombLauncherID);							//gets thread ID 
//			if (htBombLauncher[i] == NULL) {
//				_tprintf(TEXT("[Error] Creating thread htBombLauncher[%d] (%d) at server\n"), i, GetLastError());
//				return -1;
//			}
//
//		}
//	}
//	WaitForMultipleObjects(MAX_BOMBS, htBombLauncher,TRUE, INFINITE);
//	return 0;
//}

DWORD WINAPI BombMovement(LPVOID tParam) {

	int			*ThreadMustGoOn = ((BombMoves *)tParam)->TheadmustGoOn;
	HANDLE		*mhStructSync = ((BombMoves*)tParam)->mhStructSync;
	GameData	*baseGame = ((BombMoves *)tParam)->game;
	Invader		*invader = ((BombMoves*)tParam)->invader;
	

	/*This is maybe not needed... */
		int			bombNum = -1;
		for (int i = 0; i < baseGame->max_bombs; i++) {						//cicle to check if there is available slots to fire a bomb
			if (!invader->bomb[i].fired) {
				bombNum = i;
				break;
			}
		}
		if (bombNum > -1) {
			if (invader->bombRateCounter == 1) {

				WaitForSingleObject(mhStructSync, INFINITE);
				invader->bomb[bombNum].x = invader->x;		//give invaders coords to bomb
				invader->bomb[bombNum].y = invader->y;
				invader->bomb[bombNum].fired = 1;								//update bomb status
				ReleaseMutex(mhStructSync);

				while (*ThreadMustGoOn && invader->bomb[bombNum].fired) {

					if (invader->bomb[bombNum].y < baseGame->ysize - 1) {		//if bomb has not reached the end of the play area

						WaitForSingleObject(mhStructSync, INFINITE);
						invader->bomb[bombNum].y++;							//update it's position, an wait for next tick 
						BombCollision(baseGame, &invader->bomb[bombNum]);
						ReleaseMutex(mhStructSync);

						Sleep(baseGame->projectiles_speed * (*ThreadMustGoOn));	//Pratical assignment pdf, page 3, 4th rule
					}
					else {														//reset bomb to out of screen
						WaitForSingleObject(mhStructSync, INFINITE);
						ResetBomb(&invader->bomb[bombNum]);
						ReleaseMutex(mhStructSync);
					}
				}
			}
		}
	return 0;
}

DWORD WINAPI RegPathInvaders(LPVOID tParam) {

	int			*ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	GameData	*baseGame = &((SMCtrl *)tParam)->localGameData;
	HANDLE		*mhStructSync = ((SMCtrl *)tParam)->mhStructSync;

	int i, j;
	int sidestep = 4;
	int totalsteps = (baseGame->ysize - (baseGame->max_invaders/ INVADER_BY_ROW)) * sidestep;
	int regInvaderNr = (baseGame->max_invaders - baseGame->max_rand_invaders);

	DWORD			tBombLauncherID;
	HANDLE			htBombLauncher;
	BombMoves		bombMoves;
	
	while (*ThreadMustGoOn) {						//Thread main loop

		for (i = 0; (i < totalsteps) && *ThreadMustGoOn; i++) {

			/*WaitForSingleObject(mhStructSync, INFINITE);*/

			for (j = 0; (j < regInvaderNr) && *ThreadMustGoOn; j++) {
				if (!baseGame->invad[j].rand_path && baseGame->invad[j].hp > 0) {

					WaitForSingleObject(mhStructSync, INFINITE);
					baseGame->invad[j].y = (i / sidestep) + baseGame->invad[j].y_init;				//Invader goes down after n sidesteps

					if ((i % (sidestep * 2)) < sidestep)
						baseGame->invad[j].x = (i % (sidestep * 2)) + baseGame->invad[j].x_init;		//Invader goes right
					else if ((i % (sidestep * 2)) > sidestep)
						baseGame->invad[j].x--;													//Invader goes left
					InvaderCollision(baseGame, &baseGame->invad[j]);
					UpdateInvaderBombRate(baseGame, &baseGame->invad[j]);
					ReleaseMutex(mhStructSync);

					if (baseGame->invad[j].bombRateCounter == 1) {
						if (baseGame->invad[j].hp > 0) {
							bombMoves.game = baseGame;
							bombMoves.invader = &baseGame->invad[j];
							bombMoves.mhStructSync = mhStructSync;
							bombMoves.TheadmustGoOn = ThreadMustGoOn;

							WaitForSingleObject(mhStructSync, INFINITE);
							htBombLauncher = CreateThread(
								NULL,										//Thread security attributes
								0,											//Stack size
								BombMovement,								//Thread function name
								(LPVOID)&bombMoves,										//Thread parameter struct
								0,											//Creation flags
								&tBombLauncherID);							//gets thread ID 
							if (htBombLauncher == NULL) {
								_tprintf(TEXT("[Error] Creating thread htBombLauncher[%d] (%d) at server\n"), i, GetLastError());
								return -1;
							}
							ReleaseMutex(mhStructSync);
						}
					}
				}
			}
			//ReleaseMutex(mhStructSync);
			Sleep(baseGame->invaders_speed*(*ThreadMustGoOn));
		}
	}
	return 0;
}

DWORD WINAPI RandPathInvaders(LPVOID tParam) {

	int * ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	GameData *baseGame = &((SMCtrl *)tParam)->localGameData;
	HANDLE		*mhStructSync = ((SMCtrl *)tParam)->mhStructSync;

	int i, xTemp, yTemp, invalid, count;

	DWORD			tBombLauncherID;
	HANDLE			htBombLauncher;
	BombMoves		bombMoves;

	while (*ThreadMustGoOn) {						//Thread main loop
		/*WaitForSingleObject(mhStructSync, INFINITE);*/
		for (i = 0; (i < baseGame->max_invaders) && *ThreadMustGoOn; i++) {
			if (baseGame->invad[i].rand_path && baseGame->invad[i].hp >0) {

				WaitForSingleObject(mhStructSync, INFINITE);
				count = 0;
				do {
					xTemp = baseGame->invad[i].x;
					yTemp = baseGame->invad[i].y;

					switch (baseGame->invad[i].direction) {
					case 0:
						xTemp--;
						yTemp--;
						break;
					case 1:
						xTemp++;
						yTemp--;
						break;
					case 2:
						xTemp--;
						yTemp++;
						break;
					case 3:
						xTemp++;
						yTemp++;
						break;
					case 4:
						xTemp--;
						break;
					case 5:
						xTemp++;
						break;
					case 6:
						yTemp--;
						break;
					case 7:
						yTemp++;
						break;
					}

					UpdateCoords(baseGame, &yTemp);
					
					invalid = ValidateInvaderPosition(baseGame, xTemp, yTemp, i);
					if (invalid && count < 50) {
						baseGame->invad[i].direction = RandomValue(3);
						count++;
					}if (count >= 50) {  //if there is no option find another path
						baseGame->invad[i].direction = RandomValue(3)+4;
						count++;
					}
				} while (invalid && count < 100);
				if (count < 100) {
					baseGame->invad[i].x = xTemp;
					baseGame->invad[i].y = yTemp;
				}
				//if the game got here, then it does not move
				//if we want we could kill the ship... last resort
				//else {
				//	baseGame->invad[i].hp = 0;
				//}

				InvaderCollision(baseGame, &baseGame->invad[i]);
				UpdateInvaderBombRate(baseGame, &baseGame->invad[i]);
				if (baseGame->invad[i].bombRateCounter == 1) {
					bombMoves.game = baseGame;
					bombMoves.invader = &baseGame->invad[i];
					bombMoves.mhStructSync = mhStructSync;
					bombMoves.TheadmustGoOn = ThreadMustGoOn;

					htBombLauncher = CreateThread(
						NULL,										//Thread security attributes
						0,											//Stack size
						BombMovement,								//Thread function name
						(LPVOID)&bombMoves,										//Thread parameter struct
						0,											//Creation flags
						&tBombLauncherID);							//gets thread ID 
					if (htBombLauncher == NULL) {
						_tprintf(TEXT("[Error] Creating thread htBombLauncher[%d] (%d) at server\n"), i, GetLastError());
						return -1;
					}
				}
				ReleaseMutex(mhStructSync);
			}
		}
		/*ReleaseMutex(mhStructSync);*/
		Sleep((DWORD)(baseGame->invaders_speed *0.9)*(*ThreadMustGoOn));
	}

	return 0;
}

DWORD WINAPI ShipInstruction(LPVOID tParam) {
	SMCtrl		*cThread = (SMCtrl*)tParam;

	ClientMoves move;
	move.game = &cThread->localGameData;
	move.TheadmustGoOn = &cThread->ThreadMustGoOn;
	move.mhStructSync = &cThread->mhStructSync;

	int	nextOut = 0;


	while (cThread->ThreadMustGoOn) {

		//Consume item from buffer (gets a packet with a client instruction)
		move.localPacket = consumePacket(cThread, &nextOut);	//Problem here: No exit condition

		WaitForSingleObject(cThread->mhStructSync, INFINITE);

		UpdateLocalShip(&move);									//Translates instructions into actions (movement, shots...)
		ShipCollision(move.game,								//Tests if those actions are valid
			&move.game->ship[move.localPacket.owner]);

		ReleaseMutex(cThread->mhStructSync);

	}

	return 0;
}

DWORD WINAPI ShotMovement(LPVOID tParam) {

	GameData	*baseGame = ((ClientMoves *)tParam)->game;
	HANDLE		*mhStructSync = ((ClientMoves*)tParam)->mhStructSync;

	int			*ThreadMustGoOn = ((ClientMoves *)tParam)->TheadmustGoOn;
	int			owner = ((ClientMoves*)tParam)->localPacket.owner;
	int			i, shotNum = -1;

	for (i = 0; i < MAX_SHOTS; i++) {						//cicle to check if there is available slots to fire a shot
		if (!baseGame->ship[owner].shots[i].fired) {
			shotNum = i;
			break;
		}
	}

	if (shotNum > -1) {
		WaitForSingleObject(mhStructSync, INFINITE);
		baseGame->ship[owner].shots[shotNum].x = baseGame->ship[owner].x;
		baseGame->ship[owner].shots[shotNum].y = baseGame->ship[owner].y;
		baseGame->ship[owner].shots[shotNum].fired = 1;
		ReleaseMutex(mhStructSync);
		while (*ThreadMustGoOn && baseGame->ship[owner].shots[shotNum].fired) {

			if (baseGame->ship[owner].shots[shotNum].y > 0) {						//if bomb has not reached the end of the play area

				WaitForSingleObject(mhStructSync, INFINITE);
				baseGame->ship[owner].shots[shotNum].y--;							//update it's position, an wait for next tick 
				ShotCollision(baseGame, &baseGame->ship[owner].shots[shotNum]);
				ReleaseMutex(mhStructSync);
				Sleep(baseGame->ship_shot_speed * (*ThreadMustGoOn));

			}
			else {														//reset shot to out of screen
				WaitForSingleObject(mhStructSync, INFINITE);
				ResetShot(&baseGame->ship[owner].shots[shotNum]);
				ReleaseMutex(mhStructSync);
			}

		}
	}

	return 0;
}

int UpdateLocalShip(ClientMoves *move) {

	DWORD			tShotLauncherID;
	HANDLE			htShotLauncher;  
	
	//validate action
	switch (move->localPacket.instruction) {
	case 0:
		if (move->game->ship[move->localPacket.owner].x < (move->game->xsize - 1) && 
			move->game->ship[move->localPacket.owner].lives >= 0)

			move->game->ship[move->localPacket.owner].x++;
		break;
	case 1:
		if (move->game->ship[move->localPacket.owner].y<(move->game->ysize - 1) && 
			move->game->ship[move->localPacket.owner].lives >= 0)

			move->game->ship[move->localPacket.owner].y++;
		break;
	case 2:
		if (move->game->ship[move->localPacket.owner].x>0 && 
			move->game->ship[move->localPacket.owner].lives >= 0)

			move->game->ship[move->localPacket.owner].x--;
		break;
	case 3:
		if (move->game->ship[move->localPacket.owner].y>(move->game->ysize - (move->game->ysize*0.2)) && 
			move->game->ship[move->localPacket.owner].lives >= 0)

			move->game->ship[move->localPacket.owner].y--;
		break;
	case 4:
		
		htShotLauncher = CreateThread(
			NULL,										//Thread security attributes
			0,											//Stack size
			ShotMovement,								//Thread function name
			move,										//Thread parameter struct
			0,											//Creation flags
			&tShotLauncherID);							//gets thread ID 
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
	int i, j;

	game->xsize = XSIZE;
	game->ysize = YSIZE;

	game->invaders_speed = INVADER_SPEED;			// Base speed for invader
	game->max_bombs = MAX_BOMBS;					// Base max num of bombs at same time
	game->max_invaders = MAX_INVADER;				// Base num of invaders in the field
	game->max_rand_invaders = RAND_INVADER;			// Base num of invaders in the field
	game->num_players = MAX_PLAYERS;				// Base num of players
	game->ship_shot_speed = PROJECTL_SPEED;			// Base speed for defender ship
	game->projectiles_speed = PROJECTL_SPEED;		// Base speed for Powerups and invader bombs
	game->pup_duration = POWERUP_DUR;				// Base power up duration
	game->bombRate = BOMBRATE;						// Base steps until bomb launch

	for (i = 0; i < game->max_invaders; i++) {			//Instantiates all bombs outside of game and updates the status
		for (j = 0; j < game->max_bombs; j++) {
			ResetBomb(&game->invad[i].bomb[j]);
		}
		game->invad[i].bombRateCounter = RandomValue(10);
	}
	
	for (i = 0; i < game->num_players; i++) {
		game->ship[i].drunk = 0;
		game->ship[i].laser_shots = 0;
		game->ship[i].lives = 1;
		game->ship[i].x = 0;
		game->ship[i].y = 0;
		game->ship[i].shield = 0;
		for (j = 0; j < MAX_SHOTS; j++) {				//Instantiates all shots outside of game and updates the status
			ResetShot(&game->ship[i].shots[j]);
		}
		//...
	}

	return 0;
}

int ShipCollision(GameData *game, Ship *ship) {
	int i,j;
	if (ship->lives >= 0) {
		for (i = 0; i < game->max_invaders; i++) {
			if (game->invad[i].x == ship->x && game->invad[i].y == ship->y && game->invad[i].hp > 0) {
				DamageInvader(&game->invad[i]);
				DamageShip(ship);
				return 1;
			}
		}
		for (i = 0; i < game->max_invaders; i++) {
			for (j = 0; j < game->max_bombs; j++) {
				if (game->invad[i].bomb[j].x == ship->x && game->invad[i].bomb[j].y == ship->y && game->invad[i].bomb[j].fired) { //ERROR?
					ResetBomb(&game->invad[i].bomb[j]);
					if (!ship->shield)
						DamageShip(ship);
					return 1;
				}
			}
		}

		if (game->pUp.x == ship->x && game->pUp.y == ship->y && game->pUp.fired == 1) {
		//	//Update game status? like lauch a thread reset after a sleep?
		//DamageShip(ship);
			_tprintf(TEXT("\7"));
			return 1;
		}
	}
	return 0;
}

int ShotCollision(GameData *game, ShipShot *shot) {
	int i,j;
	if (shot->fired) {
		for (i = 0; i < game->max_invaders; i++) {
			if (game->invad[i].x == shot->x && game->invad[i].y == shot->y && game->invad[i].hp > 0) {
				DamageInvader(&game->invad[i]);
				ResetShot(shot);
				return 1;
			}
		}
		for (i = 0; i < game->max_invaders; i++) {
			for (j = 0; j < game->max_bombs; j++) {
				if (game->invad[i].bomb[j].x == shot->x && game->invad[i].bomb[j].y == shot->y && game->invad[i].bomb[j].fired) {
					ResetBomb(&game->invad[i].bomb[j]);
					ResetShot(shot);
					return 1;
				}
			}
		}
	}
	return 0;
}

int InvaderCollision(GameData * game, Invader * invader)
{
	int i, j;
	if (invader->hp > 0) {
		for (i = 0; i < game->num_players; i++) {
			if (game->ship[i].x == invader->x && game->ship[i].y == invader->y && game->ship[i].lives >= 0) {
				DamageShip(&game->ship[i]);
				DamageInvader(invader);
				return 1;
			}
		}
		for (i = 0; i < game->num_players; i++) {
			for (j = 0; j < MAX_SHOTS; j++) {
				if (game->ship[i].shots[j].x == invader->x && game->ship[i].shots[j].y == invader->y && game->ship[i].shots[j].fired) {
					ResetShot(&game->ship[i].shots[j]);
					DamageInvader(invader);
					return 1;
				}
			}
		}
	}
	return 0;
}

int BombCollision(GameData * game, InvaderBomb * bomb)
{
	int i,j;
	if (bomb->fired) {
		for (i = 0; i < game->num_players; i++) {
			if (game->ship[i].x == bomb->x && game->ship[i].y == bomb->y && game->ship[i].lives >= 0) {
				if (!game->ship[i].shield) {
					DamageShip(&game->ship[i]);
					ResetBomb(bomb);
					return 1;
				}
			}
		}
		for (i = 0; i < game->num_players; i++) {
			for (j = 0; j < MAX_SHOTS; j++) {
				if (game->ship[i].shots[j].x == bomb->x && game->ship[i].shots[j].y == bomb->y && game->ship[i].shots[j].fired) {
					ResetShot(&game->ship[i].shots[j]);
					ResetBomb(bomb);
					return 1;
				}
			}
		}
	}
	return 0;
}

int DamageShip(Ship *in) {				//Removes 1 live to defender ship
	in->lives--;
	if (in->lives< 0) {					//If 0, is using last live, if <0 it's dead
		ResetShip(in);
		return 1;
	}
		return 0;
}

int DamageInvader(Invader *in) {		//Removes 1 HP point to the invader
	in->hp--;							
	if (in->hp <= 0) {					//If one live, remove 1, exit game
		ResetInvader(in);
		return 1;
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

int ValidateInvaderPosition(GameData * game, int x, int y, int index)
{
	int i;
	int largerCoord = GetRegularLargerXPosition(game);

	for (i = 0; i < game->max_invaders; i++) {
		if(i!=index){
			if (!game->invad[i].rand_path) {
				if (abs(game->invad[i].x - x) < 2 && abs(game->invad[i].y - y) < 2 && game->invad[i].hp > 0) {
					return 1;
				}
			}
			else {
				if (game->invad[i].x == x && game->invad[i].y == y && game->invad[i].hp > 0) {
					return 1;
				}
				if (x >= game->xsize) {
					return 1;
				}
				if (x <= largerCoord+1) {
					return 1;
				}
			}
		}
	}
	return 0;
}

int GetRegularLargerXPosition(GameData *game) {
	int x = 0;
	int i = 0;

	for (i = 0; i < game->max_invaders; i++) {
		if (!game->invad[i].rand_path) {
			if (x < game->invad[i].x)
				x = game->invad[i].x;
		}
	}
	return x;
}

int UpdateCoords(GameData * game, int *y) {
	if (*y < 0) {
		*y = game->ysize;
	}
	if (*y > game->ysize ) {
		*y = 0;
	}
	return 0;
}

int UpdateInvaderBombRate(GameData *game, Invader *invader) {
	if (invader->hp > 0) {
		invader->bombRateCounter++;
		if (invader->bombRateCounter > game->bombRate) {
			invader->bombRateCounter = 0;
		}
	}
	else {
		ResetInvader(invader);
	}
	return 0;
}





