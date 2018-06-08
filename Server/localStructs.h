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
	GameData	*game;								//Pointer to localGamedata
	Packet		localPacket;						//stores a client packet to be translated into a game action
	int			*TheadmustGoOn;						//Pointer to cThread's exit condition
	HANDLE		*mhStructSync;						//Pointer to cThread's structSync mutex
} ClientMoves;

typedef struct {
	HANDLE		mhStructSync;
	Ship		*ship;
	PowerUp		pUp;
}PUpTimer;

#endif /* LOCALSTRUCTS_H */