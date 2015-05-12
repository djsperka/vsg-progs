#pragma once
#include "snet.h"
#include "ClientNetwork.h"
#include "NetworkData.h"

class AClient
{
public:
	AClient(char *server_addr = (char *)NULL, char *server_port = (char *)DEFAULT_PORT);
	~AClient(void);

	ClientNetwork* network;

	//bool sendPacket(unsigned int type, unsigned int length, char *data);

    char network_data[MAX_PACKET_SIZE];

    void update();
};

