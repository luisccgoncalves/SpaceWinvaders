#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "structs.h"
#include "../DLL/dll.h"

/**/

// Thread to read from memmory
typedef struct {
	HANDLE			hNewMessage;		//Shared Memory handle
	HANDLE			hSMem;				//Handle to shared memory
	LARGE_INTEGER	SMemSize;			//Stores the size of the mapped file
	char			*pSMem;				//Pointer to shared memory's first byte
	int				ThreadMustGoOn;		//Flag for thread shutdown
} SMCtrl_Thread;

int _tmain(int argc, LPTSTR argv[]) {

	#ifdef UNICODE
		_setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
	#endif
	
	SMCtrl_Thread		cThread;
	HANDLE				hCanBootNow;

	//startThread;
	
	cThread.SMemSize.QuadPart = sizeof(char); //This should be in structs.h or dll

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
	cThread.pSMem = (char *)MapViewOfFile(cThread.hSMem, FILE_MAP_ALL_ACCESS, 0, 0, cThread.SMemSize.QuadPart);
	if (cThread.pSMem == NULL) {
		_tprintf(TEXT("[Error] Mapping memory (%d)\n"), GetLastError());
		return -1;
	}

	SetEvent(hCanBootNow);

	*cThread.pSMem = 'T';  //Puts the letter T in the shared memory
	_tprintf(TEXT("This was put in shared memory -> %c\n"), *cThread.pSMem);
	
	SetEvent(cThread.hNewMessage);

	_gettchar();
	UnmapViewOfFile(cThread.pSMem);
	CloseHandle(cThread.hSMem);

	return 0;
}
