#include "game.h"
#include "score.h"

int _tmain(int argc, LPTSTR argv[]) {

#ifdef UNICODE									//Sets console to unicode
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	//StartGame thread STRUCT/HANDLE/ID
	SMCtrl			cThread;						//Thread parameter structure
	HANDLE			htGame;							//Handle to the game thread
	DWORD			tGameID;						//stores the ID of the game thread

	//GameTick thread STRUCT/HANDLE/ID
	GTickStruct		sGTick;							//Thread parameter structure
	HANDLE			htGTick;						//Handle to the game tick thread
	DWORD			tGTickID;						//Stores the ID of GTick thread

	//Gateway Message thread HANDLE/ID (uses cThread as parameter)
	HANDLE			htGReadMsg;						//Handle to the Gateway Message thread
	DWORD			tRGMsgID;						//Stores the ID of the Gateway Message thread

	HANDLE			hCanBootNow;					//Handle to event. Warns the gateway the shared memory is mapped
	
	SYSTEM_INFO		SysInfo;						//System info structure; Needed to extract dwAllocationGranularity
	DWORD			dwSysGran;						//Stores system granularity (usually arround 65KB)

	GetSystemInfo(&SysInfo);						//Used to get system granularity
	dwSysGran = SysInfo.dwAllocationGranularity;	//Used to get system granularity

	//Rounds view sizes to the neares granularity multiple
	cThread.SMemViewServer.QuadPart = ((sizeof(GameData) / dwSysGran)*dwSysGran) + dwSysGran;
	cThread.SMemViewGateway.QuadPart = ((sizeof(SMMessage) / dwSysGran)*dwSysGran) + dwSysGran;
	//No rounding needed,  parts are already multiples
	cThread.SMemSize.QuadPart = cThread.SMemViewServer.QuadPart + cThread.SMemViewGateway.QuadPart;

	readTop10FromReg(cThread.localGameData.top10); //Loads highscores from windows registry

	cThread.ThreadMustGoOn = 1;						//Preps thread to run position
	sGTick.ThreadMustGoOn = 1;						//Preps thread to run position

	cThread.mhGameData = createGameDataMutex();			//Mutex to sync read and write of gameData
	if (cThread.mhGameData == NULL) {
		_tprintf(TEXT("[Error] Mutex GameDataMutex (%d)\n"), GetLastError());
		return -1;
	}

	cThread.mhStructSync = CreateMutex(				//Used for game structure integrity
		NULL,										//Security attributes
		FALSE,										//Initial owner
		STRUCT_SYNC);								//Mutex name
	if (cThread.mhStructSync == NULL) {
		_tprintf(TEXT("[Error] Mutex StructSync (%d)\n"), GetLastError());
		return -1;
	}

	cThread.mhProdConsMut = createProdConsMutex();
	if (cThread.mhProdConsMut == NULL) {
		_tprintf(TEXT("[Error] Mutex ProdCons (%d)\n"), GetLastError());
		return -1;
	}

	cThread.shVacant = createVacantSemaphore();
	if (cThread.shVacant == NULL) {
		_tprintf(TEXT("[Error] Semaphore vacant (%d)\n"), GetLastError());
		return -1;
	}

	cThread.shOccupied = createOccupiedSemaphore();
	if (cThread.shOccupied == NULL) {
		_tprintf(TEXT("[Error] Semaphore occupied (%d)\n"), GetLastError());
		return -1;
	}

	hCanBootNow = CreateEvent(				//Creates the event to warn gateway that the shared memoy is mapped
		NULL,										//Event attributes
		FALSE,										//Manual reset (TRUE for auto-reset)
		FALSE,										//Initial state
		EVE_BOOT);									//Event name
	if (hCanBootNow == NULL) {
		_tprintf(TEXT("[Error] Event boot order(%d)\n"), GetLastError());
		return -1;
	}


	cThread.hSMServerUpdate = CreateEvent(	
		NULL, 										//Event attributes
		FALSE, 										//Manual reset (TRUE for auto-reset)
		FALSE, 										//Initial state
		EVE_SERV_UP);								//Event name
	if (cThread.hSMServerUpdate == NULL) {
		_tprintf(TEXT("[Error] Event server update (%d)\n"), GetLastError());
		return -1;
	}

	//Populate sGTick's pointers
	//InstantiateGame(&cThread.game);

	//Creates a mapped file
	if (sharedMemory(&cThread.hSMem, &cThread.SMemSize) == -1) {
		_tprintf(TEXT("[Error] Opening file mapping (%d)\n"), GetLastError());
		return -1;
	}

	//Creates a view of the desired part <GameDataView>
	if (mapGameDataView(&cThread, FILE_MAP_WRITE) == -1) {				//Checks for errors
		_tprintf(TEXT("[Error] Mapping GameData view (%d) at Server\n"), GetLastError());
		return -1;
	}

	//Creates a view of the desired part <MsgView>
	if (mapMsgView(&cThread) == -1) {		//Checks for errors
		_tprintf(TEXT("[Error] Mapping MsgView (%d)\n at Server"), GetLastError());
		return -1;
	}

	sGTick.mhGameData = cThread.mhStructSync;			//Copies Invader moving mutex to the GTick struct thread
	sGTick.hTick = cThread.hSMServerUpdate;				//Copies Event to warn gateway of memory updates  
	sGTick.localGameData = &cThread.localGameData;		//Copies gameData address to GTick
	sGTick.smGameData = cThread.pSMemGameData;			//Copies the sharedmemory pointer to GTick

	SetEvent(hCanBootNow);								//Warns gateway that Shared memory is mapped

	//Launches game tick thread
	_tprintf(TEXT("Launching game tick thread...\n"));
	htGTick = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size (0 for default)
		GameTick,									//Thread function name
		(LPVOID)&sGTick,							//Thread parameter struct
		0,											//Creation flags
		&tGTickID);									//gets thread ID to close it afterwards
	if (htGTick == NULL) {
		_tprintf(TEXT("[Error] Creating thread GTick (%d) at Server\n"), GetLastError());
	}

	//Launches gateway message receiver thread
	_tprintf(TEXT("Launching gateway message receiver thread...\n"));
	htGReadMsg = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size (0 for default)
		ShipInstruction,								//Thread function name
		(LPVOID)&cThread,							//Thread parameter struct
		0,											//Creation flags
		&tRGMsgID);									//gets thread ID to close it afterwards
	if (htGReadMsg == NULL) {
		_tprintf(TEXT("[Error] Creating thread GReadMsg (%d) at Server\n"), GetLastError());
	}

	//Launches Game thread
	_tprintf(TEXT("Launching Game thread... ENTER to quit\n"));

	htGame = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size (0 for default)
		StartGame,									//Thread function name
		(LPVOID)&cThread,							//Thread parameter struct
		0,											//Creation flags
		&tGameID);									//gets thread ID to close it afterwards
	if (htGame == NULL) {
		_tprintf(TEXT("[Error] Creating thread GAME (%d) at Server\n"), GetLastError());
	}

	//Enter to end thread and exit
	_gettchar();

	cThread.ThreadMustGoOn = 0;						//Signals thread to gracefully exit
	sGTick.ThreadMustGoOn = 0;						//Signals thread to gracefully exit


	//If this gets bigger we should maybe move all handles into an array and waitformultipleobjects instead
	WaitForSingleObject(htGame, INFINITE);			//Waits for thread to exit
	WaitForSingleObject(htGTick, INFINITE);			//Waits for thread to exit

	//needs to interrupt prodcons algorithm
	WaitForSingleObject(htGReadMsg, INFINITE);		//Waits for thread to exit

	UnmapViewOfFile(cThread.pSMemGameData);			//Unmaps view of shared memory
	UnmapViewOfFile(cThread.pSMemMessage);			//Unmaps view of shared memory
	CloseHandle(cThread.hSMem);						//Closes shared memory

	return 0;
}
