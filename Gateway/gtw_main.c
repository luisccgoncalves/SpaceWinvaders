#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "../Server/structs.h"
#include "../DLL/dll.h"

int _tmain(int argc, LPTSTR argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	HANDLE			hSMem;		//Handle to shared memory
	char			*pSMem;		//Pointer to shared memory's first byte
	LARGE_INTEGER	SMemSize;	//Stores the size of the mapped file
	HANDLE			hCanBootNow;
	HANDLE			hNewMessage;

	SMemSize.QuadPart = sizeof(char); //This should be in structs.h or dll

	//hCanBootNow = OpenEvent(EVENT_ALL_ACCESS, FALSE, (LPTSTR)TEXT("LetsBoot"));
	hCanBootNow = CreateEvent(NULL, FALSE, FALSE, TEXT("LetsBoot"));
	hNewMessage = CreateEvent(NULL, FALSE, FALSE, TEXT("NewMessage"));


	_tprintf(TEXT("Detecting if server is running.\n"));
	WaitForSingleObject(hCanBootNow, 5000);

	//Opens a mapped file by the server
	hSMem = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SMName);
	if (hSMem == NULL) {
		_tprintf(TEXT("[Error] Opening file mapping (%d)\nIs the server running?\n"), GetLastError());
		return -1;
	}

	//Creates a view of the desired part
	pSMem = (char *)MapViewOfFile(hSMem, FILE_MAP_ALL_ACCESS, 0, 0, SMemSize.QuadPart);
	if (pSMem == NULL) {
		_tprintf(TEXT("[Error] Mapping memory (%d)\nIs the server running?\n"), GetLastError());
		return -1;
	}

	//Reads the value from memory
	while (1) {
		//clean the screen
		WaitForSingleObject(hNewMessage, INFINITE);
		_tprintf(TEXT("This was read from memory -> %c\n"), *pSMem);
	}
	_gettchar();
	UnmapViewOfFile(pSMem);
	CloseHandle(hSMem);

	return 0;
}