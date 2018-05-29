#include "algorithms.h"



DWORD WINAPI InvadersBomb(LPVOID tParam) {

	int * ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	GameData *baseGame = &((SMCtrl *)tParam)->localGameData;

	//int bombs;
	
	DWORD			tBombLauncherID;
	HANDLE			htBombLauncher[MAX_BOMBS];


	for (int i = 0; i < MAX_BOMBS; i++) {


		htBombLauncher[i] = CreateThread(
			NULL,										//Thread security attributes
			0,											//Stack size
			BombMovement,								//Thread function name
			tParam,										//Thread parameter struct
			0,											//Creation flags
			&tBombLauncherID);							//gets thread ID to close it afterwards
		if (htBombLauncher[i] == NULL) {
			_tprintf(TEXT("[Error] Creating thread htBombLauncher[%d] (%d) at server\n"),i, GetLastError());
			return -1;
		}
		/*
		Here there is a massive problem. 
		Bomb movement is always getting the same invader bomb number, 
		"Answer to the Ultimate Question of Life, the Universe, and Everything" - 42
		either in game, either every time i run the damned thing. 
		So i am out of ideas. For now!
		*/

		Sleep(5000);
	}
	WaitForMultipleObjects(MAX_BOMBS, htBombLauncher,TRUE, INFINITE);
}

DWORD WINAPI BombMovement(LPVOID tParam) {

	int * ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	GameData *baseGame = &((SMCtrl *)tParam)->localGameData;

	int invPosition = -1;
	int bombNum = -1;

	for (int i = 0; i < baseGame->max_bombs; i++) {						//cicle to check if there is available slots to fire a bomb
		if (!baseGame->bomb[i].fired) {
			bombNum = i;
			break;
		}
	}

	do {																//find a random invader to send the bomb from
		invPosition = rand() % baseGame->max_invaders + 1;
		//invPosition = RandomValue(baseGame->max_invaders);
	} while (baseGame->invad[invPosition].hp == 0);

	if (bombNum > -1) {

		baseGame->bomb[bombNum].x = baseGame->invad[invPosition].x;		//give invaders coords to bomb
		baseGame->bomb[bombNum].y = baseGame->invad[invPosition].y;
		baseGame->bomb[bombNum].fired = 1;								//update bomb status

		while (*ThreadMustGoOn && baseGame->bomb[bombNum].fired/*&&bombColDetect(&bomb,tParam)*/) {
			if (baseGame->bomb[bombNum].y < baseGame->ysize) {			//if bomb has not reached the end of the play area
				baseGame->bomb[bombNum].y++;							//update it's position, an wait for next tick 

				Sleep(((baseGame->invaders_bombs_speed) / 3) * (*ThreadMustGoOn));
			}
			else {														//reset bomb to out of screen
				baseGame->bomb[bombNum].x = baseGame->xsize + 1;
				baseGame->bomb[bombNum].y = baseGame->ysize + 1;
				baseGame->bomb[bombNum].fired = 0;						//resets fired state
			}
		}
	}

	return 0;
}

DWORD WINAPI RegPathInvaders(LPVOID tParam) {

	int * ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	GameData *baseGame = &((SMCtrl *)tParam)->localGameData;
	//GameData *baseGame = ((SMCtrl *)tParam)->pSMemGameData;
	HANDLE		*mhStructSync = ((SMCtrl *)tParam)->mhStructSync;

	int i, j;
	int sidestep = 4;
	int totalsteps = (baseGame->ysize - (baseGame->max_invaders/ INVADER_BY_ROW)) * sidestep;
	int regInvaderNr = (baseGame->max_invaders - baseGame->max_rand_invaders);


	while (*ThreadMustGoOn) {						//Thread main loop

		for (i = 0; (i < totalsteps) && *ThreadMustGoOn; i++) {

			WaitForSingleObject(mhStructSync, INFINITE);

			for (j = 0; (j < regInvaderNr) && *ThreadMustGoOn; j++) {

				baseGame->invad[j].y = (i / sidestep) + baseGame->invad[j].y_init;				//Invader goes down after n sidesteps

				if ((i % (sidestep * 2)) < sidestep)
					baseGame->invad[j].x = (i % (sidestep * 2)) + baseGame->invad[j].x_init;		//Invader goes right
				else if ((i % (sidestep * 2)) > sidestep)
					baseGame->invad[j].x--;													//Invader goes left
			}

			ReleaseMutex(mhStructSync);

			Sleep(baseGame->invaders_speed*(*ThreadMustGoOn));
		}
	}

	return 0;
}

DWORD WINAPI RandPathInvaders(LPVOID tParam) {

	int * ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	GameData *baseGame = &((SMCtrl *)tParam)->localGameData;
	HANDLE		*mhStructSync = ((SMCtrl *)tParam)->mhStructSync;

	int i;
	int startIndex = baseGame->max_invaders - baseGame->max_rand_invaders;
	int maxXpos = baseGame->xsize - 1;
	int maxYpos = (int)((baseGame->ysize - 1)*0.8);

	while (*ThreadMustGoOn) {						//Thread main loop

		WaitForSingleObject(mhStructSync, INFINITE);

		for (i = startIndex; (i < baseGame->max_invaders) && *ThreadMustGoOn; i++) {

			switch (rand() % 4) {
			case 0:
				if (baseGame->invad[i].x > 0)
					baseGame->invad[i].x--;
				else
					baseGame->invad[i].x = 1;
				break;
			case 1:
				if (baseGame->invad[i].x < maxXpos)
					baseGame->invad[i].x++;
				else
					baseGame->invad[i].x = maxXpos - 1;
				break;
			case 2:
				if (baseGame->invad[i].y > 0)
					baseGame->invad[i].y--;
				else
					baseGame->invad[i].y = 1;
				break;
			case 3:
				if (baseGame->invad[i].y < maxYpos)
					baseGame->invad[i].y++;
				else
					baseGame->invad[i].y = maxYpos - 1;
				break;
			}
		}
		ReleaseMutex(mhStructSync);
		Sleep((baseGame->invaders_speed / 4)*(*ThreadMustGoOn));
	}

	return 0;
}

DWORD WINAPI ShipInstruction(LPVOID tParam) {
	SMCtrl		*cThread = (SMCtrl*)tParam;

	Packet		localpacket;
	Ship		localship;

	int	nextOut = 0;


	while (cThread->ThreadMustGoOn) {

		//Consume item from buffer
		localpacket = consumePacket(cThread, &nextOut);  //Problem here: No exit condition

		WaitForSingleObject(cThread->mhStructSync, INFINITE);

		UpdateLocalShip(&cThread->localGameData, &localpacket);

		ReleaseMutex(cThread->mhStructSync);

	}

	return 0;
}

int RandomValue(int value) {

	int num = rand() % value + 1;

	return num;
}



