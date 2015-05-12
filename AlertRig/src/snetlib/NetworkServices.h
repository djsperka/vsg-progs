#include "snet.h"
#include "NetworkData.h"

class NetworkServices
{
public:
	static int sendMessage(SOCKET curSocket, char * buffer, int bufSize);
	static int receiveMessage(SOCKET curSocket, char * buffer, int bufSize);
};

