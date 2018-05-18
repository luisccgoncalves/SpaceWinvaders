#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>
#include <process.h>
#include "../DLL/dll.h"
#include "debug.h"

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

DWORD WINAPI StartGame(LPVOID tParam) {

}

int _tmain(int argc, LPTSTR argv[]) {

	HANDLE	hPipe;				//Pipe handle
	LPTSTR	lpsPipeName = PIPE_NAME;

	HANDLE	htGame;				//Game thread
	DWORD	tGameID;			//Game thread ID

	DWORD	dwPipeMode;			//Stores pipe mode

	BOOL	running = 1;		//Main cycle exit condition
	BOOL	bSuccess;			

	_tprintf(TEXT("Connecting to gateway...\n"));

	while (running) {

		WaitNamedPipe(lpsPipeName, 10000);

		hPipe = CreateFile(
			lpsPipeName,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED,
			NULL
			);

		if (hPipe == INVALID_HANDLE_VALUE) {
			_tprintf(TEXT("ERROR opening pipe.\n"));
			break;
		}else if (hPipe == ERROR_PIPE_BUSY) {
			_tprintf(TEXT("Server busy. TBI:RETRY CONNECTION\n"));
			break;
		}
		else {
			_tprintf(TEXT("Pipe connected.\nChanging pipe mode...\n"));

			dwPipeMode = PIPE_READMODE_MESSAGE;
			bSuccess = SetNamedPipeHandleState(
				hPipe,
				&dwPipeMode,
				NULL,
				NULL
			);

			htGame = CreateThread(
				NULL,										//Thread security attributes
				0,											//Stack size (0 for default)
				StartGame,									//Thread function name
				NULL,										//Thread parameter struct
				0,											//Creation flags
				&tGameID);									//gets thread ID to close it afterwards
		}

	}
}