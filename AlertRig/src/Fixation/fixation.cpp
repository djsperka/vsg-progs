#include "fixation.h"
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
void init_triggers();

using namespace std;
using namespace alert;

ARFixationPointSpec m_afp;
COLOR_TYPE m_background;
vector<ARGratingSpec*> m_distractors;
int m_screenDistanceMM=0;
bool m_verbose=false;
TriggerVector triggers;
bool m_binaryTriggers = true;

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

	// write video pages
	init_pages();

	// init triggers
	init_triggers();

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
	triggers.addTrigger(new PageTrigger("0", 0x6, 0x0, 0xff, 0x0, 0));
	triggers.addTrigger(new PageTrigger("1", 0x6, 0x2, 0xff, 0x1, 1));
	triggers.addTrigger(new PageTrigger("2", 0x6, 0x6, 0xff, 0x3, 2));
	triggers.addTrigger(new QuitTrigger("q", 0x8, 0x8, 0xff, 0x0, 0));
}

int init_pages()
{
	int status=0;
	VSGTRIVAL fixation;
	status = init_vsg(m_screenDistanceMM, m_background);

	// reserve first level for drawing gratings with TRANSONLOWER
	PIXEL_LEVEL level_zero;
	LevelManager::instance().request_single(level_zero);

	// set fixation color
	if (get_color(m_afp.color, fixation))
	{
		cerr << "Cannot get trival for fixation color " << m_afp.color << endl;
		return 1;
	}
	vsgSetFixationColour(&fixation);



	// prepare BACKGROUND_PAGE (and display it)
	vsgSetDrawPage(vsgVIDEOPAGE, BACKGROUND_PAGE, vsgBACKGROUND);
	vsgPresent();
	
	// prepare STIMULUS_PAGE
	vsgSetDrawPage(vsgVIDEOPAGE, STIMULUS_PAGE, vsgBACKGROUND);
	vsgSetPen1(vsgFIXATION);
	m_afp.draw();

	// prepare DISTRACTOR_PAGE. Note that if there are no distractors, this page is 
	// identical to the STIMULUS_PAGE. We have to draw the fixation point last so it
	// isn't overwritten by any distractors. 
	vsgSetDrawPage(vsgVIDEOPAGE, DISTRACTOR_PAGE, vsgBACKGROUND);
	if (m_distractors.size() > 0)
	{
		// determine the level slice for each
		int islice = LevelManager::instance().remaining()/m_distractors.size();
		if (islice > 50) islice=50;
		PIXEL_LEVEL first;
		for (int i=0; i<m_distractors.size(); i++)
		{
			if (LevelManager::instance().request_range(islice, first))
			{
				cout << "Cannot get levels for distractor!" << endl;
				return 1;
			}
			else
			{
				vsgObjCreate();
				vsgObjSetDefaults();
				vsgObjSetPixelLevels(first, islice);
				m_distractors[i]->draw();
			}
		}
	}
	m_afp.draw();

	// Set trigger mode
	vsgObjSetTriggers(vsgTRIG_ONPRESENT+vsgTRIG_TOGGLEMODE,0,0);


	return status;
}