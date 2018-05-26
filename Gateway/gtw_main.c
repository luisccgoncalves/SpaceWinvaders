#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "../DLL/dll.h"
#include "../Client/debug.h"


typedef struct {
	
	HANDLE		hPipe;
	int			*ThreadMustGoOn;

	HANDLE		*hSMServerUpdate;
	GameData	*gameData;

}PipeInst;

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

		fSuccess = WriteFile(
			hPipe,
			&msg,
			sizeof(msg),
			&cbWriten,
			&overlWrite);
		if (!fSuccess) {
			if (GetLastError() == ERROR_IO_INCOMPLETE)
				return;
		}

		WaitForSingleObject(writeReady, INFINITE);

		GetOverlappedResult(
			hPipe,
			&overlWrite,
			&cbWriten,
			FALSE);
		if (cbWriten < sizeof(msg)) {
			_tprintf(TEXT("[Error] OVERLAPPED writePipeMsgs (%d)\n"), GetLastError());
			return 1;
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
	
	PipeInst	pipeStruct = *(PipeInst*)tParam;
	HANDLE		heWriteReady;
	BOOL		fSuccess = FALSE;
	GameData	clientMsg;

	if (pipeStruct.hPipe == NULL) {
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
	while (pipeStruct.ThreadMustGoOn) {

		WaitForSingleObject(pipeStruct.hSMServerUpdate,INFINITE);
		writePipeMsg(pipeStruct.hPipe, heWriteReady, *pipeStruct.gameData);

	}
	
	_tprintf(TEXT("Sent...\n"));
	return 0;
}

DWORD WINAPI CreatePipes(LPVOID tParam) {

	SMCtrl		*cThread= (SMCtrl*)tParam;
	LPTSTR		lpsPipeName = PIPE_NAME;
	HANDLE		clients[MAX_PLAYERS] = {0};
	HANDLE		h1stPipeInst;
	HANDLE		htPipeConnect[40] = { NULL }; //Update this

	HANDLE		hPipe = INVALID_HANDLE_VALUE;

	BOOL		fConnected = FALSE;
	DWORD		dwPipeThreadId;
	int			threadn = 0;

	PipeInst	pipeStruct;

	pipeStruct.ThreadMustGoOn = cThread->ThreadMustGoOn;
	pipeStruct.gameData = &cThread->gameData;
	pipeStruct.hSMServerUpdate = cThread->hSMServerUpdate;

	h1stPipeInst = CreateEvent(				//Creates the event to warn gateway that the shared memoy is mapped
		NULL,										//Event attributes
		TRUE,										//Manual reset (TRUE for auto-reset)
		FALSE,										//Initial state
		EVE_1ST_PIPE);								//Event name
	if (h1stPipeInst  == NULL) {
		_tprintf(TEXT("[Error] Event 1st pipe instance (%d)\n"), GetLastError());
		return -1;
	}

	while (cThread->ThreadMustGoOn) {
	
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

			pipeStruct.hPipe = hPipe;

			htPipeConnect[threadn] = CreateThread(
				NULL,									//Thread security attributes
				0,										//Stack size
				instanceThread,							//Thread function name
				(LPVOID)&pipeStruct,						//Thread parameter struct
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
	
	//GameData	*gameMsg;
	//gameMsg = malloc(sizeof(GameData));

	int i;

	cls(hStdout);
	hidecursor();

	while (cThread->ThreadMustGoOn) {

		WaitForSingleObject(cThread->hSMServerUpdate, INFINITE);
		WaitForSingleObject(cThread->mhStructSync, INFINITE);

		//gameMsg = cThread->pSMemGameData;
		//Copies shared memory to a local data structure
		cThread->gameData = *cThread->pSMemGameData;
		
		ReleaseMutex(cThread->mhStructSync);

		cls(hStdout);
		for (i = 0; i < MAX_INVADER; i++) {
			if (cThread->gameData.invad[i].x) {
				gotoxy(cThread->gameData.invad[i].x, cThread->gameData.invad[i].y);
				if (cThread->gameData.invad[i].rand_path)
					_tprintf(TEXT("X"));
				else
					_tprintf(TEXT("W"));
			}
		}

		if (cThread->pSMemGameData->bomb[0].y < 25) { //this needs another aproach (fired state?)
			gotoxy(cThread->gameData.bomb[0].x, cThread->gameData.bomb[0].y);
			_tprintf(TEXT("o"));
		}

		for (i = 0; i < MAX_PLAYERS; i++) {
			gotoxy(cThread->gameData.ship[i].x, cThread->gameData.ship[i].y);
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

	cThread.mhProdConsMut = OpenMutex(
		MUTEX_ALL_ACCESS,							//Desired access
		FALSE,										//Inherit handle by child processes
		MUT_PRODCONS);								//Event name
	if (cThread.mhProdConsMut == NULL) {
		_tprintf(TEXT("[Error] Mutex ProdCons (%d)\n"), GetLastError());
		return -1;
	}

	cThread.shVacant = OpenSemaphore(		//It starts with full vacancies
		SEMAPHORE_ALL_ACCESS,						//Desired access
		FALSE,										//Inherit handle by child processes
		SEM_VACANT);								//Semaphore name
	if (cThread.shVacant == NULL) {
		_tprintf(TEXT("[Error] Semaphore vacant (%d)\n"), GetLastError());
		return -1;
	}

	cThread.shOccupied = OpenSemaphore(		//It starts without occupation
		SEMAPHORE_ALL_ACCESS,						//Desired access
		FALSE,										//Inherit handle by child processes
		SEM_OCCUPIED);								//Semaphore name
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

	htCreatePipes = CreateThread(
		NULL,							//Thread security attributes
		0,								//Stack size
		CreatePipes,					//Thread function name
		(LPVOID)&cThread,				//Thread parameter struct
		0,								//Creation flags
		&tCreatePipesID);				//gets thread ID to close it afterwards
	if (htCreatePipes == NULL) {
		_tprintf(TEXT("[Error] Creating thread CreatePipes (%d) at Gateway\n"), GetLastError());
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

	//cThread.ThreadMustGoOn = 0;

	WaitForSingleObject(htSReadMsg, INFINITE);
	WaitForSingleObject(hSSendMessage, INFINITE);
	WaitForSingleObject(htCreatePipes, INFINITE);

	UnmapViewOfFile(cThread.pSMemGameData);		//Unmaps view of shared memory
	UnmapViewOfFile(cThread.pSMemMessage);		//Unmaps view of shared memory
	CloseHandle(cThread.hSMem);

	return 0;
}