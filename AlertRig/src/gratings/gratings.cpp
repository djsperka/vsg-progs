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

//ARContrastFixationPointSpec m_afp;
ARFixationPointSpec m_fixpt;
bool m_bHaveFixpt = false;
COLOR_TYPE m_background;
vector<ARGratingSpec*> m_gratings;
int m_screenDistanceMM=0;
bool m_verbose=false;
TriggerVector triggers;
bool m_binaryTriggers = true;
bool m_bCalibration = false;
bool m_driftvelTriggers = false;

const int f_iPage0 = 0;
const int f_iPage1 = 1;
const int f_iPageBlank = 2;
const int f_iPageFixpt = 3;


static void usage();
static int init_pages();
int callback(int &output, const CallbackTrigger* ptrig);


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
				cout << "Grating " << (i+1) << ": " << m_gratings[i] << endl;
			}
		}
	}

	// INit vsg
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

	//triggers.reset(vsgIOReadDigitalIn());

	cerr << "Initial triggers..." << vsgIOReadDigitalIn() << endl;


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
	while ((c = getopt(argc, argv, "s:b:hd:vaf:T")) != -1)
	{
		switch (c) 
		{
		case 'a':
			m_binaryTriggers = false;
			break;
		case 'v':
			m_verbose = true;
			break;
		case 'T':
			m_driftvelTriggers = true;
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
		case 'f':
			s.assign(optarg);
			if (parse_fixation_point(s, m_fixpt)) errflg++;
			else m_bHaveFixpt = true;
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

	if (m_gratings.size() != 2)
	{
		cerr << "Must specify exactly two gratings!" << endl;
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
	cerr << "usage: gratings -d screen_distance_MM -b g|b|w -s x,y,w,h,contrast%,sf,tf,orientation,color_vector,s|q,r|e -s x,y,w,h,contrast%,sf,tf,orientation,color_vector,s|q,r|e" << endl;
}

int init_pages()
{
	int status=0;
	int islice=50;

	// initialize video pages
	if (ARvsg::instance().init_video())
	{
		cerr << "VSG video initialization failed!" << endl;
		return 1;
	}

	if (m_bHaveFixpt)
	{
		VSGTRIVAL c;
		//m_fixpt.init(vsgFIXATION);
		get_color(m_fixpt.color, c);
		vsgSetFixationColour(&c);
	}


	// draw first grating on page 0
	vsgSetDrawPage(vsgVIDEOPAGE, f_iPage0, vsgNOCLEAR);
	m_gratings[0]->init(islice);
	m_gratings[0]->draw(true);

	// If needed, draw fixpt on same page. Must init fixpt first. 
	if (m_bHaveFixpt)
	{
		vsgSetPen1(vsgFIXATION);
		m_fixpt.draw();
	}

	// draw second grating on page 1
	vsgSetDrawPage(vsgVIDEOPAGE, f_iPage1, vsgNOCLEAR);
	m_gratings[1]->init(islice);
	m_gratings[1]->draw(true);

	// If needed, draw fixpt on same page. No need to init fixpt - that was done above. 
	if (m_bHaveFixpt)
	{
		vsgSetPen1(vsgFIXATION);
		m_fixpt.draw();
	}

	// Page 2 is blank. Page 3 will have fixpt alone, if needed. 
	vsgSetDrawPage(vsgVIDEOPAGE, f_iPageFixpt, vsgNOCLEAR);
	if (m_bHaveFixpt)
	{
		vsgSetPen1(vsgFIXATION);
		m_fixpt.draw();
	}


#if 0
	// initialize overlay pages. Overlay page 0 and 1 will be used. Init them to CLEAR. 
	if (ARvsg::instance().init_overlay())
	{
		cerr << "VSG overlay initialization failed!" << endl;
		return 1;
	}
	vsgSetDrawPage(vsgOVERLAYPAGE, 1, 0);
	if (m_bHaveFixpt)
	{
		arutil_color_to_overlay_palette(m_afp, 3);
		arutil_draw_overlay(m_afp, 3, 1);
	}
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 0);
#endif

	// trigger to turn stim 0/1 on
	triggers.addTrigger(new PageTrigger("0", 0x6, 0x2, 0x2, 0x2, 0));
	triggers.addTrigger(new PageTrigger("1", 0x6, 0x4, 0x2, 0x2, 1));
	//triggers.addTrigger(new CallbackTrigger("0", 0x6, 0x2, 0x2, 0x2, callback));
	//triggers.addTrigger(new CallbackTrigger("1", 0x6, 0x4, 0x2, 0x2, callback));

	// trigger to toggle 0/1 contrast
	triggers.addTrigger(new TogglePageTrigger("z", 0x8, 0x8|AR_TRIGGER_TOGGLE, 0x4, 0x4|AR_TRIGGER_TOGGLE, 0, 1));

	// trigger to turn stim OFF
	triggers.addTrigger(new PageTrigger("X", 0x16, 0x0, 0xa, 0x0, 4));

	// Fixation point trigger
	triggers.addTrigger(new PageTrigger("F", 0x10, 0x10, 0x8, 0x8, 3));
	//triggers.addTrigger(new PageTrigger("f", 0x10, 0x0, 0x8, 0x0, 4));

	if (m_driftvelTriggers)
	{
		triggers.addTrigger(new CallbackTrigger("T", 0x0, 0x0, 0x0, 0x0, callback));
	}

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