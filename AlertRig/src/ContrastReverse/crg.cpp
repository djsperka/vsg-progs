#include "VSGV8.H"
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__
#include <iostream>
#include <fstream>
#include <string>
#include "alertlib.h"
#include "AlertUtil.h"

using namespace std;
using namespace alert;

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")

#define OVERLAY_BLANK_PAGE 0
#define OVERLAY_DOT_PAGE 1
#define OVERLAY_DOT_APERTURE_PAGE 2

#define TRIGGERLINE_DOT 0x2
#define TRIGGERLINE_STIM 0x4


VSGCYCLEPAGEENTRY MPositions[32768];
const char *f_sequence = NULL;

const int f_iOrder = 15;
bool f_binaryTriggers = true;
TriggerVector triggers;
double f_apX=0, f_apY=0;
double f_sf = 0;
string f_sFilename;
//double f_dSegmentTime = 0;
int f_iSegmentLength = 0;
int f_iDistanceToScreen = 0;
int f_iFramesPerTerm = 0;
int f_iContrastLow = 0;
int f_iContrastHigh = 0;
ARContrastFixationPointSpec f_fixpt;
ARGratingSpec f_grating0;
ARGratingSpec f_grating1;
ARGratingSpec f_grating2;
double f_diameter = 0;
bool f_bUseCRG = false;

void usage();
int args(int argc, char **argv);
void prepareOverlay();
void prepareVideo();
void prepareCycling();
void init_triggers();
bool segStart();
bool segStop();
bool dotOnly();
bool blankPage();
int callback(int &output, const CallbackTrigger* ptrig);



void usage()
{
	cerr << "crg -p x,y -d diameter -m mseq_filename -f fixpt -l segment_length -C contrast_high,contrast_low -D screen_dist_MM -t frames_per_term [-a]" << endl;
}


// msequence -f filename -o order -r rows -c columns -R repeats,startframe,#s/#f

int args(int argc, char **argv)
{	
	bool have_fix = false;
	bool have_t = false;
	bool have_n = false;
	bool have_C = false;
	bool have_d = false;
	bool have_s = false;

	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "af:l:d:t:d:C:n:s:")) != -1)
	{
		switch (c) 
		{
		case 'a':
			f_binaryTriggers = false;
			break;
		case 'f':
			s.assign(optarg);
			if (parse_fixation_point(s, f_fixpt)) errflg++;
			else have_fix = true;
			break;
		case 's':
			s.assign(optarg);
			if (parse_grating(s, f_grating0)) errflg++;
			else 
			{
				have_s = true;
				parse_grating(s, f_grating1);
				parse_grating(s, f_grating2);
			}
			break;
		case 'n':
			s.assign(optarg);
			if (parse_integer(s, f_iSegmentLength)) errflg++;
			else have_n = true;
			break;
		case 'd':
			s.assign(optarg);
			if (parse_integer(s, f_iDistanceToScreen))
			{
				cerr << "Cannot parse screen distance (" << s << "): must be an integer." << endl;
				errflg++;
			}
			else
			{
				have_d = true;
			}
			break;
		case 't':
			s.assign(optarg);
			if (parse_integer(s, f_iFramesPerTerm))
			{
				cerr << "Cannot parse frames per term (" << s << "): must be an integer." << endl;
				errflg++;
			}
			else
			{
				have_t = true;
			}
			break;
		case 'C':
			s.assign(optarg);
			if (parse_int_pair(s, f_iContrastHigh, f_iContrastLow))
			{
				cerr << "Cannot parse contrast pair (" << s << "): expecting e.g. 100,-100" << endl;
				errflg++;
			}
			else
			{
				have_C = true;
			}
			break;
		case 'h':
			errflg++;
			break;
		case '?':
            errflg++;
			break;
		default:
			cerr << "Unhandled command line arg : " << c << endl;
			errflg++;
			break;
		}
	}

	if (!have_fix) 
	{
		cerr << "No fixation point (-f) specified!" << endl; 
		errflg++;
	}
	if (!have_d) 
	{
		cerr << "No screen distance (-d) specified!" << endl; 
		errflg++;
	}
	if (!have_n)
	{
		cerr << "No segment length (-n) specified!" << endl;
		errflg++;
	}
	if (!have_t)
	{
		cerr << "No frames_per_term value (-t) specified!" << endl;
		errflg++;
	}
	if (!have_C)
	{
		cerr << "No contrast pair (-C) specified!" << endl;
		errflg++;
	}
	if (!have_s)
	{
		cerr << "No stimulus (-s) specified!" << endl;
		errflg++;
	}
	if (errflg) 
	{
		usage();
	}
	else
	{
#if 0
		if (arutil_load_mseq(&f_sequence, f_sFilename, f_iOrder))
		{
			errflg++;
			cerr << "Error loading mseq file " << f_sFilename << endl;
		}
#endif
		f_sequence = get_msequence();

		// assign contrast values to grating specs used in crg. f_grating2 is left as-is.
		f_grating0.contrast = f_iContrastLow;
		f_grating0.tf = 0;

		f_grating1.contrast = f_iContrastHigh;
		f_grating1.tf = 0;

	}
	return errflg;
}


void prepareOverlay()
{
	// prepare overlay
	VSGTRIVAL colorTrival;
	VSGLUTBUFFER overlayLUT;
	overlayLUT[1].a=overlayLUT[1].b=overlayLUT[1].c=.5;
	get_color(f_fixpt.color, colorTrival);
	overlayLUT[2] = colorTrival;
	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&overlayLUT, 0, 3);

	vsgSetCommand(vsgOVERLAYMASKMODE);		// makes overlay pages visible

	// blank page is just plain gray
	vsgSetDrawPage(vsgOVERLAYPAGE, OVERLAY_BLANK_PAGE, 1);

	// next page has fixation point
	vsgSetDrawPage(vsgOVERLAYPAGE, OVERLAY_DOT_PAGE, 1);
	vsgSetPen1(2);
	vsgDrawOval(f_fixpt.x, -f_fixpt.y, f_fixpt.d, f_fixpt.d);

	// Now page with aperture and fixation point
	vsgSetDrawPage(vsgOVERLAYPAGE, OVERLAY_DOT_APERTURE_PAGE, 1);
	arutil_draw_aperture(f_grating0, OVERLAY_DOT_APERTURE_PAGE);
	vsgSetPen1(2);
	vsgDrawOval(f_fixpt.x, -f_fixpt.y, f_fixpt.d, f_fixpt.d);

}

void prepareVideo()
{
	// First video page has grating, second has grating with reversed contrast
	arutil_draw_grating(f_grating0, 0);
	vsgPresent();
	arutil_draw_grating(f_grating1, 1);
	vsgPresent();
}

void prepareCycling()
{
	int i;
	int iPage;

	cout << "prepareCycling, l=" << f_iSegmentLength << endl;
	for (i=0; i<f_iSegmentLength; i++)
	{
		iPage = (f_sequence[i] == '1') ? 1 : 0;
		cout << iPage << " ";
		MPositions[i].Page = iPage + vsgDUALPAGE + vsgTRIGGERPAGE;
		MPositions[i].Frames=f_iFramesPerTerm;
		MPositions[i].Stop=0;
		MPositions[i].ovPage=OVERLAY_DOT_APERTURE_PAGE;
		MPositions[i].ovXpos=0;
		MPositions[i].ovYpos=0;
	}
	cout << endl;

	
	MPositions[f_iSegmentLength].Stop=1;
	MPositions[f_iSegmentLength].Page = vsgDUALPAGE + vsgTRIGGERPAGE;
	MPositions[f_iSegmentLength].ovPage=OVERLAY_BLANK_PAGE;
	MPositions[f_iSegmentLength].ovXpos=0;
	MPositions[f_iSegmentLength].ovYpos=0;

	
	// index is now the totalnumber of positions
	vsgPageCyclingSetup(f_iSegmentLength+1, &MPositions[0]);

}


void init_triggers()
{
	triggers.addTrigger(new CallbackTrigger("S", 0x2, 0x2 | AR_TRIGGER_TOGGLE, 0x2, 0x2 | AR_TRIGGER_TOGGLE, callback));
	triggers.addTrigger(new CallbackTrigger("s", 0x4, 0x4 | AR_TRIGGER_TOGGLE, 0x4, 0x4 | AR_TRIGGER_TOGGLE, callback));
	triggers.addTrigger(new CallbackTrigger("D", 0x10, 0x10, 0x10, 0x10 | AR_TRIGGER_TOGGLE, callback));
	triggers.addTrigger(new QuitTrigger("q", 0x80, 0x80, 0xff, 0x0, 0));

	// these are for testing
	triggers.addTrigger(new CallbackTrigger("0", 0x20, 0x20 | AR_TRIGGER_TOGGLE, 0x20, 0x20 | AR_TRIGGER_TOGGLE, callback));
	triggers.addTrigger(new CallbackTrigger("1", 0x40, 0x40 | AR_TRIGGER_TOGGLE, 0x40, 0x40 | AR_TRIGGER_TOGGLE, callback));


		// Dump triggers
	std::cout << "Triggers:" << std::endl;
	for (unsigned int i=0; i<triggers.size(); i++)
	{
		std::cout << "Trigger " << i << " " << *(triggers[i]) << std::endl;
	}
	
}





bool segStart()
{
	bool bvalue = true;

	cout << "Start segment!" << endl;
	vsgSetCommand(vsgVIDEODRIFT + vsgOVERLAYDRIFT);
	vsgSetCommand(vsgCYCLEPAGEENABLE);

	return bvalue;
}



bool segStop()
{
	bool bvalue = true;
	cout << "Stop segment!" << endl;

	vsgSetCommand(vsgCYCLEPAGEDISABLE);

	blankPage();

	// trigger is written in blankPage

	return bvalue;
}

bool dotOnly()
{
	bool bvalue=true;
	vsgSetCommand(vsgCYCLEPAGEDISABLE);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, OVERLAY_DOT_PAGE);

	// write trigger line to indicate dot is on. 
	vsgIOWriteDigitalOut(TRIGGERLINE_DOT, TRIGGERLINE_DOT | TRIGGERLINE_STIM);
	return bvalue;
}

bool blankPage()
{
	bool bvalue=true;
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, OVERLAY_BLANK_PAGE);

	// write trigger to indicate stim and dot have stopped. 
	vsgIOWriteDigitalOut(0, TRIGGERLINE_DOT | TRIGGERLINE_STIM);

	return bvalue;
}

bool showPage(int iPage)
{
	vsgSetZoneDisplayPage(vsgVIDEOPAGE, iPage);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, OVERLAY_DOT_APERTURE_PAGE);
	return true;
}

// The return value from this trigger callback determines whether a vsgPresent() is issued. 
// triggers
//
// D - turn dot on (dot only - no mseq)
// S - Start msequence segment. If dot not present it is shown too. Should use this after a D
// s - turn off mseq segment. Dot removed also (blank page appears). 
//

int callback(int &output, const CallbackTrigger* ptrig)
{
	int ival=1;
	string key = ptrig->getKey();
	if (key == "S")
	{
		segStart();
	}
	else if (key == "s")
	{
		segStop();
	}
	else if (key == "D")
	{
		dotOnly();
	}
	else if (key == "0")
	{
		showPage(0);
	}
	else if (key == "1")
	{
		showPage(1);
	}
	else 
	{
		cerr << "ERROR: Unrecognized trigger!" << endl;
	}
	return 0;
}


#if 0
int writeTermsToParfile()
{
	int status = 0;
	int frame_us = vsgGetSystemAttribute(vsgFRAMETIME);						// microseconds per frame
	double dtmp = (double)frame_us / 1000000.0 * f_iFramesPerTerm;			// this is now seconds per term
	double dtmp2 = f_dSegmentTime/dtmp;
	f_iSegmentLength = (int)dtmp2 + 1;
	// Now open parfile and write that number into it, then close it. 
	ofstream out;
	out.open(f_sParfile.c_str(), std::ios::out);
	if (out.is_open())
	{
		out << f_iSegmentLength;
		out.close();
	}
	else
	{
		status = 1;
	}

	return status;
}
#endif


int main(int argc, char **argv)
{
	int istatus=0;
	COLOR_TYPE background = { gray, { 0.5, 0.5, 0.5 }};

	// check args
	if (args(argc, argv))
	{
		return 1;
	}


	// Init vsg card
	if (ARvsg::instance().init(f_iDistanceToScreen, background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}

	ARvsg::instance().ready_pulse(100);



	vsgSetCommand(vsgDISABLELUTANIM);
	vsgSetCommand(vsgPALETTERAMP);

	f_grating0.init(75);
	f_grating1.init(75);
	f_grating2.init(75);


	// Now draw pages.....
	prepareOverlay();
	prepareVideo();
	prepareCycling();

	// init triggers
	init_triggers();

	triggers.reset(vsgIOReadDigitalIn());

	// All right, start monitoring triggers........
	std::string s;
	int last_output_trigger=0;
	int input_trigger=0;
	while (1)
	{
		// If user-triggered, get a trigger entry. 
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

		TriggerFunc	tf = std::for_each(triggers.begin(), triggers.end(), 
			(f_binaryTriggers ? TriggerFunc(input_trigger, last_output_trigger) : TriggerFunc(s, last_output_trigger)));

		// Now analyze input trigger
	 	
		if (tf.quit()) break;
		else if (tf.present())
		{	
			last_output_trigger = tf.output_trigger();
			vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
			vsgPresent();
		}
	}

	vsgSetCommand(vsgCYCLEPAGEDISABLE);
	blankPage();

//	ARvsg::instance().clear();

	return 0;
}