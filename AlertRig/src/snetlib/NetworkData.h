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

#if 0
class APacket 
{
private:

	unsigned int atype;
	unsigned int alength;
	char *adata;

public:

	APacket() {};
	virtual ~APacket() {};
	unsigned int length() const { return alength; }	/* This is length of data , not of entire packet */
	unsigned int type() const { return atype; };
	char *data() const { return adata; };

	static bool pack(unsigned int type, unsigned int length, char *data)
	{
		APacketgfdghddgdghdghdghdhgdhgdghdghdghdghd
};
#endif

#if 0
struct Packet 
{

    unsigned int packet_type;

    void serialize(char * data) {
        memcpy(data, this, sizeof(Packet));
    }

    void deserialize(char * data) {
        memcpy(this, data, sizeof(Packet));
    }
};
#endif