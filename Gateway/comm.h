#ifndef COMM_H
#define COMM_H

#include "localStructs.h"

DWORD WINAPI sendPacketServer(LPVOID tParam);
//DWORD WINAPI ReadServerMsg(LPVOID tParam);

DWORD WINAPI instanceThreadRead(LPVOID tParam);
DWORD WINAPI instanceThreadWrite(LPVOID tParam);
DWORD WINAPI CreatePipes(LPVOID tParam);

#endif /* COMM_H */