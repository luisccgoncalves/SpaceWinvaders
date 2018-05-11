#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>
#include "localStructs.h"
#include "../DLL/dll.h"

DWORD WINAPI InvadersBomb(LPVOID tParam) {

	int * ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	SMGameData *lvl = ((SMCtrl *)tParam)->pSMemGameData;

	/* generate random number between 1 and 50: */
	int random = rand() % 50 + 1;

	lvl->bomb.x = lvl->invad[random].x;
	lvl->bomb.y = lvl->invad[random].y;
	while (*ThreadMustGoOn && lvl->bomb.y <25/*&&bombColDetect(&bomb,tParam)*/) {
		lvl->bomb.y++;
		Sleep(lvl->bomb.speed * (*ThreadMustGoOn));
	}

	return 0;
}

DWORD WINAPI RegPathInvaders(LPVOID tParam) {

	int * ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	SMGameData *lvl = ((SMCtrl *)tParam)->pSMemGameData;
	HANDLE		*mhStructSync = ((SMCtrl *)tParam)->mhStructSync;

	int i, j;
	int sidestep = 4;
	int totalsteps = (YSIZE - (MAX_INVADER / INVADER_BY_ROW)) * sidestep;
	int regInvaderNr = (MAX_INVADER - RAND_INVADER);


	while (*ThreadMustGoOn) {						//Thread main loop

		for (i = 0; (i < totalsteps) && *ThreadMustGoOn; i++) {

			WaitForSingleObject(mhStructSync, INFINITE);

			for (j = 0; (j < regInvaderNr) && *ThreadMustGoOn; j++) {

				lvl->invad[j].y = (i / sidestep) + lvl->invad[j].y_init;				//Invader goes down after n sidesteps

				if ((i % (sidestep * 2)) < sidestep)
					lvl->invad[j].x = (i % (sidestep * 2)) + lvl->invad[j].x_init;		//Invader goes right
				else if ((i % (sidestep * 2)) > sidestep)
					lvl->invad[j].x--;													//Invader goes left
			}

			ReleaseMutex(mhStructSync);

			Sleep(INVADER_SPEED*(*ThreadMustGoOn));
		}
	}

	return 0;
}

DWORD WINAPI RandPathInvaders(LPVOID tParam) {

	int * ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	SMGameData *lvl = ((SMCtrl *)tParam)->pSMemGameData;
	HANDLE		*mhStructSync = ((SMCtrl *)tParam)->mhStructSync;
	int i;
	int startIndex = MAX_INVADER - RAND_INVADER;
	int maxXpos = XSIZE - 1;
	int maxYpos = (int)((YSIZE - 1)*0.8);

	while (*ThreadMustGoOn) {						//Thread main loop

		WaitForSingleObject(mhStructSync, INFINITE);

		for (i= startIndex; (i < MAX_INVADER) && *ThreadMustGoOn; i++) {

			switch (rand() % 4) {
			case 0:
				if (lvl->invad[i].x > 0)
					lvl->invad[i].x--;
				else
					lvl->invad[i].x=1;
				break;
			case 1:
				if (lvl->invad[i].x < maxXpos)
					lvl->invad[i].x++;
				else
					lvl->invad[i].x= maxXpos -1;
				break;
			case 2:
				if (lvl->invad[i].y > 0)
					lvl->invad[i].y--;
				else
					lvl->invad[i].y=1;
				break;
			case 3:
				if (lvl->invad[i].y < maxYpos)
					lvl->invad[i].y++;
				else
					lvl->invad[i].y= maxYpos -1;
				break;
			}
		}

		ReleaseMutex(mhStructSync);
		Sleep((INVADER_SPEED/4)*(*ThreadMustGoOn));
	}

	return 0;
}

DWORD WINAPI StartGame(LPVOID tParam) {

	int * ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	SMGameData *lvl = ((SMCtrl *)tParam)->pSMemGameData;

	DWORD			tRegPathInvaderID;
	HANDLE			htRegPathInvader;
	DWORD			tRandPathInvaderID;
	HANDLE			htRandPathInvader;
	DWORD			tInvadersBombID;
	HANDLE			htInvadersBomb;

	int i;

	srand((unsigned)time(NULL));					//Seeds the RNG

	//Defines invader path
	for (i = 0; (i < MAX_INVADER) && *ThreadMustGoOn; i++) {		
		if (i < (MAX_INVADER-RAND_INVADER))
			lvl->invad[i].rand_path = 0;
		else
			lvl->invad[i].rand_path = 1;
	}

	//Populates invaders with coords
	for (i = 0; ((i < MAX_INVADER) && *ThreadMustGoOn); i++) {		

		if (!(lvl->invad[i].rand_path)) {			//If regular path
			
			//deploys INVADER_BY_ROW invaders per line with a spacing of 2
			lvl->invad[i].x = lvl->invad[i].x_init = (i % INVADER_BY_ROW) * 2;

			//Deploys 5 lines of invaders (MAX_INVADER/11=5)
			lvl->invad[i].y = lvl->invad[i].y_init = i / INVADER_BY_ROW;
		}
		else {
			lvl->invad[i].x = lvl->invad[i].x_init = rand() % XSIZE;
			lvl->invad[i].y = lvl->invad[i].y_init = rand() % YSIZE;
			_tprintf(TEXT("\nInvader no: %d\nX= %d\nY= %d\n%d\n"),i, lvl->invad[i].x, lvl->invad[i].y,rand());
		}
	}

	//Populates ships ######## NEEDS TO BE UPDATED TO MULTIPLAYER #########
	for (i = 0; i < MAX_PLAYERS; i++) {

		lvl->ship[i].x = 1;
		lvl->ship[i].y = 23;
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

	WaitForSingleObject(htRegPathInvader,INFINITE);
	WaitForSingleObject(htRandPathInvader, INFINITE);
	WaitForSingleObject(htInvadersBomb, INFINITE);

	return 0;

}

DWORD WINAPI GameTick(LPVOID tParam) {				//Warns gateway of structure updates
	
	GTickStruct		*sGTick;
	sGTick = (GTickStruct*)tParam;

	while (sGTick->ThreadMustGoOn) {

		Sleep(100);
		_tprintf(TEXT("."));
		WaitForSingleObject(sGTick->mhStructSync, INFINITE);
		SetEvent(sGTick->hTick);

		ReleaseMutex(sGTick->mhStructSync);
	}

	return 0;
}

DWORD WINAPI ReadGatewayMsg(LPVOID tParam) {		//Warns gateway of structure updates

	int * ThreadMustGoOn = &((SMCtrl*)tParam)->ThreadMustGoOn;
	HANDLE * hSMGatewayUpdate = ((SMCtrl*)tParam)->hSMGatewayUpdate;
	
	SMMessage * msg = (((SMCtrl *)tParam)->pSMemMessage);
	Sleep(200);
	SMMessage * copy = malloc(sizeof(SMMessage));

	CopyMemory(copy, msg, sizeof(SMMessage));

	while (*ThreadMustGoOn) {
		WaitForSingleObject(hSMGatewayUpdate, INFINITE);

		if (copy->details == NULL) {
			_tprintf(TEXT(" z "));
		}
		else {
			if (copy->details == 1) {
				_tprintf(TEXT(" ! "));
			}
			else {
				_tprintf(TEXT(" a "));
			}
		}
	}

	return 0;
}

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
	cThread.SMemViewServer.QuadPart = ((sizeof(SMGameData) / dwSysGran)*dwSysGran) + dwSysGran;
	cThread.SMemViewGateway.QuadPart = ((sizeof(SMMessage) / dwSysGran)*dwSysGran) + dwSysGran;
	//No rounding needed,  parts are already multiples
	cThread.SMemSize.QuadPart = cThread.SMemViewServer.QuadPart + cThread.SMemViewGateway.QuadPart;

	//#######################################################################################################################
	//##################################### GRANULARITY TESTS//DELETE THIS ##################################################
	//#######################################################################################################################
	_tprintf(TEXT("Sysgran: %d bytes\nSize of servstruct: %d\nSize of gateway: %d\n"), dwSysGran, sizeof(SMGameData), sizeof(SMMessage));
	_tprintf(TEXT("ServerView:\t((%d/%d)*%d)+%d=%d\n"), sizeof(SMGameData), dwSysGran, dwSysGran, dwSysGran, ((sizeof(SMGameData) / dwSysGran)*dwSysGran) + dwSysGran);
	_tprintf(TEXT("GatewayView:\t((%d/%d)*%d)+%d=%d\n"), sizeof(SMMessage),dwSysGran,dwSysGran, dwSysGran, ((sizeof(SMMessage) / dwSysGran)*dwSysGran) + dwSysGran);
	_tprintf(TEXT("TestBigView:\t((%d/%d)*%d)+%d=%d\n"), 66000, dwSysGran, dwSysGran, dwSysGran, ((66000 / dwSysGran)*dwSysGran) + dwSysGran);
	//#######################################################################################################################
	//#######################################################################################################################
	//#######################################################################################################################

	cThread.ThreadMustGoOn = 1;						//Preps thread to run position
	sGTick.ThreadMustGoOn = 1;						//Preps thread to run position

	cThread.mhStructSync = CreateMutex(				//This a test
		NULL,										//Security attributes
		FALSE,										//Initial owner
		NULL);										//Mutex name

	hCanBootNow = CreateEvent(						//Creates the event to warn gateway that the shared memoy is mapped
		NULL,										//Event attributes
		FALSE,										//Manual reset (TRUE for auto-reset)
		FALSE,										//Initial state
		TEXT("LetsBoot"));							//Event name

	cThread.hSMServerUpdate = CreateEvent(			//Creates the event to warn gateway that the shared memoy is mapped
		NULL, 										//Event attributes
		FALSE, 										//Manual reset (TRUE for auto-reset)
		FALSE, 										//Initial state
		TEXT("SMServerUpdate"));					//Event name

	cThread.hSMGatewayUpdate = CreateEvent(			//Creates the event to warn gateway that the shared memoy is mapped
		NULL, 										//Event attributes
		FALSE, 										//Manual reset (TRUE for auto-reset)
		FALSE, 										//Initial state
		TEXT("SMGatewayUpdate"));					//Event name

	//Populate sGTick's pointers
	sGTick.mhStructSync = cThread.mhStructSync;			//Copies Invader moving mutex to the GTick struct thread
	sGTick.hTick = cThread.hSMServerUpdate;			//Copies Event to warn gateway of memory updates


	//#####################################WAITING FOR PROFESSOR'S EMAIL REPLY##############################################

	//Creates a mapped file
	if (sharedMemory(&cThread.hSMem, &cThread.SMemSize) == -1) {
		_tprintf(TEXT("[Error] Opening file mapping (%d)\n"), GetLastError());
		return -1;
	}

	//Creates a view of the desired part <Server>
	mapServerView(&cThread);
	if (cThread.pSMemGameData== NULL) {				//Checks for errors
		_tprintf(TEXT("[Error] Mapping server view (%d)\n"), GetLastError());
		return -1;
	}

	//Creates a view of the desired part <Gateway>
	mapGatewayView(&cThread);
	if (cThread.pSMemMessage== NULL) {				//Checks for errors
		_tprintf(TEXT("[Error] Mapping gateway view (%d)\n"), GetLastError());
		return -1;
	}
	//######################################################################################################################


	SetEvent(hCanBootNow);							//Warns gateway that Shared memory is mapped

	//Launches game tick thread
	_tprintf(TEXT("Launching game tick thread...\n"));

	htGTick = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size (0 for default)
		GameTick,									//Thread function name
		(LPVOID)&sGTick,							//Thread parameter struct
		0,											//Creation flags
		&tGTickID);									//gets thread ID to close it afterwards

	//Launches gateway message receiver thread
	_tprintf(TEXT("Launching gateway message receiver thread...\n"));
	htGReadMsg = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size (0 for default)
		ReadGatewayMsg,								//Thread function name
		(LPVOID)&cThread,							//Thread parameter struct
		0,											//Creation flags
		&tRGMsgID);									//gets thread ID to close it afterwards

	//Launches Game thread
	_tprintf(TEXT("Launching Game thread... ENTER to quit\n"));

	htGame = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size (0 for default)
		StartGame,									//Thread function name
		(LPVOID)&cThread,							//Thread parameter struct
		0,											//Creation flags
		&tGameID);									//gets thread ID to close it afterwards

	//Enter to end thread and exit
	_gettchar();

	cThread.ThreadMustGoOn = 0;						//Signals thread to gracefully exit
	sGTick.ThreadMustGoOn = 0;						//Signals thread to gracefully exit


	//If this gets bigger we should maybe move all handles into an array and waitformultipleobjects instead
	WaitForSingleObject(htGame, INFINITE);			//Waits for thread to exit

	WaitForSingleObject(htGTick, INFINITE);			//Waits for thread to exit

	SetEvent(cThread.hSMGatewayUpdate);				//Sets event to own process, this will iterate
													//the thread main loop to check ThreadMustGoOn == 0
	WaitForSingleObject(htGReadMsg, INFINITE);		//Waits for thread to exit
	

	UnmapViewOfFile(cThread.pSMemGameData);			//Unmaps view of shared memory
	UnmapViewOfFile(cThread.pSMemMessage);			//Unmaps view of shared memory
	CloseHandle(cThread.hSMem);						//Closes shared memory

	return 0;
}
