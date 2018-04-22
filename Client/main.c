#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>
#include <process.h>
#include <conio.h> //-------(?)---------
#include "../Server/structs.h"

void populate_structs(ship * d_ship) {

	d_ship->owner.id = _getpid();
	_tprintf(TEXT("ID: %d\n"), d_ship->owner.id);
	_tprintf(TEXT("Username:"));
	_tscanf_s(TEXT("%s"), d_ship->owner.username, (unsigned)_countof(d_ship->owner.username));
	_gettchar();
}

void gotoxy(int x, int y) {

	static HANDLE hStdout = NULL;
	COORD coord;
	coord.X = x;
	coord.Y = y;
	if (!hStdout)
		hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hStdout, coord);
}

void clrscr() {
	int x, y;

	for (x=0; x < XSIZE; x++) {
		for (y=0; y < YSIZE; y++) {
			gotoxy(x, y);
			_tprintf(TEXT(" "));
		}
	}
}

void hidecursor() {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 100;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(consoleHandle, &info);
}

int _tmain(int argc, LPTSTR argv[]) {

	ship	d_ship;
	char	playing = 1, k_stroke;
	
	d_ship.x = d_ship.y = 20;


	#ifdef UNICODE
		_setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
	#endif

	populate_structs(&d_ship);
	//_gettchar();
	clrscr();

	hidecursor();

	gotoxy(d_ship.x, d_ship.y);

	while (playing) {

		if (d_ship.x >= XSIZE - 1) { d_ship.x--; gotoxy(d_ship.x, d_ship.y); }
		if (d_ship.y >= YSIZE-1){d_ship.y--; gotoxy(d_ship.x, d_ship.y); }
		if (d_ship.x <0){d_ship.x=0; gotoxy(d_ship.x, d_ship.y); }
		if (d_ship.y <0){d_ship.y=0; gotoxy(d_ship.x, d_ship.y); }

		_tprintf(TEXT("Â"));
		gotoxy(d_ship.x, d_ship.y);
		k_stroke= _getch();

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