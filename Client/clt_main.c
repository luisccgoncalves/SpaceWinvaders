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

//void populate_structs(ship * d_ship) {
//
//	d_ship->owner.id = _getpid();
//	_tprintf(TEXT("ID: %d\n"), d_ship->owner.id);
//	_tprintf(TEXT("Username:"));
//	//_tscanf_s(TEXT("%s"), d_ship->owner.username, (unsigned)_countof(d_ship->owner.username));
//	_gettchar();
//}
//
//int _tmain(int argc, LPTSTR argv[]) {
//
//	ship	d_ship;
//	char	playing = 1, k_stroke, exk_stroke;
//	HANDLE	hStdout=GetStdHandle(STD_OUTPUT_HANDLE); //Handle to stdout to clear screen ##DELETE-ME after May 12th##
//
//	d_ship.x = d_ship.y = 20;
//
//
//	#ifdef UNICODE
//		_setmode(_fileno(stdin), _O_WTEXT);
//		_setmode(_fileno(stdout), _O_WTEXT);
//	#endif
//
//	populate_structs(&d_ship);
//
//	cls(hStdout);
//
//	hidecursor();
//
//	gotoxy(d_ship.x, d_ship.y);
//
//	while (playing) {
//
//		if (d_ship.x >= XSIZE - 1) { d_ship.x--; gotoxy(d_ship.x, d_ship.y); }
//		if (d_ship.y >= YSIZE-1){d_ship.y--; gotoxy(d_ship.x, d_ship.y); }
//		if (d_ship.x <0){d_ship.x=0; gotoxy(d_ship.x, d_ship.y); }
//		if (d_ship.y <0){d_ship.y=0; gotoxy(d_ship.x, d_ship.y); }
//
//		_tprintf(TEXT("Â"));
//		gotoxy(d_ship.x, d_ship.y);
//
//		k_stroke = _gettch();
//		_tprintf(TEXT(" "));
//
//		switch (k_stroke) {
//		case 'w':
//			d_ship.y--;
//			gotoxy(d_ship.x, d_ship.y);
//			break;
//		case 's':
//			d_ship.y++;
//			gotoxy(d_ship.x, d_ship.y);
//			break;
//		case 'a':
//			d_ship.x--;
//			gotoxy(d_ship.x, d_ship.y);
//			break;
//		case 'd':
//			d_ship.x++;
//			gotoxy(d_ship.x, d_ship.y);
//			break;
//		case 27:
//			playing = 0;
//			break;
//		case -32:				//is an extended keystroke
//			switch (exk_stroke=_gettch()) {
//			case 72:
//				d_ship.y--;
//				gotoxy(d_ship.x, d_ship.y);
//				break;
//			case 80:
//				d_ship.y++;
//				gotoxy(d_ship.x, d_ship.y);
//				break;
//			case 75:
//				d_ship.x--;
//				gotoxy(d_ship.x, d_ship.y);
//				break;
//			case 77:
//				d_ship.x++;
//				gotoxy(d_ship.x, d_ship.y);
//				break;
//			}
//			break;
//		default:
//			break;
//		}
//
//
//	}
//
//	return 0;
//}

void printGame(GameData msg) {

	HANDLE		hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	int i;
	
	cls(hStdout);
	//for (i = 0; i < MAX_INVADER; i++) {
	//	_tprintf(TEXT("(%d,%d) "), msg.invad[i].x, msg.invad[i].y);
	//}
	
	for (i = 0; i < MAX_INVADER; i++) {
		if (msg.invad[i].hp) {
			gotoxy(msg.invad[i].x, msg.invad[i].y);
			if (msg.invad[i].rand_path)
				_tprintf(TEXT("X"));
			else
				_tprintf(TEXT("W"));
		}
		Sleep(100);
	}

	if (msg.bomb[0].y < 25) { //this needs another aproach (fired state?)
		gotoxy(msg.bomb[0].x, msg.bomb[0].y);
		_tprintf(TEXT("o"));
		Sleep(2000);
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

	
	_tprintf(TEXT("\nGot a message!\n = %d"), msg.invad[0].x);
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

	_tprintf(TEXT("Listening...\n"));

	while (cThreadRdGame->ThreadMustGoOn) {

		readPipeMsg(cThreadRdGame->hPipe, heReadReady);

		/*
		Here we should present to screen
		later...
		*/

	}

	return 0;
}

int _tmain(int argc, LPTSTR argv[]) {

	HANDLE		h1stPipeInst;
	HANDLE		hPipe;						//Pipe handle

	HANDLE		htReadGame;					//Game thread
	DWORD		tReadGameID;				//Game thread ID

	DWORD		dwPipeMode;					//Stores pipe mode

	BOOL		bSuccess;	

	ThreadCtrl	cThreadRdGame;
	cThreadRdGame.ThreadMustGoOn = 1;

	_tprintf(TEXT("Connecting to gateway...\n"));

	h1stPipeInst = OpenEvent(EVENT_ALL_ACCESS, FALSE, EVE_1ST_PIPE);
	if (!h1stPipeInst) {
		h1stPipeInst = CreateEvent(NULL, FALSE, FALSE, EVE_1ST_PIPE);
		_tprintf(TEXT("No pipe instances found. Waiting...\n"));
		WaitForSingleObject(h1stPipeInst, INFINITE);
	}

	do{
		
		hPipe = CreateFile(
			PIPE_NAME,
			GENERIC_READ | 
			GENERIC_WRITE,
			0|
			FILE_SHARE_READ|
			FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			0|
			FILE_FLAG_OVERLAPPED,
			NULL);

		if (GetLastError() == ERROR_PIPE_BUSY) {
				_tprintf(TEXT("Server full. Waiting for 30 seconds\n"));

				bSuccess=WaitNamedPipe(PIPE_NAME, 30000);

				if (bSuccess)
					continue;
				else
					return -1;

		}else if (hPipe == INVALID_HANDLE_VALUE) {

			_tprintf(TEXT("ERROR opening pipe. (%d)\n"), GetLastError());
			return -1;
		}
		else
			cThreadRdGame.ThreadMustGoOn = 0;

	} while (cThreadRdGame.ThreadMustGoOn);

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

	cThreadRdGame.hPipe = hPipe;
	cThreadRdGame.ThreadMustGoOn = 1;

	htReadGame = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size (0 for default)
		ReadGame,									//Thread function name
		(LPVOID)&cThreadRdGame,						//Thread parameter struct
		0,											//Creation flags
		&tReadGameID);								//gets thread ID to close it afterwards

	if (htReadGame==NULL) {
		_tprintf(TEXT("ERROR launching game thread. (%d)\n"), GetLastError());
		return -1;
	}

	_gettch();
	cThreadRdGame.ThreadMustGoOn = 0;
	WaitForSingleObject(htReadGame, INFINITE);
	CloseHandle(hPipe);

	return 0;
}