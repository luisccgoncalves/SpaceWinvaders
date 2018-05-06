#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "../Server/structs.h"
#include "../DLL/dll.h"
#include "../Client/debug.h"

typedef struct {
	//HANDLE			mhInvader;				//Handle to mutex (TEST)
	SMCtrl			smCtrl;					//Shared memory structure
	int				ThreadMustGoOn;			//Flag for thread shutdown
} SMCtrl_Thread;

DWORD WINAPI ReadServerMsg(LPVOID tParam) {				//Warns gateway of structure updates

	SMCtrl_Thread	*cThread;
	cThread = (SMCtrl_Thread*)tParam;
	HANDLE			hStdout = GetStdHandle(STD_OUTPUT_HANDLE); //Handle to stdout to clear screen ##DELETE-ME after May 12th##
	int i;

	cls(hStdout);
	hidecursor();

	while (cThread->ThreadMustGoOn) {
		WaitForSingleObject(cThread->smCtrl.hSMServerUpdate, INFINITE);
		cls(hStdout);
		for (i = 0; i < MAX_INVADER; i++) {
			gotoxy(cThread->smCtrl.pSMemServer->invad[i].x, cThread->smCtrl.pSMemServer->invad[i].y);
			_tprintf(TEXT("W"));
		}
		
		//SetEvent(cThread->smCtrl.hSMGatewayUpdate);
	}
}

int _tmain(int argc, LPTSTR argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	SMCtrl_Thread	cThread;
	HANDLE			hCanBootNow;
	HANDLE			htSReadMsg;
	HANDLE			hStdout = GetStdHandle(STD_OUTPUT_HANDLE); //Handle to stdout to clear screen ##DELETE-ME after May 12th##

	DWORD			tRSMsgID;

	SYSTEM_INFO		SysInfo;
	DWORD			dwSysGran;

	GetSystemInfo(&SysInfo);									//Used to get system granularity
	dwSysGran = SysInfo.dwAllocationGranularity;				//Used to get system granularity

	cThread.smCtrl.SMemViewServer.QuadPart = ((sizeof(SMServer_MSG) / dwSysGran)*dwSysGran) + dwSysGran;
	cThread.smCtrl.SMemViewGateway.QuadPart = ((sizeof(SMGateway_MSG) / dwSysGran)*dwSysGran) + dwSysGran;
	cThread.smCtrl.SMemSize.QuadPart = cThread.smCtrl.SMemViewServer.QuadPart + cThread.smCtrl.SMemViewGateway.QuadPart;

	cThread.ThreadMustGoOn = 1;

	//hCanBootNow = OpenEvent(EVENT_ALL_ACCESS, FALSE, (LPTSTR)TEXT("LetsBoot")); *WIP from LG
	hCanBootNow = CreateEvent(NULL, FALSE, FALSE, TEXT("LetsBoot"));

	cThread.smCtrl.hSMServerUpdate = CreateEvent(	//Creates the event to warn gateway that the shared memoy is mapped
		NULL, 										//Event attributes
		FALSE, 										//Manual reset (TRUE for auto-reset)
		FALSE, 										//Initial state
		TEXT("SMServerUpdate"));					//Event name

	cThread.smCtrl.hSMGatewayUpdate = CreateEvent(	//Creates the event to warn server that the shared memoy is mapped
		NULL, 										//Event attributes
		FALSE, 										//Manual reset (TRUE for auto-reset)
		FALSE, 										//Initial state
		TEXT("SMGatewayUpdate"));					//Event name

	_tprintf(TEXT("Detecting if server is running.\n"));
	WaitForSingleObject(hCanBootNow, 5000);

	//Opens a mapped file by the server
	cThread.smCtrl.hSMem = OpenFileMapping(
		FILE_MAP_ALL_ACCESS, 
		FALSE, 
		SMName);

	if (cThread.smCtrl.hSMem == NULL) {				//Checks for errors
		_tprintf(TEXT("[Error] Opening file mapping (%d)\nIs the server running?\n"), GetLastError());
		return -1;
	}

	//Creates a view of the desired part <Server>
	mapServerView(&cThread.smCtrl);
	if (cThread.smCtrl.pSMemServer == NULL) {		//Checks for errors
		_tprintf(TEXT("[Error] Mapping memory (%d)\nIs the server running?\n"), GetLastError());
		return -1;
	}

	//Creates a view of the desired part <Gateway>
	mapGatewayView(&cThread.smCtrl);
	if (cThread.smCtrl.pSMemGateway == NULL) {		//Checks for errors
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

	/*cls(hStdout);
	hidecursor();*/
	//while (cThread.ThreadMustGoOn) {
	//	//WaitForSingleObject(cThread.smCtrl.hSMServerUpdate, INFINITE);
	//	//cls(hStdout);
	//	//gotoxy(cThread.smCtrl.pSMemServer->pSMem.x, cThread.smCtrl.pSMemServer->pSMem.y);
	//	//_tprintf(TEXT("W"));
	//	//SetEvent(cThread.smCtrl.hSMGatewayUpdate);
	//}

	UnmapViewOfFile(cThread.smCtrl.pSMemServer);		//Unmaps view of shared memory
	UnmapViewOfFile(cThread.smCtrl.pSMemGateway);		//Unmaps view of shared memory
	CloseHandle(cThread.smCtrl.hSMem);

	return 0;
}