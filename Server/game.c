#include "game.h"
#include "..\Client\debug.h"

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
			baseGame->invad[i].x = baseGame->invad[i].x_init = RandomValue(baseGame->xsize);
			baseGame->invad[i].y = baseGame->invad[i].y_init = RandomValue(baseGame->ysize);
		}
	}

	//Populates invaders with HP
	for (i = 0; ((i < baseGame->max_invaders) && *ThreadMustGoOn); i++) {
		baseGame->invad[i].hp = 1;
	}

	//Kills a random invader ##### For testing purposes #####
	baseGame->invad[RandomValue(55)].hp = 0;

	//Populates ships ######## NEEDS TO BE UPDATED TO MULTIPLAYER #########
	for (i = 0; i < baseGame->num_players; i++) {

		baseGame->ship[i].x = 15;
		baseGame->ship[i].y = 23;
		baseGame->ship[i].lives = 0;
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
void spinnerSlash() {
	POINT			pt;
	static int		i = 0;
	char			spiner[8] = { '/','-','\\','|','/','-','\\','|' };

	GetCursorPos(&pt);
	gotoxy(79, 0);

	_tprintf(TEXT("%c"), spiner[i++]);
	gotoxy((int)pt.x, (int)pt.y);
	i = i% 8;

}
DWORD WINAPI GameTick(LPVOID tParam) {				//Warns gateway of structure updates

	GTickStruct		*sGTick = (GTickStruct*)tParam;

	while (sGTick->ThreadMustGoOn) {

		Sleep(50);
		spinnerSlash();
		FullCollision(sGTick->localGameData);

		writeGameData(sGTick->smGameData, sGTick->localGameData, sGTick->mhGameData);

		SetEvent(sGTick->hTick);
	}
	return 0;
}

