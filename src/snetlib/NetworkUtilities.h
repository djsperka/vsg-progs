#include "snet.h"
#include "NetworkData.h"

int sendPacket(SOCKET socket, APacketP packet);
int sendMsgToUDPServer(char *port, char *ip);
