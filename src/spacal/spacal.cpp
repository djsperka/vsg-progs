#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__

#include "vsgv8.h"
#include "Alertlib.h"
#include "AlertUtil.h"

//#define MASTER_SLAVE


#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment (lib, "vsgv8.lib")


using namespace std;
using namespace alert;

ARFixationPointSpec m_fixpt;
COLOR_TYPE m_background = { gray, {0.5, 0.5, 0.5}};
int m_screenDistanceMM=0;
bool m_verbose=false;
TriggerVector triggers;
bool m_binaryTriggers = true;
string m_nextfile;
string m_currfile;
int f_nextfile_index = 0;
int f_currfile_index = 0;
double f_next_X = 0.0;
double f_next_Y = 0.0;

static void usage();
static int init_pages();
int callback(int &output, const CallbackTrigger* ptrig);
int args(int argc, char **argv);
int nextfile_changed(bool bCreated, bool bDeleted);


int main (int argc, char *argv[])
{

	// Check input arguments
	if (args(argc, argv))
	{
		return 1;
	}
	else
	{
		if (m_verbose)
		{
			cout << "Screen distance " << m_screenDistanceMM << endl;
			cout << "Background color " << m_background << endl;
		}
	}

	// Init vsgs
#ifdef MASTER_SLAVE
	if (ARvsg::master().init(m_screenDistanceMM, m_background))
	{
		cerr << "Master VSG init failed!" << endl;
		return 1;
	}

	if (ARvsg::slave().init(m_screenDistanceMM, m_background))
	{
		cerr << "Slave VSG init failed!" << endl;
		return 1;
	}
#else
	if (ARvsg::instance().init(m_screenDistanceMM, m_background))
	{
		cerr << "Slave VSG init failed!" << endl;
		return 1;
	}
#endif

	// write video pages and create triggers
	init_pages();

	// Send ready pulse.
#ifdef MASTER_SLAVE
	ARvsg::master().ready_pulse();
#else
	ARvsg::instance().ready_pulse();
#endif

	// All right, start monitoring triggers........
	std::string s;
	int last_output_trigger=0;
	while (1)
	{
		// If user-triggered, get a trigger entry. 
		if (!m_binaryTriggers)
		{
			// Get a new "trigger" from user
			cout << "Enter trigger/key: ";
			cin >> s;
		}

		TriggerFunc	tf = std::for_each(triggers.begin(), triggers.end(), 
			(m_binaryTriggers ? TriggerFunc(vsgIOReadDigitalIn(), last_output_trigger) : TriggerFunc(s, last_output_trigger)));

		if (tf.quit()) break;
#if 0	 	
		else if (tf.present())
		{	
			last_output_trigger = tf.output_trigger();
			vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
//			cout << "SetTriggers=" << tf.output_trigger() << endl;
			vsgPresent();
		}
#endif
	}

#ifdef MASTER_SLAVE
	ARvsg::master().clear();
	ARvsg::slave().clear();
#else
	ARvsg::instance().clear();
#endif

	return 0;
}


int args(int argc, char **argv)
{	
	bool have_d=false;
	bool have_n = false;
	bool have_c = false;
	bool have_f = false;
	string s;
	int c;
	ARGratingSpec *pspec=NULL;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "f:b:hd:van:c:")) != -1)
	{
		switch (c) 
		{
		case 'a':
			m_binaryTriggers = false;
			break;
		case 'v':
			m_verbose = true;
			break;
		case 'b': 
			s.assign(optarg);
			if (parse_color(s, m_background)) errflg++; 
			break;
		case 'd':
			s.assign(optarg);
			if (parse_distance(s, m_screenDistanceMM)) errflg++;
			else have_d=true;
			break;
		case 'f':
			s.assign(optarg);
			if (parse_fixation_point(s, m_fixpt)) errflg++;
			else have_f = true;
			break;
		case 'n':
			m_nextfile.assign(optarg);
			have_n = true;
			break;
		case 'c':
			m_currfile.assign(optarg);
			have_c = true;
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

	if (!have_d)
	{
		cerr << "Screen distance not specified!" << endl; 
		errflg++;
	}
	if (!have_n)
	{
		cerr << "Next file (-n) not specified!" << endl; 
		errflg++;
	}
	if (!have_c)
	{
		cerr << "Current file (-c) not specified!" << endl; 
		errflg++;
	}
	if (!have_f)
	{
		cerr << "Fixation point (-f) not specified!" << endl; 
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
	cerr << "usage: spacal -f x,y,d,c -d screen_distance_MM -b g|b|w -n next_file_name -c curr_file_name" << endl;
}


int nextfile_changed(bool bCreated, bool bDeleted)
{
	ifstream in;
	ofstream out;
	istringstream iss;
	string s;
	int status=0;
	cout << "next file changed: created=" << bCreated << " deleted=" << bDeleted << endl;
	if (bDeleted) status=1;
	else
	{
		in.open(m_nextfile.c_str());
		if (getline(in, s))
		{
			iss.str(s);
			iss >> f_nextfile_index >> f_next_X >> f_next_Y;

			// TODO: Move fixation point and display it. 


			if (!iss)
			{
				cerr << "format error  in next file!" << endl;
				status = -1;
			}
			else
			{
				out.open(m_currfile.c_str(), ios::trunc);
				out << s;
				out.close();
			}
		}
		else
		{
			cerr << "Error reading from next file!" << endl;
			status = -1;
		}
		in.close();
	}
	return status;
}

int init_pages()
{
	int status = 0;
	triggers.addTrigger(new FileChangedTrigger("y", m_nextfile, nextfile_changed));

	// quit trigger
	triggers.addTrigger(new QuitTrigger("q", 0x80, 0x80, 0xff, 0x0, 0));


	// Set vsg trigger mode
	vsgObjSetTriggers(vsgTRIG_ONPRESENT+vsgTRIG_TOGGLEMODE,0,0);


	// Dump triggers
	std::cout << "Triggers:" << std::endl;
	for (unsigned int i=0; i<triggers.size(); i++)
	{
		std::cout << "Trigger " << i << " " << *(triggers[i]) << std::endl;
	}


	return status;
}


#if 0
// The return value from this trigger callback determines whether a vsgPresent() is issued. 
// Since we are using overlay zone pages for the fixation point transitions, no vsgPresent() is used. 
// Instead, we use vsgIODigitalWrite() and vsgSetZoneDisplayPage(). 

int callback(int &output, const CallbackTrigger* ptrig)
{
	int ival=0;
	string key = ptrig->getKey();
	if (key == "F")
	{
		vsgSetDrawPage(vsgOVERLAYPAGE, 1, vsgNOCLEAR);

		// trickery to get triggers out for advance
		vsgIOWriteDigitalOut(output, ptrig->outMask());
		vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 1);
	}
	else if (key == "f")
	{
		vsgSetDrawPage(vsgOVERLAYPAGE, 0, vsgNOCLEAR);

		// trickery to get triggers out for advance
		vsgIOWriteDigitalOut(output, ptrig->outMask());
		vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 0);
	}
	else if (key=="0")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);

		// trickery to get triggers out for advance
		vsgIOWriteDigitalOut(output, ptrig->outMask());
		//vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0 + vsgTRIGGERPAGE);
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);
	}
	else if (key=="1")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);

		// trickery to get triggers out for advance
		vsgIOWriteDigitalOut(output, ptrig->outMask());
		//vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1 + vsgTRIGGERPAGE);
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);
	}
	else if (key=="X")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgNOCLEAR);
		vsgSetDrawPage(vsgOVERLAYPAGE, 0, vsgNOCLEAR);

		// trickery to get triggers out for advance
		vsgIOWriteDigitalOut(output, ptrig->outMask());
		vsgSetDrawPage(vsgOVERLAYPAGE, 0, vsgNOCLEAR);
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 2 + vsgTRIGGERPAGE);
	}
	else if (key=="T")
	{
		m_gratings[0]->select();
		vsgObjSetTriggers(vsgTRIG_DRIFTVEL, 0, 0);
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);
		vsgPresent();
	}
	return ival;
}
#endif