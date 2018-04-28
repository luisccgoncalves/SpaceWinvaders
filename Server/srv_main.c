#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "structs.h"
#include "../DLL/dll.h"

/**/ //TO DLL
// Thread to read from memmory
typedef struct {
	HANDLE			hNewMessage;		//Handle to event. Warns gateway about updates in shared memory
	HANDLE			hSMem;				//Handle to shared memory
	LARGE_INTEGER	SMemSize;			//Stores the size of the mapped file
	invader			*pSMem;				//Pointer to shared memory's first byte
	int				ThreadMustGoOn;		//Flag for thread shutdown
} SMCtrl_Thread;
/**/

DWORD WINAPI InvaderDeploy(LPVOID tParam) {

	SMCtrl_Thread	*cThread;			//Pointer to thread parameter structure
	cThread = (SMCtrl_Thread *)tParam;	//Points it to the right direction

	_tprintf(TEXT("Server deployed an invader."));

	//Populates one invader with initial coords
	cThread->pSMem->x = cThread->pSMem->x_init = 0;
	cThread->pSMem->y = cThread->pSMem->x_init = 0;

	//This should be a mutex, there should be another thread to warn the gateway of struct updates
	SetEvent(cThread->hNewMessage);		

	while (cThread->ThreadMustGoOn) {	//Thread main loop

		for (cThread->pSMem->y = 0; cThread->pSMem->y <= (YSIZE - 2); cThread->pSMem->y++) {

			//Invader goes 4 spaces to the right
			for (cThread->pSMem->x = 0; cThread->pSMem->x < 4; cThread->pSMem->x++) {
				Sleep(500);				//This should be a variable. Lower number==higher dificulty
				if (cThread->ThreadMustGoOn) SetEvent(cThread->hNewMessage);	//Thread exit condition
				else return 0;
			}
			
			//Invader goes down 1 space
			cThread->pSMem->y++;

			//Invader goes 4 spaces to the left
			for (cThread->pSMem->x = 3; cThread->pSMem->x > -1; cThread->pSMem->x--) {
				Sleep(500);
				if (cThread->ThreadMustGoOn) SetEvent(cThread->hNewMessage);	//Thread exit condition
				else return 0;
			}
		}
	}
}

int _tmain(int argc, LPTSTR argv[]) {

	#ifdef UNICODE				//Sets console to unicode
		_setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
	#endif
	
	SMCtrl_Thread	cThread;		//Thread parameter structure
	HANDLE			hCanBootNow;	//Handle to event. Warns the gateway the shared memory is mapped
	DWORD			tInvaderID;		//stores the ID of the Invader thread
	HANDLE			htInvader;		//Handle to the Invader thread
	
	cThread.SMemSize.QuadPart = sizeof(invader);	//This should be in structs.h or dll
	cThread.ThreadMustGoOn = 1;						//Preps thread to run position

	hCanBootNow = CreateEvent(			//Creates the event to warn gateway that the shared memoy is mapped
		NULL,							//Event attributes
		FALSE,							//Manual reset (TRUE for auto-reset)
		FALSE,							//Initial state
		TEXT("LetsBoot"));				//Event name

	cThread.hNewMessage = CreateEvent(	//Creates the event to warn gateway that the shared memoy is mapped
		NULL, 							//Event attributes
		FALSE, 							//Manual reset (TRUE for auto-reset)
		FALSE, 							//Initial state
		TEXT("NewMessage"));			//Event name

	cThread.hSMem = CreateFileMapping(	//Maps a file in memory 
		INVALID_HANDLE_VALUE,			//Handle to file being mapped (INVALID_HANDLE_VALUE to swap)
		NULL,							//Security attributes
		PAGE_READWRITE,					//Maped file permissions
		cThread.SMemSize.HighPart,		//MaximumSizeHigh
		cThread.SMemSize.LowPart,		//MaximumSizeLow
		SMName);						//File mapping name

	if (cThread.hSMem == NULL) {				//Checks for errors
		_tprintf(TEXT("[Error] Opening file mapping (%d)\n"), GetLastError());
		return -1;
	}

	//Creates a view of the desired part
	cThread.pSMem = (invader *)MapViewOfFile(	//Casts view of shared memory to a known struct type
		cThread.hSMem,							//Handle to the whole mapped object
		FILE_MAP_ALL_ACCESS,					//Security attributes
		0,										//OffsetHIgh (0 to map the whole thing)
		0, 										//OffsetLow (0 to map the whole thing)
		cThread.SMemSize.QuadPart);				//Number of bytes to map

	if (cThread.pSMem == NULL) {				//Checks for errors
		_tprintf(TEXT("[Error] Mapping memory (%d)\n"), GetLastError());
		return -1;
	}

	SetEvent(hCanBootNow);						//Warns gateway that Shared memory is mapped

	//Launches thread
	_tprintf(TEXT("Launching thread... ENTER to quit\n"));

	htInvader = CreateThread(
		NULL,					//Thread security attributes
		0,						//Stack size
		InvaderDeploy,			//Thread function name
		(LPVOID)&cThread,		//Thread parameter struct
		0,						//Creation flags
		&tInvaderID);			//gets thread ID to close it afterwards

	//Enter to end thread and exit
	_gettchar();
	cThread.ThreadMustGoOn = 0;					//Signals thread to gracefully exit
	WaitForSingleObject(htInvader, INFINITE);	//Waits for thread to exit
	
	UnmapViewOfFile(cThread.pSMem);				//Unmaps view of shared memory
	CloseHandle(cThread.hSMem);					//Closes shared memory

	return 0;
}
