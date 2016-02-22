#include "snet.h"
#include "NetworkServices.h"
#include <ws2tcpip.h>
#include <stdio.h> 
#include "NetworkData.h"

// size of our buffer
#define DEFAULT_BUFLEN 512
// port to connect sockets through 
#define DEFAULT_PORT "6881"

class ClientNetwork
{

public:

    // for error checking function calls in Winsock library
    int iResult;

    // socket for client to connect to server
    SOCKET ConnectSocket;

    // ctor/dtor
	ClientNetwork(char *server_addr = "127.0.0.1", char *server_port = DEFAULT_PORT);
    ~ClientNetwork(void);

	int receivePackets(char *);

	// added by djs
	int sendMessage(char * buffer, int bufSize);
};

