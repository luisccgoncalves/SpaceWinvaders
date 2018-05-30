#include "game.h"

DWORD WINAPI StartGame(LPVOID tParam) {

	int * ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	GameData *baseGame = &((SMCtrl *)tParam)->localGameData;

	InstantiateGame(baseGame);

	DWORD			tRegPathInvaderID;
	HANDLE			htRegPathInvader;
	DWORD			tRandPathInvaderID;
	HANDLE			htRandPathInvader;
	DWORD			tInvadersBombID;
	HANDLE			htInvadersBomb;

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

	WaitForSingleObject(htRegPathInvader, INFINITE);
	WaitForSingleObject(htRandPathInvader, INFINITE);
	WaitForSingleObject(htInvadersBomb, INFINITE);

	return 0;
}

DWORD WINAPI GameTick(LPVOID tParam) {				//Warns gateway of structure updates

	GTickStruct		*sGTick = (GTickStruct*)tParam;

	while (sGTick->ThreadMustGoOn) {

		Sleep(50);
		_tprintf(TEXT("."));

		FullCollision(sGTick->localGameData);

		writeGameData(sGTick->smGameData, sGTick->localGameData, sGTick->mhGameData);

		SetEvent(sGTick->hTick);
	}
	return 0;
}

