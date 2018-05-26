#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "../DLL/dll.h"
#include "../Client/debug.h"

//#########################################################################################
//############################   TEMP TEST   ##############################################
//#########################################################################################

/* 
ToDo List - Please clean after every point.


-Implement the structure again, *not shore if done or not 
-remove the global writeReady, 
-Redo the writeGameData
-Remove the DEBUG prints
*/

//typedef struct {				//Don't think this is needed (right now at least)
//	HANDLE heWriteReady;
//	HANDLE heReadReady;
//	HANDLE hPipe;
//
//} PipeComm;


#define BUFSIZE 2048

//void addClient(HANDLE *c, HANDLE *newClient) {
//	for (int i = 0; i < MAX_PLAYERS; i++) {
//		if (c[i] == NULL) {
//			c[i] = newClient;
//			return;
//		}
//	}
//}
//
//void removeClient(HANDLE *c, HANDLE *oldClient) {
//	for (int i = 0; i < MAX_PLAYERS; i++) {
//		if (c[i] == oldClient) {				
//			c[i] = NULL;
//			return;
//		}
//	}
//}

//int writeGameData(HANDLE hPipe, HANDLE *writeReady, PipeGameData *game) {
//
//	BOOL fSuccess = FALSE;
//	DWORD cbWriten = 0;
//
//	OVERLAPPED overlWrite = { 0 };
//	ResetEvent(writeReady);
//	overlWrite.hEvent = writeReady;
//
//	fSuccess = WriteFile(
//		hPipe,
//		&game,
//		sizeof(PipeGameData),
//		&cbWriten,
//		&overlWrite);
//
//	WaitForSingleObject(writeReady, INFINITE);
//
//	GetOverlappedResult(
//		hPipe, 
//		&overlWrite, 
//		&cbWriten, 
//		FALSE);
//	if (cbWriten < sizeof(PipeGameData)) {
//		_tprintf(TEXT("[Error] OVERLAPPED writeGameData (%d)\n"), GetLastError());
//		return 1;
//	}
//
//}

//WIP
int writePipeMsg(HANDLE hPipe, HANDLE writeReady, GameData msg) {
	/*
	This is for sending specific messages to a 
	single pipe.
	One exmaple may be player dead for the player

	Probably we will not be able to make the pipe send two different messages
	it's easier to include THIS gtw msg on writeGameData
	*/

		BOOL fSuccess = FALSE;
		DWORD cbWriten = 0;

		OVERLAPPED overlWrite = { 0 };

		ResetEvent(writeReady);
		overlWrite.hEvent = writeReady;

		_tprintf(TEXT("[DEBUG] Inside writePipeMsg \n"));

		fSuccess = WriteFile(
			hPipe,
			&msg,
			sizeof(msg),
			&cbWriten,
			&overlWrite);
		if (!fSuccess) {
			if (GetLastError() == ERROR_IO_INCOMPLETE)
				return;
			_tprintf(TEXT("[DEBUG] Inside writePipeMsg ERROR_IO_INCOMPLETE... WHY?!?!? \n"));
		}
		_tprintf(TEXT("[DEBUG] Inside writePipeMsg before WaitForSingleObject \n"));
		WaitForSingleObject(writeReady, INFINITE);
		_tprintf(TEXT("[DEBUG] Inside writePipeMsg after WaitForSingleObject \n"));
		GetOverlappedResult(
			hPipe,
			&overlWrite,
			&cbWriten,
			FALSE);
		if (cbWriten < sizeof(msg)) {
			_tprintf(TEXT("[Error] OVERLAPPED writePipeMsgs (%d)\n"), GetLastError());
			return 1;

		}
		else {
			_tprintf(TEXT("[DEBUG] Inside writePipeMsg cbWriten was OK \n"));
		}
		return 0;
}

//int broadcastGame(HANDLE *clients, HANDLE *writeReady, PipeGameData *game) { //This needs to receive nr of connected players
//	int numWrites = 0;
//	for (int i = 0; i < MAX_PLAYERS; i++) {
//		numWrites += writeGameData(clients[i], writeReady, game);
//	}
//	return numWrites;
//}

DWORD WINAPI instanceThread(LPVOID tParam) {
	
	HANDLE		hPipe = (HANDLE)tParam;
	HANDLE		heWriteReady;
	BOOL		fSuccess = FALSE;
	GameData	clientMsg;

	if (hPipe == NULL) {
		_tprintf(TEXT("ERROR casting pipe. (%d)\n"), GetLastError());
		return -1;
	}

	heWriteReady = CreateEvent(			//Creates the event to signal access to write action
		NULL, 										//Event attributes
		TRUE, 										//Manual reset (TRUE for auto-reset)
		FALSE, 										//Initial state
		NULL);										//Event name
	if (heWriteReady == NULL) {
		_tprintf(TEXT("[Error] Event writeReady (%d)\n"), GetLastError());
		return -1;
	}



	//###################################### DEBUG STUFF #################################################
	srand((unsigned)time(NULL));					//Seeds the RNG

													//Defines invader path
	for (int i = 0; (i < MAX_INVADER); i++) {
		if (i < (MAX_INVADER - 2))
			clientMsg.invad[i].rand_path = 0;
		else
			clientMsg.invad[i].rand_path = 1;
	}

	for (int i = 0; (i < MAX_INVADER); i++) {

		if (!(clientMsg.invad[i].rand_path)) {			//If regular path

													//deploys INVADER_BY_ROW invaders per line with a spacing of 2
			clientMsg.invad[i].x = clientMsg.invad[i].x_init = (i % 11) * 2;

			//Deploys 5 lines of invaders (MAX_INVADER/11=5)
			clientMsg.invad[i].y = clientMsg.invad[i].y_init = i / 11;
		}
		else {
			clientMsg.invad[i].x = clientMsg.invad[i].x_init = rand() % XSIZE;
			clientMsg.invad[i].y = clientMsg.invad[i].y_init = rand() % YSIZE;
		}
	}
	_tprintf(TEXT("\nSending a message!\n = %d"), clientMsg.invad[0].x);
	//####################################################################################################

	_tprintf(TEXT("Sending...\n"));

	//get gamedata

	writePipeMsg(hPipe, heWriteReady, clientMsg);

	_tprintf(TEXT("Sent...\n"));
	return 0;
}

DWORD WINAPI CreatePipes(LPVOID tParam) {

	int			*ThreadMustGoOn = (int*)tParam;
	LPTSTR		lpsPipeName = PIPE_NAME;
	HANDLE		clients[MAX_PLAYERS] = {0};
	HANDLE		h1stPipeInst;
	HANDLE		htPipeConnect[40] = { NULL }; //Update this

	HANDLE		hPipe = INVALID_HANDLE_VALUE;

	BOOL		fConnected = FALSE;
	DWORD		dwPipeThreadId;
	int			threadn = 0;

	h1stPipeInst = CreateEvent(				//Creates the event to warn gateway that the shared memoy is mapped
		NULL,										//Event attributes
		TRUE,										//Manual reset (TRUE for auto-reset)
		FALSE,										//Initial state
		EVE_1ST_PIPE);								//Event name
	if (h1stPipeInst  == NULL) {
		_tprintf(TEXT("[Error] Event 1st pipe instance (%d)\n"), GetLastError());
		return -1;
	}

	while (*ThreadMustGoOn) {
	
		hPipe = CreateNamedPipe(
			lpsPipeName,
			PIPE_ACCESS_DUPLEX | 
			FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_MESSAGE | 
			PIPE_READMODE_MESSAGE | 
			PIPE_WAIT,
			MAX_PLAYERS,
			BUFSIZE,
			BUFSIZE,
			5000,														//5 secs timeout
			NULL);

		if (GetLastError() == ERROR_PIPE_BUSY) {
			_tprintf(TEXT("Max players Reached\n"));
			//WaitForSingleObject until threadn<MAX_PLAYERS
			return -1;
		}
		if (hPipe == INVALID_HANDLE_VALUE) {
			_tprintf(TEXT("[Error] Creating NamePipe (%d)\n"), GetLastError());
			return -1;
		}

		if(!threadn)
			SetEvent(h1stPipeInst);

		fConnected = ConnectNamedPipe(hPipe, NULL) ? 
						TRUE : 
						(GetLastError() == ERROR_PIPE_CONNECTED);
		
		if (fConnected) {

			_tprintf(TEXT("Someone connected!\n"));

			htPipeConnect[threadn] = CreateThread(
				NULL,									//Thread security attributes
				0,										//Stack size
				instanceThread,							//Thread function name
				(LPVOID)hPipe,							//Thread parameter struct
				0,										//Creation flags
				&dwPipeThreadId);						//gets thread ID to close it afterwards
			if (htPipeConnect[threadn] == NULL) {
				_tprintf(TEXT("[Error] Creating thread ConnectPipesThread (%d) at Gateway\n"), GetLastError());
				return -1;
			}
			else {
				threadn++;
			}
		}
		else {
			CloseHandle(hPipe);							//Frees this pipe instance
		}

	}

	WaitForMultipleObjects(threadn, htPipeConnect, TRUE, INFINITE);
}


//#########################################################################################

void simulClient(Packet * localpacket) {

	srand((unsigned)time(NULL));

	localpacket->owner = 0;

	localpacket->instruction = rand() % 4;

	Sleep(500);
	return;
}

DWORD WINAPI sendMessage(LPVOID tParam) {

	SMCtrl		*cThread = (SMCtrl*)tParam;

	Packet		localpacket;

	int nextIn = 0;

	while (cThread->ThreadMustGoOn) {

		//Produces item
		simulClient(&localpacket);

		//Puts it in buffer
		WaitForSingleObject(cThread->shVacant,INFINITE);

		WaitForSingleObject(cThread->mhProdConsMut, INFINITE);

		cThread->pSMemMessage->buffer[nextIn] = localpacket;

		nextIn = (nextIn + 1) % SMEM_BUFF;

		ReleaseMutex(cThread->mhProdConsMut);

		ReleaseSemaphore(cThread->shOccupied, 1, NULL);

	}

	return 0;
}

DWORD WINAPI ReadServerMsg(LPVOID tParam) {	

	SMCtrl		*cThread = (SMCtrl*)tParam;
	HANDLE		hStdout = GetStdHandle(STD_OUTPUT_HANDLE); //Handle to stdout to clear screen ##DELETE-ME after May 12th##
	GameData	*gameMsg;

	gameMsg = malloc(sizeof(GameData));

	int i;

	cls(hStdout);
	hidecursor();

	while (cThread->ThreadMustGoOn) {

		WaitForSingleObject(cThread->hSMServerUpdate, INFINITE);
		WaitForSingleObject(cThread->mhStructSync, INFINITE);

		gameMsg = cThread->pSMemGameData;
		//CopyMemory(gameMsg, cThread->pSMemGameData, sizeof(GameData));
		
		ReleaseMutex(cThread->mhStructSync);

		cls(hStdout);
		for (i = 0; i < MAX_INVADER; i++) {
			if (gameMsg->invad[i].hp) {
				gotoxy(gameMsg->invad[i].x, gameMsg->invad[i].y);
				if (gameMsg->invad[i].rand_path)
					_tprintf(TEXT("X"));
				else
					_tprintf(TEXT("W"));
			}
		}

		if (cThread->pSMemGameData->bomb[0].y < 25) { //this needs another aproach (fired state?)
			gotoxy(gameMsg->bomb[0].x, gameMsg->bomb[0].y);
			_tprintf(TEXT("o"));
		}

		for (i = 0; i < MAX_PLAYERS; i++) {
			gotoxy(gameMsg->ship[i].x, gameMsg->ship[i].y);
			_tprintf(TEXT("Â"));
		}
	}

	return 0;
}

int _tmain(int argc, LPTSTR argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	SMCtrl		cThread;
	HANDLE		hCanBootNow;

	HANDLE		htSReadMsg;
	DWORD		tRSMsgID;

	HANDLE		hSSendMessage;
	DWORD		tSendMessageID;

	HANDLE		htCreatePipes;
	DWORD		tCreatePipesID;

	HANDLE		hStdout = GetStdHandle(STD_OUTPUT_HANDLE); //Handle to stdout to clear screen ##DELETE-ME after May 12th##

	SYSTEM_INFO	SysInfo;
	DWORD		dwSysGran;

	GetSystemInfo(&SysInfo);									//Used to get system granularity
	dwSysGran = SysInfo.dwAllocationGranularity;				//Used to get system granularity

	cThread.SMemViewServer.QuadPart		= ((sizeof(GameData) / dwSysGran)*dwSysGran) + dwSysGran;
	cThread.SMemViewGateway.QuadPart	= ((sizeof(SMMessage) / dwSysGran)*dwSysGran) + dwSysGran;
	cThread.SMemSize.QuadPart			= cThread.SMemViewServer.QuadPart + cThread.SMemViewGateway.QuadPart;

	cThread.ThreadMustGoOn = 1;

	//###############################################################################################################
	//################################## WE SOULD MOVE THIS FURTHER DOWN... EVENTUALLY ##############################
	//###############################################################################################################

	htCreatePipes = CreateThread(
		NULL,							//Thread security attributes
		0,								//Stack size
		CreatePipes,					//Thread function name
		(LPVOID)&cThread.ThreadMustGoOn,//Thread parameter struct
		0,								//Creation flags
		&tCreatePipesID);				//gets thread ID to close it afterwards
	if (htCreatePipes == NULL) {
		_tprintf(TEXT("[Error] Creating thread CreatePipes (%d) at Gateway\n"), GetLastError());
	}

	//cThread.ThreadMustGoOn = 0;
	//WaitForSingleObject(htCreatePipes, INFINITE); //COMMENT HERE TO TEST GAME

	//###############################################################################################################

	hCanBootNow = OpenEvent(EVENT_ALL_ACCESS, FALSE, EVE_BOOT);
	if (!hCanBootNow) {
		hCanBootNow = CreateEvent(NULL, FALSE, FALSE, EVE_BOOT);
		_tprintf(TEXT("Waiting for server to boot.\n"));
		WaitForSingleObject(hCanBootNow, INFINITE);
	}

	cThread.mhGameData = createGameDataMutex();			//Mutex to sync read and write of gameData
	if (cThread.mhGameData == NULL) {
		_tprintf(TEXT("[Error] Mutex GameDataMutex (%d)\n"), GetLastError());
		return -1;
	}

	cThread.mhStructSync = OpenMutex(				
		MUTEX_ALL_ACCESS,							//Desired access
		FALSE,										//Inherit handle by child processes
		STRUCT_SYNC);								//Event name
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

	cThread.hSMServerUpdate = OpenEvent(	//Opens the event to warn gateway that the shared memoy is mapped
		EVENT_ALL_ACCESS, 							//Desired access
		FALSE, 										//Inherit handle by child processes
		EVE_SERV_UP);								//Event name
	if (cThread.hSMServerUpdate == NULL) {
		_tprintf(TEXT("[Error] Event server update (%d)\n"), GetLastError());
		return -1;
	}

	cThread.hSMGatewayUpdate = OpenEvent(	//Opens the event to warn server that the shared memoy is mapped
		EVENT_ALL_ACCESS, 							//Desired access
		FALSE, 										//Inherit handle by child processes
		EVE_GATE_UP);								//Event name
	if (cThread.hSMGatewayUpdate == NULL) {
		_tprintf(TEXT("[Error] Event gateway update (%d)\n"), GetLastError());
		return ;
	}

	//Opens a mapped file by the server
	if (sharedMemory(&cThread.hSMem, NULL) == -1) {
		_tprintf(TEXT("[Error] Opening file mapping (%d)\n"), GetLastError());
		return -1;
	}

	//Creates a view of the desired part <GameDataView>
	if (mapGameDataView(&cThread, FILE_MAP_READ) == -1) {		//Checks for errors
		_tprintf(TEXT("[Error] Mapping GameDataView (%d) at Gateway. Is the server running?\n"), GetLastError());
		return -1;
	}

	//Creates a view of the desired part <MsgView>
	if (mapMsgView(&cThread) == -1) {		//Checks for errors
		_tprintf(TEXT("[Error] Mapping MsgView (%d)\n at Gateway"), GetLastError());
		return -1;
	}


	htSReadMsg = CreateThread(
		NULL,					//Thread security attributes
		0,						//Stack size
		ReadServerMsg,			//Thread function name
		(LPVOID)&cThread,		//Thread parameter struct
		0,						//Creation flags
		&tRSMsgID);				//gets thread ID to close it afterwards
	if (htSReadMsg == NULL) {
		_tprintf(TEXT("[Error] Creating thread ReadMsg (%d) at Gateway\n"), GetLastError());
	}

	hSSendMessage = CreateThread(
		NULL,					//Thread security attributes
		0,						//Stack size
		sendMessage,			//Thread function name
		(LPVOID)&cThread,		//Thread parameter struct
		0,						//Creation flags
		&tSendMessageID);		//gets thread ID to close it afterwards
	if (hSSendMessage == NULL) {
		_tprintf(TEXT("[Error] Creating thread SendMessage (%d) at Gateway\n"), GetLastError());
	}

	WaitForSingleObject(htSReadMsg, INFINITE);
	WaitForSingleObject(hSSendMessage, INFINITE);

	UnmapViewOfFile(cThread.pSMemGameData);		//Unmaps view of shared memory
	UnmapViewOfFile(cThread.pSMemMessage);		//Unmaps view of shared memory
	CloseHandle(cThread.hSMem);

	return 0;
}