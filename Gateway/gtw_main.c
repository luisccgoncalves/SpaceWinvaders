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
#define BUFSIZE 2048
#define MAXCLIENTS 10	//SUB THIS FOR MAXPLAYERS -> AND UPDATE MAXPLAYERS


void startClients(HANDLE *c) {
	for (int i = 0; i < MAXCLIENTS; i++) {
		c[i] = NULL;
	}
}

void addClient(HANDLE *c, HANDLE *newClient) {//how to detect connection?
	for (int i = 0; i < MAXCLIENTS; i++) {
		if (c[i] == NULL) {
			c[i] = newClient;
			return;
		}
	}
}

void removeClient(HANDLE *c, HANDLE *oldClient) {//how to detect connection?
	for (int i = 0; i < MAXCLIENTS; i++) {
		if (c[i] == oldClient) {				//thread must clear HANDLE
			c[i] = NULL;
			return;
		}
	}
}

DWORD WINAPI instanceThread() {
	BOOL fSuccess = FALSE;

	return 0;
}

DWORD WINAPI CreatePipes() {

	HANDLE clients[MAXCLIENTS];
	HANDLE hPipe;
	LPTSTR	lpsPipeName = PIPE_NAME;
	HANDLE hpThread;
	HANDLE writeReady;

	BOOL fConnected = FALSE;
	DWORD dwPipeThreadId;

	writeReady = CreateEvent(			
		NULL, 										//Event attributes
		TRUE, 										//Manual reset (TRUE for auto-reset)
		FALSE, 										//Initial state
		NULL);										//Event name
	if (writeReady == NULL) {
		_tprintf(TEXT("[Error] Event writeReady (%d)\n"), GetLastError());
		return -1;
	}
	startClients(clients);
	
	while (1) {
		hPipe = CreateNamedPipe(
			lpsPipeName,
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			BUFSIZE,
			BUFSIZE,
			5000,														//5 segundos
			NULL);
		if (hPipe == NULL) {
			_tprintf(TEXT("[Error] Creating NamePipe (%d)\n"), GetLastError());
			return -1;
		}

		fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		if (fConnected) {
			hpThread = CreateThread(
				NULL,									//Thread security attributes
				0,										//Stack size
				instanceThread,							//Thread function name
				NULL,									//Thread parameter struct
				0,										//Creation flags
				&dwPipeThreadId);						//gets thread ID to close it afterwards

		}

	}

}


//#########################################################################################

void simulClient(packet * localpacket) {

	srand((unsigned)time(NULL));

	localpacket->owner = 0;

	localpacket->instruction = rand() % 4;

	Sleep(500);
	return;
}

DWORD WINAPI sendMessage(LPVOID tParam) {

	SMCtrl		*cThread = (SMCtrl*)tParam;

	packet		localpacket;

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
	SMGameData	*gameMsg;

	gameMsg = malloc(sizeof(SMGameData));

	int i;

	cls(hStdout);
	hidecursor();

	while (cThread->ThreadMustGoOn) {

		WaitForSingleObject(cThread->hSMServerUpdate, INFINITE);
		WaitForSingleObject(cThread->mhStructSync, INFINITE);
		CopyMemory(gameMsg, cThread->pSMemGameData, sizeof(SMGameData));
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

	HANDLE		hSsendMessage;
	DWORD		tSendMessageID;

	HANDLE		htCreatePipes;
	DWORD		tCreatePipesID;

	HANDLE		hStdout = GetStdHandle(STD_OUTPUT_HANDLE); //Handle to stdout to clear screen ##DELETE-ME after May 12th##

	SYSTEM_INFO	SysInfo;
	DWORD		dwSysGran;

	GetSystemInfo(&SysInfo);									//Used to get system granularity
	dwSysGran = SysInfo.dwAllocationGranularity;				//Used to get system granularity

	cThread.SMemViewServer.QuadPart		= ((sizeof(SMGameData) / dwSysGran)*dwSysGran) + dwSysGran;
	cThread.SMemViewGateway.QuadPart	= ((sizeof(SMMessage) / dwSysGran)*dwSysGran) + dwSysGran;
	cThread.SMemSize.QuadPart			= cThread.SMemViewServer.QuadPart + cThread.SMemViewGateway.QuadPart;

	cThread.ThreadMustGoOn = 1;

	//###############################################################################################################
	//################################## WE SOULD MOVE THIS FURTHER DOWN... EVENTUALLY ##############################
	//###############################################################################################################

	htCreatePipes = CreateThread(
		NULL,					//Thread security attributes
		0,						//Stack size
		CreatePipes,			//Thread function name
		NULL,					//Thread parameter struct
		0,						//Creation flags
		&tCreatePipesID);		//gets thread ID to close it afterwards

	WaitForSingleObject(htCreatePipes, INFINITE);

	//###############################################################################################################

	hCanBootNow = OpenEvent(EVENT_ALL_ACCESS, FALSE, EVE_BOOT);
	if (!hCanBootNow) {
		hCanBootNow = CreateEvent(NULL, FALSE, FALSE, EVE_BOOT);
		_tprintf(TEXT("Waiting for server to boot.\n"));
		WaitForSingleObject(hCanBootNow, INFINITE);
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


	htSReadMsg = CreateThread(
		NULL,					//Thread security attributes
		0,						//Stack size
		ReadServerMsg,			//Thread function name
		(LPVOID)&cThread,		//Thread parameter struct
		0,						//Creation flags
		&tRSMsgID);				//gets thread ID to close it afterwards

	hSsendMessage = CreateThread(
		NULL,					//Thread security attributes
		0,						//Stack size
		sendMessage,			//Thread function name
		(LPVOID)&cThread,		//Thread parameter struct
		0,						//Creation flags
		&tSendMessageID);		//gets thread ID to close it afterwards

	WaitForSingleObject(htSReadMsg, INFINITE);

	UnmapViewOfFile(cThread.pSMemGameData);		//Unmaps view of shared memory
	UnmapViewOfFile(cThread.pSMemMessage);		//Unmaps view of shared memory
	CloseHandle(cThread.hSMem);

	return 0;
}