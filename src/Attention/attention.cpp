#include "attention.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__

#include "VSGEX2.H"
#include "Alertlib.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

int args(int argc, char **argv);

using namespace std;
using namespace alert;

ARContrastFixationPointSpec m_spec_fixpt;
ARContrastFixationPointSpec m_spec_anspt_up, m_spec_anspt_down;
COLOR_TYPE m_background;
ARGratingSpec m_spec_stimulus;
bool m_bstimulus=false;
ARGratingSpec m_spec_confounder;
bool m_bconfounder=false;
vector<ARGratingSpec> m_distractors;
int m_screenDistanceMM=0;
bool m_verbose=false;
TriggerVector triggers;
bool m_binaryTriggers = true;
int m_iContrastDown=0;
int m_iContrastBase=50;
int m_iContrastUp=100;

static void usage();
static int init_pages();
static int init_answer_points();

int main (int argc, char *argv[])
{

	// Check input arguments
	if (args(argc, argv))
	{
		return 1;
	}
	else
	{
		// Set background color in fixation point and answer points
		m_spec_fixpt.background = m_background;
		m_spec_anspt_up.background = m_background;
		m_spec_anspt_down.background = m_background;

		if (m_verbose)
		{
			cout << "Screen distance " << m_screenDistanceMM << endl;
			cout << "Fixation point " << m_spec_fixpt << endl;
			cout << "Background color " << m_background << endl;
			int i;
			for (i=0; i<m_distractors.size(); i++)
			{
				cout << "Distractor " << (i+1) << ": " << m_distractors[i] << endl;
			}
		}
	}


	// prepare vsg
	if (init_vsg(m_screenDistanceMM, m_background))
	{
		cerr << "VSG initialization failed!" << endl;
		return 1;
	}

	// Init answer points
	init_answer_points();

	// write video pages
	init_pages();


	// All right, start monitoring triggers........
	std::string s;
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
			(m_binaryTriggers ? TriggerFunc(vsgIOReadDigitalIn()) : TriggerFunc(s)));

		// Now analyze input trigger
	 	
		if (tf.quit()) break;
		else if (tf.present())
		{	
			vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
			vsgPresent();
		}
	}

	clear_vsg();

	return 0;
}



int args(int argc, char **argv)
{	
	bool have_f=false;
	bool have_d=false;
	bool have_b=false;
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	ARGratingSpec agtemp;
	while ((c = getopt(argc, argv, "f:b:g:c:hd:vas:")) != -1)
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
			if (parse_fixation_point(s, m_spec_fixpt)) errflg++;
			else have_f = true;
			break;
		case 'b': 
			s.assign(optarg);
			if (parse_color(s, m_background)) errflg++; 
			else have_b = true;
			break;
		case 'd':
			s.assign(optarg);
			if (parse_distance(s, m_screenDistanceMM)) errflg++;
			else have_d=true;
			break;
		case 's':
			s.assign(optarg);
			if (!parse_grating(s, m_spec_stimulus))
			{
				m_bstimulus = true;
			}
			else errflg++;
			break;
		case 'c':
			s.assign(optarg);
			if (!parse_grating(s, m_spec_confounder))
			{
				m_bconfounder = true;
			}
			else errflg++;
			break;
		case 'g':
			s.assign(optarg);
			if (!parse_grating(s, agtemp))
			{
				m_distractors.push_back(agtemp);
			}
			else errflg++;
			break;
		case 't':
			s.assign(optarg);
			if (parse_contrast_triplet(s, m_iContrastDown, m_iContrastBase, m_iContrastUp)) errflg++;
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
	if (!have_b)
	{
		cerr << "Background color not specified!" << endl;
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
	cerr << "usage: attention -f x,y,d[,color] -d screen_distance_MM -b g|b|w [-s x,y,w,h,contrast%,sf,tf,orientation,color_vector,s|q,r|e] -c confounder_settings -f distractor_settings" << endl;
}


int init_answer_points()
{
	m_spec_anspt_up = m_spec_fixpt;
	m_spec_anspt_down = m_spec_fixpt;

	m_spec_anspt_up.x = m_spec_fixpt.x;
	m_spec_anspt_up.y = m_spec_fixpt.y + 5;
	m_spec_anspt_up.d = 0.5;
	m_spec_anspt_down.x = m_spec_fixpt.x;
	m_spec_anspt_down.y = m_spec_fixpt.y - 5;
	m_spec_anspt_down.d = 0.5;
	return 0;
}


// There are just two pages. Page 0 is blank background. 
// Page 1 has all the objects (fixation point, answer points, stimulus grating, distractors, confounder). 
// All triggers set (see init_triggers) are ContrastTriggers, meaning their action (see ContrastTrigger::execute)
// is to change the contrast of one (or more) of the objects. For convenience we wrap the objects in an ARObj
// and call ARObj::draw once (supplying pixel levels we have allocated, the background color). 
// 
// Create ARObjs here and set up triggers as well. Remember that the ContrastTrigger does cleanup on the ARObj!


int init_pages()
{
	int status=0;
	int islice;
	int ngratings=0;
	PIXEL_LEVEL lvfirst;
	VSGOBJHANDLE handle, handle2;
	ContrastTrigger *ptrig = NULL;

	// Set up quit trigger
	triggers.addTrigger(new QuitTrigger("q", 0x8, 0x8, 0xff, 0x0, 0));


	// prepare BACKGROUND_PAGE (and display it)
	vsgSetDrawPage(vsgVIDEOPAGE, BACKGROUND_PAGE, vsgBACKGROUND);
	vsgPresent();
	
	// prepare STIMULUS_PAGE
	vsgSetDrawPage(vsgVIDEOPAGE, STIMULUS_PAGE, vsgBACKGROUND);

	// See how many gratings will be drawn, then determine the slice of levels
	// that each may have.
	if (m_bstimulus) ngratings++;
	if (m_bconfounder) ngratings++;
	ngratings += m_distractors.size();
	if (ngratings > 0)
	{
		islice = LevelManager::instance().remaining()/ngratings;
		if (islice > 40) islice = 40;
		else if (islice > 30) islice = 30;
		else if (islice > 20) islice = 20;
		else islice = 10;
	}
	
	// draw stimulus grating
	if (m_bstimulus) 
	{
		if (LevelManager::instance().request_range(islice, lvfirst))
		{
			cout << "Cannot get levels for stimulus!" << endl;
			return 1;
		}
		else
		{
			handle = vsgObjCreate();
			vsgObjSetContrast(0);
			vsgObjSetPixelLevels(lvfirst, islice);
			m_spec_stimulus.draw();
			// trigger.......
			ptrig = new ContrastTrigger("S", 0x4, 0x4, 0x2, 0x2);
			ptrig->push_back( std::pair<VSGOBJHANDLE, int>(handle, 100) );
			triggers.addTrigger(ptrig);

			ptrig = new ContrastTrigger("s", 0x4, 0x0, 0x2, 0x0);
			ptrig->push_back( std::pair<VSGOBJHANDLE, int>(handle, 0) );
			triggers.addTrigger(ptrig);
		}
	}


	// Now fixation point
	if (LevelManager::instance().request_range(2, lvfirst))
	{
		cout << "Cannot get levels for fixation point!" << endl;
		return 1;
	}
	handle = vsgObjCreate();
	vsgObjSetDefaults();
	vsgObjSetContrast(0);
	vsgObjSetPixelLevels(lvfirst, 2);
	m_spec_fixpt.draw();

	// trigger for fixation point
	ptrig = new ContrastTrigger("F", 0x2, 0x2, 0x1, 0x1);
	ptrig->push_back( std::pair<VSGOBJHANDLE, int>(handle, 100) );
	triggers.addTrigger(ptrig);

	ptrig = new ContrastTrigger("f", 0x2, 0x0, 0x1, 0x0);
	ptrig->push_back( std::pair<VSGOBJHANDLE, int>(handle, 0) );
	triggers.addTrigger(ptrig);


	// Now answer point - upper
	if (LevelManager::instance().request_range(2, lvfirst))
	{
		cout << "Cannot get levels for upper answer point!" << endl;
		return 1;
	}
	handle = vsgObjCreate();
	vsgObjSetDefaults();
	vsgObjSetContrast(0);
	vsgObjSetPixelLevels(lvfirst, 2);
	m_spec_anspt_up.draw();

	// Now answer point - lower
	if (LevelManager::instance().request_range(2, lvfirst))
	{
		cout << "Cannot get levels for lower answer point!" << endl;
		return 1;
	}
	handle2 = vsgObjCreate();
	vsgObjSetDefaults();
	vsgObjSetContrast(0);
	vsgObjSetPixelLevels(lvfirst, 2);
	m_spec_anspt_down.draw();


	// trigger for answer points
	ptrig = new ContrastTrigger("A", 0x40, 0x40, 0x4, 0x4);
	ptrig->push_back( std::pair<VSGOBJHANDLE, int>(handle, 100) );
	ptrig->push_back( std::pair<VSGOBJHANDLE, int>(handle2, 100) );
	triggers.addTrigger(ptrig);

	ptrig = new ContrastTrigger("a", 0x40, 0x0, 0x4, 0x0);
	ptrig->push_back( std::pair<VSGOBJHANDLE, int>(handle, 0) );
	ptrig->push_back( std::pair<VSGOBJHANDLE, int>(handle2, 0) );
	triggers.addTrigger(ptrig);




	// Set vsg trigger mode
	vsgObjSetTriggers(vsgTRIG_ONPRESENT+vsgTRIG_TOGGLEMODE,0,0);


	return status;
}