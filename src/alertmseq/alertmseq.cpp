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
double f_apX=0;
double f_apY=0;
double f_apXCorner=0;
double f_apYCorner=0;
int f_iapXCorner;
int f_iapYCorner;
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

#define TRIGGERLINE_DOT 0x2
#define TRIGGERLINE_STIM 0x4


// draw mseq. See Reid, Victor, Shapley m-sequence paper, eq 14. 
// We assume here that n=16, r=c=16. This makes p=128. 
// Also, eq (14) assumes that terms are indexed starting at term 0 (not 1) 
// and that the first row is row "0", first column is column "0". 
//
// The technique iterates across the video memory area to be used,
// (p*r + (r-1)) rows and (c + (c-1)) columns. Each (cindex, rindex) pair
// corresponds to the msequence term 'term', which is computed using the 
// spatial offset term in eq 14. If a term is '1' in our msequence, we draw
// a white rectangle at that grid location in memory. 
//

int draw_mseq()
{
	int rindex, cindex, term;
	int nterms = pow(2, f_iOrder) - 1;

	vsgSetDrawPage(vsgVIDEOPAGE,0,0);
	vsgSetCommand(vsgPALETTERAMP);
	vsgSetPen1(255);
	vsgSetPen2(0);
	vsgSetCommand(vsgVIDEOCLEAR);

	// djs MODIFIED VERSION HERE. THE PAGE CYCLING ARRAY SETUP IS ALSO MODIFIED. 
	for (rindex=0; rindex<128*16+15; rindex++)
	{
		for (cindex=0; cindex<31; cindex++)
		{
			term=(16*128*rindex+128*cindex) % nterms;
			if (f_sequence[term]=='1') 
			{
				int xrect, yrect;
				xrect = f_iDot*cindex;
				yrect = f_iDot/f_iZoom * rindex;
				vsgDrawRect(xrect, yrect, f_iDot, f_iDot/f_iZoom);
			}
		}		
	}

	// djs original version
//	for (rindex=0; rindex<128*16+15; rindex++)
//	{
//		for (cindex=0; cindex<31; cindex++)
//		{
//			term=(16*128*rindex+128*cindex) % nterms;
//			if (f_sequence[term]=='1') 
//			{
//				vsgDrawRect(f_iDot*(cindex+.5), f_iDot/f_iZoom*(rindex+.5), f_iDot, f_iDot/f_iZoom);
//			}
//		}		
//	}

	
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
//	vsgDrawRect(f_W/2+f_apX, f_H/2-f_apY, f_w, f_h);
	vsgDrawRect(f_iapXCorner, f_iapYCorner, f_w, f_h);
	vsgSetPen1(2);
	vsgDrawOval(f_fixpt.x, f_fixpt.y, f_fixpt.d, f_fixpt.d);
}


int main(int argc, char **argv)
{
	int istatus=0;

	// Init vsg card
	istatus = vsgInit("");
	if (istatus)
	{
		cerr << "Error in vsgInit. Another VSG program may be running!" << endl;
		return 1;
	}



	// check args
	if (args(argc, argv))
	{
		return 1;
	}


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

	// djs  ADDED THIS IN TESTING. DRAWING ORIGINS OF RECT'S IS AT UPPER LEFT HAND CORNER!!!!
	vsgSetDrawMode(vsgSOLIDFILL);			// default is vsgCENTREXY! This makes it top-left-hand-corner origin
	vsgSetCommand(vsgPALETTERAMP);
	vsgSetVideoMode(vsgPANSCROLLMODE);
	getZoomFactor(f_iDot, f_iZoom);
	vsgSetCommand(vsgOVERLAYMASKMODE);		// makes overlay pages visible


	// aperture location
	f_W = vsgGetScreenWidthPixels();
	f_H = vsgGetScreenHeightPixels();
	f_w = f_iRows * f_iDot;		// the width of the entire grid, as it should appear on the screen
	f_h = f_iCols * f_iDot;		// the height of the entire grid, as it should appear on the screen

	// adjust/convert coordinates on fixation point to pixels, from degrees. 
	//
	// TODO: These conversions are incorrect. The incoming coords are in degrees, as measured with the origin in the center
	// of the screen. These conversions merely convert the magnitude in x/y to the magnitude in pixels, but don't perform
	// the coordinate transform! 

	double fx, fy;
	vsgSetViewDistMM(f_iDistanceToScreen);
	cout << "fixpt x,y=" << f_fixpt.x << "," << f_fixpt.y << endl;
	vsgUnitToUnit(vsgDEGREEUNIT, f_fixpt.x, vsgPIXELUNIT, &fx); 
	vsgUnitToUnit(vsgDEGREEUNIT, f_fixpt.y, vsgPIXELUNIT, &fy); 
	f_fixpt.x = f_W/2 + fx;
	f_fixpt.y = f_H/2 - fy;
	cout << "fixpt x,y=" << f_fixpt.x << "," << f_fixpt.y << endl;

	vsgUnitToUnit(vsgDEGREEUNIT, f_fixpt.d, vsgPIXELUNIT, &f_fixpt.d); 

	cout << "aperture x,y (degrees)=" << f_apX << "," << f_apY << endl;	
	vsgUnitToUnit(vsgDEGREEUNIT, f_apX, vsgPIXELUNIT, &fx); 
	vsgUnitToUnit(vsgDEGREEUNIT, f_apY, vsgPIXELUNIT, &fy); 
	f_apX = f_W/2 + fx;
	f_apY = f_H/2 - fy;
	cout << "aperture x,y (pixels) =" << f_apX << "," << f_apY << endl;	

	f_apXCorner = f_apX - f_w/2;
	f_apYCorner = f_apY - f_h/2;

	cout << "aperture x,y (corner, pixels) =" << f_apXCorner << "," << f_apYCorner << endl;	

	// offset number in x must be a multiple of 4
	f_iapXCorner = (int)(f_apXCorner/4 + .5) * (int)4;

	// offset in y must be a multiple of f_iZoom
	f_iapYCorner = (int)(f_apYCorner/f_iZoom + .5) * f_iZoom;

	cout << "aperture x,y (corner, pixels, rounded) =" << f_iapXCorner << "," << f_iapYCorner << endl;	

	// Now draw pages.....
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

	ARvsg::instance().clear();

	return 0;
}




void segSetFirstSegment()
{
	_segFirstTerm = 0;
	_segLastTerm = f_iSegmentLength-1;
}

// TODO: this should take into account the number of repeats!
void segAdvanceSegment()
{
	int nterms = pow(2, f_iOrder) - 1;
	_segFirstTerm = _segLastTerm + 1;
	if (_segFirstTerm == nterms) _segFirstTerm = 0;

	_segLastTerm = _segFirstTerm + f_iSegmentLength-1;
	if (_segLastTerm >= nterms)
	{
		_segLastTerm = nterms-1;
	}
	_segNTerms = _segLastTerm - _segFirstTerm + 1;
}



// djs Correct bug in usage of index (used to index MPositions) and iterm (used to index the sequence). 

void segLoadSegment()
{

	// Page cycling setup. 
	int index=0;
	for (int iterm = _segFirstTerm; iterm <= _segLastTerm; iterm++)
	{
		// WARNING : rows and columns are hardcoded here as f_iRows and f_iCols. 
		// Given the row 'irow' and column 'icolumn', the pixel in the upper left hand corner
		// of the term is at this position in video memory:
		// x = icol * f_iDot
		// y = irow * f_iDot/f_iZoom    (This takes the zoom factor in y into account)
		//
		// The offset of video memory must be such that the upper-left-hand pixel (x,y) is 
		// positioned in the upper left hand corner of the overlay aperture. Thus we have to 
		// compute the distance IN VIDEO MEMORY to offset the origin in order to make this happen. 
		// This is a straighforward calculation, with two important complicating factors:
		// 1) the y zoom factor f_iZoom. 
		// 2) video memory shifts in the x direction should be by integer multiples of 4. 
		// 
		// The second factor isn't well documented in the vsg docs, but its been confirmed by direct testing
		// and communication with the VSG folks at CRS.
		// 
		// The y zoom factor can be accounted for by doing the y-calculation and then dividing by f_iZoom. 
		// The x "4" factor can be accounted for by ensuring that all numbers feeding the calculation are 
		// integer multiples of 4. That's not as hard as it sounds. The only numbers that contribute are the 
		// screen width (actually, half of it), the size of the 16x16 array of dots (er, half of that, too), 
		// and the screen position of the aperture. That's the tricky part. 
		// 
		// In screen coordinates, where the origin is in the upper left hand corner of the screen and x (y) is
		// positive going right (down), the center of the screen is at (f_W/2, f_H/2) where f_W and f_H are
		// the screen width and height in pixels. The aperture center location is (f_apX, f_apY). The aperture
		// is f_iRows*f_iDot pixels wide and f_iCols*f_iDot pixels high (I'm only talking about the aperture as
		// seen on the screen, so I'm leaving out f_iZoom at the moment). 
		// 
		// Putting it all together, the x-coordinate (screen coords) of the left side of the aperture is 
		// 
		// f_W/2 + (f_apX - f_w/2)
		//
		// The y coordinate of the top of the aperture (in screen coords) is
		//
		// f_H/2 - (f_apY + f_h/2)
		//
		// Now some definitions.....
		//
		// f_W and f_H are the width and height of the screen in pixels
		// f_w and f_h are the width and height of the msequence grid in pixels
		// f_apX and f_apY are the coordinates of the aperture position in pixels, in a coordinate system with 
		// the origin at the center of the screen, with y positive UP (this is the way the coords of the stimuli are 
		// stored in the registry for spike2 scripts). 
		//
		// The correct offset for video memory requires some more calculation. The offset itself 
		// is given as two numbers, one for each dimension x/y. If we think of video memory as a
		// two dimensional array of pixels starting at (0,0) and running through POSITIVE values only,
		// the offset defines the pixel which should appear at the upper left hand corner of the 
		// screen. 
		//
		// This is further complicated by the fact that pixels are described by integer values, not
		// fractional values, so we should ensure that the offset is integer. Add in the y-zoom and
		// the x-4 factors and .... 
		//
		// So, if we wanted to define the offset to make our msequence term appear in the upper LH 
		// corner of the screen, we'd use the upper LH corner pixel definition above:
		//
		// x = icol * f_iDot			(f_iDot must be a multiple of 4, so this value is as well)
		// y = irow * f_iDot/f_iZoom    (This takes the zoom factor in y into account)
		// 
		// We want to make this term appear at the location of the aperture, so instead we subtract 
		// off the screen coords of the aperture's upper LH corner (this time accounting for zoom in
		// y)
		// 
		// offset_x = icol*f_iDot - [ f_W/2 + (f_apX - f_w/2) ]
		// offset_y = irow * f_iDot/f_iZoom - [ f_H/2 - (f_apY + f_h/2) ]/f_iZoom
		//
		// To make this offset work we have to make sure that the x coordinate of the aperture's 
		// position is a multiple of 4. The first part of its value, icol*f_iDot is OK because f_iDot
		// must be a multiple of 4. The second part must be rounded to the nearest factor of 4. 
		// This will cause the aperture to be shifted slightly (at most 2 pixels) from the position 
		// specified for the stimulus. Similarly, we have to round the aperture's y coord to a 
		// multiple of f_iZoom to ensure that the offset_y value is an integer. 


		
		int irow = (iterm%128) * f_iRows + floor(iterm/(128*16));
		int icol = (int)(floor(iterm/128))%f_iCols;

		MPositions[index].Page = DOT_WITH_APERTURE_PAGE+vsgDUALPAGE+vsgTRIGGERPAGE;
// djs original		MPositions[index].Xpos=-f_W/2 + f_w/2 - f_apX + icol*f_iDot;
// djs original		MPositions[index].Ypos=-f_H/(2*f_iZoom) + f_h/(2*f_iZoom) + f_apY/f_iZoom + irow*f_iDot/f_iZoom;
		MPositions[index].Xpos = icol*f_iDot - f_iapXCorner;
		MPositions[index].Ypos = irow*f_iDot/f_iZoom - f_iapYCorner/f_iZoom;

		// TEST	
		if ((iterm % 128) == 0)
		{
			cout << "term " << iterm << " " << MPositions[index].Xpos << "," << MPositions[index].Ypos << endl;
		}


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
	MPositions[index].Page = NO_APERTURE_PAGE+vsgDUALPAGE+vsgTRIGGERPAGE;
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

	// write trigger to indicate stim has started. Technically it hasn't started; this trigger should appear in the 
	// data BEFORE the onset of the frame triggers on trigger line 0. 
	vsgIOWriteDigitalOut(TRIGGERLINE_STIM, TRIGGERLINE_STIM);

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

	// trigger is written in blankPage

	return bvalue;
}



bool segNext()
{
	bool bvalue = true;
	vsgSetCommand(vsgCYCLEPAGEDISABLE);
	segAdvanceSegment();
	segLoadSegment();
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

	// write trigger line to indicate dot is on. 
	vsgIOWriteDigitalOut(TRIGGERLINE_DOT, TRIGGERLINE_DOT | TRIGGERLINE_STIM);
	return bvalue;
}

bool blankPage()
{
	bool bvalue=true;
	vsgSetCommand(vsgCYCLEPAGEDISABLE);
	vsgSetDrawPage(vsgOVERLAYPAGE, NO_APERTURE_PAGE, 1);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, NO_APERTURE_PAGE);

	// write trigger to indicate stim and dot have stopped. 
	vsgIOWriteDigitalOut(0, TRIGGERLINE_DOT | TRIGGERLINE_STIM);

	return bvalue;
}

void init_triggers()
{
	triggers.addTrigger(new CallbackTrigger("S", 0x2, 0x2 | AR_TRIGGER_TOGGLE, 0x2, 0x2 | AR_TRIGGER_TOGGLE, callback));
	triggers.addTrigger(new CallbackTrigger("s", 0x4, 0x4 | AR_TRIGGER_TOGGLE, 0x4, 0x4 | AR_TRIGGER_TOGGLE, callback));
	triggers.addTrigger(new CallbackTrigger("a", 0x8, 0x8 | AR_TRIGGER_TOGGLE, 0x8, 0x8 | AR_TRIGGER_TOGGLE, callback));
	triggers.addTrigger(new CallbackTrigger("D", 0x10, 0x10, 0x10, 0x10 | AR_TRIGGER_TOGGLE, callback));
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

// triggers
//
// D - turn dot on (dot only - no mseq)
// d - turn dot off. Blank page appears. If mseq active it is stopped, too. Use this when a "Break fixation" occurs. 
// S - Start msequence segment. If dot not present it is shown too. Should use this after a D
// s - turn off mseq segment. Dot removed also (blank page appears). 
// a - advance to next mseq segment and start it. 
//
// The triggers are designed to be used like this:
// At startup screen is blank. Fixation point turned on with "D", off with "d". When fixation is achieved, the stimulus
// is turned on with "S". If fixation is broken turn the stim (and the dots) off with "s". Don't use "d" when the stim is 
// on! If fixation is not broken before the stim runs its course, advance to the next msequence segment and begin display
// with "a". Again, if fixation is broken turn it off (dot and stim) with "s". 

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
	bool have_m = false;

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
				f_apX = (double)x;
				f_apY = (double)y;
			}
			break;
		case 'm':
			f_sFilename.assign(optarg);
			have_m = true;
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

	if (!have_m) 
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




// Zoom factor is hard coded to be 4! 

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



