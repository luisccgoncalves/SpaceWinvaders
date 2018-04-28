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
	HANDLE			hNewMessage;		//Shared Memory handle
	HANDLE			hSMem;				//Handle to shared memory
	LARGE_INTEGER	SMemSize;			//Stores the size of the mapped file
	invader			*pSMem;				//Pointer to shared memory's first byte
	int				ThreadMustGoOn;		//Flag for thread shutdown
} SMCtrl_Thread;
/**/


int _tmain(int argc, LPTSTR argv[]) {

	#ifdef UNICODE
		_setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
	#endif
	
	SMCtrl_Thread		cThread;
	HANDLE				hCanBootNow;

	//startThread;
	
	cThread.SMemSize.QuadPart = sizeof(invader); //This should be in structs.h or dll

	hCanBootNow = CreateEvent(NULL,FALSE,FALSE,TEXT("LetsBoot"));
	cThread.hNewMessage = CreateEvent(NULL, FALSE, FALSE, TEXT("NewMessage"));

	//Maps a file in memory 
	cThread.hSMem = CreateFileMapping(
		INVALID_HANDLE_VALUE, 
		NULL, 
		PAGE_READWRITE, 
		cThread.SMemSize.HighPart, 
		cThread.SMemSize.LowPart, 
		SMName);

	if (cThread.hSMem == NULL) {
		_tprintf(TEXT("[Error] Opening file mapping (%d)\n"), GetLastError());
		return -1;
	}

	//Creates a view of the desired part
	cThread.pSMem = (invader *)MapViewOfFile(cThread.hSMem, FILE_MAP_ALL_ACCESS, 0, 0, cThread.SMemSize.QuadPart);
	if (cThread.pSMem == NULL) {
		_tprintf(TEXT("[Error] Mapping memory (%d)\n"), GetLastError());
		return -1;
	}

	SetEvent(hCanBootNow);


	_tprintf(TEXT("Server deployed an invader. Ctrl+C to quit"));

	cThread.pSMem->x = 0;
	cThread.pSMem->y = 0;

	SetEvent(cThread.hNewMessage);

	while (1) {
		
		for (cThread.pSMem->y = 0; cThread.pSMem->y <= (YSIZE - 2); cThread.pSMem->y++) {
			for (cThread.pSMem->x = 0; cThread.pSMem->x < 4; cThread.pSMem->x++) {
				Sleep(500);
				SetEvent(cThread.hNewMessage);
			}
			cThread.pSMem->y++;
			for (cThread.pSMem->x = 3; cThread.pSMem->x > -1; cThread.pSMem->x--) {
				Sleep(500);
				SetEvent(cThread.hNewMessage);
			}
		}
	}


	_gettchar();
	UnmapViewOfFile(cThread.pSMem);
	CloseHandle(cThread.hSMem);

	return 0;
}
