#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include "Alertlib.h"
#include "AlertUtil.h"

#pragma comment(lib, "vsgv8.lib")

using namespace std;
using namespace alert;

#define BACKGROUND_PAGE 0
#define FIXATION_PAGE 1
#define STIMULUS_PAGE 2

typedef enum tuning_type { tt_orientation, tt_contrast, tt_spatial, tt_temporal, tt_area, tt_none_specified } tuning_type_t;

tuning_type_t m_tuning_type = tt_none_specified;
//double m_tuned_param_min = 0; 
//double m_tuned_param_max = 0;
vector<double> m_tuned_param_vec;
double m_tuned_param_current = 0;
int m_nsteps = 0;
int m_istep_current = 0;
int m_iSavedContrast = 0;
bool m_bStimIsOff = true;

int m_screenDistanceMM = 0;
COLOR_TYPE m_background = gray;
VSGOBJHANDLE m_handle0;
VSGOBJHANDLE m_handle1;
ARGratingSpec m_stim;
//ARContrastFixationPointSpec m_fp;
ARFixationPointSpec m_fp;

double m_dCurrentOri = 0;
int m_ipage=0;
bool m_binaryTriggers = true;			// if false, will look at stdin for ascii triggers (for testing)
bool m_verbose = false;					// words, words, words,....
TriggerVector m_triggers;

//void init_pages();
void init_pages2();
//void init_triggers();
//void init_triggers2();
//void initfunc(int ipage, void *data);
int args(int argc, char **argv);
void usage();
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

	init_pages2();

//	ARvsg::instance().clear();

	return 0;
}


void init_pages2()
{
	// initialize video pages
	if (ARvsg::instance().init_video())
	{
		cerr << "VSG video initialization failed!" << endl;
	}


	// initialize video pages
	if (ARvsg::instance().init_overlay())
	{
		cerr << "VSG overlay initialization failed!" << endl;
	}


	// Issue "ready" triggers to spike2.
	// These commands pulse spike2 port 6. 
	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x20, 0);
	vsgPresent();

	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x00, 0);
	vsgPresent();


	// allocate some levels for the grating

	PIXEL_LEVEL stimFirstLevel;
	LevelManager::instance().request_range(50, stimFirstLevel);

	// init the stim. This call creates a vsg object. Have to set draw page to a video page, otherwise vsg tells us
	// that there's only pixel levels 0-3 available. 

	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	m_stim.init(stimFirstLevel, 50);

	// Next, draw full screen grating on video page 0

	arutil_draw_grating_fullscreen(m_stim, 0);

	// put color in overlay palette

	if (arutil_color_to_overlay_palette(m_fp, 2))
	{
		cerr << "Cannot put fp color in overlay palette" << endl;
	}

	// prepare overlay page 0 - on second thought that's done already. Its just blank....

	// prepare overlay page 1 - just a fixation point

	arutil_draw_overlay(m_fp, 2, 1);

	// prepare overlay page 2 - a fixation point and aperture

	arutil_draw_aperture(m_stim, 2);
	arutil_draw_overlay(m_fp, 2, 2);


	vsgIOWriteDigitalOut(0x2, 0xffff);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 0);
//	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 0 + vsgTRIGGERPAGE);

	vsgIOWriteDigitalOut(0x4, 0xffff);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 1);
//	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 1 + vsgTRIGGERPAGE);
	
	vsgIOWriteDigitalOut(0x8, 0xffff);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 2);
//	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 2 + vsgTRIGGERPAGE);


	// Now draw overlay page 3, changing the aperture size

	m_stim.w = m_stim.h = 1;
	vsgSetDrawPage(vsgOVERLAYPAGE, 3, 1);
	arutil_draw_aperture(m_stim, 3);
	arutil_draw_overlay(m_fp, 2, 3);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 3 + vsgTRIGGERPAGE);
	

#if 0

	PIXEL_LEVEL fpLevel;
	LevelManager::instance().request_single(fpLevel);
	m_fp.init(fpLevel, 1);		// object created, levels set
//	m_fp.setContrast(100);
	m_fp.draw();

	m_stim.init(50);
	switch(m_tuning_type)
	{
	case tt_contrast:
		// Save the min contrast value for this type
		m_tuned_param_current = m_iSavedContrast = m_tuned_param_vec[0];
		break;
	case tt_spatial:
		m_tuned_param_current = m_stim.sf = m_tuned_param_vec[0];
		break;
	case tt_temporal:
		m_tuned_param_current = m_stim.tf = m_tuned_param_vec[0];
		break;
	case tt_orientation:
		m_tuned_param_current = m_stim.orientation = m_tuned_param_vec[0];
		break;
	case tt_area:
		m_tuned_param_current = m_stim.w = m_stim.h = m_tuned_param_vec[0];
		break;
	default:
		cerr << "Error in init_pages: unknown tuning type!" << endl;
	}
	m_stim.setContrast(0);
	m_stim.drawOnce();
	m_bStimIsOff = true;
	vsgPresent();
#endif
}


#if 0
	
	// Initialize pages
	init_pages2();

	// initialize triggers
	init_triggers2();

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


void init_pages2()
{
	// Save contrast value, since stim will be initially drawn OFF (i.e. contrast set to 0)
	m_iSavedContrast = m_stim.contrast;

	// initialize video pages
	if (ARvsg::instance().init_video())
	{
		cerr << "VSG video initialization failed!" << endl;
	}

	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);

	PIXEL_LEVEL fpLevel;
	LevelManager::instance().request_single(fpLevel);
	m_fp.init(fpLevel, 1);		// object created, levels set
//	m_fp.setContrast(100);
	m_fp.draw();

	m_stim.init(50);
	switch(m_tuning_type)
	{
	case tt_contrast:
		// Save the min contrast value for this type
		m_tuned_param_current = m_iSavedContrast = m_tuned_param_vec[0];
		break;
	case tt_spatial:
		m_tuned_param_current = m_stim.sf = m_tuned_param_vec[0];
		break;
	case tt_temporal:
		m_tuned_param_current = m_stim.tf = m_tuned_param_vec[0];
		break;
	case tt_orientation:
		m_tuned_param_current = m_stim.orientation = m_tuned_param_vec[0];
		break;
	case tt_area:
		m_tuned_param_current = m_stim.w = m_stim.h = m_tuned_param_vec[0];
		break;
	default:
		cerr << "Error in init_pages: unknown tuning type!" << endl;
	}
	m_stim.setContrast(0);
	m_stim.drawOnce();
	m_bStimIsOff = true;
	vsgPresent();

}




void init_pages()
{
	// Save contrast value, since stim will be initially drawn OFF (i.e. contrast set to 0)
	m_iSavedContrast = m_stim.contrast;

	// initialize video pages
	if (ARvsg::instance().init_video())
	{
		cerr << "VSG video initialization failed!" << endl;
	}

	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);

	m_fp.init(2);
	m_fp.setContrast(0);
	m_fp.draw();

	m_stim.init(50);
	switch(m_tuning_type)
	{
	case tt_contrast:
		// Save the min contrast value for this type
		m_tuned_param_current = m_iSavedContrast = m_tuned_param_vec[0];
		break;
	case tt_spatial:
		m_tuned_param_current = m_stim.sf = m_tuned_param_vec[0];
		break;
	case tt_temporal:
		m_tuned_param_current = m_stim.tf = m_tuned_param_vec[0];
		break;
	case tt_orientation:
		m_tuned_param_current = m_stim.orientation = m_tuned_param_vec[0];
		break;
	case tt_area:
		m_tuned_param_current = m_stim.w = m_stim.h = m_tuned_param_vec[0];
		break;
	default:
		cerr << "Error in init_pages: unknown tuning type!" << endl;
	}
	m_stim.setContrast(0);
	m_stim.drawOnce();
	m_bStimIsOff = true;
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

#endif

int args(int argc, char **argv)
{	
	bool have_f=false;		// have fixation spec
	bool have_d=false;		// have screen dist
	bool have_g=false;		// have stim (grating) spec
	bool have_tt = false;	// have tuning type?

	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "avf:b:d:g:hC:T:S:O:A:")) != -1)
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
			s.assign(optarg);
			if (parse_grating(s, m_stim)) errflg++;
			else have_g = true;;
			break;
		case 'h':
			errflg++;
			break;
		case 'C':
			s.assign(optarg);
			if (parse_tuning_list(s, m_tuned_param_vec, m_nsteps)) errflg++;
			else 
			{
				have_tt = true;
				m_tuning_type = tt_contrast;
			}
			break;
		case 'O':
			s.assign(optarg);
			if (parse_tuning_list(s, m_tuned_param_vec, m_nsteps)) errflg++;
			else 
			{
				have_tt = true;
				m_tuning_type = tt_orientation;
			}
			break;
		case 'S':
			s.assign(optarg);
			if (parse_tuning_list(s, m_tuned_param_vec, m_nsteps)) errflg++;
			else 
			{
				have_tt = true;
				m_tuning_type = tt_spatial;
			}
			break;
		case 'T':
			s.assign(optarg);
			if (parse_tuning_list(s, m_tuned_param_vec, m_nsteps)) errflg++;
			else 
			{
				have_tt = true;
				m_tuning_type = tt_temporal;
			}
			break;
		case 'A':
			s.assign(optarg);
			if (parse_tuning_list(s, m_tuned_param_vec, m_nsteps)) errflg++;
			else 
			{
				have_tt = true;
				m_tuning_type = tt_area;
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
	if (!have_g)
	{
		cerr << "Stimulus grating not specified!" << endl; 
		errflg++;
	}
	if (!have_tt)
	{
		cerr << "Tuning variable not specified!" << endl; 
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
	cerr << "usage: tuning -f x,y,d[,color] -d screen_distance_MM -b g|b|w -s x,y,w,h,contrast%,sf,tf,orientation,color_vector,s|q,r|e" << endl;
}
