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
bool dotOnly();
bool blankPage();



#define NO_APERTURE_PAGE 1
#define DOT_WITH_APERTURE_PAGE 0


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


	vsgSetDrawPage(vsgOVERLAYPAGE, NO_APERTURE_PAGE, 1);
	vsgSetDrawPage(vsgOVERLAYPAGE, DOT_WITH_APERTURE_PAGE, 1);
	vsgSetPen1(0);	// that's clear on the overlay page!
	vsgDrawRect(f_W/2+f_apX, f_H/2-f_apY, f_w, f_h);
	vsgSetPen1(2);
	vsgDrawOval(f_fixpt.x, f_fixpt.y, f_fixpt.d, f_fixpt.d);
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


	// adjust/convert coordinates on fixation point to pixels, from degrees. 
	vsgSetViewDistMM(f_iDistanceToScreen);
	vsgUnitToUnit(vsgDEGREEUNIT, f_fixpt.x, vsgPIXELUNIT, &f_fixpt.x); 
	vsgUnitToUnit(vsgDEGREEUNIT, f_fixpt.y, vsgPIXELUNIT, &f_fixpt.y); 
	vsgUnitToUnit(vsgDEGREEUNIT, f_fixpt.d, vsgPIXELUNIT, &f_fixpt.d); 


	// Issue "ready" triggers to spike2.
	// These commands pulse spike2 port 6. 
	vsgObjCreate();
	vsgObjSetPixelLevels(200, 2);
	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x20, 0);
	vsgPresent();

	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x00, 0);
	vsgPresent();
	vsgSetCommand(vsgDISABLELUTANIM);


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
	blankPage();


	// draw the msequence into videomemory
	draw_mseq();

	segSetFirstSegment();
	segLoadSegment();
	
	// init triggers
	init_triggers();
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
			vsgPresent();
		}
	}

	vsgSetCommand(vsgCYCLEPAGEDISABLE);
	blankPage();

	ARvsg::instance().clear();

	return 0;
}




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

		MPositions[index].Page = DOT_WITH_APERTURE_PAGE+vsgDUALPAGE+vsgTRIGGERPAGE;
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
	MPositions[index].Page = NO_APERTURE_PAGE+vsgDUALPAGE;
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
	// Set up the no aperture page - it will be shown when the segment ends
	vsgSetDrawPage(vsgOVERLAYPAGE, NO_APERTURE_PAGE, 1);
	vsgSetPen1(2);
	vsgDrawOval(f_fixpt.x, f_fixpt.y, f_fixpt.d, f_fixpt.d);
	// Now start cycling
	vsgSetCommand(vsgVIDEODRIFT+vsgOVERLAYDRIFT);			// allows us to move the offset of video memory
	vsgSetCommand(vsgCYCLEPAGEENABLE);
	return bvalue;
}



bool segStop()
{
	bool bvalue = true;
	vsgSetCommand(vsgCYCLEPAGEDISABLE);
	blankPage();
	return bvalue;
}



bool segNext()
{
	bool bvalue = true;
	vsgSetCommand(vsgCYCLEPAGEDISABLE);
	segAdvanceSegment();
	segStart();
	return bvalue;
}


bool dotOnly()
{
	bool bvalue=true;
	vsgSetCommand(vsgCYCLEPAGEDISABLE);
	vsgSetDrawPage(vsgOVERLAYPAGE, NO_APERTURE_PAGE, 1);
	vsgSetPen1(2);
	vsgDrawOval(f_fixpt.x, f_fixpt.y, f_fixpt.d, f_fixpt.d);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, NO_APERTURE_PAGE);
	return bvalue;
}

bool blankPage()
{
	bool bvalue=true;
	vsgSetCommand(vsgCYCLEPAGEDISABLE);
	vsgSetDrawPage(vsgOVERLAYPAGE, NO_APERTURE_PAGE, 1);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, NO_APERTURE_PAGE);
	return bvalue;
}

void init_triggers()
{
	triggers.addTrigger(new CallbackTrigger("S", 0x2, 0x2 | AR_TRIGGER_TOGGLE, 0x2, 0x2 | AR_TRIGGER_TOGGLE, callback));
	triggers.addTrigger(new CallbackTrigger("s", 0x4, 0x4 | AR_TRIGGER_TOGGLE, 0x4, 0x4 | AR_TRIGGER_TOGGLE, callback));
	triggers.addTrigger(new CallbackTrigger("a", 0x8, 0x8 | AR_TRIGGER_TOGGLE, 0x8, 0x8 | AR_TRIGGER_TOGGLE, callback));
	triggers.addTrigger(new CallbackTrigger("D", 0x10, 0x10 | AR_TRIGGER_TOGGLE, 0x10, 0x10 | AR_TRIGGER_TOGGLE, callback));
	triggers.addTrigger(new CallbackTrigger("d", 0x40, 0x40 | AR_TRIGGER_TOGGLE, 0x40, 0x40 | AR_TRIGGER_TOGGLE, callback));
	triggers.addTrigger(new QuitTrigger("q", 0x80, 0x80, 0xff, 0x0, 0));


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
		segStart();
	}
	else if (key == "s")
	{
		segStop();
	}
	else if (key == "a")
	{
		segNext();
	}
	else if (key == "D")
	{
		dotOnly();
	}
	else if (key == "d")
	{
		blankPage();
	}
	else 
	{
		cerr << "ERROR: Unrecognized trigger!" << endl;
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
	while ((c = getopt(argc, argv, "f:l:t:vp:d:am:D:")) != -1)
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
		case 'm':
			f_sFilename.assign(optarg);
			have_f = true;
			break;
		case 'f':
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



