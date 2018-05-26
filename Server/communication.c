#include "communication.h"

//void consumePacket(SMCtrl *tParam, int *nextOut, Packet *localpacket) {
//
//	SMCtrl		*cThread = (SMCtrl*)tParam;
//
//	//wait occupied semaphore
//	WaitForSingleObject(cThread->shOccupied, INFINITE);
//
//	//wait mutex
//	WaitForSingleObject(cThread->mhProdConsMut, INFINITE);
//
//	//copy buffer[nextout] to local
//	//CopyMemory(localpacket, &cThread->pSMemMessage->buffer[*nextOut], sizeof(localpacket));
//	*localpacket = cThread->pSMemMessage->buffer[*nextOut];
//
//	//nextout++
//	*nextOut = (*nextOut + 1) % SMEM_BUFF;
//
//	//release mutex
//	ReleaseMutex(cThread->mhProdConsMut);
//
//	//release semaphore vacant	
//	ReleaseSemaphore(cThread->shVacant, 1, NULL);
//}


