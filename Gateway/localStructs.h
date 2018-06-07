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
	HANDLE		*pSMemGameData;
	HANDLE		*mhGameData;


}PipeInstWrt;

typedef struct {

	HANDLE		hPipe;
	SMCtrl		*cThread;

}PipeInstRd;

#endif /* LOCALSTRUCTS_H */