#include "snet.h"
#include "UDPClient.h"
#include "stdio.h"

// libs

#ifdef _DEBUG
#pragma comment(lib, "dsnet.lib")
#else
#pragma comment(lib, "snet.lib")
#endif

int main(int argc, char **argv)
{
	int status;
	UDPClient client(argv[1], atoi(argv[2]));
	status = client.sendMessage(argv[3], strlen(argv[3]));
	printf("sendMessage status %d\n", status);
	return 0;
}
