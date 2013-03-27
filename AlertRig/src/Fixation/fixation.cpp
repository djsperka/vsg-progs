#include "fixation.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__

#include "vsgv8.h"
#include "Alertlib.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment (lib, "vsgv8.lib")

using namespace std;
using namespace alert;

int args(int argc, char **argv);
void init_triggers();
int callback(int &output, const CallbackTrigger* ptrig);

ARContrastFixationPointSpec f_afp;
COLOR_TYPE f_background;
vector<ARGratingSpec*> f_distractors;
int f_screenDistanceMM=0;
bool f_verbose=false;
TriggerVector f_triggers;
bool f_binaryTriggers = true;
bool f_bCalibration = false;
double f_dCalibrationOffset = 0.0;
int f_pulse = 0x2;
int f_iCount = 0;
double f_dFixationX = 0;
double f_dFixationY = 0;

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
		if (f_verbose)
		{
			cout << "Screen distance " << f_screenDistanceMM << endl;
			cout << "Fixation point " << f_afp << endl;
			cout << "Background color " << f_background << endl;
			for (unsigned int i=0; i<f_distractors.size(); i++)
			{
				cout << "Distractor " << (i+1) << ": " << f_distractors[i] << endl;
			}
		}
	}


	// INit vsg
	if (ARvsg::instance().init(f_screenDistanceMM, f_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}

#if 0
	// initialize video pages
	if (ARvsg::instance().init_video())
	{
		cerr << "VSG video initialization failed!" << endl;
		return 1;
	}
#endif

	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);

	// write video pages
	init_pages();

	// init triggers
	init_triggers();

	// Issue "ready" triggers to spike2.
	ARvsg::instance().ready_pulse(500, f_pulse);

	// Start monitoring triggers........
	std::string s;
	int last_output_trigger=0;
	f_triggers.reset(vsgIOReadDigitalIn());
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
			if (IS_VISAGE)
			{
				vsgSetTriggerOptions(vsgTRIGOPT_PRESENT, 0, vsgTRIG_OUTPUTMARKER, 0.5, 0, tf.output_trigger() << 1, 0x1FE);
			}
			else
			{
				vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
			}
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
	string s;
	int c;
	ARGratingSpec *pspec=NULL;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "f:b:g:hd:vaC:")) != -1)
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
			if (parse_fixation_point(s, f_afp)) errflg++;
			else 
			{
				have_f = true;
				f_dFixationX = f_afp.x;
				f_dFixationY = f_afp.y;
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
		case 'g':
			pspec = new ARGratingSpec();
			s.assign(optarg);
			if (!parse_grating(s, *pspec))
			{
				f_distractors.push_back(pspec);
			}
			else errflg++;
			break;
		case 'h':
			errflg++;
			break;
		case 'C':
			s.assign(optarg);
			if (parse_double(s, f_dCalibrationOffset))
			{
				cerr << "Cannot parse calibration offset!" << endl;
				errflg++;
			}
			else
			{
				f_bCalibration = true;
			}
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
	if (errflg) 
	{
		usage();
	}
	return errflg;
}

void usage()
{
	cerr << "usage: fixation -f x,y,d[,color] -d screen_distance_MM -b g|b|w [-g x,y,w,h,contrast%,sf,tf,orientation,color_vector,s|q,r|e]" << endl;
}


void init_triggers()
{
	std::vector<std::pair<std::string, int> >vecInputs;

	f_triggers.addTrigger(new PageTrigger("F", 0x2, 0x2, 0x2, 0x2, 1));
	//f_triggers.addTrigger(new CallbackTrigger("S", 0x4, 0x4, 0x4, 0x4, callback));
	//f_triggers.addTrigger(new CallbackTrigger("s", 0x4, 0x0, 0x4, 0x0, callback));
	f_triggers.addTrigger(new PageTrigger("X", 0x2, 0x0, 0x2, 0x0, 0));
	//f_triggers.addTrigger(new CallbackTrigger("a", 0x8, 0x8|AR_TRIGGER_TOGGLE, 0x8, 0x8|AR_TRIGGER_TOGGLE, callback));

	// MultiInput, single output trigger


	vecInputs.push_back(std::pair< string, int>("3", 0x1c));
	vecInputs.push_back(std::pair< string, int>("2", 0x18));
	vecInputs.push_back(std::pair< string, int>("1", 0x14));
	vecInputs.push_back(std::pair< string, int>("6", 0x2c));
	vecInputs.push_back(std::pair< string, int>("5", 0x28));
	vecInputs.push_back(std::pair< string, int>("4", 0x24));
	vecInputs.push_back(std::pair< string, int>("9", 0x3c));
	vecInputs.push_back(std::pair< string, int>("8", 0x38));
	vecInputs.push_back(std::pair< string, int>("7", 0x34));
	f_triggers.push_back(new MultiInputSingleOutputCallbackTrigger(vecInputs, 0x3c, 0x8, 0x8|AR_TRIGGER_TOGGLE, callback));

	// quit trigger
	f_triggers.addTrigger(new QuitTrigger("q", 0x40, 0x40, 0xff, 0x0, 0));

#if 0

	f_triggers.addTrigger(new PageTrigger("0", 0x2, 0x0, 0xff, 0x0, 0));
	f_triggers.addTrigger(new PageTrigger("1", 0x2, 0x2, 0xff, 0x2, 1));
	f_triggers.addTrigger(new CallbackTrigger("a", 0x4, 0x4, 0x4, 0x4, callback));

	f_triggers.addTrigger(new QuitTrigger("q", 0x40, 0x40, 0xff, 0x0, 0));

#endif

	// Dump triggers
	std::cout << "Triggers:" << std::endl;
	for (unsigned int i=0; i<f_triggers.size(); i++)
	{
		std::cout << "Trigger " << i << " " << *(f_triggers[i]) << std::endl;
	}


}


// The return value from this trigger callback determines whether a vsgPresent() is issued. 

int callback(int &output, const CallbackTrigger* ptrig)
{
	int ival=1;
	string key = ptrig->getKey();

	if (f_bCalibration)
	{
		int ix = 0, iy = 0;
		if (key == "1")
		{
			ix = 1; 
			iy = 1;
		}
		else if (key == "2")
		{
			ix = 2;
			iy = 1;
		}
		else if (key == "3")
		{
			ix = 3;
			iy = 1;
		}
		else if (key == "4")
		{
			ix = 1; 
			iy = 2;
		}
		else if (key == "5")
		{
			ix = 2;
			iy = 2;
		}
		else if (key == "6")
		{
			ix = 3;
			iy = 2;
		}
		else if (key == "7")
		{
			ix = 1; 
			iy = 3;
		}
		else if (key == "8")
		{
			ix = 2;
			iy = 3;
		}
		else if (key == "9")
		{
			ix = 3;
			iy = 3;
		}
		f_afp.x = f_dCalibrationOffset * (ix-2);
		f_afp.y = f_dCalibrationOffset * (iy-2);
		vsgSetDrawPage(vsgVIDEOPAGE, STIMULUS_PAGE, vsgBACKGROUND);
		f_afp.draw();
		ival = 0;	// do not give vsgPresent()
	}
	return ival;
}

int init_pages()
{
	int status=0;
	vsgSetDrawPage(vsgVIDEOPAGE, STIMULUS_PAGE, vsgBACKGROUND);

	if (f_distractors.size() > 0 && !f_bCalibration)
	{
		// determine the level slice for each
		int islice = 250/f_distractors.size();
		if (islice > 50) islice=50;
		for (unsigned int i=0; i<f_distractors.size(); i++)
		{
			f_distractors[i]->init(islice);
			f_distractors[i]->draw();
		}
	}

	// draw fixpt last so it isn't overwritten by stimuli
	f_afp.init(2);
	f_afp.draw();

	vsgSetDrawPage(vsgVIDEOPAGE, BACKGROUND_PAGE, vsgBACKGROUND);
	vsgPresent();

	// Set trigger mode
	vsgObjSetTriggers(vsgTRIG_ONPRESENT+vsgTRIG_TOGGLEMODE,0,0);


	return status;
}