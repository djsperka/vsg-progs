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
bool f_binaryTriggers = true;
bool f_verbose = false;
COLOR_TYPE f_background = { gray, {0, 0, 0}};
int f_screenDistanceMM = 0;
ARContrastFixationPointSpec f_fp;
vector<ARContrastFixationPointSpec*> f_targets;
vector<int> f_vecTargetOrder;
vector<int>::const_iterator f_iterator;
TriggerVector f_triggers;

// parse args
int args(int argc, char **argv);

// errors? dump usage and exit. 
void usage();

void init_pages();
void init_triggers();
int callback(int &output, const CallbackTrigger* ptrig);



int main(int argc, char **argv)
{
	// parse command line args
	if (args(argc, argv)) return 1;

	// Init vsg
	if (ARvsg::instance().init(f_screenDistanceMM, f_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}

	init_pages();

	init_triggers();

	// Issue "ready" triggers to spike2.
	// These commands pulse spike2 port 6. 

	ARvsg::instance().ready_pulse();

	f_triggers.reset(vsgIOReadDigitalIn());

	// Initialize the target order iterator
	f_iterator = f_vecTargetOrder.begin();

	// All right, start monitoring triggers........
	string s;
	int last_output_trigger=0;
	while (1)
	{
		// If user-triggered, get a trigger entry. 
		if (!f_binaryTriggers)
		{
			// Get a new "trigger" from user
			cout << "Enter trigger/key: ";
			cin >> s;
		}

		TriggerFunc	tf = std::for_each(f_triggers.begin(), f_triggers.end(), 
			(f_binaryTriggers ? TriggerFunc(vsgIOReadDigitalIn(), last_output_trigger) : TriggerFunc(s, last_output_trigger)));

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
	if (ARvsg::instance().init(f_screenDistanceMM, f_background))
	{
		cerr << "VSG video initialization failed!" << endl;
	}

	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);

	f_fp.init(2);
	f_fp.setContrast(0);
	f_fp.draw();

	for (unsigned int i=0; i<f_targets.size(); i++)
	{
		if (f_verbose) 
		{
			cout << "Target " << i << " : " << *f_targets[i] << endl;
		}
		f_targets[i]->init(2);
		f_targets[i]->setContrast(0);
		f_targets[i]->draw();
	}
	
	vsgPresent();
}


void init_triggers()
{
	// triggers for stim will be CallbackTriggers, all using the same callback function
	f_triggers.addTrigger(new CallbackTrigger("F", 0x2, 0x2, 0x2, 0x2, callback));
	f_triggers.addTrigger(new CallbackTrigger("f", 0x2, 0x0, 0x2, 0x0, callback));
	f_triggers.addTrigger(new CallbackTrigger("S", 0x4, 0x4, 0x4, 0x4, callback));
	f_triggers.addTrigger(new CallbackTrigger("s", 0x4, 0x0, 0x4, 0x0, callback));
	f_triggers.addTrigger(new CallbackTrigger("a", 0x8, 0x8 | AR_TRIGGER_TOGGLE, 0x8, 0x8 | AR_TRIGGER_TOGGLE, callback));
	f_triggers.addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));
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
		f_iterator++;
		if (f_iterator == f_vecTargetOrder.end()) f_iterator = f_vecTargetOrder.begin();
	}
	else if (key == "s")
	{
		f_targets[*f_iterator]->setContrast(0);
		for (vector<int>::iterator iter = f_vecTargetOrder.begin(); iter < f_vecTargetOrder.end(); iter++)
		{
			f_targets[*iter]->setContrast(0);
		}
	}
	else if (key == "S")
	{
		f_targets[*f_iterator]->setContrast(100);
	}
	else if (key == "F")
	{
		f_fp.setContrast(100);
	}
	else if (key == "f")
	{
		f_fp.setContrast(0);
	}

	return ival;
}

int args(int argc, char **argv)
{	
	bool have_f=false;		// have fixation spec
	bool have_d=false;		// have screen dist
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	ARContrastFixationPointSpec *pspec;

	while ((c = getopt(argc, argv, "avf:t:b:d:ho:")) != -1)
	{
		switch (c) 
		{
		case 'a':
			f_binaryTriggers = false;
			break;
		case 'v':
			f_verbose = true;
			break;
		case 'f': 
			s.assign(optarg);
			if (parse_fixation_point(s, f_fp)) errflg++;
			else have_f = true;
			break;
		case 't': 
			s.assign(optarg);
			pspec = new ARContrastFixationPointSpec();
			s.assign(optarg);
			if (!parse_fixation_point(s, *pspec))
			{
				f_targets.push_back(pspec);
			}
			else 
			{
				errflg++;
				cerr << "Error in target spec (" << s << ")" << endl;
				delete(pspec);
			}
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
		case 'o':
			s.assign(optarg);
			if (parse_int_list(s, f_vecTargetOrder))
			{
				cerr << "Error in target order list. Expecting comma-separated list of integers." << endl;
				errflg++;
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

	// Make sure there were some targets
	if (f_targets.size()==0)
	{
		cerr << "No targets specified!" << endl;
		errflg++;
	}

	// If no random order specified, populate the target order vector with 1,2,...,n
	if (!errflg && f_vecTargetOrder.size()==0)
	{
		for (unsigned int i=0; i < f_targets.size(); i++) f_vecTargetOrder.push_back(i);
	}



	if (errflg) 
	{
		usage();
	}
	return errflg;
}


void usage()
{
	cerr << "usage: starstim -f x,y,d[,color] -d screen_distance_MM -b g|b|w -t x,y,d,[color] [-t x,y,d,[color] ...] [-o n0,n1,n2,...]" << endl;
}

