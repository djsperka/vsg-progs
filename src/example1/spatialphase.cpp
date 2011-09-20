#include <iostream>
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

int args(int argc, char **argv);
void init_triggers();
int callback(int &output, const CallbackTrigger* ptrig);

using namespace std;
using namespace alert;

ARContrastFixationPointSpec m_afp;
COLOR_TYPE m_background;
vector<ARGratingSpec*> m_gratings;
int m_screenDistanceMM=0;
bool m_verbose=false;
TriggerVector triggers;
bool m_binaryTriggers = true;
bool m_bCalibration = false;
const int f_iPage0 = 0;
const int f_iPage1 = 1;
const int f_iPageBlank = 2;


static void usage();
static int init_pages();


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
			for (unsigned int i=0; i<m_gratings.size(); i++)
			{
				cout << "Grating " << (i+1) << ": " << *m_gratings[i] << endl;
			}
		}
	}

	// Init vsg
	if (ARvsg::instance().init(m_screenDistanceMM, m_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}

	// write video pages and create triggers
	init_pages();

	// Issue "ready" triggers to spike2.
	// These commands pulse spike2 port 6. 
	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x20, 0);
	vsgPresent();

	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x00, 0);
	vsgPresent();

	triggers.reset(vsgIOReadDigitalIn());



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

		// Now analyze input trigger
	 	
		if (tf.quit()) break;
		else if (tf.present())
		{	
			last_output_trigger = tf.output_trigger();
			vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
//			cout << "SetTriggers=" << tf.output_trigger() << endl;
			vsgPresent();
		}
	}

	ARvsg::instance().clear();
	ARvsg::instance().release_lock();

	return 0;
}



int args(int argc, char **argv)
{	
	bool have_d=false;
	string s;
	int c;
	ARGratingSpec *pspec=NULL;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "s:b:hd:va")) != -1)
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
		case 's':
			pspec = new ARGratingSpec();
			s.assign(optarg);
			if (!parse_grating(s, *pspec))
			{
				m_gratings.push_back(pspec);
			}
			else errflg++;
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

	if (m_gratings.size() < 1)
	{
		cerr << "Must specify a grating!" << endl;
		errflg++;
	}
	if (!have_d)
	{
		cerr << "Screen distance not specified!" << endl; 
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
	cerr << "usage: example1 -d screen_distance_MM -b g|b|w -s x,y,w,h,contrast%,sf,tf,orientation,color_vector,s|q,r|e -p phase" << endl;
}

int init_pages()
{
	int status=0;
	int islice=50;
	ContrastTrigger *ptrig = NULL;
	CallbackTrigger *pcall = NULL;
	string s;

	// set current display page to page 1.
	vsgSetZoneDisplayPage(vsgVIDEOPAGE, f_iPage1);

	// We will draw on page 0, so we will not see drawing artifacts. 
	// The draw page is not displayed until vsgPresent() is called. 
	vsgSetDrawPage(vsgVIDEOPAGE, f_iPage0, vsgBACKGROUND);

	// draw first grating. Note that with the above setup, gratings should be drawn
	// with draw() or draw((long)vsgTRANSONLOWER) or draw(false)
	m_gratings[0]->init(islice);
	m_gratings[0]->draw();
	m_gratings[0]->setContrast(0);
	triggers.addTrigger(new CallbackTrigger("S", 0x2, 0x2, 0x2, 0x2, callback));
	triggers.addTrigger(new CallbackTrigger("s", 0x2, 0x0, 0x2, 0x2, callback));

	if (m_gratings.size() > 1)
	{
		m_gratings[1]->init(islice);
		m_gratings[1]->draw(false);
		m_gratings[1]->setContrast(0);
		triggers.addTrigger(new CallbackTrigger("T", 0x4, 0x4, 0x2, 0x2, callback));
		triggers.addTrigger(new CallbackTrigger("t", 0x4, 0x0, 0x2, 0x2, callback));
	}

	// This will make the current draw page the current display page. 
	vsgPresent();



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



// The return value from this trigger callback determines whether a vsgPresent() is issued. 

int callback(int &output, const CallbackTrigger* ptrig)
{
	int ival=1;
	string key = ptrig->getKey();


//	cout << "callback: key " << ptrig->getKey() << endl;

	if (key == "S")
	{
		m_gratings[0]->setContrast(100);
		m_gratings[0]->select();
		vsgObjResetDriftPhase();
	}
	else if (key == "s")
	{
		m_gratings[0]->setContrast(0);
	}
	else if (key == "T")
	{
		if (m_gratings.size() > 1)
		{
			m_gratings[1]->setContrast(100);
			m_gratings[1]->select();
			vsgObjResetDriftPhase();
		}
	}
	else if (key == "t")
	{
		if (m_gratings.size() > 1)
		{
			m_gratings[1]->setContrast(0);
		}
	}

	return ival;
}
