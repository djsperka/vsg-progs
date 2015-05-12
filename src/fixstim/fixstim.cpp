/* $Id: fixstim.cpp,v 1.23 2015-05-12 17:19:32 devel Exp $*/
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>

/* Have to include snet.h before vsgv8.h */
#include "snet.h"
#include "vsgv8.h"

/* for multithreading */
#include <process.h>


#include "Alertlib.h"
#include "AlertUtil.h"
#include "StimSet.h"
#include "StimSetCRG.h"
#include "FGGXStimSet.h"
#include "EQStimSet.h"
#include "AttentionStimSet.h"
#include "SSInfo.h"
#include "UDPServer.h"
#include "UDPClient.h"
#include "FXGStimParameterList.h"
#include "MultiParameterFXGStimSet.h"
#include "FixUStim.h"
#include "CMouseUStim.h"
#include "CalibrationUStim.h"

using namespace std;
using namespace alert;
using namespace boost;
using namespace boost::filesystem;

// libs

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#pragma comment(lib, "dsnet.lib")
#else
#pragma comment(lib, "alert.lib")
#pragma comment(lib, "snet.lib")
#endif
#pragma comment (lib, "vsgv8.lib")


// globals
bool f_binaryTriggers = true;
bool f_verbose = false;
bool f_dumpStimSetsOnly = false;
bool f_bPresentOnTrigger = false;
string f_sTriggeredTriggers;
DWORD f_ulTriggerArmed = vsgDIG1;
bool f_bUseLock = false;
COLOR_TYPE f_background = { gray, {0.5, 0.5, 0.5}};
ARContrastFixationPointSpec f_fixpt;
ARGratingSpec f_grating;
ARXhairSpec f_xhair;
vector<ARGratingSpec> f_vecGratings;
vector<AttentionCue> f_vecAttentionCues;
StimSet *f_pStimSet = NULL;			// This is for master in dualVSG mode
bool f_bUsingMultiParameterStimSet = false;
int f_iDistanceToScreenMM = -1;
TriggerVector triggers;
int f_pulse = 0x40;
bool f_bDaemon = false;
//bool f_bClient = false;
//string f_sClientArgs;
//bool f_bClientSendQuit = false;
int f_iDaemonPort = 0;
string f_sDaemonHostIP = "127.0.0.1";
bool f_quit = false;
double f_dSlaveXOffset = 0.0;
double f_dSlaveYOffset = 0.0;
UDPServer *f_udpServer;
path f_pathCues;			// root path for cue files (see EQStimSet, 'W' arg)
int errflg = 0;

// These are the args allowed and which are handled by prargs. Do not use 'F' - it is reserved for 
// passing a command file.
const char *f_allowedServerArgs = "vu:b:d:";

// function prototypes

void serverLoop(void * arg);
int run_fixstim();
int prargs_callback(int c, string& arg);
int prargs_client_callback(int c, string& arg);
int prargs_server_callback(int c, string& arg);
void usage();
void init_triggers();
void init_globals();
int callback(int &output, const CallbackTrigger* ptrig);

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




int main (int argc, char *argv[])
{
	int status;

	// handler for ctrl-c
	SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler, TRUE );

	// Check input arguments for three possibilities:
	// 1. run fixstim server (-u)
	// 2. send fixstim commands to remote server (-c)
	// 3. run as standalone fixstim instance (legacy behavior)

	status = prargs(argc, argv, prargs_server_callback, f_allowedServerArgs, 'F');
	if (!status)
	{
		// init vsg. 
		cerr << "Running fixstim as daemon at address " << f_sDaemonHostIP << " port " << f_iDaemonPort << endl;
		cerr << "Initialize VSG..." << endl;
		if (UStim::initialize(ARvsg::instance(), f_iDistanceToScreenMM, f_background))
		{
			cerr << "VSG init failed!!!" << endl;
			return 1;
		}

		f_udpServer = new UDPServer(f_sDaemonHostIP.c_str(), f_iDaemonPort);
		if (!f_udpServer->isReady())
		{
			cerr << "Cannot start server." << endl;
		}
		else
		{
			HANDLE hThread;
			cerr << "Start listening thread, waiting for connections." << endl;
			// create thread with arbitrary argument for the run function
			hThread = (HANDLE)_beginthread( serverLoop, 0, (void*)12);
			WaitForSingleObject(hThread, INFINITE);
			printf("Listening thread finished.\n");
		}
	}
	else 
	{
		// process as a standalone fixstim run
		FixUStim fixstim(true);
		if (fixstim.parse(argc, argv))
		{
			fixstim.run_stim(ARvsg::instance());
		}
	}
	return 0;
}

void serverLoop(void * arg) 
{ 
	const unsigned int bufLength = 8192;
	string s;
	char buffer[bufLength];
	try
	{
		while(!f_quit) 
		{
			string sargs;
			int status;
			status = f_udpServer->checkMessage(buffer, bufLength);
			if (status > 0)
			{
				sargs.clear();
				sargs.assign(buffer, status);
				cout << "serverLoop(): got msg (length " << status << "): " << sargs << endl;

				if (sargs.find("quit") == 0)
				{
					cout << "serverLoop(): Quitting..." << endl;
					f_quit = true;
				}
				else if (sargs.find("cmouse") == 0)
				{
					CMouseUStim cmouseustim;
					if (cmouseustim.parses(sargs))
					{
						cerr << "serverLoop(): starting cmouse stimulus..." << endl;
						cmouseustim.run_stim(ARvsg::instance());
						ARvsg::instance().clear();
						cout << "serverLoop(): cmouse stimulus done." << endl;
					}
					else
					{
						cerr << "serverLoop(): CMouseUStim could not parse args." << endl;
					}
				}
				else if (sargs.find("fixstim") == 0)
				{
					FixUStim fixstim;
					if (fixstim.parses(sargs))
					{
						cerr << "serverLoop(): starting fixstim stimulus..." << endl;
						fixstim.run_stim(ARvsg::instance());
						ARvsg::instance().clear();
						cout << "serverLoop(): fixstim stimulus done." << endl;
					}
					else
					{
						cerr << "serverLoop(): FixUStim could not parse args." << endl;
					}
				}
				else if (sargs.find("calibration") == 0)
				{
					CalibrationUStim calustim;
					if (calustim.parses(sargs))
					{
						cerr << "serverLoop(): starting calibration stimulus..." << endl;
						calustim.run_stim(ARvsg::instance());
						ARvsg::instance().clear();
						cout << "serverLoop(): calibration stimulus done." << endl;
					}
					else
					{
						cerr << "serverLoop(): CalibrationUStim could not parse args." << endl;
					}
				}
				else
				{
					cout << "serverLoop(): No handler for this command." << endl;
				}

				init_globals();
				cout << "serverLoop(): waiting for message..." << endl;
			}
			Sleep(500);
		}
	} catch (std::exception& e) {
		cout << e.what() << endl;
	}
}

void init_globals()
{
	cout << "serverLoop(): clearing all vsg objects" << endl;
	ARvsg::instance().reset_available_levels();
	for (int i=2; i<vsgGetSystemAttribute(vsgNUMOBJECTS); i++) { vsgObjDestroy(i); }
}

void init_triggers()
{
	triggers.addTrigger(new CallbackTrigger("F", 0x2, 0x2, 0x2, 0x2, callback));
	triggers.addTrigger(new CallbackTrigger("S", 0x4, 0x4, 0x4, 0x4, callback));
	triggers.addTrigger(new CallbackTrigger("s", 0x4, 0x0, 0x4, 0x0, callback));
	triggers.addTrigger(new CallbackTrigger("X", 0x6, 0x0, 0x6, 0x0, callback));
	triggers.addTrigger(new CallbackTrigger("a", 0x8, 0x8|AR_TRIGGER_TOGGLE, 0x8, 0x8|AR_TRIGGER_TOGGLE, callback));
	triggers.addTrigger(new CallbackTrigger("u", 0x20, 0x20|AR_TRIGGER_TOGGLE, 0x10, 0x10|AR_TRIGGER_TOGGLE, callback));
	triggers.addTrigger(new CallbackTrigger("v", 0x40, 0x40|AR_TRIGGER_TOGGLE, 0x20, 0x20|AR_TRIGGER_TOGGLE, callback));

	// hack
	triggers.addTrigger(new CallbackTrigger("1", 0, 0, 0, 0, callback));
	triggers.addTrigger(new CallbackTrigger("2", 0, 0, 0, 0, callback));
	triggers.addTrigger(new CallbackTrigger("3", 0, 0, 0, 0, callback));
	triggers.addTrigger(new CallbackTrigger("4", 0, 0, 0, 0, callback));
	triggers.addTrigger(new CallbackTrigger("5", 0, 0, 0, 0, callback));
	triggers.addTrigger(new CallbackTrigger("6", 0, 0, 0, 0, callback));
	triggers.addTrigger(new CallbackTrigger("7", 0, 0, 0, 0, callback));


	// quit trigger
	triggers.addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));

	return;
}

int callback(int &output, const CallbackTrigger* ptrig)
{
	return  f_pStimSet->handle_trigger(ptrig->getKey());
}

int prargs_server_callback(int c, string& arg)
{	
	static bool have_d = false;
	switch(c)
	{
	case 'v':
		f_verbose = true;
		break;
	case 'u':
		{
			int pos = arg.find(":", 0);
			if (pos == string::npos)
			{
				cerr << "prargs_server_callback(): Bad server arg(" << arg << ") expecting aaa.bbb.ccc.ddd:port" << endl;
				errflg++;
			}
			else
			{
				f_sDaemonHostIP = arg.substr(0, pos);
				if (parse_integer(arg.substr(pos+1, arg.length()), f_iDaemonPort))
				{
					cerr << "prargs_server_callback(): Bad server arg(" << arg << ") expecting aaa.bbb.ccc.ddd:port" << endl;
					errflg++;
				}
				else
				{
					f_bDaemon = true;
					cerr << "prargs_server_callback(): Will act as server with host ip " << f_sDaemonHostIP << " port " << f_iDaemonPort << endl;
				}
			}
			break;
		}
	case 'b': 
		if (parse_color(arg, f_background)) errflg++; 
		break;
	case 'd':
		if (parse_distance(arg, f_iDistanceToScreenMM)) errflg++;
		else have_d=true;
		break;
	case 0:
		break;
	default:
		{
			cerr << "prargs_server_callback(): Unknown option - " << (char)c << endl;
			errflg++;
			break;
		}
	}

	return errflg;
}


