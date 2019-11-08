#pragma once
#include "snet.h"

#define MAX_PACKET_SIZE 1000000

enum APacketTypes {

    INIT_CONNECTION = 0,
	INFO = 1,
	COMMAND = 2

};


/* 
 * APacketP packet = createPacket(INFO, 10);
 * strcpy(packet->data, "Hello");
 * ...
 * free(packet);
 */

struct apacket
{
	unsigned int type;
	unsigned int length;
	char data[1];
};
typedef struct apacket* APacketP;

APacketP createPacket(unsigned int type, unsigned int length);

