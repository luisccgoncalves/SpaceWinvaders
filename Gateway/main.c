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

	HANDLE hSMem;
	char *pSMem;
	LARGE_INTEGER SMemSize;
	SMemSize.QuadPart = sizeof(char);

	hSMem = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SMName);

	pSMem = (char *)MapViewOfFile(hSMem, FILE_MAP_ALL_ACCESS, 0, 0, SMemSize.QuadPart);

	_tprintf(TEXT("This was read from memory -> %c\n"), *pSMem);

	_gettchar();

	return 0;
}