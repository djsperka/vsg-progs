#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__

#include "vsgv8.h"
#include "Alertlib.h"
#include "AlertUtil.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment (lib, "vsgv8.lib")

using namespace std;
using namespace alert;

ARContrastFixationPointSpec f_masterFixpt;
ARContrastFixationPointSpec f_slaveFixpt;
bool f_bHaveFixpt = false;
COLOR_TYPE f_background;
vector<ARGratingSpec*> f_masterGratings;
vector<ARGratingSpec*> f_slaveGratings;
ARXhairSpec f_masterXhair;
ARXhairSpec f_slaveXhair;
bool f_bXhair = false;
int f_screenDistanceMM=0;
bool f_verbose=false;
TriggerVector triggers;
bool f_binaryTriggers = true;
bool f_bRivalry = false;
string f_szOffsetFile;
float f_fSlaveXOffset = 0.0;
float f_fSlaveYOffset = 0.0;
bool f_bSlaveSynch = false;
const int f_iPage0 = 0;
const int f_iPage1 = 1;
const int f_iPageBlank = 2;
const int f_iPageFixpt = 3;


void usage();
void init_rivalry_vsg(ARvsg& vsg, ARFixationPointSpec& fixpt, ARGratingSpec& g0, ARGratingSpec& g1, bool useXhair, ARXhairSpec& xh);
int init_rivalry();
int callback(int &output, const CallbackTrigger* ptrig);
int args(int argc, char **argv);
void print_stuff();


int main (int argc, char *argv[])
{

	// Check input arguments
	if (args(argc, argv))
	{
		return 1;
	}

	// Open and read offset file, then update slave grating positions. 
	if (f_bRivalry)
	{
		ifstream in;
		in.open(f_szOffsetFile.c_str());
		in >> f_fSlaveXOffset >> f_fSlaveYOffset;
		if (!in)
		{
			cerr << "Format error in offset file!" << endl;
			return 1;
		}
		else
		{
			// Slave gratings are identical to master gratings, except for position
			cerr << "Offset file " << f_szOffsetFile << endl;
			cerr << "Slave gratings offset by (" << f_fSlaveXOffset << ", " << f_fSlaveYOffset << ")" << endl;
			for (unsigned int i=0; i<f_slaveGratings.size(); i++)
			{
				f_slaveGratings[i]->x += f_fSlaveXOffset;
				f_slaveGratings[i]->y += f_fSlaveYOffset;
			}
			f_slaveFixpt.x += f_fSlaveXOffset;
			f_slaveFixpt.y += f_fSlaveYOffset;
		}
	}
	if (f_verbose)
	{
		cout << "Screen distance " << f_screenDistanceMM << endl;
		cout << "Background color " << f_background << endl;
		for (unsigned int i=0; i<f_masterGratings.size(); i++)
		{
			cout << "Master Grating " << (i+1) << ": " << f_masterGratings[i] << endl;
		}
		for (unsigned int i=0; i<f_slaveGratings.size(); i++)
		{
			cout << "Slave  Grating " << (i+1) << ": " << f_slaveGratings[i] << endl;
		}
	}

	// Init master VSG and pages
	if (init_rivalry())
	{
		cerr << "VSG initialization failed." << endl;
		return 1;
	}

	// Ready pulses...
	ARvsg::master().select();
	ARvsg::master().ready_pulse(250, 0x2);
	ARvsg::slave().select();
	ARvsg::slave().ready_pulse(250, 0x2);


	// All right, start monitoring triggers........
	std::string s;
	int last_output_trigger=0;
	long input_trigger = 0;
	while (1)
	{
		// If user-triggered, get a trigger entry. 
		if (!f_binaryTriggers)
		{
			// Get a new "trigger" from user
			cout << "Enter trigger/key: ";
			cin >> s;
		}
		else
		{
			ARvsg::master().select();
			input_trigger = vsgIOReadDigitalIn();
		}

		MasterSlaveTriggerFunc	tf = std::for_each(triggers.begin(), triggers.end(), 
			(f_binaryTriggers ? MasterSlaveTriggerFunc(input_trigger, last_output_trigger) : MasterSlaveTriggerFunc(s, last_output_trigger)));

		// Now analyze input trigger
	 	
		if (tf.quit()) break;
		else if (tf.present())
		{	
			// Exactly what type of present do we need? 
			last_output_trigger = tf.output_trigger();
			if (tf.deferred() & (PLEASE_PRESENT_MASTER | PLEASE_PRESENT_SLAVE))
			{
				VSGPAGEDESCRIPTOR page;
				if (tf.deferred() & PLEASE_PRESENT_MASTER)
				{
					ARvsg::master().select();
					vsgGetCurrentDrawPage(&page);
					//cerr << "M: z=" << vsgGetZoneDisplayPage(vsgVIDEOPAGE) << " d=" << page.Page << endl;
					//f_masterGratings[0]->select();
					vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
					vsgPresent();
				}
				if (tf.deferred() & PLEASE_PRESENT_SLAVE)
				{
					ARvsg::slave().select();
					vsgGetCurrentDrawPage(&page);
					//cerr << "S: z=" << vsgGetZoneDisplayPage(vsgVIDEOPAGE) << " d=" << page.Page << endl;
					//f_slaveGratings[0]->select();
					vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
					vsgPresent();
				}
			}
		}

		Sleep(100);
	}
	ARvsg::master().select();
	ARvsg::master().clear();
	ARvsg::slave().select();
	ARvsg::slave().clear();

	return 0;
}



void print_stuff()
{
	ARvsg::master().select();
	cerr << "Master page " << vsgGetZoneDisplayPage(vsgVIDEOPAGE) << endl;
	ARvsg::slave().select();
	cerr << "Slave page " << vsgGetZoneDisplayPage(vsgVIDEOPAGE) << endl;
}


// The return value from this trigger callback determines whether a vsgPresent() is issued. 

int callback(int &output, const CallbackTrigger* ptrig)
{
	int status=0;
	string key = ptrig->getKey();
	if (key == "5")
	{
		ARvsg::master().select();
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0 + vsgTRIGGERPAGE);
		ARvsg::slave().select();
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0 + vsgTRIGGERPAGE);
		status = 0;
	}
	else if (key == "6")
	{
		ARvsg::master().select();
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0 + vsgTRIGGERPAGE);
		ARvsg::slave().select();
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1 + vsgTRIGGERPAGE);
		status = 0;
	}
	else if (key == "9")
	{
		ARvsg::master().select();
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1 + vsgTRIGGERPAGE);
		ARvsg::slave().select();
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0 + vsgTRIGGERPAGE);
		status = 0;
	}
	else if (key == "A")
	{
		ARvsg::master().select();
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1 + vsgTRIGGERPAGE);
		ARvsg::slave().select();
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1 + vsgTRIGGERPAGE);
		status = 0;
	}
	else if (key == "G")
	{
		ARvsg::master().select();
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		f_masterGratings[0]->setContrast(100);
		f_masterGratings[0]->resetDriftPhase();
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		f_masterGratings[1]->setContrast(100);
		f_masterGratings[1]->resetDriftPhase();
		ARvsg::slave().select();
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		f_slaveGratings[0]->setContrast(100);
		f_slaveGratings[0]->resetDriftPhase();
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		f_slaveGratings[1]->setContrast(100);
		f_slaveGratings[1]->resetDriftPhase();

		status = PLEASE_PRESENT_MASTER | PLEASE_PRESENT_SLAVE;
	}
	else if (key == "g")
	{
		int page;

		// select master
		ARvsg::master().select();

		// Get current display page. Leave that page as draw page
		page = vsgGetZoneDisplayPage(vsgVIDEOPAGE);
		vsgSetDrawPage(vsgVIDEOPAGE, 1-page, vsgNOCLEAR);
		f_masterGratings[1-page]->setContrast(100);
		f_masterGratings[1-page]->resetDriftPhase();
		vsgSetDrawPage(vsgVIDEOPAGE, page, vsgNOCLEAR);
		f_masterGratings[page]->setContrast(100);
		f_masterGratings[page]->resetDriftPhase();

		// Select slave
		ARvsg::slave().select();
		
		// Get current display page. Leave that page as draw page
		page = vsgGetZoneDisplayPage(vsgVIDEOPAGE);
		vsgSetDrawPage(vsgVIDEOPAGE, 1-page, vsgNOCLEAR);
		f_slaveGratings[1-page]->setContrast(100);
		f_slaveGratings[1-page]->resetDriftPhase();
		vsgSetDrawPage(vsgVIDEOPAGE, page, vsgNOCLEAR);
		f_slaveGratings[page]->setContrast(100);
		f_slaveGratings[page]->resetDriftPhase();

		status = PLEASE_PRESENT_MASTER | PLEASE_PRESENT_SLAVE;
	}
	else if (key == "X")
	{
		ARvsg::master().select();
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 2 + vsgTRIGGERPAGE);
		ARvsg::slave().select();
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 2 + vsgTRIGGERPAGE);
		status = 0;
	}
	else if (key == "x")
	{
		ARvsg::master().select();
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		f_masterGratings[0]->setContrast(0);
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		f_masterGratings[1]->setContrast(0);
		vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgNOCLEAR);
		ARvsg::slave().select();
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		f_slaveGratings[0]->setContrast(0);
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		f_slaveGratings[1]->setContrast(0);
		vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgNOCLEAR);
		status = PLEASE_PRESENT_MASTER | PLEASE_PRESENT_SLAVE;
	}
	else if (key == "P")
	{
		VSGCYCLEPAGEENTRY f_MPositions[5];

		// prepare cycling for master: Page 0 -> Page 0 -> Page 2(Stop)
		f_MPositions[0].Frames = 70;
		f_MPositions[0].Page = 0;
		f_MPositions[0].Stop = 0;
		f_MPositions[1].Frames = 70;
		f_MPositions[1].Page = 0 + vsgTRIGGERPAGE;
		f_MPositions[1].Stop = 0;
		f_MPositions[2].Frames = 70;
		f_MPositions[2].Page = 2;
		f_MPositions[2].Stop = 1;
		ARvsg::master().select();
		vsgPageCyclingSetup(3, &f_MPositions[0]);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);

		// prepare cycling for slave: Page 1 -> Page 0 -> Page 2(Stop)
		f_MPositions[0].Frames = 70;
		f_MPositions[0].Page = 1;
		f_MPositions[0].Stop = 0;
		f_MPositions[1].Frames = 70;
		f_MPositions[1].Page = 0 + vsgTRIGGERPAGE;
		f_MPositions[1].Stop = 0;
		f_MPositions[2].Frames = 70;
		f_MPositions[2].Page = 2;
		f_MPositions[2].Stop = 1;
		ARvsg::slave().select();
		vsgPageCyclingSetup(3, &f_MPositions[0]);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 0;
	}
	else if (key == "Y")
	{
		status = PLEASE_PRESENT_MASTER | PLEASE_PRESENT_SLAVE;
	}
	else if (key == "Z")
	{
		// stop cycling on each card
		ARvsg::master().select();
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		ARvsg::slave().select();
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
	}
	return status;
}


// init_rivalry_vsg
//
// Initializes pages for a single vsg (which is assumed to have been initialized and selected). 
// The grating specs g0 and g1 are drawn on pages 0 and 1 respectively. 
//

void init_rivalry_vsg(ARvsg& vsg, ARFixationPointSpec& fixpt, ARGratingSpec& g0, ARGratingSpec& g1, bool bUseXhair, ARXhairSpec& xhair)
{
	int islice = 50;

	vsg.select();
	vsgIOWriteDigitalOut(0x0, 0xff);

	// Init slave objects
	fixpt.init(vsg, 2);
	g0.init(vsg, islice);
	g1.init(vsg, islice);
	if (bUseXhair)
	{
		xhair.init(vsg, 16);
	}

	// page 2 is background - clear it and display during initialization
	vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgBACKGROUND);
	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 2);

	// page 0 has fixpt, xhair and grating[0], grating initially set contrast 0
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	if (bUseXhair)
	{
		xhair.draw();
	}
	g0.draw((long)vsgTRANSONLOWER);
	g0.setContrast(0);
	fixpt.draw();

	// Page 1 has fixpt and grating[1]
	vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);
	if (bUseXhair)
	{
		xhair.draw();
	}
	g1.draw((long)vsgTRANSONLOWER);
	g1.setContrast(0);
	fixpt.draw();

	// reset draw page to page 2 and issue vsgPresent().
	vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgNOCLEAR);
	if (bUseXhair)
	{
		xhair.draw();
	}
	vsgPresent();

}

int init_rivalry()
{
	int status = 0;
	int islice = 50;
	std::vector<std::pair<std::string, int> >vecInputs;



	// Init master vsg
	if (ARvsg::master().init(f_screenDistanceMM, f_background))
	{
		cerr << "VSG init for master failed!" << endl;
		return -1;
	}

	// Now init pages on the master vsg
	cerr << "Master gratings[0] " << *f_masterGratings[0] << endl;
	cerr << "Master gratings[1] " << *f_masterGratings[1] << endl;

	// if xhairs to be used, set their centers using the grating position
	if (f_bXhair)
	{
		f_masterXhair.x = f_masterGratings[0]->x;
		f_masterXhair.y = f_masterGratings[0]->y;
	}
	init_rivalry_vsg(ARvsg::master(), f_masterFixpt, *f_masterGratings[0], *f_masterGratings[1], f_bXhair, f_masterXhair);

	// Init slave vsg
	if (ARvsg::slave().init(f_screenDistanceMM, f_background, true, f_bSlaveSynch))
	{
		cerr << "VSG init for slave failed!" << endl;
		return -1;
	}

	// Now init pages on the slave vsg
	cerr << "Slave gratings[0] " << *f_slaveGratings[0] << endl;
	cerr << "Slave gratings[1] " << *f_slaveGratings[1] << endl;

	// if xhairs to be used, set their centers using the grating position
	if (f_bXhair)
	{
		f_slaveXhair.x = f_slaveGratings[0]->x;
		f_slaveXhair.y = f_slaveGratings[0]->y;
	}

	init_rivalry_vsg(ARvsg::slave(), f_slaveFixpt, *f_slaveGratings[0], *f_slaveGratings[1], f_bXhair, f_slaveXhair);

	// Triggers
	//triggers.addTrigger(new CallbackTrigger("1", 0x1E, 0xA, 0x4, 0x4|AR_TRIGGER_TOGGLE, callback));
	//triggers.addTrigger(new CallbackTrigger("2", 0x1E, 0x14, 0x4, 0x4|AR_TRIGGER_TOGGLE, callback));
	//triggers.addTrigger(new CallbackTrigger("3", 0x1E, 0xC, 0x4, 0x4|AR_TRIGGER_TOGGLE, callback));
	//triggers.addTrigger(new CallbackTrigger("4", 0x1E, 0x12, 0x4, 0x4|AR_TRIGGER_TOGGLE, callback));
	//triggers.addTrigger(new CallbackTrigger("5", 0x1E, 0x0, 0x4, 0x4|AR_TRIGGER_TOGGLE, callback));
	vecInputs.push_back(std::pair< string, int>("5", 0xA));
	vecInputs.push_back(std::pair< string, int>("A", 0x14));
	vecInputs.push_back(std::pair< string, int>("6", 0xC));
	vecInputs.push_back(std::pair< string, int>("9", 0x12));
	vecInputs.push_back(std::pair< string, int>("X", 0x0));
	triggers.push_back(new MultiInputSingleOutputCallbackTrigger(vecInputs, 0x1E, 0x4, 0x4|AR_TRIGGER_TOGGLE, callback));

	// Trigger to activate/deactivate gratings
	vecInputs.clear();
	vecInputs.push_back(std::pair< string, int>("g", 0x20));
	vecInputs.push_back(std::pair< string, int>("x", 0x40));
	triggers.push_back(new MultiInputSingleOutputCallbackTrigger(vecInputs, 0x60, 0x2, 0x2|AR_TRIGGER_TOGGLE, callback));
	//triggers.addTrigger(new CallbackTrigger("g", 0x20, 0x20, 0x2, 0x2|AR_TRIGGER_TOGGLE, callback));
	//triggers.addTrigger(new CallbackTrigger("x", 0x40, 0x40, 0x2, 0x2|AR_TRIGGER_TOGGLE, callback));

	// quit trigger
	triggers.addTrigger(new QuitTrigger("q", 0x80, 0x80, 0xff, 0x0, 0));

	// Dump triggers
	std::cout << "Triggers:" << std::endl;
	for (unsigned int i=0; i<triggers.size(); i++)
	{
		std::cout << "Trigger " << i << " " << *(triggers[i]) << std::endl;
	}

	return status;
}


int args(int argc, char **argv)
{	
	bool have_d=false;
	string s;
	int c;
	ARGratingSpec *pspec1=NULL;
	ARGratingSpec *pspec2=NULL;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "s:b:hd:vaf:r:SX:")) != -1)
	{
		switch (c) 
		{
		case 'a':
			f_binaryTriggers = false;
			break;
		case 'v':
			f_verbose = true;
			break;
		case 'b': 
			s.assign(optarg);
			if (parse_color(s, f_background)) errflg++; 
			break;
		case 'S':
			f_bSlaveSynch = true;
			break;
		case 'd':
			s.assign(optarg);
			if (parse_distance(s, f_screenDistanceMM)) errflg++;
			else have_d=true;
			break;
		case 's':
			pspec1 = new ARGratingSpec();
			pspec2 = new ARGratingSpec();
			s.assign(optarg);
			if (!parse_grating(s, *pspec1) && !parse_grating(s, *pspec2))
			{
				f_masterGratings.push_back(pspec1);
				f_slaveGratings.push_back(pspec2);
			}
			else errflg++;
			break;
		case 'f':
			s.assign(optarg);
			if (parse_fixation_point(s, f_masterFixpt)) errflg++;
			else 
			{
				f_bHaveFixpt = true;
				f_slaveFixpt = f_masterFixpt;
			}
			break;
		case 'r':
			f_bRivalry = true;
			f_szOffsetFile.assign(optarg);
			break;
		case 'X':
			s.assign(optarg);
			if (parse_xhair(s, f_masterXhair)) errflg++;
			else
			{
				f_bXhair = true;
				f_slaveXhair = f_masterXhair;
			}
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

	if (f_masterGratings.size() != 2)
	{
		cerr << "Must specify exactly two gratings!" << endl;
		errflg++;
	}
	if (!have_d)
	{
		cerr << "Screen distance not specified!" << endl; 
		errflg++;
	}
	if (!f_bRivalry)
	{
		cerr << "Must specify offset file! (-r)" << endl;
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
	cerr << "usage: rivalry -r offsetfile -d screen_distance_MM -b g|b|w -f x,y,d,color -s x,y,w,h,contrast%,sf,tf,orientation,color_vector,s|q,r|e -s x,y,w,h,contrast%,sf,tf,orientation,color_vector,s|q,r|e" << endl;
}

