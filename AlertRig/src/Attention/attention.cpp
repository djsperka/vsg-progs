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
double m_anspt_offset_degrees = 5;
double m_anspt_diameter_degrees = 0.5;
COLOR_TYPE m_background;
ARGratingSpec m_spec_stimulus;
bool m_bstimulus=false;
ARGratingSpec m_spec_distractor;
bool m_bdistractor=false;
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
			cout << "Fixation point" << m_spec_fixpt << endl;
			cout << "Background color " << m_background << endl;
			if (m_bstimulus) cout << "Stimulus : " << m_spec_stimulus << endl;
			if (m_bdistractor) cout << "Distractor : " << m_spec_distractor << endl;
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
//	if (init_overlay())
//	{
//		cerr << "Overlay page init failed!" << endl;
//		return 1;
//	}

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
			cout << "OUT: " << tf.output_trigger() << endl;
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
	while ((c = getopt(argc, argv, "f:b:g:hd:vas:t:A:D:")) != -1)
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
		case 'g':
			s.assign(optarg);
			if (!parse_grating(s, m_spec_distractor))
			{
				m_bdistractor = true;
			}
			else errflg++;
			break;
		case 't':
			s.assign(optarg);
			if (parse_contrast_triplet(s, m_iContrastDown, m_iContrastBase, m_iContrastUp)) errflg++;
			else have_t = true;
			break;
		case 'A':
			s.assign(optarg);
			if (parse_double(s, m_anspt_offset_degrees)) errflg++;
			break;
		case 'D':
			s.assign(optarg);
			if (parse_double(s, m_anspt_diameter_degrees)) errflg++;
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
	cerr << "usage: attention -f x,y,d[,color] -d screen_distance_MM -b g|b|w [-s x,y,w,h,contrast%,sf,tf,orientation,color_vector,s|q,r|e] -g distractor_settings" << endl;
}


int init_answer_points()
{
	m_spec_anspt_up = m_spec_fixpt;
	m_spec_anspt_down = m_spec_fixpt;

	m_spec_anspt_up.x = m_spec_fixpt.x;
	m_spec_anspt_up.y = m_spec_fixpt.y + m_anspt_offset_degrees;
	m_spec_anspt_up.d = m_anspt_diameter_degrees;
	m_spec_anspt_down.x = m_spec_fixpt.x;
	m_spec_anspt_down.y = m_spec_fixpt.y -  + m_anspt_offset_degrees;
	m_spec_anspt_down.d = m_anspt_diameter_degrees;

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
	if (m_bdistractor) m_spec_distractor.drawOverlay();
	m_spec_fixpt.drawOverlay();
	m_spec_anspt_up.drawOverlay();
	m_spec_anspt_down.drawOverlay();
	return 0;
}




// The return value from this trigger callback determines whether a vsgPresent() is issued. 

int callback(int &output, const CallbackTrigger* ptrig)
{
	int ival=1;
	VSGTRIVAL from, to;
	string key = ptrig->getKey();
	if (key == "S")
	{
		m_spec_anspt_up.setContrast(100); 
		m_spec_anspt_down.setContrast(100);

		get_colorvector(m_spec_stimulus.cv, from, to);
		m_spec_stimulus.select();
		vsgObjSetColourVector(&from, &to, vsgBIPOLAR);
		vsgObjSetSpatialPhase(0);


		get_colorvector(m_spec_distractor.cv, from, to);
		m_spec_distractor.select();
		vsgObjSetColourVector(&from, &to, vsgBIPOLAR);
		vsgObjSetSpatialPhase(0);

		m_spec_stimulus.setContrast(m_iContrastBase);
		m_spec_distractor.setContrast(m_iContrastBase);
	}
	else if (key == "s")
	{
		get_color(m_background, from);
		m_spec_stimulus.select();
		vsgObjSetColourVector(&from, &from, vsgBIPOLAR);
		m_spec_distractor.select();
		vsgObjSetColourVector(&from, &from, vsgBIPOLAR);

	}
	else if (key == "X")
	{
		m_spec_fixpt.setContrast(0);
		m_spec_anspt_up.setContrast(0); 
		m_spec_anspt_down.setContrast(0);

		get_color(m_background, from);
		m_spec_stimulus.select();
		vsgObjSetColourVector(&from, &from, vsgBIPOLAR);
		m_spec_distractor.select();
		vsgObjSetColourVector(&from, &from, vsgBIPOLAR);
	}
	else if (key == "F")
	{
//		m_spec_fixpt.color.type = red;
//		m_spec_fixpt.draw();
		get_color(m_background, from);
		to.a = 1; to.b = to.c = 0;
		m_spec_fixpt.select();
		vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);
		m_spec_fixpt.setContrast(100);
	}
	else if (key == "f")
	{
		m_spec_fixpt.setContrast(0);
	}
	else if (key == "G")
	{
//		m_spec_fixpt.color.type = green;
//		m_spec_fixpt.draw();
		get_color(m_background, from);
		to.b = 1; to.a = to.c = 0;
		m_spec_fixpt.select();
		vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);
		m_spec_fixpt.setContrast(100);
	}
	else if (key == "g")
	{
		m_spec_fixpt.setContrast(0);
	}

	return ival;
}



// There are just two pages. Page 0 is blank background. 
// Page 1 has all the objects (fixation point, answer points, stimulus grating, distractor). 
// All triggers set (see init_triggers) are ContrastTriggers, meaning their action (see ContrastTrigger::execute)
// is to change the contrast of one (or more) of the objects. For convenience we wrap the objects in an ARObj
// and call ARObj::draw once (supplying pixel levels we have allocated, the background color). 
// 
// Create ARObjs here and set up triggers as well. Remember that the ContrastTrigger does cleanup on the ARObj!


int init_pages()
{
	int status=0;
	int islice=50;
	ContrastTrigger *ptrig = NULL;
	ContrastTrigger *ptrigStimON = NULL;
	ContrastTrigger *ptrigStimOFF = NULL;
	ContrastTrigger *ptrigStimUP = NULL;
	ContrastTrigger *ptrigStimDOWN = NULL;
	ContrastTrigger *ptrigDistractorUP = NULL;
	ContrastTrigger *ptrigDistractorDOWN = NULL;
	VSGTRIVAL bg;


	// initialize video pages
	if (ARvsg::instance().init_video())
	{
		cerr << "VSG video initialization failed!" << endl;
		return 1;
	}

	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);

	
	get_color(m_background, bg);
	m_spec_stimulus.init(islice);
	m_spec_stimulus.draw(true);
	vsgObjSetColourVector(&bg, &bg, vsgBIPOLAR);
//	m_spec_stimulus.setContrast(0);

	m_spec_distractor.init(islice);
	m_spec_distractor.draw(true);
//	m_spec_distractor.setContrast(0);
	vsgObjSetColourVector(&bg, &bg, vsgBIPOLAR);

	// Now fixation point
	m_spec_fixpt.init(2);
	m_spec_fixpt.draw();
	m_spec_fixpt.setContrast(0);

	// Now answer point - upper

	m_spec_anspt_up.init(2);
	m_spec_anspt_up.draw();
	m_spec_anspt_up.setContrast(0);
	m_spec_anspt_down.init(2);
	m_spec_anspt_down.draw();
	m_spec_anspt_down.setContrast(0);


	// triggers for fixation point
	/* DJS - move these triggers to callback
	ptrig = new ContrastTrigger("F", 0x2, 0x2, 0x1, 0x1);
	ptrig->push_back( std::pair<VSGOBJHANDLE, int>(m_spec_fixpt.handle(), 100) );
	triggers.addTrigger(ptrig);

	ptrig = new ContrastTrigger("f", 0x2, 0x0, 0x1, 0x0);
	ptrig->push_back( std::pair<VSGOBJHANDLE, int>(m_spec_fixpt.handle(), 0) );
	triggers.addTrigger(ptrig);
	*/

	triggers.addTrigger(new CallbackTrigger("F", 0x2, 0x2, 0x1, 0x1, callback));
	triggers.addTrigger(new CallbackTrigger("f", 0x2, 0x0, 0x1, 0x1, callback));
	triggers.addTrigger(new CallbackTrigger("G", 0x4, 0x4, 0x1, 0x1, callback));
	triggers.addTrigger(new CallbackTrigger("g", 0x4, 0x0, 0x1, 0x1, callback));

	
	// trigger to turn stim, distractor and answer points ON
	triggers.addTrigger(new CallbackTrigger("S", 0x8, 0x8, 0x2, 0x2, callback));

	// trigger to turn stimand distractor OFF (answer points remain on)
	triggers.addTrigger(new CallbackTrigger("s", 0x8, 0x0, 0x2, 0x2, callback));

	// trigger to turn stim, distractor, answer points and fixation point OFF
	triggers.addTrigger(new CallbackTrigger("X", 0xE, 0x0, 0xb, 0x0, callback));

	// trigger to turn stim contrast UP
	ptrigStimUP = new ContrastTrigger("C", 0xf0, 0x10, 0x8, 0x8);
	ptrigStimUP->push_back( std::pair<VSGOBJHANDLE, int>(m_spec_stimulus.handle(), m_iContrastUp) );
	triggers.addTrigger(ptrigStimUP);

	// trigger to turn stim contrast DOWN
	ptrigStimDOWN = new ContrastTrigger("c", 0xf0, 0x20, 0x8, 0x8);
	ptrigStimDOWN->push_back( std::pair<VSGOBJHANDLE, int>(m_spec_stimulus.handle(), m_iContrastDown) );
	triggers.addTrigger(ptrigStimDOWN);

	// trigger to turn distractor contrast UP
	ptrigDistractorUP = new ContrastTrigger("D", 0xf0, 0x40, 0x8, 0x8);
	ptrigDistractorUP->push_back( std::pair<VSGOBJHANDLE, int>(m_spec_distractor.handle(), m_iContrastUp) );
	triggers.addTrigger(ptrigDistractorUP);

	// trigger to turn distractor contrast DOWN
	ptrigDistractorDOWN = new ContrastTrigger("d", 0xf0, 0x80, 0x8, 0x8);
	ptrigDistractorDOWN->push_back( std::pair<VSGOBJHANDLE, int>(m_spec_distractor.handle(), m_iContrastDown) );
	triggers.addTrigger(ptrigDistractorDOWN);

	// quit trigger
	triggers.addTrigger(new QuitTrigger("q", 0xf0, 0xf0, 0xff, 0x0, 0));

	// Set vsg trigger mode
	vsgObjSetTriggers(vsgTRIG_ONPRESENT+vsgTRIG_TOGGLEMODE,0,0);


	return status;
}

