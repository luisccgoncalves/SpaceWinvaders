#include "comm.h"

//DWORD WINAPI sendPacketServer(LPVOID tParam) {
//	/*
//	This message calls the DLL function writePacket
//	and sends a packet to the server trhough the Shared Memmory
//	*/
//
//	SMCtrl		*cThread = (SMCtrl*)tParam;
//	//Packet		localpacket;
//	int			nextIn = 0;
//
//	while (cThread->ThreadMustGoOn) {
//		WaitForSingleObject(cThread->heGotPacket, INFINITE);
//		_tprintf(TEXT("[DEBUG] GOT KEY %d \n"), cThread->localPacket.instruction);
//		writePacket(cThread, &nextIn, cThread->localPacket);
//	}
//
//	return 0;
//}

//DWORD WINAPI ReadServerMsg(LPVOID tParam) {
//
//	SMCtrl		*cThread = (SMCtrl*)tParam;
//
//	while (cThread->ThreadMustGoOn) {
//
//		WaitForSingleObject(cThread->hSMServerUpdate, INFINITE);
//		cThread->localGameData = consumeGameData(cThread->pSMemGameData, cThread->mhGameData);
//		//setEvent to pipes
//	}
//
//	return 0;
//}

DWORD WINAPI instanceThreadRead(LPVOID tParam) {
	PipeInstRd	pipeStruct = *(PipeInstRd*)tParam;
	HANDLE		heReadReady;
	HANDLE		mhReadPipe;
	BOOL		fSuccess = FALSE;
	int			nextIn = 0;
	
	Packet		instancePacket;

	if (pipeStruct.hPipe == NULL) {
		_tprintf(TEXT("[Error] instanceThreadRead casting pipe. (%d)\n"), GetLastError());
		return -1;
	}

	heReadReady = CreateEvent(			//Creates the event to signal access to write action
		NULL, 										//Event attributes
		TRUE, 										//Manual reset (TRUE for auto-reset)
		FALSE, 										//Initial state
		NULL);										//Event name
	if (heReadReady == NULL) {
		_tprintf(TEXT("[Error] Event writeReady (%d)\n"), GetLastError());
		return -1;
	}

	mhReadPipe = OpenMutex(
		MUTEX_ALL_ACCESS,							//Desired access
		FALSE,										//Inherit handle by child processes
		TEXT("instanceThreadRead"));				//Event name
	if (mhReadPipe == NULL) {
		mhReadPipe = CreateMutex(					//Used for game structure integrity
			NULL,									//Security attributes
			FALSE,									//Initial owner
			TEXT("instanceThreadRead"));			//Mutex name
	}

	while (pipeStruct.cThread->ThreadMustGoOn) {

		readPipePacket(pipeStruct.hPipe, heReadReady, &instancePacket);
		_tprintf(TEXT("[DEBUG] GOT KEY %d "), instancePacket.instruction);

		WaitForSingleObject(mhReadPipe, INFINITE);

		writePacket(pipeStruct.cThread, &nextIn, instancePacket);

		ReleaseMutex(mhReadPipe);

	}
	return 0;
}

DWORD WINAPI instanceThreadWrite(LPVOID tParam) {

	PipeInstWrt	pipeStruct = *(PipeInstWrt*)tParam;
	HANDLE		heWriteReady;
	BOOL		fSuccess = FALSE;
	GameData	localGameData;

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

		WaitForSingleObject(pipeStruct.hSMServerUpdate, INFINITE);
		localGameData = consumeGameData(pipeStruct.pSMemGameData, pipeStruct.mhGameData);
		writePipeMsg(pipeStruct.hPipe, heWriteReady, localGameData);
	}

	return 0;
}

DWORD WINAPI CreatePipes(LPVOID tParam) {

	SMCtrl		*cThread = (SMCtrl*)tParam;
	LPTSTR		lpsPipeName = PIPE_NAME;

	HANDLE		h1stPipeInst;
	HANDLE		htPipeConnectW[40] = { NULL }; //Update this
	HANDLE		htPipeConnectR[40] = { NULL }; //Update this
	int			threadn = 0;

	HANDLE		hPipe = INVALID_HANDLE_VALUE;

	BOOL		fConnected = FALSE;

	PipeInstWrt	pipeStructWrite;
	PipeInstRd	pipeStructRead;

	pipeStructWrite.ThreadMustGoOn = &cThread->ThreadMustGoOn;
	pipeStructWrite.hSMServerUpdate = cThread->hSMServerUpdate;
	pipeStructWrite.mhGameData = cThread->mhGameData;
	pipeStructWrite.pSMemGameData = cThread->pSMemGameData;

	pipeStructRead.cThread = cThread;

	h1stPipeInst = CreateEvent(				//Creates the event to warn clients that the 1st pipe instance was created
		NULL,										//Event attributes
		TRUE,										//Manual reset (TRUE for auto-reset)
		FALSE,										//Initial state
		EVE_1ST_PIPE);								//Event name
	if (h1stPipeInst == NULL) {
		_tprintf(TEXT("[Error] Event 1st pipe instance (%d)\n"), GetLastError());
		return -1;
	}

	while (cThread->ThreadMustGoOn) {

		hPipe = CreateNamedPipe(
			lpsPipeName,
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT | PIPE_ACCEPT_REMOTE_CLIENTS,
			MAX_PLAYERS,
			BUFSIZE,
			BUFSIZE,
			5000,														//5 secs timeout
			NULL);

		if (GetLastError() == ERROR_PIPE_BUSY) {
			_tprintf(TEXT("[Error] Max players Reached\n"));
			//WaitForSingleObject until threadn<MAX_PLAYERS
			return -1;
		}
		if (hPipe == INVALID_HANDLE_VALUE) {
			_tprintf(TEXT("[Error] Creating NamePipe (%d)\n"), GetLastError());
			return -1;
		}

		if (!threadn)
			SetEvent(h1stPipeInst);

		fConnected = ConnectNamedPipe(hPipe, NULL) ?
			TRUE :
			(GetLastError() == ERROR_PIPE_CONNECTED);

		if (fConnected) {

			_tprintf(TEXT("[DEBUG] Someone connected!\n"));

			pipeStructWrite.hPipe = hPipe;
			pipeStructRead.hPipe = hPipe;

			htPipeConnectR[threadn] = CreateThread(
				NULL,									//Thread security attributes
				0,										//Stack size
				instanceThreadRead,						//Thread function name
				(LPVOID)&pipeStructRead,				//Thread parameter struct
				0,										//Creation flags
				NULL);									//gets thread ID to close it afterwards
			if (htPipeConnectR[threadn] == NULL) {
				_tprintf(TEXT("[Error] Creating thread ConnectPipesThreadRead (%d) at Gateway\n"), GetLastError());
				return -1;
			}
			else {

				htPipeConnectW[threadn] = CreateThread(
					NULL,								//Thread security attributes
					0,									//Stack size
					instanceThreadWrite,				//Thread function name
					(LPVOID)&pipeStructWrite,			//Thread parameter struct
					0,									//Creation flags
					NULL);								//gets thread ID to close it afterwards
				if (htPipeConnectW[threadn] == NULL) {
					_tprintf(TEXT("[Error] Creating thread ConnectPipesThreadWrite (%d) at Gateway\n"), GetLastError());
					return -1;
				}
				else {
					threadn++;
				}
			}
		}
		else {
			CloseHandle(hPipe);							//Frees this pipe instance
		}

	}

	WaitForMultipleObjects(threadn, htPipeConnectW, TRUE, INFINITE);
	WaitForMultipleObjects(threadn, htPipeConnectR, TRUE, INFINITE);

	return 0;
}