#include "snet.h"
#include "ClientNetwork.h"


ClientNetwork::ClientNetwork(char *server_addr, char *server_port)
{
    // create WSADATA object
    WSADATA wsaData;

    // socket
    ConnectSocket = INVALID_SOCKET;

    // holds address info for socket to connect to
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        exit(1);
    }



    // set address info
    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;  //TCP connection!!!

	
    //resolve server address and port 
    //iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	iResult = getaddrinfo((server_addr ? server_addr : "127.0.0.1"), server_port, &hints, &result);

    if( iResult != 0 ) 
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        exit(1);
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) 
	{

		//struct addrinfo *res;   // populated elsewhere in your code
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)ptr->ai_addr;
		char ipAddress[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(ipv4->sin_addr), ipAddress, INET_ADDRSTRLEN);

		printf("Connecting to address: %s\n", ipAddress);

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);

        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            exit(1);
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

        if (iResult == SOCKET_ERROR)
        {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            printf ("The server is down... did not connect");
        }
    }

    // no longer need address info for server
    freeaddrinfo(result);



    // if connection failed
    if (ConnectSocket == INVALID_SOCKET) 
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
        exit(1);
    }

	// Set the mode of the socket to be nonblocking
    u_long iMode = 1;

    iResult = ioctlsocket(ConnectSocket, FIONBIO, &iMode);
    if (iResult == SOCKET_ERROR)
    {
        printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        exit(1);        
    }

	//disable nagle
    char value = 1;
    setsockopt( ConnectSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof( value ) );
}


ClientNetwork::~ClientNetwork(void)
{
	closesocket(ConnectSocket);
	WSACleanup();
}

int ClientNetwork::receivePackets(char * recvbuf) 
{
    iResult = NetworkServices::receiveMessage(ConnectSocket, recvbuf, MAX_PACKET_SIZE);

    if ( iResult == 0 )
    {
        printf("Connection closed\n");
        closesocket(ConnectSocket);
        WSACleanup();
        exit(1);
    }

    return iResult;
}

int ClientNetwork::sendMessage(char *buffer, int bufsiz)
{
	iResult = NetworkServices::sendMessage(ConnectSocket, buffer, bufsiz);
    if ( iResult == 0 )
    {
		printf("ClientNetwork::Connection closed\n");
        closesocket(ConnectSocket);
        WSACleanup();
    }
	return iResult;
}
