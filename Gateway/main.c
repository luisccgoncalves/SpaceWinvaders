#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

int _tmain(int argc, LPTSTR argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	_tprintf(TEXT("I'm a gateway!\n"));
	_gettchar();

	return 0;
}