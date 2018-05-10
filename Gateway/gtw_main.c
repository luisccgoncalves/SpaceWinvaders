#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "../DLL/dll.h"
#include "../Client/debug.h"

DWORD WINAPI ReadServerMsg(LPVOID tParam) {				//Warns gateway of structure updates

	SMCtrl	*cThread;
	cThread = (SMCtrl*)tParam;
	HANDLE			hStdout = GetStdHandle(STD_OUTPUT_HANDLE); //Handle to stdout to clear screen ##DELETE-ME after May 12th##
	
	SMMessage *msg = ((SMCtrl *)tParam)->pSMemMessage;

	int i;

	cls(hStdout);
	hidecursor();

	while (cThread->ThreadMustGoOn) {
		WaitForSingleObject(cThread->hSMServerUpdate, INFINITE);
		cls(hStdout);
		for (i = 0; i < MAX_INVADER; i++) {
			gotoxy(cThread->pSMemGameData->invad[i].x, cThread->pSMemGameData->invad[i].y);
			if (cThread->pSMemGameData->invad[i].rand_path)
				_tprintf(TEXT("X"));
			else
				_tprintf(TEXT("W"));
			if (cThread->pSMemGameData->bomb.y < 25) {
				gotoxy(cThread->pSMemGameData->bomb.x, cThread->pSMemGameData->bomb.y);
				_tprintf(TEXT("0"));
			}
		}
		//temporary test
		msg->details = 1;

		SetEvent(cThread->hSMGatewayUpdate);
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
	HANDLE		hStdout = GetStdHandle(STD_OUTPUT_HANDLE); //Handle to stdout to clear screen ##DELETE-ME after May 12th##

	DWORD		tRSMsgID;

	SYSTEM_INFO	SysInfo;
	DWORD		dwSysGran;

	GetSystemInfo(&SysInfo);									//Used to get system granularity
	dwSysGran = SysInfo.dwAllocationGranularity;				//Used to get system granularity

	cThread.SMemViewServer.QuadPart		= ((sizeof(SMGameData) / dwSysGran)*dwSysGran) + dwSysGran;
	cThread.SMemViewGateway.QuadPart	= ((sizeof(SMMessage) / dwSysGran)*dwSysGran) + dwSysGran;
	cThread.SMemSize.QuadPart			= cThread.SMemViewServer.QuadPart + cThread.SMemViewGateway.QuadPart;

	cThread.ThreadMustGoOn = 1;

	hCanBootNow = OpenEvent(EVENT_ALL_ACCESS, FALSE, TEXT("LetsBoot"));
	if (!hCanBootNow) {
		hCanBootNow = CreateEvent(NULL, FALSE, FALSE, TEXT("LetsBoot"));
		_tprintf(TEXT("Waiting for server to boot.\n"));
		WaitForSingleObject(hCanBootNow, INFINITE);
	}

	cThread.hSMServerUpdate = CreateEvent(	//Creates the event to warn gateway that the shared memoy is mapped
		NULL, 										//Event attributes
		FALSE, 										//Manual reset (TRUE for auto-reset)
		FALSE, 										//Initial state
		TEXT("SMServerUpdate"));					//Event name

	cThread.hSMGatewayUpdate = CreateEvent(	//Creates the event to warn server that the shared memoy is mapped
		NULL, 										//Event attributes
		FALSE, 										//Manual reset (TRUE for auto-reset)
		FALSE, 										//Initial state
		TEXT("SMGatewayUpdate"));					//Event name

	//Opens a mapped file by the server
	if (sharedMemory(&cThread.hSMem, NULL) == -1) {
		_tprintf(TEXT("[Error] Opening file mapping (%d)\n"), GetLastError());
		return -1;
	}

	if (cThread.hSMem == NULL) {				//Checks for errors
		_tprintf(TEXT("[Error] Opening file mapping (%d)\n"), GetLastError());
		return -1;
	}

	//Creates a view of the desired part <Server>
	mapServerView(&cThread);
	if (cThread.pSMemGameData== NULL) {		//Checks for errors
		_tprintf(TEXT("[Error] Mapping memory (%d)\nIs the server running?\n"), GetLastError());
		return -1;
	}

	//Creates a view of the desired part <Gateway>
	mapGatewayView(&cThread);
	if (cThread.pSMemMessage== NULL) {		//Checks for errors
		_tprintf(TEXT("[Error] Mapping memory (%d)\n @ Gateway"), GetLastError());
		return -1;
	}

	htSReadMsg = CreateThread(
		NULL,					//Thread security attributes
		0,						//Stack size
		ReadServerMsg,			//Thread function name
		(LPVOID)&cThread,		//Thread parameter struct
		0,						//Creation flags
		&tRSMsgID);				//gets thread ID to close it afterwards

	WaitForSingleObject(htSReadMsg, INFINITE);

	UnmapViewOfFile(cThread.pSMemGameData);		//Unmaps view of shared memory
	UnmapViewOfFile(cThread.pSMemMessage);		//Unmaps view of shared memory
	CloseHandle(cThread.hSMem);

	return 0;
}