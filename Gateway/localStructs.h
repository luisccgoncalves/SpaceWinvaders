#ifndef LOCALSTRUCTS_H
#define LOCALSTRUCTS_H

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "../DLL/dll.h"
//#include "../Client/debug.h"

#define BUFSIZE 2048

typedef struct {

	HANDLE		hPipe;

	int			*ThreadMustGoOn;
	HANDLE		*hSMServerUpdate;
	GameData	*pSMemGameData;
	HANDLE		*mhGameData;
	//GameData	*localGameData;

}PipeInstWrt;

typedef struct {

	HANDLE		hPipe;
	int			*ThreadMustGoOn;

	HANDLE		*heGotPacket;
	Packet		*localPacket;

}PipeInstRd;

#endif /* LOCALSTRUCTS_H */