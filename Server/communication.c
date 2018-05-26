#include "communication.h"

void consumePacket(SMCtrl *tParam, int *nextOut, Packet *localpacket) {

	SMCtrl		*cThread = (SMCtrl*)tParam;

	//wait occupied semaphore
	WaitForSingleObject(cThread->shOccupied, INFINITE);

	//wait mutex
	WaitForSingleObject(cThread->mhProdConsMut, INFINITE);

	//copy buffer[nextout] to local
	//CopyMemory(localpacket, &cThread->pSMemMessage->buffer[*nextOut], sizeof(localpacket));
	*localpacket = cThread->pSMemMessage->buffer[*nextOut];

	//nextout++
	*nextOut = (*nextOut + 1) % SMEM_BUFF;

	//release mutex
	ReleaseMutex(cThread->mhProdConsMut);

	//release semaphore vacant	
	ReleaseSemaphore(cThread->shVacant, 1, NULL);
}

DWORD WINAPI ReadGatewayMsg(LPVOID tParam) {
	SMCtrl		*cThread = (SMCtrl*)tParam;

	Packet		localpacket;
	Ship		localship;

	int	nextOut = 0;


	while (cThread->ThreadMustGoOn) {

		//Consume item from buffer
		consumePacket(tParam, &nextOut, &localpacket);

		WaitForSingleObject(cThread->mhStructSync, INFINITE);
		ReleaseMutex(cThread->mhStructSync);

		//UpdateLocalShip(cThread->pSMemGameData, &localpacket);
		UpdateLocalShip(&cThread->game.gameData, &localpacket);

		//CopyMemory(cThread->pSMemGameData, &cThread->game.gameData, sizeof(GameData));

		WaitForSingleObject(cThread->mhStructSync, INFINITE);
		ReleaseMutex(cThread->mhStructSync);
	}

	return 0;
}

DWORD WINAPI WriteGatewayMsg(LPVOID tParam) {
	/*
	Here we will recieve either a copy
	either a pointer to a Game or GameData
	object to send it!
	*/
}