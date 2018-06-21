#ifndef DATA_STRUCTS_H
#define DATA_STRUCTS_H

#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <sddl.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#include "../DLL/dll.h"
#include "resource.h"


#pragma comment(lib, "advapi32.lib")

#define		BUFSIZE		2048
#define		BMP_NUM		15



typedef struct {

	int		ThreadMustGoOn;			//Exit condition

	HANDLE	hPipe;					//Handle to pipe instance

	HANDLE	heWriteReady;			//Handle for Overlaped I/O
	HANDLE	heReadReady;			//Handle for Overlaped I/O

	int		owner;					//Stores the client's ship position in the ship array
	int		logged;

	TCHAR	username[SMALL_BUFF];
	TCHAR	userlogin[SMALL_BUFF];
	TCHAR	password[SMALL_BUFF];
	TCHAR	domain[SMALL_BUFF];
	int		remoteLogin;

	Packet	token;
	int		packetUpd;

	GameData localGame;
}ThreadCtrl;

typedef struct {
	TCHAR	up, down, right, left, fire;
	BOOL	defaultKeys;
} GameKeys;

HANDLE		rectEvent;				//Handle to event to update main window
#endif /* DATA_STRUCTS_H */
