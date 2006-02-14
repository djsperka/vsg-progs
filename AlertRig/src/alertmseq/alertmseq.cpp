#include "VSGV8.H"
#include "msequence.h"
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__
#include <iostream>
#include <cstdio>
#include <cmath>
#include "alertlib.h"

using namespace std;
using namespace alert;

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")



VSGCYCLEPAGEENTRY MPositions[32768];

int f_iRows=16;
int f_iCols=16;
bool f_verbose;
int f_iOrder=15;		// order of the m-sequence. Expect 2**order-1 terms in the sequence.
string f_sFilename;	// sequence filename
bool f_isRepeating = false;
int f_iSegmentLength=0;
int f_iFramesPerTerm=1;
int f_iDot = 4;
int f_iZoom = 4;
float f_apX=0;
float f_apY=0;
float f_W, f_H;
float f_w, f_h;
char *f_sequence=NULL;
int _segFirstTerm;		// first term in current segment
int _segLastTerm;		// last term in current segment
int _segNTerms;			// number of terms in current segment
TriggerVector triggers;
bool f_binaryTriggers = true;
ARContrastFixationPointSpec f_fixpt;
int f_iDistanceToScreen = 0;

void segSetFirstSegment();
void segAdvanceSegment();
void segLoadSegment();
void init_triggers();
int callback(int &output, const CallbackTrigger* ptrig);




#define STATE_UNDEFINED		-2
#define STATE_QUIT			-1
#define STATE_STOPPED		0
#define STATE_RUNNING		1
int f_iState = STATE_UNDEFINED;



// draw mseq, assuming that 
int draw_mseq()
{
	int rindex, cindex, term;
	int nterms = pow(2, f_iOrder) - 1;

	vsgSetDrawPage(vsgVIDEOPAGE,0,0);
	vsgSetCommand(vsgPALETTERAMP);
	vsgSetPen1(255);
	vsgSetPen2(0);
	vsgSetCommand(vsgVIDEOCLEAR);

	for (rindex=0; rindex<128*16+15; rindex++)
	{
		for (cindex=0; cindex<31; cindex++)
		{
			term=(16*128*rindex+128*cindex) % nterms;
			if (f_sequence[term]=='1') 
			{
				vsgDrawRect(f_iDot*(cindex+.5), f_iDot/f_iZoom*(rindex+.5), f_iDot, f_iDot/f_iZoom);
			}
		}		
	}
	return 0;
}



int load_mseq(string& filename)
{
	int istatus=0;
	int nterms = pow(2, f_iOrder) -1;

	// Open mseq file
	FILE* fp=fopen(filename.c_str(), "r");
	if (!fp) 
	{
		istatus=1;
		cerr << "Cannot open sequence file " << filename << endl;
	}
	else
	{
		f_sequence = (char *)malloc(nterms+1);
		memset(f_sequence, 0, nterms+1);
		if (!fread(f_sequence, sizeof(char), nterms, fp))
		{
			istatus=2;
			cerr << "Expected " << nterms << " terms in seq. Check mseq file " << filename << endl;
		}
		else if ((int)strlen(f_sequence) != nterms)
		{
			istatus=3;
			cerr << "Expected " << nterms << " terms in seq. Found " << strlen(f_sequence) << ". Check mseq file." << endl;
		}
		fclose(fp);
	}

	return istatus;
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


	vsgSetDrawPage(vsgOVERLAYPAGE, 2, 1);
	vsgDrawOval(f_fixpt.x, f_fixpt.y, f_fixpt.d, f_fixpt.d);

	// Overlay page 1 will have no aperture. It will serve as a blank page before and after stimulus starts. 
	vsgSetDrawPage(vsgOVERLAYPAGE, 1, 1);
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1);
	vsgSetPen1(0);	// that's clear on the overlay page!
	vsgDrawRect(f_W/2+f_apX, f_H/2-f_apY, f_w, f_h);
	vsgSetPen1(2);
	vsgDrawOval(f_fixpt.x, f_fixpt.y, f_fixpt.d, f_fixpt.d);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 1);
}


int main(int argc, char **argv)
{
	int istatus=0;

	// Init vsg card
	istatus = vsgInit("");


	// check args
	if (args(argc, argv))
	{
		return 1;
	}


	// setup vsg
	vsgSetDrawOrigin(0,0);
	vsgSetCommand(vsgPALETTERAMP);
	vsgSetVideoMode(vsgPANSCROLLMODE);
	getZoomFactor(f_iDot, f_iZoom);
	vsgSetCommand(vsgOVERLAYMASKMODE);		// makes overlay pages visible


	// aperture location
	f_W = vsgGetScreenWidthPixels();
	f_H = vsgGetScreenHeightPixels();
	f_w = f_iRows * f_iDot;		// the width of the entire grid, as it should appear on the screen
	f_h = f_iCols * f_iDot;		// the height of the entire grid, as it should appear on the screen


	prepareOverlay();


	// draw the msequence into videomemory
	draw_mseq();

	vsgSetCommand(vsgVIDEODRIFT+vsgOVERLAYDRIFT);			// allows us to move the offset of video memory
	segSetFirstSegment();
	segLoadSegment();
	f_iState = STATE_STOPPED;
	
	// init triggers
	init_triggers();

#if 0
	// Issue "ready" triggers to spike2.
	// These commands pulse spike2 port 6. 
	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x20, 0);
	vsgPresent();

	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x00, 0);
	vsgPresent();
#endif

	triggers.reset(vsgIOReadDigitalIn());



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
			last_output_trigger = tf.output_trigger();
			vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
//			cout << "SetTriggers=" << tf.output_trigger() << endl;
			vsgPresent();
		}
	}

	vsgSetCommand(vsgCYCLEPAGEDISABLE);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 1);

	ARvsg::instance().clear();

	return 0;
}



















#if 0



	int iState = STATE_STOPPED;
	while (iState != STATE_QUIT)
	{
	}
	
	// reset timer and start cycling

	cout << "time us = " << f_pr->getTimeUS() << endl;
	cout << "uspf = " << vsgGetSystemAttribute(vsgFRAMETIME) << endl;
	vsgResetTimer();
	vsgSetCommand(vsgVIDEODRIFT+vsgOVERLAYDRIFT);			// allows us to move the offset of video memory
	vsgSetCommand(vsgCYCLEPAGEENABLE);
	while (vsgGetTimer() < f_pr->getTimeUS())
	{
//		cout << "sleeping, timer=" << vsgGetTimer() << " < " << f_pr->getTimeUS() << endl;
		Sleep(1000);
	}
	vsgSetCommand(vsgCYCLEPAGEDISABLE);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 1);

	Sleep(5000);	// per Daniel request.

	return 0;
}

#endif



void segSetFirstSegment()
{
	_segFirstTerm = 1;
	_segLastTerm = f_iSegmentLength;
}

// TODO: this should take into account the number of repeats!
void segAdvanceSegment()
{
	int nterms = pow(2, f_iOrder) - 1;
	_segFirstTerm = _segLastTerm + 1;
	_segLastTerm += f_iSegmentLength;
	if (_segLastTerm > nterms)
	{
		_segLastTerm = nterms;
	}
	_segNTerms = _segLastTerm - _segFirstTerm + 1;
}


void segLoadSegment()
{

	// Page cycling setup. 
	int index=0;
	for (int iterm = _segFirstTerm; iterm <= _segLastTerm; iterm++)
	{
		// WARNING : rows and columns are hardcoded here. 
		int irow = (index%128) * f_iRows + floor(index/(128*16));
		int icol = (int)(floor(index/128))%f_iCols;

		MPositions[index].Page = 0+vsgDUALPAGE+vsgTRIGGERPAGE;
		MPositions[index].Xpos=-f_W/2 + f_w/2 - f_apX + icol*f_iDot;
		MPositions[index].Ypos=-f_H/(2*f_iZoom) + f_h/(2*f_iZoom) + f_apY/f_iZoom + irow*f_iDot/f_iZoom;

		if (index==0)
		{
			cout << "xy=" << MPositions[index].Xpos << "," << MPositions[index].Ypos << endl;
		}
		MPositions[index].Frames=f_iFramesPerTerm;
		MPositions[index].Stop=0;
		MPositions[index].ovPage=0;
		MPositions[index].ovXpos=0;
		MPositions[index].ovYpos=0;
		index++;
	}

	
	MPositions[index].Stop=1;
	MPositions[index].Page = 0+vsgDUALPAGE;
	MPositions[index].ovPage=1;
	MPositions[index].ovXpos=0;
	MPositions[index].ovYpos=0;
	index++;

	
	// index is now the totalnumber of positions
	vsgPageCyclingSetup(index, &MPositions[0]);

}


bool segStart()
{
	bool bvalue = true;
	if (f_iState == STATE_STOPPED)
	{
		vsgSetCommand(vsgVIDEODRIFT+vsgOVERLAYDRIFT);			// allows us to move the offset of video memory
		vsgSetCommand(vsgCYCLEPAGEENABLE);
		f_iState = STATE_RUNNING;
	}
	else
	{
		cerr << "ERROR: Cannot start segment unless in STOPPED state." <<endl;
		bvalue = false;
	}
	return bvalue;
}



bool segStop()
{
	bool bvalue = true;
	if (f_iState == STATE_RUNNING)
	{
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 1);
		f_iState = STATE_STOPPED;
	}
	else
	{
		cerr << "ERROR: Cannot stop segment unless in RUNNING state." <<endl;
		bvalue = false;
	}
	return bvalue;
}



bool segNext()
{
	bool bvalue = true;
	vsgSetCommand(vsgCYCLEPAGEDISABLE);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 2);
	segAdvanceSegment();
	vsgSetCommand(vsgVIDEODRIFT+vsgOVERLAYDRIFT);			// allows us to move the offset of video memory
	vsgSetCommand(vsgCYCLEPAGEENABLE);
	f_iState = STATE_RUNNING;
	return bvalue;
}

void init_triggers()
{
	triggers.addTrigger(new CallbackTrigger("S", 0x2, 0x2, 0x2, 0x2, callback));
	triggers.addTrigger(new CallbackTrigger("s", 0x2, 0x0, 0x2, 0x0, callback));
	triggers.addTrigger(new CallbackTrigger("a", 0x4, 0x4 | AR_TRIGGER_TOGGLE, 0x4, 0x4 | AR_TRIGGER_TOGGLE, callback));
	triggers.addTrigger(new QuitTrigger("q", 0x8, 0x8, 0xff, 0x0, 0));


		// Dump triggers
	std::cout << "Triggers:" << std::endl;
	int i;
	for (i=0; i<triggers.size(); i++)
	{
		std::cout << "Trigger " << i << " " << *(triggers[i]) << std::endl;
	}
	
}



// The return value from this trigger callback determines whether a vsgPresent() is issued. 
// Care must be taken when the tuning type is "tt_contrast": do not call setContrast when the 
// stim is OFF because the subsequent vsgPresent (which is necessary for the trigger to be issued)
// will then make the stim visible again. 

int callback(int &output, const CallbackTrigger* ptrig)
{
	int ival=1;
	string key = ptrig->getKey();
	if (key == "S")
	{
		if (segStart())
		{
		}
	}
	else if (key == "s")
	{
		if (segStop())
		{
		}
	}
	else if (key == "a")
	{
		segNext();
	}
	return 0;
}


// msequence -f filename -o order -r rows -c columns -R repeats,startframe,#s/#f

int args(int argc, char **argv)
{	
	bool have_f=false;
	// djs - remove orc options. The prog is basically hard-coded to use a 16x16 msequence of
	// order 15. 
	bool have_o=true;
	bool have_r=true;
	bool have_c=true;
	bool have_xy=false;
	bool have_l=false;
	bool have_F=false;

	bool have_t=false;
	bool have_d=false;
	bool have_D=false;
	bool have_fix=false;

	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "f:l:t:vp:d:aF:D:")) != -1)
	{
		switch (c) 
		{
		case 'a':
			f_binaryTriggers = false;
			break;
		case 'p':
			double x,y;
			s.assign(optarg);
			if (parse_xy(s, x, y))
			{
				cerr << "Cannot parse pixel position (" << s << "): expecting integer pixel positions x,y, no spaces." <<endl;
			}
			else
			{
				have_xy = true;
				f_apX = (float)x;
				f_apY = (float)y;
			}
			break;
		case 'f':
			f_sFilename.assign(optarg);
			have_f = true;
			break;
		case 'F':
			s.assign(optarg);
			if (parse_fixation_point(s, f_fixpt)) errflg++;
			else have_fix = true;
			break;

		case 'o':
			s.assign(optarg);
			if (parse_integer(s, f_iOrder))
			{
				cerr << "Cannot parse order (" << s << "): must be an integer." << endl;
				errflg++;
			}
			else 
			{
				have_o = true;
			}
			break;
		case 'd':
			s.assign(optarg);
			if (parse_integer(s, f_iDot))
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
			if (parse_integer(s, f_iRows))
			{
				cerr << "Cannot parse rows (" << s << "): must be an integer." << endl;
				errflg++;
			}
			else
			{
				have_r = true;
			}
			break;
		case 'c':
			s.assign(optarg);
			if (parse_integer(s, f_iCols))
			{
				cerr << "Cannot parse columns (" << s << "): must be an integer." << endl;
				errflg++;
			}
			else
			{
				have_c = true;
			}
			break;
		case 'v':
			f_verbose = true;
			break;
		case 'l':
			s.assign(optarg);
			if (parse_integer(s, f_iSegmentLength))
			{
				cerr << "Cannot parse segment length (" << s << "): must be an integer." << endl;
				errflg++;
			}
			else
			{
				have_l = true;
			}
			break;
		case 'D':
			s.assign(optarg);
			if (parse_integer(s, f_iDistanceToScreen))
			{
				cerr << "Cannot parse screen distance (" << s << "): must be an integer." << endl;
				errflg++;
			}
			else
			{
				have_D = true;
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
		cerr << "No sequence file specified!" << endl; 
		errflg++;
	}
	if (!have_fix) 
	{
		cerr << "No fixation point specified!" << endl; 
		errflg++;
	}
	if (!have_D) 
	{
		cerr << "No screen distance specified!" << endl; 
		errflg++;
	}
	if (!have_o)
	{
		cerr << "Sequence order not specified!" << endl; 
		errflg++;
	}
	if (!have_r || !have_c)
	{
		cerr << "Both rows and columns must be specified!" << endl; 
		errflg++;
	}
	if (!have_l)
	{
		cerr << "No segment length specified!" << endl;
		errflg++;
	}
	if (!have_t)
	{
		cerr << "No frames_per_term value specified!" << endl;
		errflg++;
	}
	if (!have_xy)
	{
		cerr << "No position (-p) specified!" << endl;
		errflg++;
	}
	if (errflg) 
	{
		usage();
	}
	else
	{
		if (load_mseq(f_sFilename))
		{
			errflg++;
			cerr << "Error loading mseq file " << f_sFilename << endl;
		}
	}
	return errflg;
}

void usage()
{
	cerr << "usage: msequence -f filename -o order -r rows -c columns -R repeats,first_frame,last_frame|<seconds>s|<frames>f" << endl;
}

int getZoomFactor(int idot, int& zoom)
{
	int status=0;

#if 0
	if (!(idot%16))
	{
		zoom = 16;
		vsgSetVideoMode(vsgZOOM16);
	}
	else if (!(idot%8))
	{
		zoom = 8;
		vsgSetVideoMode(vsgZOOM8);
	}
	else if (!(idot%4))
	{
#endif
		zoom = 4;
		vsgSetVideoMode(vsgZOOM4);
#if 0
	}
	else if (!(idot%2))
	{
		zoom = 2;
		vsgSetVideoMode(vsgZOOM2);
	}
	else
	{
		zoom=1;
	}
#endif
	return status;
}



