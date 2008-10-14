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


using namespace std;
using namespace alert;


#define MAX_GRATINGS 7
int f_nGratings = 0;
ARGratingSpec f_specGratings[MAX_GRATINGS];
COLOR_TYPE f_colorBackground;
bool f_binaryTriggers = true;
bool f_verbose = false;
int f_screenDistanceMM;
TriggerVector triggers;

int args(int argc, char **argv);
void usage();
int init_page();




int main (int argc, char *argv[])
{
	int status=0;
	int islice=50;
	int i;

	// Check input arguments
	if (args(argc, argv))
	{
		return 1;
	}

	if (f_verbose)
	{
		cout << "Background color   : " << f_colorBackground << endl;
		cout << "Screen distance(mm): " << f_screenDistanceMM << endl;
		cout << "Number of gratings : " << f_nGratings << endl; 
		for (i=0; i<f_nGratings; i++)
		{
			cout << "Grating " << i << ": " << f_specGratings[i] << endl;
		}
	}

	// INit vsg
	if (ARvsg::instance().init(f_screenDistanceMM, f_colorBackground))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}

	// initialize video pages
	if (ARvsg::instance().init_video())
	{
		cerr << "VSG video initialization failed!" << endl;
		return 1;
	}

	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);


	// init and draw gratings
	islice = 240/f_nGratings;

	for (i=0; i<f_nGratings; i++)
	{
		f_specGratings[i].init(islice);
//		f_specGratings[i].drawOnce();
//		f_specGratings[i].draw();		// this gives square apertures
		f_specGratings[i].draw(true);
	}

	// Set vsg trigger mode
	vsgObjSetTriggers(vsgTRIG_DRIFTVEL, 0, 0);
	vsgPresent();

	// quit trigger
	triggers.addTrigger(new QuitTrigger("q", 0x1, 0x1, 0x0, 0x0, 0));


	// All right, start monitoring triggers........
	std::string s;
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

		TriggerFunc	tf = std::for_each(triggers.begin(), triggers.end(), 
			(f_binaryTriggers ? TriggerFunc(vsgIOReadDigitalIn(), last_output_trigger) : TriggerFunc(s, last_output_trigger)));

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
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	bool have_b = false;
	bool have_d = false;

	while ((c = getopt(argc, argv, "d:g:ab:vh")) != -1)
	{
		switch (c) 
		{
		case 'a':
			f_binaryTriggers = false;
			break;
		case 'v':
			f_verbose = true;
			break;
		case 'b': 
			s.assign(optarg);
			if (parse_color(s, f_colorBackground)) errflg++; 
			else have_b = true;
			break;
		case 'd':
			s.assign(optarg);
			if (parse_distance(s, f_screenDistanceMM)) errflg++;
			else have_d=true;
			break;
		case 'g':
			s.assign(optarg);
			if (!parse_grating(s, f_specGratings[f_nGratings]))
			{
				f_nGratings++;
			}
			else 
			{
				cerr << "Bad grating spec: " << s << endl;
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
	cerr << "usage: MultiGrating -d screen_distance_MM -b g|b|w|(r/g/b) -g x,y,w,h,contrast%,sf,tf,orientation,color_vector,s|q,r|e [-g ...]" << endl;
}

