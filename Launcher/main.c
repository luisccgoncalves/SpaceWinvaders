#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#define SIZE 99999
int boot(TCHAR * pProc) {

	TCHAR	procname[20];
	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	_tcscpy_s(procname, _countof(procname), pProc);
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	return CreateProcess(NULL, procname, NULL, NULL, 0, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
}

int _tmain(int argc, LPTSTR argv[]) {

	_tprintf(TEXT("Launching processes...\n"));
	boot(TEXT("Server.exe"));
	boot(TEXT("Gateway.exe"));
	boot(TEXT("Client.exe"));

	return 0;
}