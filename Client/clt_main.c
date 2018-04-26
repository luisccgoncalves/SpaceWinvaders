#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>
#include <process.h>
#include "../Server/structs.h"

void populate_structs(ship * d_ship) {

	d_ship->owner.id = _getpid();
	_tprintf(TEXT("ID: %d\n"), d_ship->owner.id);
	_tprintf(TEXT("Username:"));
	_tscanf_s(TEXT("%s"), d_ship->owner.username, (unsigned)_countof(d_ship->owner.username));
	_gettchar();
}

//######################################################################################################################################################################
//###################################################### USED ONLY ON TEXT MODE DELETE AFTER MAY 12TH ##################################################################
//######################################################################################################################################################################

void gotoxy(int x, int y) {

	static HANDLE hStdout = NULL;
	COORD coord;
	coord.X = x;
	coord.Y = y;
	if (!hStdout)
		hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hStdout, coord);
}

#define PERR(bSuccess, api){if(!(bSuccess)) printf("%s:Error %d from %s on line %d\n", __FILE__, GetLastError(), api, __LINE__);}

void cls(HANDLE hConsole)
{
	COORD coordScreen = { 0, 0 };    
	BOOL bSuccess;
	DWORD cCharsWritten;
	CONSOLE_SCREEN_BUFFER_INFO csbi; /* to get buffer info */
	DWORD dwConSize;                 

	bSuccess = GetConsoleScreenBufferInfo(hConsole, &csbi);
	PERR(bSuccess, "GetConsoleScreenBufferInfo");
	dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

	bSuccess = FillConsoleOutputCharacter(hConsole, (TCHAR) ' ',
		dwConSize, coordScreen, &cCharsWritten);
	PERR(bSuccess, "FillConsoleOutputCharacter");

	bSuccess = GetConsoleScreenBufferInfo(hConsole, &csbi);
	PERR(bSuccess, "ConsoleScreenBufferInfo");

	bSuccess = FillConsoleOutputAttribute(hConsole, csbi.wAttributes,
		dwConSize, coordScreen, &cCharsWritten);
	PERR(bSuccess, "FillConsoleOutputAttribute");

	bSuccess = SetConsoleCursorPosition(hConsole, coordScreen);
	PERR(bSuccess, "SetConsoleCursorPosition");
	return;
}

void hidecursor() {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 100;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(consoleHandle, &info);
}

//######################################################################################################################################################################
//######################################################################################################################################################################

int _tmain(int argc, LPTSTR argv[]) {

	ship	d_ship;
	char	playing = 1, k_stroke;
	HANDLE	hStdout=GetStdHandle(STD_OUTPUT_HANDLE); //Handle to stdout to clear screen ##DELETE-ME after May 12th##

	d_ship.x = d_ship.y = 20;


	#ifdef UNICODE
		_setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
	#endif

	populate_structs(&d_ship);

	cls(hStdout);

	hidecursor();

	gotoxy(d_ship.x, d_ship.y);

	while (playing) {

		if (d_ship.x >= XSIZE - 1) { d_ship.x--; gotoxy(d_ship.x, d_ship.y); }
		if (d_ship.y >= YSIZE-1){d_ship.y--; gotoxy(d_ship.x, d_ship.y); }
		if (d_ship.x <0){d_ship.x=0; gotoxy(d_ship.x, d_ship.y); }
		if (d_ship.y <0){d_ship.y=0; gotoxy(d_ship.x, d_ship.y); }

		_tprintf(TEXT("Â"));
		gotoxy(d_ship.x, d_ship.y);
		k_stroke= _gettch();

		switch (k_stroke) {
		case 'w':
			_tprintf(TEXT(" "));
			d_ship.y--;
			gotoxy(d_ship.x, d_ship.y);
			break;
		case 's':
			_tprintf(TEXT(" "));
			d_ship.y++;
			gotoxy(d_ship.x, d_ship.y);
			break;
		case 'a':
			_tprintf(TEXT(" "));
			d_ship.x--;
			gotoxy(d_ship.x, d_ship.y);
			break;
		case 'd':
			_tprintf(TEXT(" "));
			d_ship.x++;
			gotoxy(d_ship.x, d_ship.y);
			break;
		default:
			playing = 0;
			break;
		}


	}

	return 0;
}