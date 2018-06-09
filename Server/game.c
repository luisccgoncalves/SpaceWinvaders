#include "game.h"
#include "..\Client\debug.h"

DWORD WINAPI StartGame(LPVOID tParam) {

	int			*ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	GameData	*baseGame = &((SMCtrl *)tParam)->localGameData;
	
	/*Thread Management*/
	DWORD			tRegPathInvaderID;
	HANDLE			htRegPathInvader;

	DWORD			tRandPathInvaderID;
	HANDLE			htRandPathInvader;

	DWORD			tPowerUpsID;
	HANDLE			htPowerUps;

	int i;

	/* Fills bombMoves structure */
	BombMoves	bombMoves;
	bombMoves.mhStructSync = &((SMCtrl *)tParam)->mhStructSync;
	bombMoves.TheadmustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	bombMoves.game = baseGame;
	
	InstantiateGame(baseGame); /*Needs work*/ /*Move into level later*/
	
	if (DefineInvadersType(baseGame, ThreadMustGoOn)) {				//Defines invader path
		_tprintf(TEXT("[Error] Error defining invaders path! \n"));
	}

	if (GiveInvadersHP(baseGame, ThreadMustGoOn)) {					//Gives invaders HP
		_tprintf(TEXT("[Error] Error giving invaders HP! \n"));
	}

	/*This needs thought and atomization*/
	//Populates invaders with coords
	for (i = 0; ((i < baseGame->max_invaders) && *ThreadMustGoOn); i++) {

		if (!(baseGame->invad[i].rand_path)) {			//If regular path

													//deploys INVADER_BY_ROW invaders per line with a spacing of 2
			baseGame->invad[i].x = baseGame->invad[i].x_init = (i % INVADER_BY_ROW) * 2;

			//Deploys 5 lines of invaders (MAX_INVADER/11=5)
			baseGame->invad[i].y = baseGame->invad[i].y_init = i / INVADER_BY_ROW;
		}
		else {
			do {
				baseGame->invad[i].x = baseGame->invad[i].x_init = RandomValue((baseGame->xsize / 2)) + (baseGame->xsize / 3);
				baseGame->invad[i].y = baseGame->invad[i].y_init = RandomValue(baseGame->ysize);
				baseGame->invad[i].direction = RandomValue(3);

			} while (ValidateInvaderPosition(baseGame, baseGame->invad[i].x, baseGame->invad[i].y, i));
		}
	}



	//for (i = 0; ((i < baseGame->max_invaders) && *ThreadMustGoOn); i++) {
	//	bombMoves.invader = &baseGame->invad[i];

	//	htInvadersBomb[i] = CreateThread(
	//		NULL,										//Thread security attributes
	//		0,											//Stack size
	//		InvadersBomb,								//Thread function name
	//		&bombMoves,									//Thread parameter struct
	//		0,											//Creation flags
	//		&tInvadersBombID);							//gets thread ID 
	//}

	//Populates ships ######## NEEDS TO BE UPDATED TO MULTIPLAYER #########
	for (i = 0; i < baseGame->num_players; i++) {

		baseGame->ship[i].x = i+RandomValue(baseGame->xsize/4);
		baseGame->ship[i].y = 23;
	}

	htRegPathInvader = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size
		RegPathInvaders,							//Thread function name
		tParam,										//Thread parameter struct
		0,											//Creation flags
		&tRegPathInvaderID);						//gets thread ID 

	htRandPathInvader = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size
		RandPathInvaders,							//Thread function name
		tParam,										//Thread parameter struct
		0,											//Creation flags
		&tRandPathInvaderID);						//gets thread ID 

	htPowerUps = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size
		PowerUps,									//Thread function name
		tParam,										//Thread parameter struct
		0,											//Creation flags
		&tPowerUpsID);							//gets thread ID 

	WaitForSingleObject(htRegPathInvader, INFINITE);
	WaitForSingleObject(htRandPathInvader, INFINITE);
	//WaitForMultipleObjects(MAX_INVADER, htInvadersBomb, TRUE, INFINITE);

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

		writeGameData(sGTick->smGameData, sGTick->localGameData, sGTick->mhGameData);

		SetEvent(sGTick->hTick);
	}
	return 0;
}

