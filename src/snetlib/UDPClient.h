#include "snet.h"
#define BUFFER_SIZE 4096

class UDPClient
{
private:
	char buffer[BUFFER_SIZE];			/* Where to store received data */
	int port;
	char host_name[256];				/* Name of the server */
	WSAData wsadata;
	SOCKET sd;
	struct sockaddr_in server_addr;
public:
	UDPClient(const char *host, unsigned int port);
	~UDPClient() {};
	int checkMessage();
	int sendMessage(const char *msg, unsigned int length);
	bool isReady() { return sd != INVALID_SOCKET; };

};


