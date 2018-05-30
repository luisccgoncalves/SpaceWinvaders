#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>
#include <process.h>
#include "../DLL/dll.h"
#include "debug.h"

typedef struct {
	HANDLE	hPipe;
	int		ThreadMustGoOn;
}ThreadCtrl;

void printGame(GameData msg) {

	HANDLE		hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	int i;

	cls(hStdout);

	for (i = 0; i < MAX_INVADER; i++) {
		if (msg.invad[i].hp) {
			gotoxy(msg.invad[i].x, msg.invad[i].y);
			if (msg.invad[i].rand_path)
				_tprintf(TEXT("X"));
			else
				_tprintf(TEXT("W"));
		}
	}

	for (i = 0; i < MAX_BOMBS; i++) {
		if (msg.bomb[i].fired) {
			gotoxy(msg.bomb[i].x, msg.bomb[i].y);
			_tprintf(TEXT("o"));
		}
	}

	for (i = 0; i < MAX_SHOTS; i++) {
		if (msg.shot[i].fired) {
			gotoxy(msg.shot[i].x, msg.shot[i].y);
			_tprintf(TEXT("|"));
		}
	}

	for (i = 0; i < MAX_PLAYERS; i++) {
		gotoxy(msg.ship[i].x, msg.ship[i].y);
		_tprintf(TEXT("Â"));
	}


}

int readPipeMsg(HANDLE hPipe, HANDLE readReady) {

	OVERLAPPED	OvrRd = { 0 };
	DWORD		dwBytesRead = 0;
	BOOL		bSuccess = FALSE;

	GameData	msg;

	OvrRd.hEvent = readReady;
	ResetEvent(readReady);

	bSuccess = ReadFile(
		hPipe,
		&msg,
		sizeof(msg),
		&dwBytesRead,
		&OvrRd);

	WaitForSingleObject(readReady, INFINITE);

	GetOverlappedResult(
		hPipe,
		&OvrRd,
		&dwBytesRead,
		FALSE);

	if (dwBytesRead < sizeof(msg)) {
		if (GetLastError() == ERROR_BROKEN_PIPE) {
			_tprintf(TEXT("Connection lost.\n"));
			return -1;
		}
		else
			_tprintf(TEXT("\nReadFile failed. Error = %d"), GetLastError());
	}

	printGame(msg);

	return 0;
}

DWORD WINAPI ReadGame(LPVOID tParam) {

	ThreadCtrl	*cThreadRdGame = (ThreadCtrl*)tParam;
	HANDLE		heReadReady;

	if (cThreadRdGame->hPipe == NULL) {
		_tprintf(TEXT("ERROR casting pipe. (%d)\n"), GetLastError());
		return -1;
	}

	heReadReady = CreateEvent(
		NULL,										//Event attributes
		TRUE,										//Manual reset (TRUE for auto-reset)
		FALSE,										//Initial state
		NULL);										//Event name
	if (heReadReady == NULL) {
		_tprintf(TEXT("[Error] Event ReadReady(%d)\n"), GetLastError());
		return -1;
	}

	while (cThreadRdGame->ThreadMustGoOn) {

		readPipeMsg(cThreadRdGame->hPipe, heReadReady);

		/*
		Here we should present to screen
		later...
		*/

	}

	return 0;
}

DWORD WINAPI GetKey(LPVOID tParam) {

	ThreadCtrl	*cThread = (ThreadCtrl*)tParam;
	char k_stroke, exk_stroke;

	while (cThread->ThreadMustGoOn) {
		k_stroke = _gettch();
		_tprintf(TEXT("\7\n"));
		switch (k_stroke) {
		case 'w':
			break;
		case 's':
			break;
		case 'a':
			break;
		case 'd':
			break;
		case 27:
			cThread->ThreadMustGoOn = 0;
			break;
		case -32:				//is an extended keystroke
			switch (exk_stroke = _gettch()) {
			case 72:
				break;
			case 80:
				break;
			case 75:
				break;
			case 77:
				break;
			}
			break;
		default:
			break;
		}

	}

	return 0;
}
int StartPipeListener(HANDLE *hPipe) {

	HANDLE		h1stPipeInst;
	DWORD		dwPipeMode;					//Stores pipe mode

	BOOL		bSuccess;
	BOOL		bRunning;


	bRunning = TRUE;

	h1stPipeInst = OpenEvent(EVENT_ALL_ACCESS, FALSE, EVE_1ST_PIPE);
	if (!h1stPipeInst) {
		h1stPipeInst = CreateEvent(NULL, FALSE, FALSE, EVE_1ST_PIPE);
		_tprintf(TEXT("No pipe instances found. Waiting...\n"));
		WaitForSingleObject(h1stPipeInst, INFINITE);
	}

	do {

		*hPipe = CreateFile(
			PIPE_NAME,
			GENERIC_READ |
			GENERIC_WRITE,
			0 |
			FILE_SHARE_READ |
			FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			0 |
			FILE_FLAG_OVERLAPPED,
			NULL);

		if (GetLastError() == ERROR_PIPE_BUSY) {
			_tprintf(TEXT("Server full. Waiting for 30 seconds\n"));

			bSuccess = WaitNamedPipe(PIPE_NAME, 30000);

			if (bSuccess)
				continue;
			else
				return -1;

		}
		else if (*hPipe == INVALID_HANDLE_VALUE) {

			_tprintf(TEXT("ERROR opening pipe. (%d)\n"), GetLastError());
			return -1;
		}
		else
			bRunning=FALSE;

	} while (bRunning);

	_tprintf(TEXT("Pipe connected.\nChanging pipe mode...\n"));

	dwPipeMode = PIPE_READMODE_MESSAGE;
	bSuccess = SetNamedPipeHandleState(
		hPipe,
		&dwPipeMode,
		NULL,
		NULL);

	if (!bSuccess) {
		_tprintf(TEXT("ERROR setting pipe mode. (%d)\n"), GetLastError());
		return -1;
	}

	return 0;
}

int _tmain(int argc, LPTSTR argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	HANDLE		hPipe;						//Pipe handle

	HANDLE		htReadGame;					//Game thread
	DWORD		tReadGameID;				//Game thread ID

	HANDLE		htGetKey;
	DWORD		tGetKeyID;

	ThreadCtrl	cThreadRdGame;
	cThreadRdGame.ThreadMustGoOn = 1;

	if (!StartPipeListener(&hPipe)) {
		_tprintf(TEXT("Error launching pipe listener...\n"));
		return -1;
	}

	cThreadRdGame.hPipe = hPipe;

	htReadGame = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size (0 for default)
		ReadGame,									//Thread function name
		(LPVOID)&cThreadRdGame,						//Thread parameter struct
		0,											//Creation flags
		&tReadGameID);								//gets thread ID to close it afterwards

	if (htReadGame == NULL) {
		_tprintf(TEXT("ERROR launching game thread. (%d)\n"), GetLastError());
		return -1;
	}

	htGetKey = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size (0 for default)
		GetKey,										//Thread function name
		(LPVOID)&cThreadRdGame,						//Thread parameter struct
		0,											//Creation flags
		&tGetKeyID);								//gets thread ID to close it afterwards

	if (htReadGame == NULL) {
		_tprintf(TEXT("ERROR launching game thread. (%d)\n"), GetLastError());
		return -1;
	}

	WaitForSingleObject(htGetKey, INFINITE);
	WaitForSingleObject(htReadGame, INFINITE);
	CloseHandle(hPipe);

	return 0;
}