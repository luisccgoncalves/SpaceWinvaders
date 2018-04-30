#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "../Server/structs.h"
#include "../DLL/dll.h"
#include "../Client/debug.h"

/**/ //TO DLL
// Thread to read from memmory
//typedef struct {
//	HANDLE			hSMServerUpdate;		//Handle to event. Warns gateway about updates in shared memory
//	HANDLE			hSMGatewayUpdate;		//Handle to event. Warns server about updates in shared memory
//	HANDLE			mhInvader;				//Handle to mutex (TEST)
//	HANDLE			hSMem;					//Handle to shared memory
//	LARGE_INTEGER	SMemSize;				//Stores the size of the mapped file
//	LARGE_INTEGER	SMemViewServer;				//Stores the size of the mapped file
//	LARGE_INTEGER	SMemViewGateway;				//Stores the size of the mapped file
//	SMServer_MSG	*pSMemServer;			//Pointer to shared memory's first byte
//	SMGateway_MSG	*pSMGateway;			//Pointer to shared memory's first byte
//	//invader			*pSMem;					//Pointer to shared memory's first byte
//	//char			*pSMGateway;			//Pointer to shared memory's first byte
//	int				ThreadMustGoOn;			//Flag for thread shutdown
//} SMCtrl_Thread;
/**/


/**/
//typedef struct {
//	HANDLE			hSMServerUpdate;		//Handle to event. Warns gateway about updates in shared memory
//	HANDLE			mhInvader;				//Handle to mutex (TEST)
//	HANDLE			hSMem;					//Handle to shared memory
//	LARGE_INTEGER	SMemSize;				//Stores the size of the mapped file
//	LARGE_INTEGER	SMemViewServer;				//Stores the size of the mapped file
//	invader			*pSMem;					//Pointer to shared memory's first byte
//	int				ThreadMustGoOn;			//Flag for thread shutdown
//} SMCtrl_ServerThread;
//
//typedef struct {
//	HANDLE			hSMGatewayUpdate;		//Handle to event. Warns server about updates in shared memory
//	HANDLE			hSMem;					//Handle to shared memory
//	LARGE_INTEGER	SMemSize;				//Stores the size of the mapped file
//	LARGE_INTEGER	SMemViewGateway;				//Stores the size of the mapped file
//	char			*pSMGateway;			//Pointer to shared memory's first byte
//	int				ThreadMustGoOn;			//Flag for thread shutdown
//} SMCtrl_GatewayThread;
/**/

typedef struct {
	//HANDLE			mhInvader;				//Handle to mutex (TEST)
	SMCtrl			smCtrl;					//Shared memory structure
	int				ThreadMustGoOn;			//Flag for thread shutdown
} SMCtrl_Thread;

int _tmain(int argc, LPTSTR argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	SMCtrl_Thread cThread;
	HANDLE			hCanBootNow;
	HANDLE			hStdout = GetStdHandle(STD_OUTPUT_HANDLE); //Handle to stdout to clear screen ##DELETE-ME after May 12th##
	
	SYSTEM_INFO		SysInfo;
	DWORD			dwSysGran;
	DWORD			dwViewServerStart;

	GetSystemInfo(&SysInfo);									//Used to get system granularity
	dwSysGran = SysInfo.dwAllocationGranularity;				//Used to get system granularity

	cThread.smCtrl.SMemViewServer.QuadPart = ((sizeof(SMServer_MSG) / dwSysGran)*dwSysGran) + dwSysGran;
	cThread.smCtrl.SMemViewGateway.QuadPart = ((sizeof(SMGateway_MSG) / dwSysGran)*dwSysGran) + dwSysGran;
	cThread.smCtrl.SMemSize.QuadPart = cThread.smCtrl.SMemViewServer.QuadPart + cThread.smCtrl.SMemViewGateway.QuadPart;

	cThread.ThreadMustGoOn = 1;

	//hCanBootNow = OpenEvent(EVENT_ALL_ACCESS, FALSE, (LPTSTR)TEXT("LetsBoot"));
	hCanBootNow = CreateEvent(NULL, FALSE, FALSE, TEXT("LetsBoot"));

	cThread.smCtrl.hSMServerUpdate = CreateEvent(		//Creates the event to warn gateway that the shared memoy is mapped
		NULL, 									//Event attributes
		FALSE, 									//Manual reset (TRUE for auto-reset)
		FALSE, 									//Initial state
		TEXT("SMServerUpdate"));				//Event name

	//cThread.hSMGatewayUpdate = CreateEvent(		//Creates the event to warn server that the shared memoy is mapped
	//	NULL, 									//Event attributes
	//	FALSE, 									//Manual reset (TRUE for auto-reset)
	//	FALSE, 									//Initial state
	//	TEXT("SMGatewayUpdate"));				//Event name


	_tprintf(TEXT("Detecting if server is running.\n"));
	WaitForSingleObject(hCanBootNow, 500);

	//Opens a mapped file by the server
	cThread.smCtrl.hSMem = OpenFileMapping(
		FILE_MAP_ALL_ACCESS, 
		FALSE, 
		SMName);

	if (cThread.smCtrl.hSMem == NULL) {
		_tprintf(TEXT("[Error] Opening file mapping (%d)\nIs the server running?\n"), GetLastError());
		return -1;
	}

	//Creates a view for the server message
	cThread.smCtrl.pSMemServer= (SMServer_MSG *)MapViewOfFile(	//Casts view of shared memory to a known struct type
		cThread.smCtrl.hSMem,							//Handle to the whole mapped object
		FILE_MAP_READ,							//Security attributes
		0,
		0,
		cThread.smCtrl.SMemViewServer.QuadPart);		//Number of bytes to map

	if (cThread.smCtrl.pSMemServer == NULL) {
		_tprintf(TEXT("[Error] Mapping memory (%d)\nIs the server running?\n"), GetLastError());
		return -1;
	}

	//Creates a view of the desired part <Gateway>
	cThread.smCtrl.pSMemGateway= (SMGateway_MSG *)MapViewOfFile(	//Casts view of shared memory to a known struct type
		cThread.smCtrl.hSMem,								//Handle to the whole mapped object
		FILE_MAP_WRITE,								//Security attributes
		cThread.smCtrl.SMemViewServer.HighPart,			//OffsetHIgh (0 to map the whole thing)
		cThread.smCtrl.SMemViewServer.LowPart, 			//OffsetLow (0 to map the whole thing)
		cThread.smCtrl.SMemViewGateway.QuadPart);			//Number of bytes to map

	if (cThread.smCtrl.pSMemGateway == NULL) {				//Checks for errors
		_tprintf(TEXT("[Error] Mapping memory (%d)\n @ Gateway"), GetLastError());
		return -1;
	}

	cls(hStdout);
	hidecursor();
	while (cThread.ThreadMustGoOn) {
		WaitForSingleObject(cThread.smCtrl.hSMServerUpdate, INFINITE);
		cls(hStdout);
		gotoxy(cThread.smCtrl.pSMemServer->pSMem.x, cThread.smCtrl.pSMemServer->pSMem.y);
		_tprintf(TEXT("W"));
		//SetEvent(cThread.hSMGatewayUpdate);
	}

	UnmapViewOfFile(cThread.smCtrl.pSMemServer);
	CloseHandle(cThread.smCtrl.hSMem);

	return 0;
}