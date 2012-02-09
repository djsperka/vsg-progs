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

int args(int argc, char **argv);
void init_triggers();

using namespace std;
using namespace alert;

ARContrastFixationPointSpec m_afp;
COLOR_TYPE m_background;
vector<ARGratingSpec*> m_distractors;
int m_screenDistanceMM=0;
bool m_verbose=false;
TriggerVector triggers;
bool m_binaryTriggers = true;
bool m_bCalibration = false;
double m_dCalibrationOffset = 0.0;

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
			cout << "Fixation point " << m_afp << endl;
			cout << "Background color " << m_background << endl;
			for (unsigned int i=0; i<m_distractors.size(); i++)
			{
				cout << "Distractor " << (i+1) << ": " << m_distractors[i] << endl;
			}
		}
	}


	// INit vsg
	if (ARvsg::instance().init(m_screenDistanceMM, m_background))
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
	ARvsg::instance().ready_pulse(500);

	// Start monitoring triggers........
	std::string s;
	int last_output_trigger=0;
	triggers.reset(vsgIOReadDigitalIn());
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
			pspec = new ARGratingSpec();
			s.assign(optarg);
			if (!parse_grating(s, *pspec))
			{
				m_distractors.push_back(pspec);
			}
			else errflg++;
			break;
		case 'h':
			errflg++;
			break;
		case 'C':
			s.assign(optarg);
			if (parse_double(s, m_dCalibrationOffset))
			{
				cerr << "Cannot parse calibration offset!" << endl;
				errflg++;
			}
			else
			{
				m_bCalibration = true;
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
	triggers.addTrigger(new PageTrigger("0", 0x2, 0x0, 0xff, 0x0, 0));
	triggers.addTrigger(new PageTrigger("1", 0x2, 0x2, 0xff, 0x2, 1));
	triggers.addTrigger(new QuitTrigger("q", 0x40, 0x40, 0xff, 0x0, 0));


		// Dump triggers
	std::cout << "Triggers:" << std::endl;
	for (unsigned int i=0; i<triggers.size(); i++)
	{
		std::cout << "Trigger " << i << " " << *(triggers[i]) << std::endl;
	}


}

int init_pages()
{
	int status=0;
	vsgSetDrawPage(vsgVIDEOPAGE, STIMULUS_PAGE, vsgBACKGROUND);

	if (m_distractors.size() > 0)
	{
		// determine the level slice for each
		int islice = 250/m_distractors.size();
		if (islice > 50) islice=50;
		for (unsigned int i=0; i<m_distractors.size(); i++)
		{
			m_distractors[i]->init(islice);
			m_distractors[i]->draw();
		}
	}

	// draw fixpt last so it isn't overwritten by stimuli
	m_afp.init(2);
	m_afp.draw();

	vsgSetDrawPage(vsgVIDEOPAGE, BACKGROUND_PAGE, vsgBACKGROUND);
	vsgPresent();

	// Set trigger mode
	vsgObjSetTriggers(vsgTRIG_ONPRESENT+vsgTRIG_TOGGLEMODE,0,0);


	return status;
}