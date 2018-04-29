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
typedef struct {
	HANDLE			hSMServerUpdate;		//Handle to event. Warns gateway about updates in shared memory
	HANDLE			hSMGatewayUpdate;		//Handle to event. Warns server about updates in shared memory
	HANDLE			mhInvader;				//Handle to mutex (TEST)
	HANDLE			hSMem;					//Handle to shared memory
	LARGE_INTEGER	SMemSize;				//Stores the size of the mapped file
	LARGE_INTEGER	SMemViewServer;				//Stores the size of the mapped file
	LARGE_INTEGER	SMemViewGateway;				//Stores the size of the mapped file
	invader			*pSMem;					//Pointer to shared memory's first byte
	char			*pSMGateway;			//Pointer to shared memory's first byte
	int				ThreadMustGoOn;			//Flag for thread shutdown
} SMCtrl_Thread;
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

int _tmain(int argc, LPTSTR argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	SMCtrl_Thread cThread;
	HANDLE			hCanBootNow;
	HANDLE			hStdout = GetStdHandle(STD_OUTPUT_HANDLE); //Handle to stdout to clear screen ##DELETE-ME after May 12th##

	cThread.SMemSize.QuadPart = sizeof(invader)+sizeof(char); //This should be in structs.h or dll
	cThread.SMemViewServer.QuadPart = sizeof(invader);
	cThread.SMemViewGateway.QuadPart = sizeof(char);

	cThread.ThreadMustGoOn = 1;

	//hCanBootNow = OpenEvent(EVENT_ALL_ACCESS, FALSE, (LPTSTR)TEXT("LetsBoot"));
	hCanBootNow = CreateEvent(NULL, FALSE, FALSE, TEXT("LetsBoot"));

	cThread.hSMServerUpdate = CreateEvent(		//Creates the event to warn gateway that the shared memoy is mapped
		NULL, 									//Event attributes
		FALSE, 									//Manual reset (TRUE for auto-reset)
		FALSE, 									//Initial state
		TEXT("SMServerUpdate"));				//Event name

	cThread.hSMGatewayUpdate = CreateEvent(		//Creates the event to warn gateway that the shared memoy is mapped
		NULL, 									//Event attributes
		FALSE, 									//Manual reset (TRUE for auto-reset)
		FALSE, 									//Initial state
		TEXT("SMGatewayUpdate"));				//Event name


	_tprintf(TEXT("Detecting if server is running.\n"));
	WaitForSingleObject(hCanBootNow, 5000);

	//Opens a mapped file by the server
	cThread.hSMem = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SMName);
	if (cThread.hSMem == NULL) {
		_tprintf(TEXT("[Error] Opening file mapping (%d)\nIs the server running?\n"), GetLastError());
		return -1;
	}

	//Creates a view of the desired part
	cThread.pSMem = (invader *)MapViewOfFile(	//Casts view of shared memory to a known struct type
		cThread.hSMem,							//Handle to the whole mapped object
		FILE_MAP_READ,							//Security attributes
		0,
		0,
		//cThread.SMemViewServer.HighPart,		//OffsetHIgh (0 to map the whole thing)
		//cThread.SMemViewServer.LowPart, 		//OffsetLow (0 to map the whole thing)
		cThread.SMemViewServer.QuadPart);		//Number of bytes to map

	if (cThread.pSMem == NULL) {
		_tprintf(TEXT("[Error] Mapping memory (%d)\nIs the server running?\n"), GetLastError());
		return -1;
	}

	//Creates a view of the desired part @ Gateway
	cThread.pSMGateway = (char *)MapViewOfFile(	//Casts view of shared memory to a known struct type
		cThread.hSMem,								//Handle to the whole mapped object
		FILE_MAP_WRITE,						//Security attributes
		0,
		0,
		//cThread.SMemViewGateway.HighPart,			//OffsetHIgh (0 to map the whole thing)
		//cThread.SMemViewGateway.LowPart, 			//OffsetLow (0 to map the whole thing)
		cThread.SMemViewGateway.QuadPart);			//Number of bytes to map

	if (cThread.pSMGateway == NULL) {				//Checks for errors
		_tprintf(TEXT("[Error] Mapping memory (%d)\n @ Gateway"), GetLastError());
		return -1;
	}

	cls(hStdout);
	hidecursor();
	while (cThread.ThreadMustGoOn) {
		WaitForSingleObject(cThread.hSMServerUpdate, INFINITE);
		cls(hStdout);
		gotoxy(cThread.pSMem->x, cThread.pSMem->y);
		_tprintf(TEXT("W"));
		SetEvent(cThread.hSMGatewayUpdate);
	}

	UnmapViewOfFile(cThread.pSMem);
	CloseHandle(cThread.hSMem);

	return 0;
}