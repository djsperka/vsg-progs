#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
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


// globals
bool m_binaryTriggers = true;
bool m_verbose = false;
COLOR_TYPE m_background = gray;
int m_screenDistanceMM = 0;
ARContrastFixationPointSpec m_fp;
ARContrastFixationPointSpec m_target;
vector<int> m_vecTargetOrder;
int m_iTarget=0;

// parse args
int args(int argc, char **argv);

// errors? dump usage and exit. 
void usage();

// Parse input target order file
int parse_input_file(string filename, vector<int>& vec);



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



void init_pages()
{

	// initialize video pages
	if (ARvsg::instance().init_video())
	{
		cerr << "VSG video initialization failed!" << endl;
	}

	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);

	m_fp.init(2);
	m_fp.setContrast(0);
	m_fp.draw();




	
	vsgPresent();

}


void init_triggers()
{
	// trigger for fixation point ON
	ContrastTrigger *ptrigContrast;
	ptrigContrast = new ContrastTrigger("F", 0x2, 0x2, 0x1, 0x1);
	ptrigContrast->push_back( std::pair<VSGOBJHANDLE, int>(m_fp.handle(), 100) );
	m_triggers.addTrigger(ptrigContrast);

	// trigger for fixation point OFF
	ptrigContrast = new ContrastTrigger("f", 0x2, 0x0, 0x1, 0x0);
	ptrigContrast->push_back( std::pair<VSGOBJHANDLE, int>(m_fp.handle(), 0) );
	m_triggers.addTrigger(ptrigContrast);

	// triggers for stim will be CallbackTriggers, all using the same callback function
	m_triggers.addTrigger(new CallbackTrigger("S", 0x4, 0x4, 0x2, 0x2, callback));
	m_triggers.addTrigger(new CallbackTrigger("s", 0x4, 0x0, 0x2, 0x0, callback));
	m_triggers.addTrigger(new CallbackTrigger("a", 0x8, 0x8 | AR_TRIGGER_TOGGLE, 0x4, 0x4 | AR_TRIGGER_TOGGLE, callback));
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
		if (m_istep_current < m_nsteps)
		{
			m_tuned_param_current = m_tuned_param_vec[++m_istep_current];
//			m_tuned_param_current += (m_tuned_param_max - m_tuned_param_min)/m_nsteps;
//			m_istep_current++;
		}
		else
		{
			m_tuned_param_current = m_tuned_param_vec[0];
			m_istep_current = 0;
		}

		std::cout << "Tuned param current(" << m_istep_current << ") = " << m_tuned_param_current << std::endl;
		switch (m_tuning_type)
		{
		case tt_contrast:
			m_iSavedContrast = (int)m_tuned_param_current;
			// If the stim is currently on, this will make the change to the contrast visible when the 
			// vsgPresent is issued. 
			if (!m_bStimIsOff) m_stim.setContrast((int)m_tuned_param_current);
			break;
		case tt_spatial:
			m_stim.sf = m_tuned_param_current;
			break;
		case tt_temporal:
			m_stim.setTemporalFrequency(m_tuned_param_current);
			break;
		case tt_orientation:
			m_stim.orientation = m_tuned_param_current;
			break;
		case tt_area:
			// If the area covered is smaller than the last time, we have to erase the last grating. 
			if (m_stim.h > m_tuned_param_current)
			{
				std::cout << "Smaller!" << std::endl;
				m_stim.drawBackground();
				m_fp.draw();
			}
			m_stim.h = m_stim.w = m_tuned_param_current;
			break;
		default:
			cerr << "Error in trigger callback: unknown tuning type!" << endl;
		}
		m_stim.redraw(true);

	}
	else if (key == "s")
	{
		// Turn off stimulus by setting contrast to 0.
		if (!m_bStimIsOff)
		{
			m_iSavedContrast = m_stim.contrast;
			m_stim.setContrast(0);
			m_bStimIsOff = true;
			m_stim.redraw(true);
		}
		else
		{
			cout << "Ignore \"s\" trigger: stim is already off." << endl;
		}
	}
	else if (key == "S")
	{
		// Turn on stimulus by setting contrast to m_iSavedContrast.
		if (m_bStimIsOff)
		{
			cout << "Set stim to " << m_iSavedContrast << endl;
			m_stim.setContrast(m_iSavedContrast);
			m_bStimIsOff = false;
			m_stim.redraw(true);
		}
		else
		{
			cout << "Ignore \"S\" trigger: stim is already on." << endl;
		}
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
	bool have_f=false;		// have target spec
	bool have_d=false;		// have screen dist
	bool have_n=false;		// have # of targets
	bool have_r=false;		// set if randomized order file is provided (not required)
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "avf:t:b:d:hn:r:")) != -1)
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
		int i;
		for (int i=1; i<= m_nTargets; i++) m_vecTargetOrder.push_back(i);
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

