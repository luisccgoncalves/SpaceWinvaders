#include "comm.h"

//#########################################################################################
//############################   TEMP TEST   ##############################################
//#########################################################################################

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
				return -1;
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

int readPipePacket(HANDLE hPipe, HANDLE readReady, Packet * pipePacket) { //probably the event needs to be a pointer

	OVERLAPPED	OvrRd = { 0 };
	DWORD		dwBytesRead = 0;
	BOOL		bSuccess = FALSE;

	//Packet		pipePacket;

	OvrRd.hEvent = readReady;
	ResetEvent(readReady);

	bSuccess = ReadFile(
		hPipe,
		pipePacket,
		sizeof(Packet),
		&dwBytesRead,
		&OvrRd);

	WaitForSingleObject(readReady, INFINITE);

	GetOverlappedResult(
		hPipe,
		&OvrRd,
		&dwBytesRead,
		FALSE);

	if (dwBytesRead < sizeof(Packet)) {
		if (GetLastError() == ERROR_BROKEN_PIPE) {
			_tprintf(TEXT("[Error] Connection lost.\n"));
			return -1;
		}
		else
			_tprintf(TEXT("[Error] ReadFile failed. Error = %d \n"), GetLastError());
	}

	return 0;
}




//#########################################################################################
/*
To test move this to client
*/

/*
void simulClient(Packet * localpacket) {

	localpacket->owner = 0;

	localpacket->instruction = RandomValue(5);

	Sleep(500);
	return;
}
*/
//#########################################################################################


int _tmain(int argc, LPTSTR argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	SMCtrl		cThread;
	HANDLE		hCanBootNow;

	//HANDLE		hSSendMessage;
	//DWORD		tSendMessageID;

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

	cThread.mhGameData = createGameDataMutex();			//Mutex to sync read and write of localGameData
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

	cThread.heGotPacket = CreateEvent(
		NULL, 										//Event attributes
		FALSE, 										//Manual reset (TRUE for auto-reset)
		FALSE, 										//Initial state
		NULL);										//Event name
	if (cThread.heGotPacket == NULL) {
		_tprintf(TEXT("[Error] Event server update (%d)\n"), GetLastError());
		return -1;
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

	//hSSendMessage = CreateThread(
	//	NULL,								//Thread security attributes
	//	0,									//Stack size
	//	sendPacketServer,					//Thread function name
	//	(LPVOID)&cThread,					//Thread parameter struct
	//	0,									//Creation flags
	//	&tSendMessageID);					//gets thread ID to close it afterwards
	//if (hSSendMessage == NULL) {
	//	_tprintf(TEXT("[Error] Creating thread SendMessage (%d) at Gateway\n"), GetLastError());
	//}

	//cThread.ThreadMustGoOn = 0;

	//WaitForSingleObject(hSSendMessage, INFINITE);
	WaitForSingleObject(htCreatePipes, INFINITE);

	UnmapViewOfFile(cThread.pSMemGameData);		//Unmaps view of shared memory
	UnmapViewOfFile(cThread.pSMemMessage);		//Unmaps view of shared memory
	CloseHandle(cThread.hSMem);

	return 0;
}