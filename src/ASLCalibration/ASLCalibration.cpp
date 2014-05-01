// ASLCalibration.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ASLCalibration.h"
#include "ASLSerial.h"

#include "VSGV8.H"
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__
#include "alertlib.h"
#include "AlertUtil.h"
#include <string>
#include <fstream>
using namespace std;
using namespace alert;

#define XDAT_OFFSET_BIT 0x4
#define XDAT_ENABLE_BIT 0x2
#define XDAT_QUIT_BIT 0x80
#define	XDAT_JUICE_MASK 0x1
#define SPIKE2_JUICE_MASK 0x2
#define SPIKE2_QUIT_BIT 0x4
#define SPIKE2_TRANSITION_BIT 0x8

COLOR_TYPE f_background;
int f_screenDistanceMM=0;
bool f_verbose=false;
double f_dCalibrationOffset = 0.0;
float f_fSlaveXOffset = 0.0;
float f_fSlaveYOffset = 0.0;
ARFixationPointSpec f_afp;

#ifndef MULTIPLE_PAGE_SOLUTION
ARContrastFixationPointSpec f_fixpts[10];	// first 9 are for master, 10th is for slave
#endif

PIXEL_LEVEL f_fixation_level;
string f_szConfigFile;
long f_lComPort = 2;
bool f_bRivalry = false;
string f_szOffsetFile;
TriggerVector triggers;
bool f_binaryTriggers = true;
bool f_bypassPhaseI = false;	// only valid when doing rivalry

int init_calibration();
void usage();
int init_pages();
int init_triggers();
int callback(int &output, const CallbackTrigger* ptrig);
int offsetfile_changed(bool bCreated, bool bDeleted);
void init_page(int ipage, void *unused);
void init_slave_page(int ipage, void *unused);
int args(int argc, char **argv);
void update_slave_fixpt();





#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	long spike2_output_bits = 0;
	int idot;
	int xdat;
	int lastxdat = 0;
	int ijuice;
	int iquit;
	int lastidot=-1;
	int ioffset = 0;
	int idisplay = 0;
	bool bOffsetFileWritten = false;
	bool bOffsetReceived = false;
	int last_output_trigger=0;
	long input_trigger = 0;
	string s;

	// Check input arguments
	if (args(argc, argv))
	{
		return 1;
	}
	else
	{
		if (f_verbose)
		{
			cout << "Screen distance " << f_screenDistanceMM << endl;
			cout << "Fixation point " << f_afp << endl;
			cout << "Background color " << f_background << endl;
		}
	}


	// COM initialzation
	CoInitialize(NULL);

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		return 1;
	}

	// Initialize VSG card or cards

	if (init_calibration())
	{
		cerr << "VSG initialization failed." << endl;
		return 1;
	}

	//Init triggers - hack alert!
	init_triggers();

	// bypass phase I?

	if (!f_bRivalry || (f_bRivalry && !f_bypassPhaseI))
	{
		cout << "Connecting to ASL controller...." << endl;

		if (aslserial_connect(f_szConfigFile, f_lComPort))
		{
			cerr << "Failed to connect to ASL serial port." << endl;
			return 1;
		}
		else 
		{
			// Initialize output juice_bit
			vsgIOWriteDigitalOut(0, SPIKE2_JUICE_MASK);


			// Loop forever. Break out of loop when calibration quit signal received.
			// djs 6-21-2010 Modifications to this loop for rivalry. 
			// When running in rivalry mode (-r on command line) we are using two VSGs. 
			// The alertlib uses dual singletons (doesn't sound right, but that's what 
			// it is) ARvsg::master() and ARvsg::slave(). We have to make sure VSG commands
			// are aimed at the correct VSG card using vsgInitSelectDevice(). Since its rare
			// that the slave is used (we'll only use it for displaying dot numbers 10-14)
			// I'm going to leave the card set for the master all the time and only switch it
			// when one of those dot numbers is received. Below I send the ready signal and 
			// set that up. 

			ARvsg::instance().ready_pulse(1000, 0x4);


			// Trigger loop for phase I of calibration. Monkey calibration uses ONLY phase I, and Rivalry
			// calibration uses phase I and phase II. In Rivalry calibration we can bypass phase I (useful
			// for testing) by using the "-B" command line arg in conjunction with the "-r filename" arg. 

			iquit = 0;
			while (!iquit)
			{
				ijuice = 0;
				iquit = 0;
				ioffset = 0;
				idisplay = 0;
				if (!aslserial_get(&idot, &xdat, &f_fSlaveXOffset, &f_fSlaveYOffset))
				{
					if (idot > 0 && idot < 10)
					{
						if (idot != lastidot)
						{
							idisplay = 1;
							cerr << "Dot " << idot << endl; 

							// change dot on screen here
							if (lastidot > 0) f_fixpts[lastidot-1].setContrast(0);
							if (idot > 0) f_fixpts[idot-1].setContrast(100);
							vsgPresent();
							lastidot = idot;
						}
					}
				}

				input_trigger = vsgIOReadDigitalIn();
				TriggerFunc	tf = std::for_each(triggers.begin(), triggers.end(), 
					(f_binaryTriggers ? TriggerFunc(input_trigger, last_output_trigger) : TriggerFunc(s, last_output_trigger)));

				// Now analyze input trigger
			 	
				if (tf.quit()) break;

				Sleep(100);
			}
			aslserial_disconnect();

		}
	}   // if (!f_bRivalry || (f_bRivalry && !f_bypassPhaseI))   -- Phase I calibration

	return 0;
}


int init_calibration()
{
	int status=0;
	VSGTRIVAL fixation_color;

	if (get_color(f_afp.color, fixation_color))
	{
		cerr << "Cannot get trival for fixation color " << f_afp.color << endl;
		return 2;
	}

	// Init vsg for single vsg usage
	if (!f_bRivalry)
	{
		if (ARvsg::instance().init(f_screenDistanceMM, f_background))
		{
			cerr << "VSG init failed!" << endl;
			return 1;
		}

		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
		for (unsigned int i=0; i<9; i++)
		{
			f_fixpts[i].color = f_afp.color;
			f_fixpts[i].d = f_afp.d;
			f_fixpts[i].x = (((int)i % 3) - 1) * f_dCalibrationOffset;
			f_fixpts[i].y = (((int)i / 3) - 1) * f_dCalibrationOffset;
			f_fixpts[i].init(ARvsg::instance(), 2);
			f_fixpts[i].setContrast(0);
			f_fixpts[i].draw();
		}
		vsgPresent();
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);
	}

	return status;
}


// init triggers for phase II of rivalry calibration. 
// There are just 4 triggers. Two are for showing/hiding the fixation points on the master
// and slave screen -- these are implemented as callback triggers. 
// A third is a quit trigger. The fourth is a FileChanged trigger used
// to communicate changes to the offset of the fixation point on the slave screen. 

int init_triggers()
{
	int status=0;
	// quit trigger
	triggers.addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));
	return status;
}



// The return value from this trigger callback determines whether a vsgPresent() is issued. 
// Since we are using overlay zone pages for the fixation point transitions, no vsgPresent() is used. 
// Instead, we use vsgIODigitalWrite() and vsgSetZoneDisplayPage(). 

int callback(int &output, const CallbackTrigger* ptrig)
{
	int ival=0;
	string key = ptrig->getKey();
	if (key == "M")
	{
		ARvsg::master().select();
		f_fixpts[4].setContrast(100);
		ival = PLEASE_PRESENT_MASTER;
	}
	else if (key == "m")
	{
		ARvsg::master().select();
		f_fixpts[4].setContrast(0);
		ival = PLEASE_PRESENT_MASTER;
	}
	else if (key == "S")
	{
		ARvsg::slave().select();
		f_fixpts[9].setContrast(100);
		ival = PLEASE_PRESENT_SLAVE;
	}
	else if (key == "s")
	{
		ARvsg::slave().select();
		f_fixpts[9].setContrast(0);
		ival = PLEASE_PRESENT_SLAVE;
	}
	return ival;
}


int offsetfile_changed(bool bCreated, bool bDeleted)
{
	ifstream in;
	ofstream out;
	istringstream iss;
	string s;
	int status=0;
	if (f_verbose) cerr << "offset file changed: created=" << bCreated << " deleted=" << bDeleted << endl;

	in.open(f_szOffsetFile.c_str());
	in >> f_fSlaveXOffset >> f_fSlaveYOffset;
	if (!in)
	{
		cerr << "Format error in offset file!" << endl;
		status = -1;
	}
	else
	{
		update_slave_fixpt();
	}
	in.close();

	return status;
}

// Redraw slave fixpt. Make sure current view page is page 0 before calling. 
void update_slave_fixpt()
{
	cerr << "Updating slave fixpt offset to " << f_fSlaveXOffset << ", " << f_fSlaveYOffset << endl;
	ARvsg::slave().select();
	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);
	vsgObjDestroy(f_fixpts[9].handle());
	vsgPresent();
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	f_fixpts[9].init(ARvsg::slave(), 2);
	f_fixpts[9].x = f_afp.x + f_fSlaveXOffset;
	f_fixpts[9].y = f_afp.y + f_fSlaveYOffset;
	f_fixpts[9].setContrast(0);
	f_fixpts[9].draw();
	cerr << "Slave fixpt updated: " << f_fixpts[9] << endl;
	vsgPresent();
	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);
}

int args(int argc, char **argv)
{	
	bool have_f=false;
	bool have_d=false;
	bool have_i=false;
	bool have_offset = false;
	string s;
	int c;
	int i;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "f:b:hd:vC:i:r:vBc:")) != -1)
	{
		switch (c) 
		{
		case 'B':
			f_bypassPhaseI = true;
			break;
		case 'v':
			f_verbose = true;
			break;
		case 'f': 
			s.assign(optarg);
			if (parse_fixation_point(s, f_afp)) errflg++;
			else have_f = true;
			break;
		case 'b': 
			s.assign(optarg);
			if (parse_color(s, f_background)) errflg++; 
			break;
		case 'd':
			s.assign(optarg);
			if (parse_distance(s, f_screenDistanceMM)) errflg++;
			else have_d=true;
			break;
		case 'c':
			s.assign(optarg);
			if (parse_integer(s, i)) errflg++;
			else f_lComPort = (long)i;
			break;
		case 'C':
			s.assign(optarg);
			if (parse_double(s, f_dCalibrationOffset)) errflg++;
			else have_offset = true;
			break;
		case 'i':
			f_szConfigFile.assign(optarg);
			have_i = true;
			break;
		case 'r':
			f_bRivalry = true;
			f_szOffsetFile.assign(optarg);
			break;
		case 'h':
			errflg++;
			break;
		case '?':
            errflg++;
			break;
		default:
			errflg++;
			break;
		}
	}

	if (!have_f) 
	{
		cerr << "Fixation point not specified!" << endl; 
		errflg++;
	}
	if (!have_d)
	{
		cerr << "Screen distance not specified!" << endl; 
		errflg++;
	}
	if (!have_offset)
	{
		cerr << "Calibration dots offset not specified!" << endl; 
		errflg++;
	}
	if (!have_i)
	{
		cerr << "Serial out config file not specified!" << endl;
		errflg++;
	}
	if (errflg) 
	{
		usage();
	}
	return errflg;
}

void usage()
{
	cerr << "usage: calibration -f x,y,d[,color] -d screen_distance_MM -b g|b|w -C dot_offset_degrees" << endl;
}
