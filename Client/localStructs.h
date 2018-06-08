#ifndef LOCALSTRUCTS_H
#define LOCALSTRUCTS_H

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
//#include "../DLL/dll.h"

typedef struct {
	HANDLE	hPipe;
	int		ThreadMustGoOn;
}ThreadCtrl;

#endif /* LOCALSTRUCTS_H */
