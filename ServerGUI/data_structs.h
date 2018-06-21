#ifndef DATA_STRUCTS_H
#define DATA_STRUCTS_H

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>

#include "../DLL/dll.h"

#define INV_KILL	10
#define RINV_KILL	15
#define BOMB_KILL	20
#define PUP_KILL	30

typedef struct {
	HANDLE			*hTick;							//Handle to event. Warns gateway about updates in shared memory
	int				ThreadMustGoOn;
	HANDLE			mhGameData;

	GameData		*localGameData;
	GameData		*smGameData;
}GTickStruct;

typedef struct {
	GameData	*game;								//Pointer to localGamedata
	Packet		localPacket;						//stores a client packet to be translated into a game action
	int			*TheadmustGoOn;						//Pointer to cThread's exit condition
	HANDLE		mhStructSync;						//Pointer to cThread's structSync mutex
} ClientMoves;

typedef struct {
	HANDLE		mhStructSync;
	Ship		*ship;
	GameData	*game;
	PowerUp		pUp;
}PUpTimer;

typedef struct {
	GameData	*game;								//Pointer to localGamedata
	Invader		*invader;

	int			*TheadmustGoOn;						//Pointer to cThread's exit condition
	HANDLE		mhStructSync;						//Pointer to cThread's structSync mutex
}BombMoves;

#endif /* DATA_STRUCTS_H */
