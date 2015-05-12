/* timeserv.c */
/* A simple UDP server that sends the current date and time to the client */
/* Last modified: September 20, 2005 */
/* http://www.gomorgan89.com */
/* Link with library file wsock32.lib */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "UDPServer.h"
using namespace std;


UDPServer::UDPServer(const char *host, unsigned int port)
{
	int iResult;
    struct sockaddr_in my_addr;    // my address information

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsadata);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
    }

    if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
    	fprintf(stderr, "cannot open socket.");
		return;
    }

	memset(&my_addr, 0, sizeof(struct sockaddr));
    my_addr.sin_family = AF_INET;         // host byte order
    my_addr.sin_port = htons(port);	     // short, network byte order
    my_addr.sin_addr.S_un.S_addr = inet_addr(host);

    iResult = bind(sd, (struct sockaddr *)&my_addr, sizeof(my_addr));
	if (iResult != 0)
    {
		fprintf(stderr, "bind failed with error %d\n", iResult);
		return;
    }

    // Set the mode of the socket to be nonblocking
    u_long iMode = 1;
    iResult = ioctlsocket(sd, FIONBIO, &iMode);

    if (iResult == SOCKET_ERROR) {
        printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
        closesocket(sd);
        WSACleanup();
    }

}




int UDPServer::checkMessage(char *buf, int length)
{
	int numbytes=0;
	int addr_len;
	struct sockaddr_in addr_from;

	// receive message. This read may block, depending on how the socket was configured.
	addr_len = sizeof(addr_from);
	numbytes = recvfrom(sd, buf, length, 0, (struct sockaddr*)&addr_from, &addr_len);
	return numbytes;
};
