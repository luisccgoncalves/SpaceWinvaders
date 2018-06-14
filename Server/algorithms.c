#include "algorithms.h"

int markPlayerReady(ClientMoves *ps) {
	int i;

	WaitForSingleObject(ps->mhStructSync, INFINITE);
	for (i = 0; i < MAX_PLAYERS; i++) {
		if (ps->localPacket.Id == ps->game->logged[i].Id) {
			ps->game->logged[i].isReady = TRUE;
			_tprintf(TEXT("Player %s is ready to play!\n"), ps->game->logged[i].username);
			break;										
		}
	}
	ReleaseMutex(ps->mhStructSync);

	if (i < MAX_PLAYERS)
		return 0;												//Found a player, marked it as ready
	else
		return 1;												//No player was found with that ID
}

int handShakeClient(ClientMoves *ps) {

	int i;

	if (ps->localPacket.instruction==5) {
		//Authentication
		_tprintf(TEXT("Received a Handshake request!\n"));

		WaitForSingleObject(ps->mhStructSync, INFINITE);
		for (i = MAX_PLAYERS; i >= 0; i--) {
			if (ps->localPacket.Id == ps->game->logged[i].Id)
				break;
		}
		ReleaseMutex(ps->mhStructSync);

		if (i >= 0) {
			_tprintf(TEXT("User already logged.\n"));
		}
		else {
			WaitForSingleObject(ps->mhStructSync, INFINITE);
			for (i = 0; i < MAX_PLAYERS; i++) {
				if (ps->game->logged[i].Id == 0) {
					ps->game->logged[i].Id = ps->localPacket.Id;
					_tcscpy_s(ps->game->logged[i].username, SMALL_BUFF, ps->localPacket.username);
					break;
				}
			}
			ReleaseMutex(ps->mhStructSync);

			if (i < MAX_PLAYERS)
				_tprintf(TEXT("User \"%s\" logged in with the ID:%d.\n"), ps->localPacket.username, ps->localPacket.Id);
			else
				_tprintf(TEXT("Server is full!\n"));
		}
	}
	else {
		//Deauth
	}
	return 0;
}

DWORD WINAPI PowerUpTimer(LPVOID tParam) {

	PUpTimer	timerStr = *(PUpTimer*)tParam;

	//PowerUp duration timer
	for (int i = 0; i < 10 && (timerStr.ship->lives >=0 ); i++) {
		Sleep(timerStr.pUp.duration / 10);
	}

	//Decreases the corresponding powerup flag to account for double buffs
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
	case 4:
		timerStr.game->ice--;
		break;
	case 5:
		timerStr.game->plusSpeed--;
		break;
	case 6:
		timerStr.game->battery--;
		break;
	case 7:
		break;
	default:
		break;
	}
	ReleaseMutex(timerStr.mhStructSync);

	return 0;
}

void PowerUpShip(GameData *game, Ship *ship, PowerUp *pUp, HANDLE mutex) {

	HANDLE		htPowerUpTimer;
	PUpTimer	tParam;

	tParam.mhStructSync = mutex;
	tParam.game = game;
	tParam.ship = ship;
	tParam.pUp = *pUp;
	

	//Increases the corresponding powerup flag to account for double buffs
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
	case 4:
		game->ice++;
		break;
	case 5:
		game->plusSpeed++;
		break;
	case 6:
		game->battery++;
		break;
	case 7:
		ship->lives++;
		break;
	default:
		break;
	}
	
	//Launches a thread with the powerUp timeout, 
	//kill the ship to exit this thread gracefully (no pointer to ThreadMustGoOn)
	htPowerUpTimer = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size
		PowerUpTimer,								//Thread function name
		(LPVOID)&tParam,							//Thread parameter struct
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
	pUp.type = RandomValue(7);		//Random type between 0 and 6

	pUp.duration = duration;
	return pUp;
}

DWORD WINAPI PowerUps(LPVOID tParam) {

	SMCtrl	*cThread = (SMCtrl*)tParam;
	int		*ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;

	while (*ThreadMustGoOn) {

		//Sleeps for 10 seconds, afterwards flips a coin to break the loop every second
		//This is the timeout between powerUp drops, not the duration
		for (int i = 0; i < 20 && *ThreadMustGoOn; i++) {
			if (i > 10)
				if (RandomValue(2))
					break;
			Sleep(1000 * (*ThreadMustGoOn));
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

			//Tests for collisions and apply the powerup if needed
  			if (cThread->localGameData.pUp.y>(cThread->localGameData.ysize*0.2))
				PowerUpCollision(&cThread->localGameData, &cThread->localGameData.pUp, &cThread->mhStructSync);
			ReleaseMutex(cThread->mhStructSync);

			Sleep(cThread->localGameData.projectiles_speed*(*ThreadMustGoOn));		//Pratical assignment pdf, page 3, 4th rule
		}

		//Hides the power up to restart the loop
		cThread->localGameData.pUp.fired = 0;
	}

	return 0;
}

DWORD WINAPI BombMovement(LPVOID tParam) {

	int			*ThreadMustGoOn = ((BombMoves *)tParam)->TheadmustGoOn;
	HANDLE		*mhStructSync = ((BombMoves*)tParam)->mhStructSync;
	GameData	*baseGame = ((BombMoves *)tParam)->game;
	Invader		*invader = ((BombMoves*)tParam)->invader;

	/*This is maybe not needed... */
	int			bombNum = -1;

	for (int i = 0; i < baseGame->max_bombs; i++) {							//cicle to check if there is available slots to fire a bomb
		if (!invader->bomb[i].fired) {
			bombNum = i;
			break;
		}
	}
	if (bombNum > -1) {
		if (invader->bombRateCounter == 1) {

			WaitForSingleObject(mhStructSync, INFINITE);
			invader->bombRateCounter++;
			invader->bomb[bombNum].x = invader->x;							//give invaders coords to bomb
			invader->bomb[bombNum].y = invader->y;
			invader->bomb[bombNum].fired = 1;								//update bomb status
			ReleaseMutex(mhStructSync);

			while (*ThreadMustGoOn && invader->bomb[bombNum].fired) {

				if (invader->bomb[bombNum].y < baseGame->ysize - 1) {		//if bomb has not reached the end of the play area

					WaitForSingleObject(mhStructSync, INFINITE);
					invader->bomb[bombNum].y++;								//update it's position, an wait for next tick 
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
	HANDLE		mhStructSync = ((SMCtrl *)tParam)->mhStructSync;

	int i, j,index;
	int sidestep = 4;				//hardcoded
	int totalsteps = (baseGame->ysize - (baseGame->max_invaders/ INVADER_BY_ROW)) * sidestep;
	int regInvaderNr = (baseGame->max_invaders - baseGame->max_rand_invaders);

	BombMoves		bombMoves;
	
	while (*ThreadMustGoOn) {						//Thread main loop

		for (i = 0; (i < totalsteps) && *ThreadMustGoOn; i++) {

			for (j = 0; (j < regInvaderNr) && *ThreadMustGoOn; j++) {
				if (!baseGame->invad[j].rand_path && baseGame->invad[j].hp > 0) {

					WaitForSingleObject(mhStructSync, INFINITE);
					if (baseGame->ice == 1) { //##### HERE
						index = i;
						baseGame->invad[j].y = (index / sidestep) + baseGame->invad[j].y_init;				//Invader goes down after n sidesteps

						if ((index % (sidestep * 2)) < sidestep)
							baseGame->invad[j].x = (index % (sidestep * 2)) + baseGame->invad[j].x_init;	//Invader goes right
						else if ((index % (sidestep * 2)) > sidestep)
							baseGame->invad[j].x--;															//Invader goes left
					}
					else {
						i = index;
					}
					//Tests collision
					InvaderCollision(baseGame, &baseGame->invad[j]);

					//Updates bombrate loop counter
					baseGame->invad[j].bombRateCounter = ++baseGame->invad[j].bombRateCounter % baseGame->bombRate;

					ReleaseMutex(mhStructSync);

					if (baseGame->invad[j].bombRateCounter == 1) {

						WaitForSingleObject(mhStructSync, INFINITE);
						bombMoves.game = baseGame;
						bombMoves.invader = &baseGame->invad[j];
						bombMoves.mhStructSync = mhStructSync;
						bombMoves.TheadmustGoOn = ThreadMustGoOn;
						ReleaseMutex(mhStructSync);

						if (!BombLauncher(&bombMoves)) {					/*Bomb thread launcher*/
							_tprintf(TEXT("[Error] placing invader bombs ! \n"));
						}
					}
				}
			}
			Sleep((baseGame->invaders_speed/baseGame->plusSpeed)*(*ThreadMustGoOn));
		}
	}
	return 0;
}

DWORD WINAPI RandPathInvaders(LPVOID tParam) {

	int			* ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	GameData	*baseGame = &((SMCtrl *)tParam)->localGameData;
	HANDLE		mhStructSync = ((SMCtrl *)tParam)->mhStructSync;

	int			i, xTemp, yTemp, invalid, count;
	BombMoves	bombMoves;

	while (*ThreadMustGoOn) {						//Thread main loop

		for (i = (baseGame->max_invaders - baseGame->max_rand_invaders); (i < baseGame->max_invaders) && *ThreadMustGoOn; i++) {
			if (baseGame->invad[i].rand_path && baseGame->invad[i].hp >0) {

				WaitForSingleObject(mhStructSync, INFINITE);
				count = 0;
				if (baseGame->ice == 1) {
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
						}
						else if (count >= 50) {  //if there is no option find another path
							baseGame->invad[i].direction = RandomValue(3) + 4;
							count++;
						}

					} while (invalid && count < 100);
					if (count < 100) {
						baseGame->invad[i].x = xTemp;
						baseGame->invad[i].y = yTemp;
					}
				}
				//Tests collision
				InvaderCollision(baseGame, &baseGame->invad[i]);
				//Updates bombrate loop counter
				baseGame->invad[i].bombRateCounter = ++baseGame->invad[i].bombRateCounter % baseGame->bombRate;

				if (baseGame->invad[i].bombRateCounter == 1) {
					bombMoves.game = baseGame;
					bombMoves.invader = &baseGame->invad[i];
					bombMoves.mhStructSync = mhStructSync;
					bombMoves.TheadmustGoOn = ThreadMustGoOn;
					
					if (!BombLauncher(&bombMoves)) {					/*Bomb thread launcher*/
						_tprintf(TEXT("[Error] placing invaders ships ! \n"));
					}
				}
				ReleaseMutex(mhStructSync);
			}
		}
		Sleep((DWORD)((baseGame->invaders_speed / baseGame->plusSpeed) *0.9)*(*ThreadMustGoOn));
	}

	return 0;
}

DWORD WINAPI PacketListener(LPVOID tParam) {

	SMCtrl		*cThread = (SMCtrl*)tParam;

	ClientMoves move;
	move.game = &cThread->localGameData;
	move.TheadmustGoOn = &cThread->ThreadMustGoOn;
	move.mhStructSync = &cThread->mhStructSync;

	while (cThread->ThreadMustGoOn) {

		//Consume item from buffer (gets a packet with a client instruction)
		move.localPacket = consumePacket(cThread);	//Problem here: No exit condition

		if (move.localPacket.instruction < 5) {						//Instructions [0,1,2,3,4]

			WaitForSingleObject(cThread->mhStructSync, INFINITE);
			UpdateLocalShip(&move);									//Translates instructions into ship actions (movement, shots...)
			ShipCollision(move.game,								//Tests if those actions are valid
				&move.game->ship[move.localPacket.owner],
				cThread->mhStructSync);
			ReleaseMutex(cThread->mhStructSync);
		}
		else if (move.localPacket.instruction<7) {					//Instructions [5,6]

			handShakeClient(&move);									//Manages auth/deauth packets
		}
		else if (move.localPacket.instruction<8) {					//Instructions [7]

			markPlayerReady(&move);
		}

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
				Sleep((baseGame->ship_shot_speed)*(*ThreadMustGoOn));

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

int DefenderShipsCollision(GameData *game, int x, int y, int index) {
	int i;

	for (i = 0; i < game->num_players; i++) {
		if (i != index) {
			if (game->ship[i].lives >= 0 && game->ship[i].x == x && game->ship[i].y == y) {
				return 0;
			}
		}
	}
	return 1;
}

int UpdateLocalShip(ClientMoves *move) {

	DWORD			tShotLauncherID;
	HANDLE			htShotLauncher;
	DWORD			timeNow;

	int xTemp, yTemp;
	int index = move->localPacket.owner;

	xTemp = move->game->ship[index].x;
	yTemp = move->game->ship[index].y;

	//validate action
	switch (move->localPacket.instruction) {
	case 0:
		if (move->game->ship[index].x < (move->game->xsize - 1) && move->game->ship[index].lives >= 0)
			if (DefenderShipsCollision(move->game, ++xTemp, yTemp, index)) {
				move->game->ship[index].x++;
			}
		break;
	case 1:
		if (move->game->ship[index].y<(move->game->ysize - 1) && move->game->ship[index].lives >= 0)
			if (DefenderShipsCollision(move->game, xTemp, ++yTemp, index)) {
				move->game->ship[index].y++;
			}
		break;
	case 2:
		if (move->game->ship[index].x>0 && move->game->ship[index].lives >= 0)
			if (DefenderShipsCollision(move->game, --xTemp, yTemp, index)) {
				move->game->ship[index].x--;
			}
		break;
	case 3:
		if (move->game->ship[index].y>(move->game->ysize - (move->game->ysize*0.2)) && move->game->ship[index].lives >= 0)
			if (DefenderShipsCollision(move->game, xTemp, --yTemp, index)) {
				move->game->ship[index].y--;
			}
		break;
	case 4:
		/* calculate time in millisenconds since last shot is fired*/
		timeNow = GetTickCount();
		if (timeNow - (move->game->ship[index].shotTimeStamp) >= (DWORD)(move->game->shotRate / move->game->battery)) {
			move->game->ship[index].shotTimeStamp = timeNow;

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
		}
		break;

	default:
		break;
	}

	return 0;
}

//int SystemTimeStamp(DWORD* timeStamp) {
//	/*
//	Creates a timestamp
//	Used to display top 10
//	*/
//	SYSTEMTIME time;
//	GetSystemTime(&time);									//Populates structure with system time
//
//	_stprintf_s(timeStamp,
//		SMALL_BUFF,
//		TEXT("(%02d:%02d %02d/%02d/%d)"),					//Copies the values with the format (HH:MM DD/MM/YYY)
//		time.wHour, time.wMinute,
//		time.wDay, time.wMonth, time.wYear);
//
//	return 0;
//}

int InstantiateGame(GameData *game) {
	int i, j;
	/*Ambient variables*/
	game->xsize = XSIZE;
	game->ysize = YSIZE;

	game->invaders_speed =		INVADER_SPEED;				// Base speed for invader
	game->max_bombs =			MAX_BOMBS;					// Base max num of bombs at same time
	game->max_invaders =		MAX_INVADER;				// Base num of invaders in the field
	game->max_rand_invaders =	RAND_INVADER;				// Base num of invaders in the field
	game->num_players =			MAX_PLAYERS;				// Base num of players
	game->ship_shot_speed =		PROJECTL_SPEED;				// Base speed for defender ship
	game->projectiles_speed =	PROJECTL_SPEED;				// Base speed for Powerups and invader bombs
	game->pup_duration =		POWERUP_DUR;				// Base power up duration
	game->bombRate =			BOM_BRATE;					// Base steps until bomb launch
	game->shotRate =			SHOT_RATE;
	game->plusSpeed = 1;
	game->ice = 1;
	game->battery = 1;

	/*Bombs*/
	for (i = 0; i < game->max_invaders; i++) {				//Instantiates all bombs outside of game and updates the status
		for (j = 0; j < game->max_bombs; j++) {
			ResetBomb(&game->invad[i].bomb[j]);
		}
		game->invad[i].bombRateCounter = RandomValue(10);
	}
	/*Ships & shots*/
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
		game->ship[i].shotTimeStamp = 0;
		//...
	}

	return 0;
}

int ShipCollision(GameData *game, Ship *ship, HANDLE mhStructSync) {

	int i,j;

	if (ship->lives >= 0) {

		//Check for collision with invaders
		for (i = 0; i < game->max_invaders; i++) {
			if (game->invad[i].x == ship->x && game->invad[i].y == ship->y && game->invad[i].hp > 0) {
				DamageInvader(&game->invad[i]);
				DamageShip(ship);
				return 1;
			}
		}

		//Check for collisions with bombs
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

		//Check for collisions with powerups
		if (game->pUp.x == ship->x && game->pUp.y == ship->y && game->pUp.fired == 1) {
			ShipPowerUpCollision(game, ship, &game->pUp, mhStructSync);
			return 1;
		}
	}
	return 0;
}

int ShotCollision(GameData *game, ShipShot *shot) {

	int i,j;

	if (shot->fired) {

		//Tests shot collision with invaders
		for (i = 0; i < game->max_invaders; i++) {

			if (game->invad[i].hp > 0 &&
				game->invad[i].x == shot->x && 
				game->invad[i].y == shot->y ) {

				DamageInvader(&game->invad[i]);
				ResetShot(shot);
				return 1;
			}

			//Tests shot collision with bombs
			for (j = 0; j < game->max_bombs; j++) {

				if (game->invad[i].bomb[j].fired &&
					game->invad[i].bomb[j].x == shot->x && 
					game->invad[i].bomb[j].y == shot->y ) {

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

int PowerUpCollision(GameData * game, PowerUp *pUp, HANDLE mhStructSync) {
	int j;
	if (pUp->y>(game->ysize*0.2)) {
		for (j = 0; j < game->num_players && pUp->fired; j++) {
			if (pUp->x == game->ship[j].x && pUp->y == game->ship[j].y) {

				PowerUpShip(game, &game->ship[j],	pUp, mhStructSync);
				pUp->fired = 0;
				return 1;
			}
		}
	}
	return 0;
}

int ShipPowerUpCollision(GameData * game, Ship * ship, PowerUp *pUp, HANDLE mhStructSync) {
	int j;
	if (pUp->y>(game->ysize*0.2)) {
		for (j = 0; j < game->num_players && pUp->fired; j++) {
			if (pUp->x == ship->x && pUp->y == ship->y) {

				PowerUpShip(game, ship, pUp, mhStructSync);
				pUp->fired = 0;
				return 1;
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
		*y = (int)(game->ysize*0.8);
	}
	if (*y > game->ysize*0.8 ) {
		*y = 0;
	}
	return 0;
}

int BombLauncher(BombMoves *bombMoves) {
	DWORD			tBombLauncherID;
	HANDLE			htBombLauncher;
	
	htBombLauncher = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size
		BombMovement,								//Thread function name
		bombMoves,									//Thread parameter struct
		0,											//Creation flags
		&tBombLauncherID);							//gets thread ID 
	if (htBombLauncher == NULL) {
		_tprintf(TEXT("[Error] Creating thread htBombLauncher (%d) at server\n"), GetLastError());
		return 0;
	}
	return 1;
}




