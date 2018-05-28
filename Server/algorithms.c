#include "algorithms.h"

DWORD WINAPI InvadersBomb(LPVOID tParam) {

	int * ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	GameData *baseGame = &((SMCtrl *)tParam)->localGameData;
	/*GameData *lvl = ((SMCtrl *)tParam)->pSMemGameData;*/

	/* generate random number between 1 and 50: */
	int random = rand() % 50 + 1;

	baseGame->bomb[0].x = baseGame->invad[random].x;
	baseGame->bomb[0].y = baseGame->invad[random].y;
	while (*ThreadMustGoOn && baseGame->bomb[0].y <25/*&&bombColDetect(&bomb,tParam)*/) {
		baseGame->bomb[0].y++;
		/*Sleep(lvl->bomb[0].speed * (*ThreadMustGoOn));*/
		Sleep(250 * (*ThreadMustGoOn));
	}

	return 0;
	/*

	This needs to be a diferent thread. 
	One to launch the falling bomb, from which the code above can be part.
	
	One thread evaluates the oportunity to lauch bombs and starts (or wake up) another thread.
	Tther starts the trajectory of a bomb untill desapearing or collision

	*/
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



