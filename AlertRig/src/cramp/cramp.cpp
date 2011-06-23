#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include "Alertlib.h"
#include "Alertutil.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")

using namespace std;
using namespace alert;

bool f_binaryTriggers = true;
bool f_verbose = false;
COLOR_VECTOR_TYPE f_cv = { b_w, {0, 0, 0}, {1, 1, 1}};	// color vector; default is black->white
COLOR_TYPE f_background = { gray, {0.5, 0.5, 0.5}};
PIXEL_LEVEL f_level;
double f_temporalFrequency = 1.0;
ARContrastRectangleSpec f_rect;
TriggerVector f_triggers;


int args(int argc, char **argv);
void usage();
int callback(int &output, const CallbackTrigger* ptrig);
void init_triggers();
void init_pages();

int main(int argc, char **argv)
{
	string s;
	int last_output_trigger=0;
	int input_trigger=0;
	f_rect.color.type = red;
	f_rect.x = f_rect.y = 0;
	f_rect.w = 800;
	f_rect.h = 600;

	if (args(argc, argv))
	{
		return 1;
	}

	if (ARvsg::instance().init(0, f_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}

	init_pages();
	init_triggers();



	// Start monitoring triggers........
	while (1)
	{
		if (!f_binaryTriggers)
		{
			// Get a new "trigger" from user
			cout << "Enter trigger/key: ";
			cin >> s;
		}
		else
		{
			input_trigger = vsgIOReadDigitalIn();
		}

		TriggerFunc	tf = std::for_each(f_triggers.begin(), f_triggers.end(), 
			(f_binaryTriggers ? TriggerFunc(input_trigger, last_output_trigger) : TriggerFunc(s, last_output_trigger)));

		// Now analyze input trigger
	 	
		if (tf.quit())
		{
			break;
		}
		else if (tf.present())
		{	
			last_output_trigger = tf.output_trigger();
//			vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
			vsgPresent();
		}

		// Throttle cpu usage a little. No need to be in hyperspeed checking for triggers here. 
		Sleep(100);
	}


	return 0;
}

void init_pages()
{
	unsigned int i;
	short table[1024];

	// Clear page 1 and display it
	vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);
	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);

	// Clear page 0 and draw
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// these lines give a square. Need to block exit with a read or something so the 
	// ARvsg destructor does not clear the screen!
//	vsgPaletteSet(1, 1, &f_red.color);
//	vsgSetPen1(1);
//	vsgDrawRect(0, 0, 10, 10);
//	vsgSetDisplayPage(0);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	f_rect.init(1);
	f_rect.draw();

	// The draw() method sets a unipolar color vector from the background color to the set color. 
	// Now that its done we have to re-do some stuff. 
	VSGTRIVAL from, to;
	get_colorvector(f_cv, from, to);
	f_rect.select();
	vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);

	for (i=0; i<1024; i++)
	{
		table[i] = -32767 + (short)(((float)i/1023.0f)*65534.0);
	}
	vsgObjTableLoad(vsgTWTABLE, table, 0, 1024);
	vsgObjSetTemporalFrequency(0);
	// Setting temporal phase to 0 will start stimulus at background level (assuming background is 
	// midpoint of cv). 
	vsgObjSetTemporalPhase(0);
	vsgPresent();
	return;
}

void init_triggers()
{
	f_triggers.addTrigger(new CallbackTrigger("s", 0x2, 0x2, 0x1, 0x1, callback));
	f_triggers.addTrigger(new CallbackTrigger("X", 0x2, 0x0, 0x1, 0x0, callback));
	f_triggers.addTrigger(new QuitTrigger("q", 0x80, 0x80, 0xff, 0x0, 0));

		// Dump triggers
	std::cout << "Triggers:" << std::endl;
	for (unsigned int i=0; i<f_triggers.size(); i++)
	{
		std::cout << "Trigger " << i << " " << *(f_triggers[i]) << std::endl;
	}
	
}

int callback(int &output, const CallbackTrigger* ptrig)
{
	int ival=1;
	string key = ptrig->getKey();
	cout << "Callback " << key << endl;
	if (key == "s")
	{
		vsgObjSetTriggers(vsgTRIG_ONPRESENT+vsgTRIG_TEMPFREQ+vsgTRIG_OUTPUTMARKER, output, -180);
		vsgObjSetTemporalFrequency(f_temporalFrequency);
	}
	else if (key == "X")
	{
		f_rect.select();
		vsgObjSetTemporalFrequency(0);
		vsgObjSetTemporalPhase(0);
		vsgObjSetTriggers(vsgTRIG_ONPRESENT+vsgTRIG_OUTPUTMARKER, output, 0);
	}

	return ival;
}

int args(int argc, char **argv)
{	
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	bool have_freq = false;

	while ((c = getopt(argc, argv, "c:f:avh")) != -1)
	{
		switch (c) 
		{
		case 'a':
			f_binaryTriggers = false;
			break;
		case 'c':
			s.assign(optarg);
			if (parse_colorvector(s, f_cv))
			{
				cerr << "Cannot parse color vector (" << s << ")." << endl;
				errflg++;
			}
			break;
		case 'f':
			s.assign(optarg);
			if (parse_double(s, f_temporalFrequency))
			{
				cerr << "Cannot parse temporal frequency (" << s << ")" << endl;
				errflg++;
			}
			else
			{
				have_freq = true;
			}
			break;
		case 'v':
			f_verbose = true;
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

	if (errflg) 
	{
		usage();
	}
	return errflg;
}

void usage()
{
	cerr << "Usage: cramp [-a] -f freq [-c colorvector]" << endl;
}
