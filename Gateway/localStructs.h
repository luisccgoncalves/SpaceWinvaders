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
//#include "../Client/debug.h"

#pragma comment(lib, "advapi32.lib")

#define BUFSIZE 2048

typedef struct {

	HANDLE		hPipe;

	int			*ThreadMustGoOn;
	HANDLE		hSMServerUpdate;
	GameData	*pSMemGameData;
	HANDLE		mhGameData;

}PipeInstWrt;

typedef struct {

	HANDLE		hPipe;
	SMCtrl		*cThread;

}PipeInstRd;

#endif /* LOCALSTRUCTS_H */