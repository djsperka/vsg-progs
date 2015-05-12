#include "snet.h"
#include "NetworkData.h"
#include "NetworkUtilities.h"
#include "NetworkServices.h"

int sendPacket(SOCKET socket, APacketP packet)
{
	unsigned int length = sizeof(struct apacket);
	if (packet->length > 0) length = length + packet->length-1;
	return NetworkServices::sendMessage(socket, (char *)packet, length);
}

