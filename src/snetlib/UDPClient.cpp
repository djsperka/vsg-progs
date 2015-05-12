#include "UDPClient.h"
#include "stdio.h"
#include "stdlib.h"

UDPClient::UDPClient(const char *host, unsigned int port)
{
	int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsadata);
    if (iResult != 0) 
	{
        printf("WSAStartup failed with error: %d\n", iResult);
		return;
    }

	// create socket
    if ((sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) 
	{
    	fprintf(stderr, "cannot open socket.");
		return;
    }

	int optVal;
	int optLen = sizeof(int);
	unsigned int optUIVal;

	if (getsockopt(sd, SOL_SOCKET, SO_MAX_MSG_SIZE, (char *)&optUIVal, &optLen) != SOCKET_ERROR)
	{
		fprintf(stderr, "SO_MAX_MSG_SIZE %ul\n", optUIVal);
	}
	else
		fprintf(stderr, "SO_MAX_MSG_SIZE error\n");

    //setup address structure
    memset((char *) &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.S_un.S_addr = inet_addr(host);
}


int UDPClient::sendMessage(const char *msg, unsigned int len)
{
	int status;
    status = sendto(sd, msg, len, 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    return status;
}
