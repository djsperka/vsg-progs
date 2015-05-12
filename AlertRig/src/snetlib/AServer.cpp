#include "snet.h"
#include "AServer.h"

unsigned int AServer::client_id; 

AServer::AServer(void)
{
    // id's to assign clients for our table
    client_id = 0;

    // set up the server network to listen 
    network = new ServerNetwork(); 
}

AServer::~AServer(void)
{
}

void AServer::update()
{
    // get new clients
	printf("calling network->acceptNewClient...\n");
   if(network->acceptNewClient(client_id))
   {
        printf("client %d has been connected to the server\n",client_id);

        client_id++;
   }
   else
   {
	   printf("no connection\n");
   }

   receiveFromClients();
}

void AServer::receiveFromClients()
{
	APacketP packet = NULL;

    // go through all clients
    std::map<unsigned int, SOCKET>::iterator iter;

    for(iter = network->sessions.begin(); iter != network->sessions.end(); iter++)
    {
        int data_length = network->receiveData(iter->first, network_data);

        if (data_length <= 0) 
        {
            //no data recieved
            continue;
        }

        unsigned int i = 0;
		printf("Unpacking packet length %d\n", data_length);
        while (i < (unsigned int)data_length) 
        {
			packet = (APacketP)(&network_data[i]);
			i += data_length;
            switch (packet->type) {

                case INIT_CONNECTION:

                    printf("server received init packet from client\n");

					break;

				case COMMAND:

					printf("Server received command from client: %.*s\n", packet->length, packet->data);
					break;

                default:

                    printf("error in packet types\n");

                    break;
            }
        }
    }
}

#if 0
void AServer::sendInfoPackets(char *msg)
{
    // send action packet
    const unsigned int packet_size = sizeof(Packet);
    char packet_data[packet_size];

    Packet packet;
    packet.packet_type = ACTION_EVENT;

    packet.serialize(packet_data);

    network->sendToAll(packet_data,packet_size);
}
#endif