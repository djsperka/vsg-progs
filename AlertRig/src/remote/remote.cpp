/* $Id: remote.cpp,v 1.2 2016-05-18 19:36:46 devel Exp $*/
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "BlockingTCPClientSend.h"

//#include "snet.h"
//#include "UDPServer.h"
//#include "UDPClient.h"

using namespace std;

// libs

//#ifdef _DEBUG
//#pragma comment(lib, "dsnet.lib")
//#else
//#pragma comment(lib, "snet.lib")
//#endif



int main(int argc, char **argv)
{
	int status;
	if (argc > 2)
	{
		// expect first arg to be host
		// expect second arg to be port
		// no checking
		int iport = atoi(argv[2]);
		char *server = argv[1];
		cout << "Got server " << string(server) << " port " << iport << endl;

// push rest of args into a stringsteam
		stringstream ss;

		for (int i=3; i<argc; i++)
		{
			cout << i << ":" << argv[i] << endl;
			ss << argv[i] << " ";
		}

		cout << "Got remainder: " << ss.str() << endl;

		BlockingTCPClientSendString(std::string(argv[1]), std::string(argv[2]), ss.str());
		//UDPClient *client = new UDPClient(server, iport);
		//status = client->sendMessage(ss.str().c_str(), ss.str().length());
		//cout << "status " << status << endl;
		//delete client;
	}
	else
	{
		cout << "not enough args" << endl;
	}

	return 0;
}


