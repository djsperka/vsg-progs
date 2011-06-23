#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include "Alertlib.h"
#include "AlertUtil.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")

using namespace std;
using namespace alert;

const double PI(4.0 * atan2(1.0, 1.0)); 

// globals
bool m_binaryTriggers = true;
bool m_verbose = false;
COLOR_TYPE m_background = { gray, {0, 0, 0}};
int m_screenDistanceMM = 0;
ARContrastFixationPointSpec m_fp;
ARContrastFixationPointSpec m_target;
vector<int> m_vecTargetOrder;
vector<VSGOBJHANDLE> m_vecHandles;
int m_nTargets=0;
int m_iCurrentTarget = 0;
TriggerVector m_triggers;
double m_dOffsetDegrees = 0;

// parse args
int args(int argc, char **argv);

// errors? dump usage and exit. 
void usage();

// Parse input target order file
int parse_input_file(string filename, vector<int>& vec);

void init_pages();
void init_triggers();
int callback(int &output, const CallbackTrigger* ptrig);



int main(int argc, char **argv)
{
	// parse command line args
	if (args(argc, argv)) return 1;

	// Init vsg
	if (ARvsg::instance().init(m_screenDistanceMM, m_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}

	init_pages();

	init_triggers();

	// Issue "ready" triggers to spike2.
	// These commands pulse spike2 port 6. 
	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x20, 0);
	vsgPresent();
	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x00, 0);
	vsgPresent();

	m_triggers.reset(vsgIOReadDigitalIn());


	// All right, start monitoring triggers........
	string s;
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


// Generate target objects and draw them. 
// Assumes m_target has been parsed, m_vecHandles exists, and m_nTargets has been parsed as well. 
// Creates 'm_nTargets' fix points. Create the one for m_target, then rotate them around the center point. 

void generate_targets()
{
	double angle;
	double offset;
	double xorigin, yorigin;

	angle = 2*PI/m_nTargets;
	offset = m_dOffsetDegrees * PI/180;
	xorigin = m_fp.x;
	yorigin = m_fp.y;

#if 0
  	m_target.init(2);
	m_target.setContrast(0);
	m_target.draw();
	m_vecHandles.push_back(m_target.handle());
#endif

	// Now take m_target.x,y and rotate. 
	for (int i=0; i<m_nTargets; i++)
	{
		ARContrastFixationPointSpec t;
		double theta = angle * i + offset;
		t.x = (m_target.x - xorigin) * cos(theta) - (m_target.y - yorigin) * sin(theta) + xorigin;
		t.y = (m_target.x - xorigin) * sin(theta) + (m_target.y - yorigin) * cos(theta) + yorigin;
		t.d = m_target.d;
		t.color = m_target.color;
		t.init(2);
		t.setContrast(0);
		t.draw();
		m_vecHandles.push_back(t.handle());
	}
}

void init_pages()
{

	// initialize video pages
	if (ARvsg::instance().init(m_screenDistanceMM, m_background))
	{
		cerr << "VSG video initialization failed!" << endl;
	}

	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);

	m_fp.init(2);
	m_fp.setContrast(0);
	m_fp.draw();

	generate_targets();
	
	vsgPresent();

}


void init_triggers()
{
	// triggers for stim will be CallbackTriggers, all using the same callback function
	m_triggers.addTrigger(new CallbackTrigger("F", 0x2, 0x2, 0x2, 0x2, callback));
	m_triggers.addTrigger(new CallbackTrigger("f", 0x2, 0x0, 0x2, 0x0, callback));
	m_triggers.addTrigger(new CallbackTrigger("S", 0x4, 0x4, 0x4, 0x4, callback));
	m_triggers.addTrigger(new CallbackTrigger("s", 0x4, 0x0, 0x4, 0x0, callback));
	m_triggers.addTrigger(new CallbackTrigger("a", 0x8, 0x8 | AR_TRIGGER_TOGGLE, 0x8, 0x8 | AR_TRIGGER_TOGGLE, callback));
	m_triggers.addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));
}



// The return value from this trigger callback determines whether a vsgPresent() is issued. 
// Care must be taken when the tuning type is "tt_contrast": do not call setContrast when the 
// stim is OFF because the subsequent vsgPresent (which is necessary for the trigger to be issued)
// will then make the stim visible again. 

int callback(int &output, const CallbackTrigger* ptrig)
{
	int ival=1;
	string key = ptrig->getKey();
	if (key == "a")
	{
		m_iCurrentTarget++;
		if (m_iCurrentTarget == m_vecTargetOrder.size()) m_iCurrentTarget = 0;
	}
	else if (key == "s")
	{
		vsgObjSelect(m_vecHandles[m_vecTargetOrder[m_iCurrentTarget]]);
		vsgObjSetContrast(0);
	}
	else if (key == "S")
	{
		vsgObjSelect(m_vecHandles[m_vecTargetOrder[m_iCurrentTarget]]);
		vsgObjSetContrast(100);
	}
	else if (key == "F")
	{
		m_fp.setContrast(100);
	}
	else if (key == "f")
	{
		m_fp.setContrast(0);
	}

	return ival;
}



// Open the file 'filename' and read ints into the vector. Return 0 if the file is found and read OK, 
// 1 otherwise (writes err msg to stderr). 

int parse_input_file(string filename, vector<int>& vec)
{
	// Now open parfile and write that number into it, then close it. 
	ifstream in;
	int i;
	in.open(filename.c_str(), std::ios::in);
	if (!in)
	{
		cerr << "Cannot open input file " << filename;
		return 1;
	}

	while (in >> i) vec.push_back(i);
	in.close();

	return 0;
}


int args(int argc, char **argv)
{	
	bool have_f=false;		// have fixation spec
	bool have_t=false;		// have target spec
	bool have_d=false;		// have screen dist
	bool have_n=false;		// have # of targets
	bool have_r=false;		// set if randomized order file is provided (not required)
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "avf:t:b:d:hn:r:o:")) != -1)
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
			if (parse_fixation_point(s, m_fp)) errflg++;
			else have_f = true;
			break;
		case 't': 
			s.assign(optarg);
			if (parse_fixation_point(s, m_target)) errflg++;
			else have_t = true;
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
		case 'n':
			s.assign(optarg);
			if (parse_integer(s, m_nTargets)) errflg++;
			else have_n=true;
			break;
		case 'r':
			s.assign(optarg);
			if (parse_input_file(s, m_vecTargetOrder)) errflg++;
			else have_r = true;
			break;
		case 'o':
			s.assign(optarg);
			if (parse_double(s, m_dOffsetDegrees)) errflg++;
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
	if (!have_t) 
	{
		cerr << "Target point not specified!" << endl; 
		errflg++;
	}
	if (!have_d)
	{
		cerr << "Screen distance not specified!" << endl; 
		errflg++;
	}
	if (!have_n)
	{
		cerr << "Number of targets not specified!" << endl; 
		errflg++;
	}

	// If no random order file specified, populate the target order vector with 1,2,...,n
	if (!errflg && !have_r)
	{
		for (int i=0; i < m_nTargets; i++) m_vecTargetOrder.push_back(i);
	}

	if (errflg) 
	{
		usage();
	}
	return errflg;
}


void usage()
{
	cerr << "usage: starstim -f x,y,d[,color] -d screen_distance_MM -b g|b|w -s x,y,w,h,contrast%,sf,tf,orientation,color_vector,s|q,r|e" << endl;
}

