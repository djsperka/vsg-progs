#include "snet.h"
#include "AClient.h"
#include "NetworkUtilities.h"

AClient::AClient(char *server_addr, char *server_port)
{

	printf("AClient: connect to server at %s:%s\n", server_addr, server_port);
    network = new ClientNetwork(server_addr, server_port);

    // send init packet
	printf("AClient: send init packet\n");
	APacketP packet = createPacket(INIT_CONNECTION, 0);
	sendPacket(this->network->ConnectSocket, packet);
	free(packet);

	printf("AClient: send command packet\n");
	packet = createPacket(COMMAND, 6);
	strcpy(packet->data, "Hello");
	sendPacket(this->network->ConnectSocket, packet);
	free(packet);
}


AClient::~AClient(void)
{
}

#if 0
bool AClient::sendPacket(APacketP packet)
{
	bool bval = false;
	unsigned int length = sizeof(struct apacket);
	if (packet->length > 0) length = length + packet->length-1;
	NetworkServices::sendMessage(network->ConnectSocket, (char *)packet, length);
	return bval;
}
#endif

#if 0
void AClient::update()
{
    Packet packet;
    int data_length = network->receivePackets(network_data);

    if (data_length <= 0) 
    {
        //no data recieved
        return;
    }

    int i = 0;
    while (i < (unsigned int)data_length) 
    {
        packet.deserialize(&(network_data[i]));
        i += sizeof(Packet);

        switch (packet.packet_type) {

            case ACTION_EVENT:

                printf("client received action event packet from server\n");

                sendActionPackets();

                break;

            default:

                printf("error in packet types\n");

                break;
        }
    }
}
#endif