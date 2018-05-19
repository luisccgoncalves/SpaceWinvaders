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
	HANDLE	*hPipe;
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

DWORD WINAPI ReadGame(LPVOID tParam) {

	int		*ThreadMustGoOn = &((ThreadCtrl*)tParam)->ThreadMustGoOn;
	HANDLE	hPipe = &((ThreadCtrl*)tParam)->hPipe;

	if (hPipe == NULL) {
		_tprintf(TEXT("ERROR casting pipe. (%d)\n"), GetLastError());
		return -1;
	}

	while (*ThreadMustGoOn) {

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

	_tprintf(TEXT("Connecting to gateway...\n"));

	h1stPipeInst = OpenEvent(EVENT_ALL_ACCESS, FALSE, EVE_1ST_PIPE);
	if (!h1stPipeInst) {
		h1stPipeInst = CreateEvent(NULL, FALSE, FALSE, EVE_1ST_PIPE);
		_tprintf(TEXT("No pipe instances found. Waiting...\n"));
		WaitForSingleObject(h1stPipeInst, INFINITE);
	}

	while (1) {
		
		hPipe = CreateFile(
			PIPE_NAME,
			GENERIC_READ | GENERIC_WRITE,
			0|FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED,
			NULL
			);

		if (hPipe != INVALID_HANDLE_VALUE)
			break;

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

	}

	_tprintf(TEXT("Pipe connected.\nChanging pipe mode...\n"));

	dwPipeMode = PIPE_READMODE_MESSAGE;
	bSuccess = SetNamedPipeHandleState(
		hPipe,
		&dwPipeMode,
		NULL,
		NULL
	);

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

	_tprintf(TEXT("All is OK, ENTER to quit.\n"));
	_gettch();

	cThreadRdGame.ThreadMustGoOn = 0;
	WaitForSingleObject(htReadGame, INFINITE);

	return 0;
}