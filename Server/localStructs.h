#ifndef LOCALSTRUCTS_H
#define LOCALSTRUCTS_H

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>

#include "../DLL/dll.h"

typedef struct {
	HANDLE			*hTick;							//Handle to event. Warns gateway about updates in shared memory
	int				ThreadMustGoOn;
	HANDLE			*mhGameData;

	GameData		*localGameData;
	GameData		*smGameData;
}GTickStruct;

typedef struct {
	GameData	*game;
	Packet		localPacket;
	int			*TheadmustGoOn;
} ClientMoves;

#endif /* LOCALSTRUCTS_H */