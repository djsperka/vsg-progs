#include "snet.h"
#include "AServer.h"
#include "UDPServer.h"
// used for multi-threading
#include <process.h>

// libs

#ifdef _DEBUG
#pragma comment(lib, "dsnet.lib")
#else
#pragma comment(lib, "snet.lib")
#endif

void serverLoop(void *);
void clientLoop(void);

//AServer * server;
UDPServer *userver;
bool f_quit = false;
#define BUFFER_LENGTH 2048
char buffer[BUFFER_LENGTH];

BOOL CtrlHandler( DWORD fdwCtrlType ) 
{ 
  switch( fdwCtrlType ) 
  { 
    // Handle the CTRL-C signal. 
    case CTRL_C_EVENT: 
		f_quit = true;
	    printf( "Ctrl-C event\n\n" );
		return( TRUE );
	default:
		return FALSE;
  }
}

int main(int argc, char **argv)
{
	HANDLE hThread;

	SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler, TRUE );

	// initialize the server
	//server = new AServer();

	userver = new UDPServer(argv[1], atoi(argv[2]));
	if (!userver->isReady())
	{
		printf("not ready\n");
		exit(1);
	}

	printf("Start thread\n");
	// create thread with arbitrary argument for the run function
    hThread = (HANDLE)_beginthread( serverLoop, 0, (void*)12);
	WaitForSingleObject(hThread, INFINITE);
	printf("end thread\n");

	return 0;
}

void serverLoop(void * arg) 
{ 
    while(!f_quit) 
    {
		printf("chkMessage()\n");
		int status;
        status = userver->checkMessage(buffer, BUFFER_LENGTH);
		if (status > 0) printf("Got msg %.*s\n", status, buffer);

		Sleep(500);
    }
}

