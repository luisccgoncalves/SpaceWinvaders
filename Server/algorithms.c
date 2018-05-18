#include "algorithms.h"

DWORD WINAPI InvadersBomb(LPVOID tParam) {

	int * ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	SMGameData *lvl = ((SMCtrl *)tParam)->pSMemGameData;

	/* generate random number between 1 and 50: */
	int random = rand() % 50 + 1;

	lvl->bomb[0].x = lvl->invad[random].x;
	lvl->bomb[0].y = lvl->invad[random].y;
	while (*ThreadMustGoOn && lvl->bomb[0].y <25/*&&bombColDetect(&bomb,tParam)*/) {
		lvl->bomb[0].y++;
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
	SMGameData *lvl = ((SMCtrl *)tParam)->pSMemGameData;
	HANDLE		*mhStructSync = ((SMCtrl *)tParam)->mhStructSync;

	int i, j;
	int sidestep = 4;
	int totalsteps = (YSIZE - (MAX_INVADER / INVADER_BY_ROW)) * sidestep;
	int regInvaderNr = (MAX_INVADER - RAND_INVADER);


	while (*ThreadMustGoOn) {						//Thread main loop

		for (i = 0; (i < totalsteps) && *ThreadMustGoOn; i++) {

			WaitForSingleObject(mhStructSync, INFINITE);

			for (j = 0; (j < regInvaderNr) && *ThreadMustGoOn; j++) {

				lvl->invad[j].y = (i / sidestep) + lvl->invad[j].y_init;				//Invader goes down after n sidesteps

				if ((i % (sidestep * 2)) < sidestep)
					lvl->invad[j].x = (i % (sidestep * 2)) + lvl->invad[j].x_init;		//Invader goes right
				else if ((i % (sidestep * 2)) > sidestep)
					lvl->invad[j].x--;													//Invader goes left
			}

			ReleaseMutex(mhStructSync);

			Sleep(INVADER_SPEED*(*ThreadMustGoOn));
		}
	}

	return 0;
}

DWORD WINAPI RandPathInvaders(LPVOID tParam) {

	int * ThreadMustGoOn = &((SMCtrl *)tParam)->ThreadMustGoOn;
	SMGameData *lvl = ((SMCtrl *)tParam)->pSMemGameData;
	HANDLE		*mhStructSync = ((SMCtrl *)tParam)->mhStructSync;
	int i;
	int startIndex = MAX_INVADER - RAND_INVADER;
	int maxXpos = XSIZE - 1;
	int maxYpos = (int)((YSIZE - 1)*0.8);

	while (*ThreadMustGoOn) {						//Thread main loop

		WaitForSingleObject(mhStructSync, INFINITE);

		for (i = startIndex; (i < MAX_INVADER) && *ThreadMustGoOn; i++) {

			switch (rand() % 4) {
			case 0:
				if (lvl->invad[i].x > 0)
					lvl->invad[i].x--;
				else
					lvl->invad[i].x = 1;
				break;
			case 1:
				if (lvl->invad[i].x < maxXpos)
					lvl->invad[i].x++;
				else
					lvl->invad[i].x = maxXpos - 1;
				break;
			case 2:
				if (lvl->invad[i].y > 0)
					lvl->invad[i].y--;
				else
					lvl->invad[i].y = 1;
				break;
			case 3:
				if (lvl->invad[i].y < maxYpos)
					lvl->invad[i].y++;
				else
					lvl->invad[i].y = maxYpos - 1;
				break;
			}
		}

		ReleaseMutex(mhStructSync);
		Sleep((INVADER_SPEED / 4)*(*ThreadMustGoOn));
	}

	return 0;
}



