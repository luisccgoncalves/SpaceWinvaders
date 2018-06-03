#ifndef COMM_H
#define COMM_H

#include "localStructs.h"

DWORD WINAPI sendPacketServer(LPVOID tParam);
DWORD WINAPI ReadServerMsg(LPVOID tParam);

#endif /* COMM_H */