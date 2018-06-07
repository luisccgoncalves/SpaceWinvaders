#include "comm.h"

DWORD WINAPI sendPacketServer(LPVOID tParam) {
	/*
	This message calls the DLL function writePacket
	and sends a packet to the server trhough the Shared Memmory
	*/

	SMCtrl		*cThread = (SMCtrl*)tParam;
	//Packet		localpacket;
	int			nextIn = 0;

	while (cThread->ThreadMustGoOn) {
		WaitForSingleObject(cThread->heGotPacket, INFINITE);
		_tprintf(TEXT("[DEBUG] GOT KEY %d \n"), cThread->localPacket.instruction);
		writePacket(cThread, &nextIn, cThread->localPacket);
	}

	return 0;
}

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

	while (pipeStruct.ThreadMustGoOn) {

		readPipePacket(pipeStruct.hPipe, heReadReady, &instancePacket);
		_tprintf(TEXT("GOT KEY %d "), instancePacket.instruction);

		WaitForSingleObject(mhReadPipe, INFINITE);

		*pipeStruct.localPacket = instancePacket;

		ReleaseMutex(mhReadPipe);
		SetEvent(*pipeStruct.heGotPacket);

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