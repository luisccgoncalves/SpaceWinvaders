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
		} while (baseGame->invad[invPosition].hp <= 0);

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
	HANDLE		*mhStructSync = ((SMCtrl *)tParam)->mhStructSync;

	int i, j;
	int sidestep = 4;
	int totalsteps = (baseGame->ysize - (baseGame->max_invaders/ INVADER_BY_ROW)) * sidestep;
	int regInvaderNr = (baseGame->max_invaders - baseGame->max_rand_invaders);


	while (*ThreadMustGoOn) {						//Thread main loop

		for (i = 0; (i < totalsteps) && *ThreadMustGoOn; i++) {

			WaitForSingleObject(mhStructSync, INFINITE);

			for (j = 0; (j < regInvaderNr) && *ThreadMustGoOn; j++) {
				if (!baseGame->invad[j].rand_path && baseGame->invad[i].hp > 0) {

					baseGame->invad[j].y = (i / sidestep) + baseGame->invad[j].y_init;				//Invader goes down after n sidesteps

					if ((i % (sidestep * 2)) < sidestep)
						baseGame->invad[j].x = (i % (sidestep * 2)) + baseGame->invad[j].x_init;		//Invader goes right
					else if ((i % (sidestep * 2)) > sidestep)
						baseGame->invad[j].x--;													//Invader goes left
					InvaderCollision(baseGame, &baseGame->invad[j]);
				}
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

	int i, xTemp, yTemp, invalid, count;

	while (*ThreadMustGoOn) {						//Thread main loop

		WaitForSingleObject(mhStructSync, INFINITE);

		for (i = 0; (i < baseGame->max_invaders) && *ThreadMustGoOn; i++) {
			if (baseGame->invad[i].rand_path && baseGame->invad[i].hp >0) {
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
			}
		}
		ReleaseMutex(mhStructSync);
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

	int			*ThreadMustGoOn = ((ClientMoves *)tParam)->TheadmustGoOn;
	GameData	*baseGame = ((ClientMoves *)tParam)->game;
	int			owner = ((ClientMoves*)tParam)->localPacket.owner;
	HANDLE		*mhStructSync = ((ClientMoves*)tParam)->mhStructSync;
	int			shotNum = -1;
		
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

			while (*ThreadMustGoOn && baseGame->shot[shotNum].fired) {

				if (baseGame->shot[shotNum].y > 0) {						//if bomb has not reached the end of the play area

					WaitForSingleObject(mhStructSync, INFINITE);
					baseGame->shot[shotNum].y--;							//update it's position, an wait for next tick 
					ShotCollision(baseGame, &baseGame->shot[shotNum]);
					ReleaseMutex(mhStructSync);
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
	HANDLE			htShotLauncher;  
	
	//validate action
	switch (move->localPacket.instruction) {
	case 0:
		if (move->game->ship[move->localPacket.owner].x < (move->game->xsize - 1) && move->game->ship[move->localPacket.owner].lives >= 0)
			move->game->ship[move->localPacket.owner].x++;
		break;
	case 1:
		if (move->game->ship[move->localPacket.owner].y<(move->game->ysize - 1) && move->game->ship[move->localPacket.owner].lives >= 0)
			move->game->ship[move->localPacket.owner].y++;
		break;
	case 2:
		if (move->game->ship[move->localPacket.owner].x>0 && move->game->ship[move->localPacket.owner].lives >= 0)
			move->game->ship[move->localPacket.owner].x--;
		break;
	case 3:
		if (move->game->ship[move->localPacket.owner].y>(move->game->ysize - (move->game->ysize*0.2)) && move->game->ship[move->localPacket.owner].lives >= 0)
			move->game->ship[move->localPacket.owner].y--;
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

	for (i = 0; i < game->max_bombs; i++) {			//Instantiates all bombs outside of game and updates the status
		ResetBomb(&game->bomb[i]);
	}

	for (i = 0; i < MAX_SHOTS; i++) {				//Instantiates all shots outside of game and updates the status
		ResetShot(&game->shot[i]);
	}

	for (i = 0; i < game->num_players; i++) {
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
	if (ship->lives >= 0) {
		for (i = 0; i < game->max_bombs; i++) {
			if (game->bomb[i].x == ship->x && game->bomb[i].y == ship->y && game->bomb[i].fired) { //ERROR?
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
	}
	return 0;
}

int ShotCollision(GameData *game, ShipShot *shot) {
	int i = 0;
	if (shot->fired) {
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
	}
	return 0;
}

int InvaderCollision(GameData * game, Invader * invader)
{
	int i = 0;
	if (invader->hp > 0) {
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
	return 0;
}

int BombCollision(GameData * game, InvaderBomb * bomb)
{
	int i = 0;
	if (bomb->fired) {
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
	return 0;
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

int sortTop10(HighScore *top10) {
	/*
	Regular bubble sort algorithm
	Values are stored alphabetically in the registry
	*/

	HighScore temp;
	int i, j;

	for (i = 0; i < 9; ++i) 
		for (j = 0; j < 9 - i; ++j) {
			if (top10[j].score < top10[j + 1].score) {
				temp = top10[j + 1];
				top10[j + 1] = top10[j];
				top10[j] = temp;
			}
		}
	return 0;
}
int writeTop10ToReg(HighScore *top10) {

	HKEY	key;
	DWORD	dwDisposition;
	LONG	lResult;

	lResult = RegDeleteKeyEx(							//Deletes the HighScores subkey
		HKEY_CURRENT_USER,
		REG_SUBKEY,
		KEY_WOW64_32KEY,
		0);
	if (lResult != ERROR_SUCCESS) {
		_tprintf(TEXT("[Error] Deleting registry key (%d)\n"), GetLastError());
		return -1;
	}

	lResult = RegCreateKeyEx(							//Creates a new, blank subkey
		HKEY_CURRENT_USER,								//A handle to an open registry key.
		REG_SUBKEY,										//The name of a subkey that this function opens or creates.
		0,												//This parameter is reserved and must be zero.
		NULL,											//The user-defined class type of this key.
		REG_OPTION_NON_VOLATILE,						//Options: This key is not volatile, information is preserved after restart.
		KEY_SET_VALUE,									//A mask that specifies the access rights for the key to be created.
		NULL,											//Inheritance: NULL= Not inherited
		&key,											//A pointer to a variable that receives a handle to the opened or created key.
		&dwDisposition);								//A pointer to a variable that receives REG_CREATED_NEW_KEY or REG_OPENED_EXISTING_KEY
	if (lResult != ERROR_SUCCESS) {
		_tprintf(TEXT("[Error] Creating registry key (%d)\n"), GetLastError());
		return -1;
	}

	for (int i = 0; i < 10 && top10[i].score!=0; i++) {
		lResult=RegSetValueEx(							//Stores Username:score in the previously created key.
			key,										//A handle to an open registry key.
			top10[i].timestamp,							//The name of the value to be set.
			0,											//This parameter is reserved and must be zero.
			REG_DWORD,									//The type of data pointed to by the lpData parameter
			(LPBYTE)&top10[i].score,					//The data to be stored.
			sizeof(DWORD));								//The size of the information pointed to by the lpData parameter, in bytes.
		if (lResult != ERROR_SUCCESS) {
			_tprintf(TEXT("[Error] Creating registry key (%d)\n"), GetLastError());
			return -1;
		}
	}
																								
	RegCloseKey(key);									//Closes the key

	return 0;
}

int readTop10FromReg(HighScore * top10) {

	HKEY	key;
	TCHAR	lpValueName[SMALL_BUFF];
	DWORD	lpData, lpType;
	DWORD	szValueNameRead, szValueName= SMALL_BUFF * sizeof(TCHAR);
	DWORD	szDataRead, szData=sizeof(DWORD);
	DWORD	nRegValues;
	LONG	lResult;
	DWORD	dwDisposition;

	//Populates top 10 with no scores
	for (int i = 0; i < 10; i++) {
		_tcscpy_s(top10[i].timestamp, szValueName, TEXT("NO RECORD"));
		top10[i].score = 0;
	}

	//Opens a key for writing, if non existant, creates a new one
	lResult = RegCreateKeyEx(
		HKEY_CURRENT_USER,									//A handle to an open registry key.
		REG_SUBKEY,											//The name of a subkey that this function opens or creates.
		0,													//This parameter is reserved and must be zero.
		NULL,												//The user-defined class type of this key.
		REG_OPTION_NON_VOLATILE,							//Options: This key is not volatile, information is preserved after restart.
		KEY_READ,											//A mask that specifies the access rights for the key to be created.
		NULL,												//Inheritance: NULL= Not inherited
		&key,												//A pointer to a variable that receives a handle to the opened or created key.
		&dwDisposition);									//A pointer to a variable that receives REG_CREATED_NEW_KEY or REG_OPENED_EXISTING_KEY
	if (lResult != ERROR_SUCCESS) {
		_tprintf(TEXT("[Error] Creating registry key (%d)\n"), GetLastError());
		return -1;
	}

	if (dwDisposition == REG_CREATED_NEW_KEY) {				//Registry was empty, nothing to copy
		RegCloseKey(key);
		return 0;											//Returns an empty top 10
	}

	//Query how many values the subkey has
	RegQueryInfoKey(
		key, NULL, NULL, NULL, NULL, NULL, NULL,			//Key to check
		&nRegValues, NULL, NULL, NULL, NULL);				//Pointer to be filled with the number of values stored

	//Reads nRegvalues from registry to Top10 
	for (int i = 0; lResult == ERROR_SUCCESS && i<(int)nRegValues; i++) {
		szValueNameRead = szValueName;
		szDataRead = szData;
		lResult = RegEnumValue(
			key,											//A handle to an open registry key.
			i,												//The index of the value to be retrieved.
			lpValueName, 									//A pointer to a buffer that receives the name of the value as a null-terminated string. 
			&szValueNameRead, 								//Specifies the size of the buffer pointed to by the lpValueName parameter, in characters.
			NULL, 											//Reserved
			&lpType, 										//A pointer to a variable that receives a code indicating the type of data
			(LPBYTE)&lpData,								//A pointer to a buffer that receives the data for the value entry.
			&szDataRead);									//A pointer to a variable that specifies the size of the buffer pointed to by the lpData parameter, in bytes.
		if (lpType != REG_DWORD) {							//Ignores all non DWORD values
			i--;
			nRegValues--;
		}else if (lResult == ERROR_SUCCESS) {				//Copy the value to Top10
			_tcscpy_s(top10[i].timestamp, 
				szValueNameRead+1,							//Adding 1 to szValueNameRead to account for NULL terminator
				lpValueName);		
			top10[i].score = lpData;
		}
	}

	RegCloseKey(key);

	sortTop10(top10);										//Sorts the array by score

	return 0;
}

int SystemTimeString(TCHAR * timeString) {
	/*
	Creates a timestamp
	Used to display top 10
	*/
	SYSTEMTIME time;

	GetLocalTime(&time);									//Populates structure with local time
	_stprintf_s(timeString,									
		SMALL_BUFF,
		TEXT("(%02d:%02d %02d/%02d/%d)"),					//Copies the values with the format (HH:MM DD/MM/YYY)
		time.wHour, time.wMinute, 
		time.wDay, 	time.wMonth, time.wYear);

	return 0;
}

int addScoretoTop10(int score, HighScore * top10) {

	int i;

	for (i=9; (i >= 0) && ((DWORD)score > top10[i].score) ; i--) {
		top10[i]=top10[i - 1];								//Finds a space for the new highscore
	}

	if (i < 9) {
		SystemTimeString(top10[i + 1].timestamp);			//Gets a timestamp for posterity
		top10[i + 1].score = score;							//Copies current score to top10
		writeTop10ToReg(top10);								//Write top10 to registry
		return 0;											//Return 0 if "New Highscore!"
	}
	else
		return 1;											//Return 1 if no new highscore
}



