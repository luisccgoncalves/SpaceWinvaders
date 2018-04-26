#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "structs.h"
#include "../DLL/dll.h"

int _tmain(int argc, LPTSTR argv[]) {

	#ifdef UNICODE
		_setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
	#endif

	HANDLE			hSMem;		//Handle to shared memory
	char			*pSMem;		//Pointer to shared memory's first byte
	LARGE_INTEGER	SMemSize;	//Stores the size of the mapped file

	SMemSize.QuadPart = sizeof(char); //This should be in structs.h or dll

	//Maps a file in memory 
	hSMem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, SMemSize.HighPart, SMemSize.LowPart, SMName);
	if (hSMem == NULL) {
		_tprintf(TEXT("[Error] Opening file mapping (%d)\n"), GetLastError());
		return -1;
	}

	//Creates a view of the desired part
	pSMem = (char *)MapViewOfFile(hSMem, FILE_MAP_ALL_ACCESS, 0, 0, SMemSize.QuadPart);
	if (pSMem == NULL) {
		_tprintf(TEXT("[Error] Mapping memory (%d)\n"), GetLastError());
		return -1;
	}

	*pSMem = 'T';  //Puts the letter T in the shared memory

	_tprintf(TEXT("This was put in shared memory -> %c\n"), *pSMem);
	_gettchar();
	UnmapViewOfFile(pSMem);
	CloseHandle(hSMem);

	return 0;
}
