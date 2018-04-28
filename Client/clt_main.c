#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>
#include <process.h>
#include "../Server/structs.h"
#include "debug.h"

void populate_structs(ship * d_ship) {

	d_ship->owner.id = _getpid();
	_tprintf(TEXT("ID: %d\n"), d_ship->owner.id);
	_tprintf(TEXT("Username:"));
	_tscanf_s(TEXT("%s"), d_ship->owner.username, (unsigned)_countof(d_ship->owner.username));
	_gettchar();
}

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