/* $Id: fixstim.cpp,v 1.25 2016-04-01 22:23:35 devel Exp $*/
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

/* need to use this form of bind.hpp and the namespace to avoid compilation errors. */
#include <boost/bind/bind.hpp>
using namespace boost::placeholders;

#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__

/* git commit number */
#include "sha.h"
#define XSTR(x) STR(x)
#define STR(x) #x

/* for ASL (calibration) dll */
#include "stdafx.h"
#include "ASLSerial.h"

/* Have to include snet.h before vsgv8.h */
//#include "snet.h"
#include "vsgv8.h"

/* for multithreading */
#include <process.h>

/* for getenv */
#include <cstdlib>

/* smart ptrs*/
#include <memory>

/* my stguff */
#include "Alertlib.h"
#include "AlertUtil.h"
#include "StimSet.h"
#include "StimSetCRG.h"
#include "FGGXStimSet.h"
#include "EQStimSet.h"
#include "AttentionStimSet.h"
#include "SSInfo.h"
//#include "UDPServer.h"
//#include "UDPClient.h"
#include "AsyncTCPServerWrapper.h"
#include "FXGStimParameterList.h"
#include "MultiParameterFXGStimSet.h"
#include "FixUStim.h"
#include "CMouseUStim.h"
#include "CalibrationUStim.h"
#include "StarUStim.h"
#include "BeatUStim.h"
#include "BarUStim.h"
#include "MSequenceUStim.h"
#include "TcpUStim.h"

using namespace alert;

// globals
bool f_binaryTriggers = true;
bool f_dumpStimSetsOnly = false;
bool f_bPresentOnTrigger = false;
string f_sTriggeredTriggers;
DWORD f_ulTriggerArmed = vsgDIG1;
bool f_bUseLock = false;
COLOR_TYPE f_background(gray);
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
int f_iDaemonPort = 0;
string f_sDaemonHostIP = "127.0.0.1";
bool f_bHaveGammaFile = false;
string f_sGammaFile = string();
bool f_quit = false;
double f_dSlaveXOffset = 0.0;
double f_dSlaveYOffset = 0.0;
//UDPServer *f_udpServer;
std::unique_ptr<AsyncTCPServerWrapper> f_pTCPServerWrapper;
int errflg = 0;


// These are the args allowed and which are handled by prargs. Do not use 'F' - it is reserved for 
// passing a command file.
const char *f_allowedServerArgs = "u:b:d:M:";

// calibration
IASLSerialOutPort3* f_gpISerialOutPort = NULL;



// function prototypes

int init_calibration();
//void serverLoop(void * arg);
bool fixstim_server_callback(const std::string& sargs, std::ostream& out);
int run_fixstim();
int prargs_callback(int c, string& arg);
int prargs_server_callback(int c, string& arg);
void usage();
void init_triggers();
void init_globals();
int callback(int &output, const CallbackTrigger* ptrig);
bool loadGammaData(const std::string& filename);

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

int init_calibration()
{
	int status = 0;

	// COM initialzation
	CoInitialize(NULL);

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		status = -1;
	}
	else
	{
		// Create COM object
		HRESULT hr = CoCreateInstance(CLSID_ASLSerialOutPort3, NULL, CLSCTX_INPROC_SERVER,
			IID_IASLSerialOutPort3, (void**)&f_gpISerialOutPort);
		if (FAILED(hr))
		{
			cerr << "Error creating COM Server ASLSerialOutLib3 (" << hr << ")" << endl;
			if (hr == S_OK) cerr << "S_OK" << endl;
			else if (hr == REGDB_E_CLASSNOTREG)
			{
				CComBSTR bsError;
				f_gpISerialOutPort->GetLastError(&bsError);
				CString strError = bsError;
				cerr << "REGDB_E_CLASSNOTREG" << endl;
				cerr << bsError << endl;
			}
			else if (hr == CLASS_E_NOAGGREGATION) cerr << "CLASS_E_NOAGGREGATION" << endl;
			else if (hr == E_NOINTERFACE) cerr << "E_NOINTERFACE" << endl;
			else cerr << "unknown error " << hr << endl;
			status = 1;
		}
	}
	return status;
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

	cerr << "fixstim " << string(XSTR(GIT_VERSION)) << endl;
	status = prargs(argc, argv, prargs_server_callback, f_allowedServerArgs, 'F');
	if (!status)
	{
		std::string sGammaFile;
		char* p = NULL;
		// before we initialize the vsg, check if we should load a gamma file. 
		// If value is passed on command line (f_sGammaFile), then use that. 
		// If no value on command line, then check env variable VSG_GAMMA_FILE. If set, its value is taken as the name of the gamma file to be used. 
		// Default is to have no gamma file. 

		if (f_sGammaFile.size() > 0)
			sGammaFile = f_sGammaFile;
		else if (NULL != (p = getenv("VSG_GAMMA_FILE")))
			sGammaFile = string(p);

		cerr << "Running fixstim server at address " << f_sDaemonHostIP << " port " << f_iDaemonPort << endl;
		if (sGammaFile.size() > 0)
			cerr << "Using gamma file: " << sGammaFile << endl;
		else
			cerr << "Using default VSG gamma file for the current monitor configuration." << endl;

		cerr << "Initialize VSG..." << endl;
		if (UStim::initialize(ARvsg::instance(), f_iDistanceToScreenMM, f_background, sGammaFile))
		{
			cerr << "VSG init failed!!!" << endl;
			return 1;
		}

		cerr << "Initialize ASL..." << endl;
		if (init_calibration())
		{
			cerr << "ASL init failed!!!" << endl;
			return 1;
		}

		f_pTCPServerWrapper = std::unique_ptr<AsyncTCPServerWrapper>(new AsyncTCPServerWrapper(boost::bind(fixstim_server_callback, _1, _2), 7000, 26));	// 26 = ^Z

		// this is a blocking call, until someone calls f_pTCPServerWrapper->quit(). That should be done inside of the callback function when 'quit' is sent. 
		f_pTCPServerWrapper->start();
	}
	return 0;
}

// the TCP server calls this when a full command message (one that ends with ^Z) is received.
bool fixstim_server_callback(const std::string & sargs, std::ostream & out)
{
	std::cout << "fixstim_server_callback: " << sargs << std::endl;

	if (sargs.find("quit") == 0)
	{
		out << "OK;";
		cerr << "fixstim_server_callback(): got quit msg..." << endl;
		f_pTCPServerWrapper->stop();
	}
	else if (sargs.find("cmouse") == 0)
	{
		CMouseUStim cmouseustim;
		if (cmouseustim.parses(sargs))
		{
			out << "OK;";
			cerr << "fixstim_server_callback(): starting cmouse stimulus..." << endl;
			cmouseustim.run_stim(ARvsg::instance());
			ARvsg::instance().clear();
			cout << "fixstim_server_callback(): cmouse stimulus done." << endl;
		}
		else
		{
			out << "ERR - CMouseUStim could not parse args;";
			cerr << "fixstim_server_callback(): CMouseUStim could not parse args." << endl;
		}
	}
	else if (sargs.find("fixstim") == 0)
	{
		FixUStim fixstim;
		if (fixstim.parses(sargs))
		{
			out << "OK;";
			cerr << "fixstim_server_callback(): starting fixstim stimulus..." << endl;
			fixstim.run_stim(ARvsg::instance());
			ARvsg::instance().clear();
			cout << "fixstim_server_callback(): fixstim stimulus done." << endl;
		}
		else
		{
			out << "ERR - FixUStim could not parse args;";
			cerr << "fixstim_server_callback(): FixUStim could not parse args." << endl;
		}
	}
	else if (sargs.find("calibration") == 0)
	{
		CalibrationUStim calustim(f_gpISerialOutPort);
		if (calustim.parses(sargs))
		{
			out << "OK;";
			cerr << "fixstim_server_callback(): starting calibration stimulus..." << endl;
			calustim.run_stim(ARvsg::instance());
			ARvsg::instance().clear();
			cout << "fixstim_server_callback(): calibration stimulus done." << endl;
		}
		else
		{
			out << "ERR - CalibrationUStim could not parse args;";
			cerr << "fixstim_server_callback(): CalibrationUStim could not parse args." << endl;
		}
	}
	else if (sargs.find("starstim") == 0)
	{
		StarUStim starustim;
		if (starustim.parses(sargs))
		{
			out << "OK;";
			cerr << "fixstim_server_callback(): starting starstim stimulus..." << endl;
			starustim.run_stim(ARvsg::instance());
			ARvsg::instance().clear();
			cout << "fixstim_server_callback(): starstim stimulus done." << endl;
		}
		else
		{
			out << "ERR - StarUStim could not parse args;";
			cerr << "fixstim_server_callback(): StarUStim could not parse args." << endl;
		}
	}
	else if (sargs.find("barstim") == 0)
	{
		BarUStim barustim;
		if (barustim.parses(sargs))
		{
			out << "OK;";
			cerr << "fixstim_server_callback(): starting barstim stimulus..." << endl;
			barustim.run_stim(ARvsg::instance());
			ARvsg::instance().clear();
			cout << "fixstim_server_callback(): barstim stimulus done." << endl;
		}
		else
		{
			out << "ERR - BarUStim could not parse args;";
			cerr << "fixstim_server_callback(): BarUStim could not parse args." << endl;
		}
	}
	else if (sargs.find("beat") == 0)
	{
		BeatUStim beatstim;
		if (beatstim.parses(sargs))
		{
			out << "OK;";
			cerr << "fixstim_server_callback(): starting beat stimulus..." << endl;
			beatstim.run_stim(ARvsg::instance());
			ARvsg::instance().clear();
			cout << "fixstim_server_callback(): beat stimulus done." << endl;
		}
		else
		{
			out << "ERR - BeatUStim could not parse args;";
			cerr << "fixstim_server_callback(): BeatUStim could not parse args." << endl;
		}
	}
	else if (sargs.find("msequence") == 0)
	{
		MSequenceUStim msq;
		if (msq.parses(sargs))
		{
			out << "OK;";
			cerr << "fixstim_server_callback(): starting msequence stimulus..." << endl;
			msq.run_stim(ARvsg::instance());
			ARvsg::instance().clear();
			cout << "fixstim_server_callback(): msequence stimulus done." << endl;
		}
		else
		{
			out << "ERR - MSequenceUStim could not parse args;";
			cerr << "fixstim_server_callback(): MSequenceUStim could not parse args." << endl;
		}
	}
	else if (sargs.find("tcp") == 0)
	{
		TcpUStim tcp;
		if (tcp.parses(sargs))
		{
			out << "OK;";
			cerr << "fixstim_server_callback(): starting tcp stimulus..." << endl;
			tcp.run_stim(ARvsg::instance());
			ARvsg::instance().clear();
			cout << "fixstim_server_callback(): msequence stimulus done." << endl;
		}
		else
		{
			out << "ERR - TcpUStim could not parse args;";
			cerr << "fixstim_server_callback(): TcpUStim could not parse args." << endl;
		}
	}
	else
	{
		cout << "fixstim_server_callback(): No handler for this command." << endl;
	}

	ARvsg::instance().reinit();
	return true;
}

void init_globals()
{
	cout << "serverLoop(): clearing all vsg objects except dummy object (" << ARvsg::instance().dummyObjectHandle() << ")" << endl;
	ARvsg::instance().reset_available_levels();
	for (int i=1; i<vsgGetSystemAttribute(vsgNUMOBJECTS); i++) { if (i != ARvsg::instance().dummyObjectHandle()) vsgObjDestroy(i); }
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
				}
			}
			break;
		}
	case 'M':
	{
		f_bHaveGammaFile = true;
		f_sGammaFile = arg;
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


