#ifndef LOCALSTRUCTS_H
#define LOCALSTRUCTS_H

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

#define BUFSIZE 2048



typedef struct {

	int		ThreadMustGoOn;			//Exit condition

	HANDLE	hPipe;					//Handle to pipe instance

	HANDLE	heWriteReady;			//Handle for Overlaped I/O
	HANDLE	heReadReady;			//Handle for Overlaped I/O

	int		owner;					//Stores the client's ship position in the ship array

}ThreadCtrl;

void printGame(GameData msg);
int readPipeMsg(HANDLE hPipe, HANDLE readReady, GameData * msg);
int writePipeMsg(HANDLE hPipe, HANDLE writeReady, Packet msg);
int StartPipeListener(HANDLE *hPipe);
int createProdConsEvents(ThreadCtrl * ps);
int markPlayerReady(ThreadCtrl * ps, Packet token);
Packet handShakeServer(ThreadCtrl * ps);

DWORD WINAPI ReadGame(LPVOID tParam);
DWORD WINAPI GetKey(LPVOID tParam);

#endif /* LOCALSTRUCTS_H */
