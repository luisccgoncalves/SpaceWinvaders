#include "game.h"
#include "..\Client\debug.h"

DWORD WINAPI StartLobby(LPVOID tParam) {

	int			*ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	SMCtrl		*cThread = (SMCtrl*)tParam;

	Packet		lPacket;
	//int			nextOut = 0;	//Possible problem here (move this to cThread?)
	int			i;

	while (*ThreadMustGoOn) {

		lPacket = consumePacket(cThread);

		if (lPacket.instruction == 5) {

			_tprintf(TEXT("Received a Handshake request!\n"));

			WaitForSingleObject(cThread->mhStructSync, INFINITE);
			for (i = MAX_PLAYERS; i >=0; i--) {
				if (lPacket.Id == cThread->localGameData.logged[i].Id)
					break;
			}
			ReleaseMutex(cThread->mhStructSync);

			if (i >= 0) {
				_tprintf(TEXT("User already logged.\n"));
			}
			else {

				WaitForSingleObject(cThread->mhStructSync, INFINITE);
				for (i = 0; i < MAX_PLAYERS; i++) {
					if (cThread->localGameData.logged[i].Id == 0) {
						cThread->localGameData.logged[i].Id = lPacket.Id;
						_tcscpy_s(cThread->localGameData.logged[i].username, SMALL_BUFF, lPacket.username);
						break;
					}
				}
				ReleaseMutex(cThread->mhStructSync);

				if (i < MAX_PLAYERS)
					_tprintf(TEXT("User \"%s\" logged in with the ID:%d.\n"),lPacket.username, lPacket.Id);
				else 
					_tprintf(TEXT("Server is full!\n"));
			}

		}
	}

	return 0;
}
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

	InstantiateGame(baseGame); /*Needs work*/ /*Move into level later*/
	
	if (!DefineInvadersType(baseGame, ThreadMustGoOn)) {				//Defines invader path
		_tprintf(TEXT("[Error] Defining invaders path! \n"));
	}

	if (!GiveInvadersHP(baseGame, ThreadMustGoOn)) {					//Gives invaders HP
		_tprintf(TEXT("[Error] Giving invaders HP! \n"));
	}

	if (!PlaceDefenders(baseGame, ThreadMustGoOn)) {					//Gives invaders HP
		_tprintf(TEXT("[Error] placing defender ships ! \n"));
	}

	if (!OriginalPosition(baseGame, ThreadMustGoOn)) {					//Gives invaders HP
		_tprintf(TEXT("[Error] placing invaders ships ! \n"));
	}

	/*This needs thought and atomization*/
	//Populates invaders with coords
	//for (i = 0; ((i < baseGame->max_invaders) && *ThreadMustGoOn); i++) {

	//	if (!(baseGame->invad[i].rand_path)) {			//If regular path

	//												//deploys INVADER_BY_ROW invaders per line with a spacing of 2
	//		baseGame->invad[i].x = baseGame->invad[i].x_init = (i % INVADER_BY_ROW) * 2;

	//		//Deploys 5 lines of invaders (MAX_INVADER/11=5)
	//		baseGame->invad[i].y = baseGame->invad[i].y_init = i / INVADER_BY_ROW;
	//	}
	//	else {
	//		do {
	//			baseGame->invad[i].x = baseGame->invad[i].x_init = RandomValue((baseGame->xsize / 2)) + (baseGame->xsize / 3);
	//			baseGame->invad[i].y = baseGame->invad[i].y_init = RandomValue(baseGame->ysize);
	//			baseGame->invad[i].direction = RandomValue(3);

	//		} while (ValidateInvaderPosition(baseGame, baseGame->invad[i].x, baseGame->invad[i].y, i));
	//	}
	//}

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
		//spinnerSlash();

		writeGameData(sGTick->smGameData, sGTick->localGameData, sGTick->mhGameData);

		SetEvent(sGTick->hTick);
	}
	return 0;
}

