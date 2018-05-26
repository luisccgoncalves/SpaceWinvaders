#include "game.h"

DWORD WINAPI StartGame(LPVOID tParam) {

	int * ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	//GameData *baseGame = ((SMCtrl *)tParam)->pSMemGameData;
	
	Game *baseGame = &((SMCtrl *)tParam)->game;
	//InstantiateGame(baseGame);

	/*
	HERE we will need to CREATE a game and instantiate a lvl ands stuffs
	*/

	DWORD			tRegPathInvaderID;
	HANDLE			htRegPathInvader;
	DWORD			tRandPathInvaderID;
	HANDLE			htRandPathInvader;
	DWORD			tInvadersBombID;
	HANDLE			htInvadersBomb;

	int i;

	srand((unsigned)time(NULL));					//Seeds the RNG

													//Defines invader path
	for (i = 0; (i < baseGame->max_invaders) && *ThreadMustGoOn; i++) {
		if (i < (baseGame->max_invaders - baseGame->max_rand_invaders))
			baseGame->gameData.invad[i].rand_path = 0;
		else
			baseGame->gameData.invad[i].rand_path = 1;
	}

	//Populates invaders with coords
	for (i = 0; ((i < baseGame->max_invaders) && *ThreadMustGoOn); i++) {

		if (!(baseGame->gameData.invad[i].rand_path)) {			//If regular path

													//deploys INVADER_BY_ROW invaders per line with a spacing of 2
			baseGame->gameData.invad[i].x = baseGame->gameData.invad[i].x_init = (i % INVADER_BY_ROW) * 2;

			//Deploys 5 lines of invaders (MAX_INVADER/11=5)
			baseGame->gameData.invad[i].y = baseGame->gameData.invad[i].y_init = i / INVADER_BY_ROW;
		}
		else {
			baseGame->gameData.invad[i].x = baseGame->gameData.invad[i].x_init = rand() % baseGame->gameData.xsize;
			baseGame->gameData.invad[i].y = baseGame->gameData.invad[i].y_init = rand() % baseGame->gameData.ysize;
		}
	}

	//Populates invaders with HP
	for (i = 0; ((i < baseGame->max_invaders) && *ThreadMustGoOn); i++) {
		baseGame->gameData.invad[i].hp = 1;
	}

	//Kills a random invader ##### For testing purposes #####
	baseGame->gameData.invad[rand() % 55].hp = 0;

	//Populates ships ######## NEEDS TO BE UPDATED TO MULTIPLAYER #########
	for (i = 0; i < baseGame->num_players; i++) {

		baseGame->gameData.ship[i].x = 15;
		baseGame->gameData.ship[i].y = 23;
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

	WaitForSingleObject(htRegPathInvader, INFINITE);
	WaitForSingleObject(htRandPathInvader, INFINITE);
	WaitForSingleObject(htInvadersBomb, INFINITE);

	return 0;

}

DWORD WINAPI GameTick(LPVOID tParam) {				//Warns gateway of structure updates

	GTickStruct		*sGTick = (GTickStruct*)tParam;
	GameData *local = sGTick->localGameData;

	while (sGTick->ThreadMustGoOn) {

		Sleep(100);
		_tprintf(TEXT("."));
		WaitForSingleObject(sGTick->mhStructSync, INFINITE);

		CopyMemory(sGTick->smGameData, local, sizeof(GameData));
		/*
		Here we will write to the SharedMemory
		...
		working on it!
		*/
		SetEvent(sGTick->hTick);

		ReleaseMutex(sGTick->mhStructSync);
	}

	return 0;
}

int UpdateLocalShip(GameData *game, Packet *localpacket) {
	//validate action

	//_tprintf(TEXT("[DEBUG] UpdateLocalShip (%d) \n"), localpacket->instruction);

	/*
	this is recieving a value from 0 to 3
	maybe this is not the final values
	*/

	switch (localpacket->instruction) {
	case 0:
		if (game->ship->x<(game->xsize - 1))
			game->ship->x++;
		break;
	case 1:
		if (game->ship->y<(game->ysize - 1))
			game->ship->y++;
		break;
	case 2:
		if (game->ship->x>0)
			game->ship->x--;
		break;
	case 3:
		if (game->ship->y>(game->ysize - (game->ysize*0.2)))
			if (game->ship->y>(game->ysize - (game->ysize*0.2)))
				game->ship->y--;
		break;
	default:
		break;
	}
	return 0;
}

int InstantiateGame(Game *game) {

	/*
	this is for filling the game, 
	that then will be altered later
	...
	another function?
	*/

	game->gameData.xsize = XSIZE;
	game->gameData.ysize = YSIZE;

	game->invaders_bombs_speed =	INVADER_SPEED;		// not correct
	game->invaders_speed =			INVADER_SPEED;		// Base speed for invader
	game->max_bombs =				MAX_BOMBS;			// Base max num of bombs at same time
	game->max_invaders =			MAX_INVADER;		// Base num of invaders in the field
	game->max_rand_invaders	=		RAND_INVADER;		// Base num of invaders in the field
	game->num_players =				MAX_PLAYERS;		// Base num of players
	game->power_up_speed =			INVADER_SPEED;		// Base speed for power up
	game->ship_shot_speed =			INVADER_SPEED;		// Base speed for defender ship

	return 0;
}