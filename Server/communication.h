#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "localStructs.h"

void consumePacket(SMCtrl *tParam, int * nextOut, packet *localpacket);
DWORD WINAPI ReadGatewayMsg(LPVOID tParam);

#endif // COMMUNICATION_H
