#include "attention.h"
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

#pragma comment(lib, "vsgv8.lib")


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
static int init_overlay();

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
			cout << "Fixation point " << m_spec_fixpt << endl;
			cout << "Background color " << m_background << endl;
			if (m_bstimulus) cout << "Stimulus : " << m_spec_stimulus << endl;
			if (m_bconfounder) cout << "Confounder : " << m_spec_confounder << endl;
			int i;
			for (i=0; i<m_distractors.size(); i++)
			{
				cout << "Distractor " << (i+1) << ": " << m_distractors[i] << endl;
			}
		}
	}


	// Init answer points - geometry only
	init_answer_points();


	// INit vsg
	if (ARvsg::instance().init(m_screenDistanceMM, m_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}


	// Init overlay pages
	if (init_overlay())
	{
		cerr << "Overlay page init failed!" << endl;
		return 1;
	}

	// write video pages
	init_pages();

	// Dump triggers
	{
		int i;
		for (i=0; i<triggers.size(); i++)
		{
			std::cout << "Trigger " << i << " " << *(triggers[i]) << std::endl;
		}
	}


	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x20, 0);
	vsgPresent();

	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x00, 0);
	vsgPresent();


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
	bool have_b=false;
	bool have_t=false;
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	ARGratingSpec agtemp;
	while ((c = getopt(argc, argv, "f:b:g:c:hd:vas:t:")) != -1)
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
			else have_t = true;
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
	if (!have_t)
	{
		cerr << "Contrast triplet not specified!" << endl;
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


int init_overlay()
{
	// Initialize and draw overlay page
	if (ARvsg::instance().init_overlay())
	{
		cerr << "VSG overlay initialization failed!" << endl;
		return 1;
	}
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1);

	if (m_bstimulus) m_spec_stimulus.drawOverlay();
	if (m_bconfounder) m_spec_confounder.drawOverlay();
	for (int i=0; i<m_distractors.size(); i++) 
	{
		m_distractors[i].drawOverlay();
	}



	m_spec_fixpt.drawOverlay();
	m_spec_anspt_up.drawOverlay();
	m_spec_anspt_down.drawOverlay();
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
	ContrastTrigger *ptrig = NULL;
	ContrastTrigger *ptrigStimON = NULL;
	ContrastTrigger *ptrigStimOFF = NULL;
	ContrastTrigger *ptrigStimUP = NULL;
	ContrastTrigger *ptrigStimDOWN = NULL;
	ContrastTrigger *ptrigConfounderUP = NULL;
	ContrastTrigger *ptrigConfounderDOWN = NULL;



	// initialize video pages
	if (ARvsg::instance().init_video())
	{
		cerr << "VSG video initialization failed!" << endl;
		return 1;
	}

	// Set up quit trigger
//	triggers.addTrigger(new QuitTrigger("q", 0x8, 0x8, 0xff, 0x0, 0));


	// prepare BACKGROUND_PAGE (and display it)
	vsgSetDrawPage(vsgVIDEOPAGE, BACKGROUND_PAGE, vsgNOCLEAR);
	vsgPresent();
	
	// prepare STIMULUS_PAGE
	vsgSetDrawPage(vsgVIDEOPAGE, STIMULUS_PAGE, vsgNOCLEAR);

	// See how many gratings will be drawn, then determine the slice of levels
	// that each may have.
	if (m_bstimulus) ngratings++;
	if (m_bconfounder) ngratings++;
	ngratings += m_distractors.size();
	if (ngratings > 0)
	{
		cerr << "remaining " << LevelManager::instance().remaining() << " ngratings " << ngratings << endl;
		islice = (LevelManager::instance().remaining() - 10)/ngratings;
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
			m_spec_stimulus.init(islice);
			m_spec_stimulus.draw();
			m_spec_stimulus.setContrast(0);

			// trigger to turn stim ON
			ptrigStimON = new ContrastTrigger("S", 0x4, 0x4, 0x2, 0x2);
			ptrigStimON->push_back( std::pair<VSGOBJHANDLE, int>(m_spec_stimulus.handle(), m_iContrastBase) );
			triggers.addTrigger(ptrigStimON);

			// trigger to turn stim Off
			// stim off trigger also has to turn off the stim change output trigger. 
			ptrigStimOFF = new ContrastTrigger("s", 0x4, 0x0, 0x2+0x18, 0x0);
			ptrigStimOFF->push_back( std::pair<VSGOBJHANDLE, int>(m_spec_stimulus.handle(), 0) );
			triggers.addTrigger(ptrigStimOFF);

			// trigger to turn stim contrast UP
			ptrigStimUP = new ContrastTrigger("C", 0x28, 0x28, 0x18, 0x08);
			ptrigStimUP->push_back( std::pair<VSGOBJHANDLE, int>(m_spec_stimulus.handle(), m_iContrastUp) );
			triggers.addTrigger(ptrigStimUP);
	
			// trigger to turn stim contrast DOWN
			ptrigStimDOWN = new ContrastTrigger("c", 0x28, 0x08, 0x18, 0x08);
			ptrigStimDOWN->push_back( std::pair<VSGOBJHANDLE, int>(m_spec_stimulus.handle(), m_iContrastDown) );
			triggers.addTrigger(ptrigStimDOWN);



			// Draw distractors if present

			if (m_distractors.size() > 0)
			{
				for (int i=0; i<m_distractors.size(); i++) 
				{
					m_distractors[i].init(islice);
					m_distractors[i].draw();
					m_distractors[i].setContrast(0);
					ptrigStimON->push_back( std::pair<VSGOBJHANDLE, int>(m_distractors[i].handle(), m_iContrastBase) );
					ptrigStimOFF->push_back( std::pair<VSGOBJHANDLE, int>(m_distractors[i].handle(), 0) );
				}
			}		
		}
	}

	// draw confounder grating
	if (m_bconfounder) 
	{
		m_spec_confounder.init(islice);
		m_spec_confounder.draw();
		m_spec_confounder.setContrast(0);

		// trigger.......
		ptrigStimON->push_back( std::pair<VSGOBJHANDLE, int>(m_spec_confounder.handle(), m_iContrastBase) );
		ptrigStimOFF->push_back( std::pair<VSGOBJHANDLE, int>(m_spec_confounder.handle(), 0) );

		// trigger to turn confounder contrast UP
		ptrigConfounderUP = new ContrastTrigger("D", 0x30, 0x30, 0x18, 0x10);
		ptrigConfounderUP->push_back( std::pair<VSGOBJHANDLE, int>(m_spec_confounder.handle(), m_iContrastUp) );
		triggers.addTrigger(ptrigConfounderUP);

		// trigger to turn confounder contrast DOWN
		ptrigConfounderDOWN = new ContrastTrigger("d", 0x30, 0x10, 0x18, 0x10);
		ptrigConfounderDOWN->push_back( std::pair<VSGOBJHANDLE, int>(m_spec_confounder.handle(), m_iContrastDown) );
		triggers.addTrigger(ptrigConfounderDOWN);

	}


	// Now fixation point
	m_spec_fixpt.init(2);
	m_spec_fixpt.draw();
	m_spec_fixpt.setContrast(0);

	// trigger for fixation point
	ptrig = new ContrastTrigger("F", 0x2, 0x2, 0x1, 0x1);
	ptrig->push_back( std::pair<VSGOBJHANDLE, int>(m_spec_fixpt.handle(), 100) );
	triggers.addTrigger(ptrig);

	ptrig = new ContrastTrigger("f", 0x2, 0x0, 0x1, 0x0);
	ptrig->push_back( std::pair<VSGOBJHANDLE, int>(m_spec_fixpt.handle(), 0) );
	triggers.addTrigger(ptrig);


	// Now answer point - upper
	m_spec_anspt_up.init(2);
	m_spec_anspt_up.draw();
	m_spec_anspt_up.setContrast(0);
	m_spec_anspt_down.init(2);
	m_spec_anspt_down.draw();
	m_spec_anspt_down.setContrast(0);

	// trigger for answer points
	ptrig = new ContrastTrigger("A", 0x40, 0x40, 0x4, 0x4);
	ptrig->push_back( std::pair<VSGOBJHANDLE, int>(m_spec_anspt_up.handle(), 100) );
	ptrig->push_back( std::pair<VSGOBJHANDLE, int>(m_spec_anspt_down.handle(), 100) );
	triggers.addTrigger(ptrig);

	ptrig = new ContrastTrigger("a", 0x40, 0x0, 0x4, 0x0);
	ptrig->push_back( std::pair<VSGOBJHANDLE, int>(m_spec_anspt_up.handle(), 0) );
	ptrig->push_back( std::pair<VSGOBJHANDLE, int>(m_spec_anspt_down.handle(), 0) );
	triggers.addTrigger(ptrig);


	// Set vsg trigger mode
	vsgObjSetTriggers(vsgTRIG_ONPRESENT+vsgTRIG_TOGGLEMODE,0,0);


	return status;
}