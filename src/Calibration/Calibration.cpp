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
void init_page(int ipage, void *unused);

using namespace std;
using namespace alert;

COLOR_TYPE m_background;
int m_screenDistanceMM=0;
bool m_verbose=false;
TriggerVector triggers;
bool m_binaryTriggers = true;
double m_dCalibrationOffset = 0.0;
ARFixationPointSpec m_afp;
PIXEL_LEVEL m_fixation_level;

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
		}
	}


	// INit vsg
	if (ARvsg::instance().init(m_screenDistanceMM, m_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}

	VSGTRIVAL fixation_color;
	if (get_color(m_afp.color, fixation_color))
	{
		cerr << "Cannot get trival for fixation color " << m_afp.color << endl;
		return 2;
	}
	vsgSetFixationColour(&fixation_color);

	// initialize video pages
	if (ARvsg::instance().init_video_pages(init_page, NULL, NULL))
	{
		cerr << "VSG video initialization failed!" << endl;
		return 1;
	}
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);

//	vsgPaletteWrite((VSGLUTBUFFER*)&fixation_color, m_fixation_level, 1);

	// init triggers
	init_triggers();

	// Issue "ready" triggers to spike2.
	// These commands pulse spike2 port 6. 
	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x20, 0);
	vsgPresent();

	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x00, 0);
	vsgPresent();

	triggers.reset(vsgIOReadDigitalIn());


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


void init_page(int ipage, void *unused)
{
	double x, y;
	bool bDraw=false;
	switch (ipage)
	{
	case 0: break;
	case 1: 
		bDraw = true;
		x = -m_dCalibrationOffset;
		y = m_dCalibrationOffset;
		break;
	case 2:
		bDraw = true;
		x = 0;
		y = m_dCalibrationOffset;
		break;
	case 3:
		bDraw = true;
		x = m_dCalibrationOffset;
		y = m_dCalibrationOffset;
		break;
	case 4: 
		bDraw = true;
		x = -m_dCalibrationOffset;
		y = 0;
		break;
	case 5:
		bDraw = true;
		x = 0;
		y = 0;
		break;
	case 6:
		bDraw = true;
		x = m_dCalibrationOffset;
		y = 0;
		break;
	case 7: 
		bDraw = true;
		x = -m_dCalibrationOffset;
		y = -m_dCalibrationOffset;
		break;
	case 8:
		bDraw = true;
		x = 0;
		y = -m_dCalibrationOffset;
		break;
	case 9:
		bDraw = true;
		x = m_dCalibrationOffset;
		y = -m_dCalibrationOffset;
		break;
	}

	if (bDraw)
	{
		cout << "draw page " << ipage << " " << x << "," << y << " level " << m_fixation_level << endl;
		vsgSetPen1(vsgFIXATION);
		vsgDrawOval(x, y, m_afp.d, m_afp.d);
	}
		
	return;
}



int args(int argc, char **argv)
{	
	bool have_f=false;
	bool have_d=false;
	bool have_offset = false;
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "f:b:hd:vaC:")) != -1)
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
		case 'C':
			s.assign(optarg);
			if (parse_double(s, m_dCalibrationOffset)) errflg++;
			else have_offset = true;
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
	if (!have_offset)
	{
		cerr << "Calibration dots offset not specified!" << endl; 
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
	cerr << "usage: calibration -f x,y,d[,color] -d screen_distance_MM -b g|b|w -C dot_offset_degrees" << endl;
}


void init_triggers()
{
	triggers.addTrigger(new PageTrigger("0", 0x2, 0x0, 0xff, 0x0, 0));
	triggers.addTrigger(new PageTrigger("1", 0x3e, 0x16, 0xff, 0x1, 1));
	triggers.addTrigger(new PageTrigger("2", 0x3e, 0x1a, 0xff, 0x1, 2));
	triggers.addTrigger(new PageTrigger("3", 0x3e, 0x1e, 0xff, 0x1, 3));
	triggers.addTrigger(new PageTrigger("4", 0x3e, 0x26, 0xff, 0x1, 4));
	triggers.addTrigger(new PageTrigger("5", 0x3e, 0x2a, 0xff, 0x1, 5));
	triggers.addTrigger(new PageTrigger("6", 0x3e, 0x2e, 0xff, 0x1, 6));
	triggers.addTrigger(new PageTrigger("7", 0x3e, 0x36, 0xff, 0x1, 7));
	triggers.addTrigger(new PageTrigger("8", 0x3e, 0x3a, 0xff, 0x1, 8));
	triggers.addTrigger(new PageTrigger("9", 0x3e, 0x3e, 0xff, 0x1, 9));
	triggers.addTrigger(new QuitTrigger("q", 0x40, 0x40, 0xff, 0x1, 0));
}

