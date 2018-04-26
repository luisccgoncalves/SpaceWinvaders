#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "../DLL/dll.h"

int _tmain(int argc, LPTSTR argv[]) {

	#ifdef UNICODE
		_setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
	#endif

	_tprintf(TEXT("Paging file...\t\t\t[TBI]\n"));

	_tprintf(TEXT("Waiting for connection...\t[TBI]\n"));
	_tprintf(TEXT("Reading value from dll: %d\n"),nDLL);

	_gettchar();

	return 0;
}
