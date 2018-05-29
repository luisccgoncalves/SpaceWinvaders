#include "game.h"

DWORD WINAPI StartGame(LPVOID tParam) {

	int * ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	//GameData *baseGame = ((SMCtrl *)tParam)->pSMemGameData;
	
	GameData *baseGame = &((SMCtrl *)tParam)->localGameData;
	InstantiateGame(baseGame);

	//srand((unsigned)time(NULL));

	/*
	HERE we will need to CREATE a game and instantiate a lvl ands stuffs
	*/

	DWORD			tRegPathInvaderID;
	HANDLE			htRegPathInvader;
	DWORD			tRandPathInvaderID;
	HANDLE			htRandPathInvader;
	DWORD			tInvadersBombID;
	HANDLE			htInvadersBomb;
	DWORD			tShipShotsID;
	HANDLE			htShipShots;

	int i;

	

	//Defines invader path
	for (i = 0; (i < baseGame->max_invaders) && *ThreadMustGoOn; i++) {
		if (i < (baseGame->max_invaders - baseGame->max_rand_invaders))
			baseGame->invad[i].rand_path = 0;
		else
			baseGame->invad[i].rand_path = 1;
	}

	//Populates invaders with coords
	for (i = 0; ((i < baseGame->max_invaders) && *ThreadMustGoOn); i++) {

		if (!(baseGame->invad[i].rand_path)) {			//If regular path

													//deploys INVADER_BY_ROW invaders per line with a spacing of 2
			baseGame->invad[i].x = baseGame->invad[i].x_init = (i % INVADER_BY_ROW) * 2;

			//Deploys 5 lines of invaders (MAX_INVADER/11=5)
			baseGame->invad[i].y = baseGame->invad[i].y_init = i / INVADER_BY_ROW;
		}
		else {
			baseGame->invad[i].x = baseGame->invad[i].x_init = rand() % baseGame->xsize;
			baseGame->invad[i].y = baseGame->invad[i].y_init = rand() % baseGame->ysize;
		}
	}

	//Populates invaders with HP
	for (i = 0; ((i < baseGame->max_invaders) && *ThreadMustGoOn); i++) {
		baseGame->invad[i].hp = 1;
	}

	//Kills a random invader ##### For testing purposes #####
	baseGame->invad[rand() % 55].hp = 0;

	//Populates ships ######## NEEDS TO BE UPDATED TO MULTIPLAYER #########
	for (i = 0; i < baseGame->num_players; i++) {

		baseGame->ship[i].x = 15;
		baseGame->ship[i].y = 23;
	}

	htRegPathInvader = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size
		RegPathInvaders,							//Thread function name
		tParam,										//Thread parameter struct
		0,											//Creation flags
		&tRegPathInvaderID);						//gets thread ID to close it afterwards

	htRandPathInvader = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size
		RandPathInvaders,							//Thread function name
		tParam,										//Thread parameter struct
		0,											//Creation flags
		&tRandPathInvaderID);						//gets thread ID to close it afterwards

	htInvadersBomb = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size
		InvadersBomb,								//Thread function name
		tParam,										//Thread parameter struct
		0,											//Creation flags
		&tInvadersBombID);							//gets thread ID to close it afterwards

	htShipShots = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size
		ShipShots,								//Thread function name
		tParam,										//Thread parameter struct
		0,											//Creation flags
		&tShipShotsID);							//gets thread ID to close it afterwards

	WaitForSingleObject(htRegPathInvader, INFINITE);
	WaitForSingleObject(htRandPathInvader, INFINITE);
	WaitForSingleObject(htInvadersBomb, INFINITE);
	WaitForSingleObject(htShipShots, INFINITE);

	return 0;

}

DWORD WINAPI GameTick(LPVOID tParam) {				//Warns gateway of structure updates

	GTickStruct		*sGTick = (GTickStruct*)tParam;
	//GameData *local = sGTick->localGameData;

	while (sGTick->ThreadMustGoOn) {

		Sleep(50);
		_tprintf(TEXT("."));

		writeGameData(sGTick->smGameData, sGTick->localGameData, sGTick->mhGameData);

		SetEvent(sGTick->hTick);

		//	WaitForSingleObject(sGTick->mhGameData, INFINITE);
		//	CopyMemory(sGTick->smGameData, sGTick->localGameData, sizeof(GameData));
		//	SetEvent(sGTick->hTick);
		//	ReleaseMutex(sGTick->mhGameData);
	}
	return 0;
}

int UpdateLocalShip(GameData *game, Packet *localpacket) {
	//validate action

	//_tprintf(TEXT("[DEBUG] UpdateLocalShip (%d) \n"), localpacket->instruction);

	/*
	this is recieving a value from 0 to 3
	maybe this is not the final values
	...
	[IMPORTANT] Consider that we don't know if localpacket->owner
	correspond to the correct array index.

	*/

	switch (localpacket->instruction) {
	case 0:
		if (game->ship[localpacket->owner].x<(game->xsize - 1))
			game->ship[localpacket->owner].x++;
		break;
	case 1:
		if (game->ship[localpacket->owner].y<(game->ysize - 1))
			game->ship[localpacket->owner].y++;
		break;
	case 2:
		if (game->ship[localpacket->owner].x>0)
			game->ship[localpacket->owner].x--;
		break;
	case 3:
		if (game->ship[localpacket->owner].y>(game->ysize - (game->ysize*0.2)))
			game->ship[localpacket->owner].y--;
		break;
	default:
		break;
	}
	return 0;
}

int InstantiateGame(GameData *game) {

	/*
	this is for filling the game, 
	that then will be altered later
	...
	another function?
	*/

	game->xsize = XSIZE;
	game->ysize = YSIZE;

	game->invaders_bombs_speed =	INVADER_SPEED;		// not correct
	game->invaders_speed =			INVADER_SPEED;		// Base speed for invader
	game->max_bombs =				MAX_BOMBS;			// Base max num of bombs at same time
	game->max_invaders =			MAX_INVADER;		// Base num of invaders in the field
	game->max_rand_invaders	=		RAND_INVADER;		// Base num of invaders in the field
	game->num_players =				MAX_PLAYERS;		// Base num of players
	game->power_up_speed =			INVADER_SPEED;		// Base speed for power up
	game->ship_shot_speed =			INVADER_SPEED;		// Base speed for defender ship

	for (int i = 0; i < game->max_bombs; i++) {			//Instantiates all bombs outside of game and updates the status
		game->bomb[i].x = game->xsize + 1;
		game->bomb[i].y = game->ysize + 1;
		game->bomb[i].fired = 0;
	}

	for (int i = 0; i < MAX_SHOTS; i++) {			//Instantiates all bombs outside of game and updates the status
		game->shot[i].x = -1;		//maybe -1
		game->shot[i].y = -1;		//maybe -1
		game->shot[i].fired = 0;
	}

	return 0;
}