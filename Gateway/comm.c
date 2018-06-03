#include "comm.h"

DWORD WINAPI sendPacketServer(LPVOID tParam) {
	/*
	This message calls the DLL function writePacket
	and sends a packet to the server trhough the Shared Memmory
	*/

	SMCtrl		*cThread = (SMCtrl*)tParam;
	//Packet		localpacket;
	int			nextIn = 0;

	while (cThread->ThreadMustGoOn) {
		WaitForSingleObject(cThread->heGotPacket, INFINITE);
		_tprintf(TEXT("[DEBUG] GOT KEY %d \n"), cThread->localPacket.instruction);
		writePacket(cThread, &nextIn, cThread->localPacket);
	}

	return 0;
}

DWORD WINAPI ReadServerMsg(LPVOID tParam) {
	/*
	This message calls the DLL function asdfasdas
	and sends a packet to the server trhough the Shared Memmory
	*/

	SMCtrl		*cThread = (SMCtrl*)tParam;

	while (cThread->ThreadMustGoOn) {

		WaitForSingleObject(cThread->hSMServerUpdate, INFINITE);
		cThread->localGameData = consumeGameData(cThread->pSMemGameData, cThread->mhGameData);

	}

	return 0;
}