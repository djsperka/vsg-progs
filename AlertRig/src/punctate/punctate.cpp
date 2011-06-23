#include "vsgv8.h"
#include <cmath>
#include <iostream>
#include <string>
#include "Alertlib.h"
#include "AlertUtil.h"
#pragma comment(lib, "vsgv8.lib")
#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

using namespace std;

bool f_binaryTriggers = true;
bool f_verbose = false;
COLOR_TYPE f_background = { gray, {} };
COLOR_VECTOR_TYPE f_cv = { b_w, {0, 0, 0}, {1, 1, 1}};	// color vector; default is black->white
TriggerVector f_triggers;
int f_scrGridCenterX, f_scrGridCenterY;
int f_iDotSize = 0;
int f_nCols = 16;
int f_nRows = 16;
double f_tOn = 0;
double f_tOff = 0;
double f_temporalFrequency;
bool f_cramp = false;
bool f_onoff = false;
bool f_fullfield = false;
ARMultiContrastRectangleSpec f_rects;
vector< pair<int, int> > f_gridPoints;
vector< pair<int, int> > f_nongridPoints;
vector< int > f_verticalBars;
vector< int > f_horizontalBars;

/* 
 * These vars are used when seq is running. Their values are changed on the fly when 
 * populateLutBuffers is called. 
 */

int f_nextBufferIndex = 0;
int f_nextSeqTerm = 0;
int f_nextLutBufferIndex = 0;


#define MAXLUTBUFFERS 2000
char *f_sMSequence = NULL;
VSGLUTBUFFER f_buffer;

int args(int argc, char **argv);
void usage();
int init_pages();
void init_triggers();
int callback(int &output, const CallbackTrigger* ptrig);
int init_rectangle_coords();

int main(int argc, char *argv[])
{
	// Check input args
	if (args(argc, argv))
	{
		return 1;
	}

	// Init vsg - pixel units. 
	if (ARvsg::instance().init(100, f_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}
	vsgSetSpatialUnits(vsgPIXELUNIT);

	// init rectangle coords. 
	if (init_rectangle_coords())
	{
		cerr << "Cannot initialize rectangles for drawing." << endl;
		return 1;
	}

	if (init_pages())
	{
		cerr << "Cannot draw stim using given parameters." << endl;
		return 1;
	}

	init_triggers();

	// Issue "ready" triggers to spike2.
	// These commands pulse spike2 port 6. 
	vsgIOWriteDigitalOut(0xff, vsgDIG6);
	Sleep(10);
	vsgIOWriteDigitalOut(0, vsgDIG6);

	// All right, start monitoring triggers........
	std::string s;
	int last_output_trigger=0;
	int input_trigger=0;
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



int args(int argc, char **argv)
{	
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	bool have_p = false;
	bool have_P = false;
	bool have_d = false;

	while ((c = getopt(argc, argv, "avp:d:r:c:t:f:P:Z:H:V:F")) != -1)
	{
		switch (c) 
		{
		case 'a':
			f_binaryTriggers = false;
			break;
		case 'v':
			f_verbose = true;
			break;
		case 'F':
			f_fullfield = true;
			break;
		case 't':
			s.assign(optarg);
			if (parse_xy(s, f_tOn, f_tOff) || f_tOn < 0 || f_tOff < 0)
			{
				cerr << "Error in time_on,time_off pair. Must be positive pair, no spaces!" << endl;
				errflg++;
			}
			else
			{
				f_onoff = true;
			}
			break;
		case 'p':
			s.assign(optarg);
			if (parse_int_pair(s, f_scrGridCenterX, f_scrGridCenterY))
			{
				cerr << "Cannot parse grid center position (" << s << "): expecting integer positions, no spaces." <<endl;
			}
			else
			{
				have_p = true;
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
				f_cramp = true;
			}
			break;
		case 'P':
			{
				int i, j;
				s.assign(optarg);
				if (parse_int_pair(s, i, j))
				{
					cerr << "Error in input: grid position values should be an integer pair, no spaces." << endl;
					errflg++;
				}
				else
				{
					have_P = true;
					f_gridPoints.push_back( pair<int, int>(i, j) );
				}
				break;
			}
		case 'Z':
			{
				int i, j;
				s.assign(optarg);
				if (parse_int_pair(s, i, j))
				{
					cerr << "Error in input: non-grid position values should be an integer pair, no spaces." << endl;
					errflg++;
				}
				else
				{
					f_nongridPoints.push_back( pair<int, int>(i, j) );
				}
				break;
			}
		case 'H':
			{
				int i;
				s.assign(optarg);
				if (parse_integer(s, i))
				{
					cerr << "Error in input: horizontal bar position values should be an integer." << endl;
					errflg++;
				}
				else
				{
					f_horizontalBars.push_back( i );
				}
				break;
			}
		case 'V':
			{
				int i;
				s.assign(optarg);
				if (parse_integer(s, i))
				{
					cerr << "Error in input: vertical bar position values should be an integer." << endl;
					errflg++;
				}
				else
				{
					f_verticalBars.push_back( i );
				}
				break;
			}
		case 'd':
			s.assign(optarg);
			if (parse_integer(s, f_iDotSize))
			{
				cerr << "Cannot parse dot size (" << s << "): must be an integer." << endl;
				errflg++;
			}
			else
			{
				have_d = true;
			}
			break;
		case 'r':
			s.assign(optarg);
			if (parse_integer(s, f_nRows))
			{
				cerr << "Cannot parse nrows (" << s << "): must be an integer." << endl;
				errflg++;
			}
			break;
		case 'c':
			s.assign(optarg);
			if (parse_integer(s, f_nCols))
			{
				cerr << "Cannot parse ncols (" << s << "): must be an integer." << endl;
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

	if (!f_cramp && !f_onoff)
	{
		cerr << "No time_on,time_off or temp freq value specified!" << endl;
		errflg++;
	}
	else if (f_cramp && f_onoff)
	{
		cerr << "Cannot specify time_on,time_off AND temp freq value!" << endl;
		errflg++;
	}

	if (!f_fullfield)
	{

		if (have_p && !have_d)
		{
			cerr << "No dot size specified (-d)!" << endl;
			errflg++;
		}
		if (have_P && !have_p)
		{
			cerr << "Cannot specify grid points without specifying grid center position (-p x,y)." << endl;
			errflg++;
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
	cerr << "Usage: punctate [-a] [-v]" << endl;
}

int init_rectangle_coords()
{
	int status=0;
	XYWH rect;
	int gridOriginX, gridOriginY;

	// If full field, ignore grid points, bars
	if (f_fullfield)
	{
		rect.x = rect.y = 0;
		rect.w = vsgGetSystemAttribute(vsgSCREENWIDTH);
		rect.h = vsgGetSystemAttribute(vsgSCREENHEIGHT);
		f_rects.push_back(rect);
	}
	else
	{

		// rectangles from grid
		if (f_gridPoints.size() > 0)
		{
			unsigned int i;
			gridOriginX = vsgGetSystemAttribute(vsgSCREENWIDTH)/2 - f_nCols*f_iDotSize/2;
			gridOriginY = vsgGetSystemAttribute(vsgSCREENHEIGHT)/2 - f_nRows*f_iDotSize/2;
			for (i=0; i<f_gridPoints.size(); i++)
			{
				rect.x = gridOriginX + f_iDotSize * f_gridPoints[i].first;
				rect.y = gridOriginY + f_iDotSize * f_gridPoints[i].second;
				rect.w = rect.h = f_iDotSize;
				f_rects.push_back(rect);
			}
		}

		// free-floating rectangles from points. Must adjust x,y given to use origin in upper left corner. 
		// assume incoming values have x,y origin at center, y positive down
		if (f_nongridPoints.size() > 0)
		{
			unsigned int i;
			for (i=0; i<f_nongridPoints.size(); i++)
			{
				rect.x = vsgGetSystemAttribute(vsgSCREENWIDTH)/2 + f_nongridPoints[i].first;
				rect.y = vsgGetSystemAttribute(vsgSCREENHEIGHT)/2 + f_nongridPoints[i].second;
				rect.w = rect.h = f_iDotSize;
				f_rects.push_back(rect);
			}
		}

		// Vertical bars
		if (f_verticalBars.size() > 0)
		{
			unsigned int i;
			for (i=0; i<f_verticalBars.size(); i++)
			{
				rect.x = vsgGetSystemAttribute(vsgSCREENWIDTH)/2 + f_verticalBars[i];
				rect.y = 0;
				rect.w = f_iDotSize;
				rect.h = vsgGetSystemAttribute(vsgSCREENHEIGHT);
				f_rects.push_back(rect);
			}
		}

		// Horizontal bars
		if (f_horizontalBars.size() > 0)
		{
			unsigned int i;
			for (i=0; i<f_horizontalBars.size(); i++)
			{
				rect.x = 0;
				rect.y = vsgGetSystemAttribute(vsgSCREENHEIGHT)/2 + f_horizontalBars[i];
				rect.h = f_iDotSize;
				rect.w = vsgGetSystemAttribute(vsgSCREENWIDTH);
				f_rects.push_back(rect);
			}
		}
	}
	return status;
}

int init_pages()
{
	unsigned int i;
	short table[1024];
	VSGTRIVAL from, to;

	// There's no vsgBACKGROUND when using pixel units. 


	// all drawing from upper left corner
	vsgSetDrawOrigin(0,0);

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
	f_rects.color.type = red;
	f_rects.init(1);
	f_rects.draw();

	// The draw() method sets a unipolar color vector from the background color to the set color. 
	// Now that its done we have to re-do some stuff. 
	get_colorvector(f_cv, from, to);

	if (f_cramp)
	{
		f_rects.select();
		vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);
		for (i=0; i<1024; i++)
		{
			table[i] = -32767 + (short)(((float)i/1023.0f)*65534.0);
		}
		vsgObjTableLoad(vsgTWTABLE, table, 0, 1024);
	}
	else
	{
		int decayPos = (int)(1024.0 * f_tOn/(f_tOn+f_tOff));
		f_rects.select();
		vsgObjSetColourVector(&to, &from, vsgUNIPOLAR);
		vsgObjTableSquareWave(vsgTWTABLE, 0, decayPos);
	}
	vsgObjSetTemporalFrequency(0);
	// Setting temporal phase to 0 will start stimulus at background level (assuming background is 
	// midpoint of cv). 
	vsgObjSetTemporalPhase(0);
	//vsgPresent();
	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);

	return 0;
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
		//vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		vsgObjSetTriggers(vsgTRIG_ONPRESENT+vsgTRIG_TEMPFREQ+vsgTRIG_OUTPUTMARKER, output, -180);
		if (f_onoff)
			vsgObjSetTemporalFrequency(1/(f_tOn+f_tOff));
		else
			vsgObjSetTemporalFrequency(f_temporalFrequency);
	}
	else if (key == "X")
	{
		f_rects.select();
		vsgObjSetTemporalFrequency(0);
		vsgObjSetTemporalPhase(0);
		vsgObjSetTriggers(vsgTRIG_ONPRESENT+vsgTRIG_OUTPUTMARKER, output, 0);
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		//vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);
	}
	return ival;
}
