#include "snet.h"


#define BUFFER_SIZE 4096

class UDPServer
{
private:
	char buffer[BUFFER_SIZE];			/* Where to store received data */
	int port;
	char host_name[256];				/* Name of the server */
	WSAData wsadata;
	SOCKET sd;
	struct sockaddr_in server_addr;
public:
	UDPServer(const char *host, unsigned int port);
	~UDPServer() {};
	int checkMessage(char *buf, int len);
	bool isReady() { return sd != INVALID_SOCKET; };

};


