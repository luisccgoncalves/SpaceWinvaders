#ifndef COMM_H
#define COMM_H

#include "localStructs.h"

//DWORD WINAPI sendPacketServer(LPVOID tParam);
//DWORD WINAPI ReadServerMsg(LPVOID tParam);

DWORD WINAPI instanceThreadRead(LPVOID tParam);
DWORD WINAPI instanceThreadWrite(LPVOID tParam);
DWORD WINAPI CreatePipes(LPVOID tParam);

int readPipePacket(HANDLE hPipe, HANDLE readReady, Packet * pipePacket);
int writePipeMsg(HANDLE hPipe, HANDLE writeReady, GameData msg);

#endif /* COMM_H */