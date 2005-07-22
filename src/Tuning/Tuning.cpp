#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include "Alertlib.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

using namespace std;
using namespace alert;

#define BACKGROUND_PAGE 0
#define FIXATION_PAGE 1
#define STIMULUS_PAGE 2

int args(int argc, char **argv);
void init_triggers();
int init_pages();
void usage();

bool m_verbose = false;
bool m_binaryTriggers = true;
ARContrastFixationPointSpec m_afp;
ARGratingSpec m_stim;
COLOR_TYPE m_background;
int m_screenDistanceMM=0;
TriggerVector m_triggers;

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
			cout << "Fixation point " << m_afp << endl;
			cout << "Stimulus " << m_stim << endl;
			cout << "Background color " << m_background << endl;
		}
	}


	// INit vsg
	if (ARvsg::instance().init(m_screenDistanceMM, m_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}


	// initialize video pages
	if (ARvsg::instance().init_video())
	{
		cerr << "VSG video initialization failed!" << endl;
		return 1;
	}
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);

	// write video pages
	init_pages();

	// init triggers
	init_triggers();

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

		TriggerFunc	tf = std::for_each(m_triggers.begin(), m_triggers.end(), 
			(m_binaryTriggers ? TriggerFunc(vsgIOReadDigitalIn(), last_output_trigger) : TriggerFunc(s, last_output_trigger)));

		// Now analyze input trigger
	 	
		if (tf.quit()) break;
		else if (tf.present())
		{	
			last_output_trigger = tf.output_trigger();
			vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
			vsgPresent();
		}
	}

	ARvsg::instance().clear();

	return 0;
}




int args(int argc, char **argv)
{	
	bool have_f=false;
	bool have_d=false;
	bool have_g=false;
	string s;
	int c;
	ARGratingSpec *pspec=NULL;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "f:b:g:hd:va")) != -1)
	{
		switch (c) 
		{
		case 'a':
			m_binaryTriggers = false;
			break;
		case 'v':
			m_verbose = true;
			break;
		case 'f': 
			s.assign(optarg);
			if (parse_fixation_point(s, m_afp)) errflg++;
			else have_f = true;
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
		case 'g':
//			pspec = new ARGratingSpec();
			s.assign(optarg);
			if (parse_grating(s, m_stim)) errflg++;
			else have_g = true;
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
	if (!have_g)
	{
		cerr << "Grating not specified!" << endl;
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
	cerr << "usage: tuning -f x,y,d[,color] -d screen_distance_MM -b g|b|w -g x,y,w,h,contrast%,sf,tf,orientation,color_vector,s|q,r|e] [-C c1,c2...|-T t1,t2,...|-S s1,s2,...|-O o1,o2...]" << endl;
}


void init_triggers()
{
	ContrastTrigger *ptrig;

	// triggers for fix pt
	ptrig = new ContrastTrigger("F", 0x2, 0x2, 0xff, 0x1);
	ptrig->push_back( std::pair<VSGOBJHANDLE, int>(m_afp.handle(), 100) );
	m_triggers.addTrigger(ptrig);
	ptrig = new ContrastTrigger("f", 0x2, 0x0, 0xff, 0x0);
	ptrig->push_back( std::pair<VSGOBJHANDLE, int>(m_afp.handle(), 0) );
	m_triggers.addTrigger(ptrig);

	// triggers for stim
	ptrig = new ContrastTrigger("S", 0x4, 0x4, 0xff, 0x2);
	ptrig->push_back( std::pair<VSGOBJHANDLE, int>(m_stim.handle(), 100) );
	m_triggers.addTrigger(ptrig);
	ptrig = new ContrastTrigger("s", 0x4, 0x0, 0xff, 0x0);
	ptrig->push_back( std::pair<VSGOBJHANDLE, int>(m_stim.handle(), 0) );
	m_triggers.addTrigger(ptrig);

	OrientationTuningTrigger *potrig = new OrientationTuningTrigger("A", 0x8, 0x8, 0xff, 0x4, m_stim, 0, 360, 10);
	m_triggers.addTrigger(potrig);

	m_triggers.addTrigger(new QuitTrigger("q", 0x8, 0x8, 0xff, 0x0, 0));

}

int init_pages()
{
	int status=0;

	vsgSetDrawPage(vsgVIDEOPAGE, BACKGROUND_PAGE, vsgBACKGROUND);
	vsgPresent();

	// draw STIMULUS_PAGE, fp and stim here
	vsgSetDrawPage(vsgVIDEOPAGE, STIMULUS_PAGE, vsgBACKGROUND);
	m_stim.init(50);
	m_stim.drawOnce();

	// draw fixpt last so it isn't overwritten by stimuli
	m_afp.init(2);
	m_afp.draw();
	m_afp.setContrast(0);

	// Set trigger mode
	vsgObjSetTriggers(vsgTRIG_ONPRESENT+vsgTRIG_TOGGLEMODE,0,0);

	return status;
}



