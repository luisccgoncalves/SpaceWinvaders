#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "../DLL/dll.h"

int boot_gateway() {

	TCHAR	gateway[20];
	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	_tcscpy_s(gateway, _countof(gateway), TEXT("Gateway.exe"));
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	return CreateProcess(NULL, gateway, NULL, NULL, 0, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
}

int _tmain(int argc, LPTSTR argv[]) {

	#ifdef UNICODE
		_setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
	#endif

	_tprintf(TEXT("Paging file...\t\t\t[TBI]\n"));

	_tprintf(TEXT("Launching gateway...\t\t"));
	if(boot_gateway())
		_tprintf(TEXT("[OK]\n"));
	else {
		_tprintf(TEXT("[ERROR]\n"));
		return 1;
	}

	_tprintf(TEXT("Waiting for connection...\t[TBI]\n"));
	_tprintf(TEXT("Reading value from dll: %d\n"),nDLL);



	return 0;
}
