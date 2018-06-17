#ifndef DATA_COMM_H
#define DATA_COMM_H

#include "data_structs.h"

//DWORD WINAPI sendPacketServer(LPVOID tParam);
//DWORD WINAPI ReadServerMsg(LPVOID tParam);
void populateSecurityAtributes(SECURITY_ATTRIBUTES *pSA);

DWORD WINAPI instanceThreadRead(LPVOID tParam);
DWORD WINAPI instanceThreadWrite(LPVOID tParam);
DWORD WINAPI CreatePipes(LPVOID tParam);

int readPipePacket(HANDLE hPipe, HANDLE readReady, Packet * pipePacket);
int writePipeMsg(HANDLE hPipe, HANDLE writeReady, GameData msg);

#endif /* DATA_COMM_H */
