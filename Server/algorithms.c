#include "algorithms.h"

DWORD WINAPI InvadersBomb(LPVOID tParam) {

	int * ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	GameData *baseGame = &((SMCtrl *)tParam)->localGameData;

	int invPosition = -1;
	int bombNum = -1;

	for (int i = 0; i < baseGame->max_bombs; i++) {
		if (!baseGame->bomb[i].fired) {
		bombNum = i;
			break;
		}
		//if (baseGame->bomb[i].y >= baseGame->ysize) {
		//	bombNum = i;
		//	break;
		//}
	}

	do {
		invPosition = rand() % baseGame->max_invaders + 1;
	} while (baseGame->invad[invPosition].hp == 0);

	if (bombNum > -1) {

		baseGame->bomb[bombNum].x = baseGame->invad[invPosition].x;
		baseGame->bomb[bombNum].y = baseGame->invad[invPosition].y;
		baseGame->bomb[bombNum].fired = 1;

		while (*ThreadMustGoOn && baseGame->bomb[bombNum].fired/*&&bombColDetect(&bomb,tParam)*/) {
			if (baseGame->bomb[bombNum].y < baseGame->ysize) {
				baseGame->bomb[bombNum].y++;

				Sleep(((baseGame->invaders_bombs_speed)/3) * (*ThreadMustGoOn));
			}
			else {
				baseGame->bomb[bombNum].x = baseGame->xsize + 1;
				baseGame->bomb[bombNum].y = baseGame->ysize + 1;
				baseGame->bomb[bombNum].fired = 0;
			}
		}
	}
	/*

	This needs to be a diferent thread. 
	One to launch the falling bomb, from which the code above can be part.
	
	One thread evaluates the oportunity to lauch bombs and starts (or wake up) another thread.
	Tther starts the trajectory of a bomb untill desapearing or collision

	*/
}

DWORD WINAPI BombMovement(LPVOID tParam) {

	int * ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	GameData *baseGame = &((SMCtrl *)tParam)->localGameData;

	int invPosition = -1;
	int bombNum = -1;

	for (int i = 0; i < baseGame->max_bombs; i++) {
		//if (!baseGame->bomb[i].fired) {
		//bombNum = i;
		//	break;
		//}
		if (baseGame->bomb[i].y >= baseGame->ysize) {
			bombNum = i;
			break;
		}
	}

	do {
		invPosition = rand() % baseGame->max_invaders + 1;
	} while (baseGame->invad[invPosition].hp == 0);

	if (bombNum > -1) {

		baseGame->bomb[bombNum].x = baseGame->invad[invPosition].x;
		baseGame->bomb[bombNum].y = baseGame->invad[invPosition].y;
		baseGame->bomb[bombNum].fired = 1;

		while (*ThreadMustGoOn && baseGame->bomb[bombNum].fired/*&&bombColDetect(&bomb,tParam)*/) {
			if (baseGame->bomb[bombNum].y < baseGame->ysize) {
				baseGame->bomb[bombNum].y++;

				Sleep(((baseGame->invaders_bombs_speed) / 3) * (*ThreadMustGoOn));
			}
			else {
				baseGame->bomb[bombNum].x = baseGame->xsize + 1;
				baseGame->bomb[bombNum].y = baseGame->ysize + 1;
				baseGame->bomb[bombNum].fired = 0;
			}
		}
	}
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



