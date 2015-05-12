#pragma once
#include "snet.h"
#include "ServerNetwork.h"
#include "NetworkData.h"

class AServer
{

public:

    AServer(void);
    ~AServer(void);

    void update();

	void receiveFromClients();

	void sendInfoPackets();

private:

   // IDs for the clients connecting for table in ServerNetwork 
    static unsigned int client_id;

   // The ServerNetwork object 
    ServerNetwork* network;

	// data buffer
   char network_data[MAX_PACKET_SIZE];
};