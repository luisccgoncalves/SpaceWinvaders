#include "data_server.h"
#pragma warning(disable:4996)

DWORD WINAPI LaunchServer(SMCtrl *cThread) {

	//GameTick thread STRUCT/HANDLE/ID
	GTickStruct		sGTick;										//Thread parameter structure
	HANDLE			htGTick;									//Handle to the game tick thread
	DWORD			tGTickID;									//Stores the ID of GTick thread

																//Packet Listener thread HANDLE/ID (uses cThread as parameter)
	HANDLE			htPacketListener;							//Handle to the Packet Listener thread
	DWORD			tPacketListenerID;							//Stores the ID of the Gateway Message thread

	HANDLE			hCanBootNow;								//Handle to event. Warns the gateway the shared memory is mapped

	SYSTEM_INFO		SysInfo;									//System info structure; Needed to extract dwAllocationGranularity
	DWORD			dwSysGran;									//Stores system granularity (usually arround 65KB)

	ZeroMemory(cThread, sizeof(cThread));

	GetSystemInfo(&SysInfo);									//Used to get system granularity
	dwSysGran = SysInfo.dwAllocationGranularity;				//Used to get system granularity

																//Rounds view sizes to the neares granularity multiple
	cThread->SMemViewServer.QuadPart = ((sizeof(GameData) / dwSysGran)*dwSysGran) + dwSysGran;
	cThread->SMemViewGateway.QuadPart = ((sizeof(SMMessage) / dwSysGran)*dwSysGran) + dwSysGran;
	//No rounding needed,  parts are already multiples
	cThread->SMemSize.QuadPart = cThread->SMemViewServer.QuadPart + cThread->SMemViewGateway.QuadPart;

	readTop10FromReg(cThread->localGameData.top10);				//Loads highscores from windows registry

	cThread->ThreadMustGoOn = 1;								//Preps thread to run position
	sGTick.ThreadMustGoOn = 1;									//Preps thread to run position

	cThread->mhGameData = createGameDataMutex();				//Mutex to sync read and write of gameData
	if (cThread->mhGameData == NULL) {
		_tprintf(TEXT("[Error] Mutex GameDataMutex (%d)\n"), GetLastError());
		return -1;
	}

	cThread->mhStructSync = CreateMutex(							//Used for game structure integrity
		NULL,													//Security attributes
		FALSE,													//Initial owner
		STRUCT_SYNC);											//Mutex name
	if (cThread->mhStructSync == NULL) {
		_tprintf(TEXT("[Error] Mutex StructSync (%d)\n"), GetLastError());
		return -1;
	}

	cThread->mhProdConsMut = createProdConsMutex();
	if (cThread->mhProdConsMut == NULL) {
		_tprintf(TEXT("[Error] Mutex ProdCons (%d)\n"), GetLastError());
		return -1;
	}

	cThread->shVacant = createVacantSemaphore();
	if (cThread->shVacant == NULL) {
		_tprintf(TEXT("[Error] Semaphore vacant (%d)\n"), GetLastError());
		return -1;
	}

	cThread->shOccupied = createOccupiedSemaphore();
	if (cThread->shOccupied == NULL) {
		_tprintf(TEXT("[Error] Semaphore occupied (%d)\n"), GetLastError());
		return -1;
	}

	hCanBootNow = CreateEvent(							//Creates the event to warn gateway that the shared memoy is mapped
		NULL,													//Event attributes
		FALSE,													//Manual reset (TRUE for auto-reset)
		FALSE,													//Initial state
		EVE_BOOT);												//Event name
	if (hCanBootNow == NULL) {
		_tprintf(TEXT("[Error] Event boot order(%d)\n"), GetLastError());
		return -1;
	}

	cThread->hSMServerUpdate = CreateEvent(
		NULL, 													//Event attributes
		FALSE, 													//Manual reset (TRUE for auto-reset)
		FALSE, 													//Initial state
		EVE_SERV_UP);											//Event name
	if (cThread->hSMServerUpdate == NULL) {
		_tprintf(TEXT("[Error] Event server update (%d)\n"), GetLastError());
		return -1;
	}

	if (sharedMemory(&cThread->hSMem, &cThread->SMemSize) == -1) {	//Creates a mapped file
		_tprintf(TEXT("[Error] Opening file mapping (%d)\n"), GetLastError());
		return -1;
	}

	//Creates a view of the desired part <GameDataView>
	if (mapGameDataView(cThread, FILE_MAP_WRITE) == -1) {			//Checks for errors
		_tprintf(TEXT("[Error] Mapping GameData view (%d) at Server\n"), GetLastError());
		return -1;
	}

	//Creates a view of the desired part <MsgView>
	if (mapMsgView(cThread) == -1) {		//Checks for errors
		_tprintf(TEXT("[Error] Mapping MsgView (%d)\n at Server"), GetLastError());
		return -1;
	}

	sGTick.mhGameData = cThread->mhStructSync;					//Copies Invader moving mutex to the GTick struct thread
	sGTick.hTick = cThread->hSMServerUpdate;					//Copies Event to warn gateway of memory updates  
	sGTick.localGameData = &cThread->localGameData;				//Copies gameData address to GTick
	sGTick.smGameData = cThread->pSMemGameData;				//Copies the sharedmemory pointer to GTick

	SetEvent(hCanBootNow);										//Warns gateway that Shared memory is mapped

	_tprintf(TEXT("[DEBUG] Launching game tick thread...\n"));	//Launches game tick thread
	htGTick = CreateThread(
		NULL,													//Thread security attributes
		0,														//Stack size (0 for default)
		GameTick,												//Thread function name
		(LPVOID)&sGTick,										//Thread parameter struct
		0,														//Creation flags
		&tGTickID);												//gets thread ID 
	if (htGTick == NULL) {
		_tprintf(TEXT("[Error] Creating thread GTick (%d) at Server\n"), GetLastError());
	}


	_tprintf(TEXT("[DEBUG] Launching gateway message receiver thread...\n"));	//Launches gateway message receiver thread
	htPacketListener = CreateThread(
		NULL,													//Thread security attributes
		0,														//Stack size (0 for default)
		PacketListener,											//Thread function name
		(LPVOID)cThread,										//Thread parameter struct
		0,														//Creation flags
		&tPacketListenerID);									//gets thread ID 
	if (htPacketListener == NULL) {
		_tprintf(TEXT("[Error] Creating thread GReadMsg (%d) at Server\n"), GetLastError());
	}

	/* If this gets bigger we should maybe move all handles into an array and waitformultipleobjects instead */
	WaitForSingleObject(htGTick, INFINITE);						//Waits for thread to exit
																/* Needs to interrupt prodcons algorithm */
	WaitForSingleObject(htPacketListener, INFINITE);			//Waits for thread to exit

	UnmapViewOfFile(cThread->pSMemGameData);						//Unmaps view of shared memory
	UnmapViewOfFile(cThread->pSMemMessage);						//Unmaps view of shared memory
	CloseHandle(&cThread->hSMem);									//Closes shared memory

	return 0;

}

int StartLobby(SMCtrl * cThread) {


	HANDLE		htGame;										//Handle to the game thread
	DWORD		tGameID;									//stores the ID of the game thread

	WaitForSingleObject(cThread->mhStructSync, INFINITE);
	loadShips(cThread->localGameData.ship, cThread->localGameData.logged);
	ReleaseMutex(cThread->mhStructSync);



	htGame = CreateThread(
		NULL,													//Thread security attributes
		0,														//Stack size (0 for default)
		StartGame,												//Thread function name
		(LPVOID)cThread,													//Thread parameter struct
		0,														//Creation flags
		&tGameID);												//gets thread ID 
	if (htGame == NULL) {
		_tprintf(TEXT("[Error] Creating thread GAME (%d) at Server\n"), GetLastError());
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

	baseGame->gameRunning = 1;

	if (!DefineInvadersType(baseGame, ThreadMustGoOn)) {				//Defines invader path type
		_tprintf(TEXT("[Error] Defining invaders path! \n"));
	}

	if (!GiveInvadersHP(baseGame, ThreadMustGoOn)) {					//Gives invaders HP
		_tprintf(TEXT("[Error] Giving invaders HP! \n"));
	}

	if (!PlaceDefenders(baseGame, ThreadMustGoOn)) {					//places the defender ships
		_tprintf(TEXT("[Error] placing defender ships ! \n"));
	}

	if (!OriginalPosition(baseGame, ThreadMustGoOn)) {					//Gives invaders original position
		_tprintf(TEXT("[Error] placing invaders ships ! \n"));
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
	WaitForSingleObject(htPowerUps, INFINITE);

	return 0;
}

DWORD WINAPI GameTick(LPVOID tParam) {				//Warns gateway of structure updates

	GTickStruct		*sGTick = (GTickStruct*)tParam;
	int i;

	while (sGTick->ThreadMustGoOn) {

		Sleep(50);
		writeGameData(sGTick->smGameData, sGTick->localGameData, sGTick->mhGameData);

		SetEvent(sGTick->hTick);

		if (sGTick->localGameData->gameRunning) {
			for (i = 0; i < sGTick->localGameData->num_players; i++) {
				if (sGTick->localGameData->ship[i].lives >= 0)
					break;
			}
			if (i == sGTick->localGameData->num_players)
				sGTick->localGameData->gameRunning = 0;
		}
	}
	return 0;
}

int loadShips(Ship *ship, Player *player) {

	int shipsLogged = 0;

	for (int i = 0; i < MAX_PLAYERS; i++) {
		if (player[i].isReady) {
			ship[shipsLogged].id = player[i].Id;
			_tcscpy_s(ship[shipsLogged].username, SMALL_BUFF, player[i].username);
			shipsLogged++;
		}
	}

	return shipsLogged;
}

int updateConfigGameValues(ConfigurableVars vars, GameData *game, HANDLE mhStructSync) {
	WaitForSingleObject(mhStructSync, INFINITE);

	game->num_players = vars.players;
	game->max_invaders = vars.maxInvaders;
	game->max_rand_invaders = vars.hardInvaders;
	game->invaders_speed = vars.invadersSpeed;
	game->projectiles_speed = vars.projectileSpeed;
	game->bombRate = vars.bombRate;
	game->shotRate = vars.shotRate;
	game->moveRate = vars.moveRate;
	game->pup_duration = vars.pUpDuration;
	game->gameCreated = 1;
	ReleaseMutex(mhStructSync);
	
	return 1;
}