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
int f_iBarWidth=0;				// vertical bar width, pixels. Computed after input args. 
int f_nRepeats;
int f_iFramesPerTerm;
bool f_verbose = false;
int f_nBars = 16;
int f_iOrder = 15;
bool f_bVertical = true;	// when false, horizontal bars
int f_iCenterCoord = -1;	// Center  pos of bars, in pixels measured from left hand side (vertibar) or top (horizontabar). -1=center
string f_sFilename;
COLOR_TYPE f_background = { gray, {} };
TriggerVector triggers;
PageCyclingTrigger *f_ptrigCycling = NULL;
bool f_outline = false;
bool f_terms = false;	// testing only
int f_p;				// 2**f_iOrder / f_nBars
int f_M;

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
int init_pages(int nCols, int iBarWidth, int iCenterCoord);
void init_triggers();
int callback(int &output, const CallbackTrigger* ptrig);
int populateLutBuffers(int &firstTerm, int nTerms, int &startBufferIndex);
void initLutBuffers();

int main(int argc, char *argv[])
{
	// Check input args
	if (args(argc, argv))
	{
		return 1;
	}

	// INit vsg
	if (ARvsg::instance().init(-1, f_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}

	if (init_pages(f_nBars, f_iBarWidth, f_iCenterCoord))
	{
		cerr << "Cannot draw vertibars using given parameters." << endl;
		return 1;
	}

	initLutBuffers();

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
		// the callback function should return 0 ALWAYS in this program -- no vsgPresent() should get called. 
		if (tf.quit()) 
		{
			// quitting in the middle of page cycling requires special handling
			vsgSetCommand(vsgCYCLELUTDISABLE);
			vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 0);
			break;
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
	bool have_t = false;
	bool have_f = false;
	bool have_o = false;
	bool have_n = false;

	while ((c = getopt(argc, argv, "t:f:avw:n:x:y:Ho:")) != -1)
	{
		switch (c) 
		{
		case 'a':
			f_binaryTriggers = false;
			break;
		case 'f':
			f_sFilename.assign(optarg);
			have_f = true;
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
		case 'x':
			s.assign(optarg);
			if (parse_integer(s, f_iCenterCoord))
			{
				cerr << "Cannot parse center x pos (" << s << "): must be an integer." << endl;
				errflg++;
			}
			break;
		case 'y':
			s.assign(optarg);
			if (parse_integer(s, f_iCenterCoord))
			{
				cerr << "Cannot parse center y pos (" << s << "): must be an integer." << endl;
				errflg++;
			}
			break;
		case 'H':
			f_bVertical = false;
			break;
		case 'w':
			s.assign(optarg);
			if (parse_integer(s, f_iBarWidth))
			{
				cerr << "Cannot parse bar width (" << s << "): must be an integer." << endl;
				errflg++;
			}
			break;
		case 'n':
			s.assign(optarg);
			if (parse_integer(s, f_nBars))
			{
				cerr << "Cannot parse nbars (" << s << "): must be an integer." << endl;
				errflg++;
			}
			else
			{
				have_n = true;
			}
			break;
		case 'v':
			f_verbose = true;
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
		case 'O':
			f_outline = true;
			break;
		case 'K':
			f_terms = true;
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
		cerr << "No stim file specified (-f)." << endl;
		errflg++;
	}
	if (!have_t)
	{
		cerr << "No frames_per_term value specified!" << endl;
		errflg++;
	}
	if (errflg) 
	{
		usage();
	}
	else
	{
		if (arutil_load_mseq(&f_sMSequence, f_sFilename, f_iOrder))
		{
			errflg++;
			cerr << "Error loading mseq file " << f_sFilename << endl;
		}

		f_M = (int)pow(2.0f, (float)f_iOrder); 
		f_p = f_M/f_nBars;
		if ((f_p * f_nBars) != f_M)
		{
			errflg++;
			cerr << "Number of columns must divide into 2**" << f_iOrder << " = " << f_M << ", the number of terms in the sequence." << endl;
		}
	}

	return errflg;
}

void usage()
{
	cerr << "Usage: vertibar -f msequence_file -n nbars [-x|y center_coord] -t frames_per_term [-a] [-v]" << endl;
}


int init_pages(int nCols, int iBarWidth, int iCenterCoord)
{
	int status=0;
	int useCenterCoord;
	int useBarWidth;
	int scrHeight, scrWidth;
	int bufferLeft, bufferRight;	// pixel buffers
	int bufferTop, bufferBottom;	// pixel buffers
	int i;
	VSGTRIVAL color;

	// all drawing is done relative to upper left corner of video page
	vsgSetDrawOrigin(0,0);

	// background color lut buffer
	get_color(f_background, color);
	for (i=0; i<nCols+1; i++)
	{
		f_buffer[i] = color;
	}
	vsgPaletteWrite(&f_buffer, 0, nCols+1);

	// compute parameters used for drawing
	scrHeight = vsgGetSystemAttribute(vsgSCREENHEIGHT);
	scrWidth = vsgGetSystemAttribute(vsgSCREENWIDTH);

	if (f_bVertical)
	{
		if (iCenterCoord < 0) 
		{
			useCenterCoord = scrWidth/2;
		}
		else
		{
			useCenterCoord = iCenterCoord;
		}

		if (iBarWidth == 0)
		{
			useBarWidth = scrWidth/nCols;
		}
		else
		{
			useBarWidth = iBarWidth;
		}

		bufferLeft = useCenterCoord - nCols*useBarWidth/2;
		bufferRight = scrWidth - bufferLeft - nCols*useBarWidth;

		// Sanity check 
		if (bufferLeft < 0) 
		{
			cerr << "Vertibars don't fit on screen. Left hand buffer is negative. Make columns narrower, make fewer columns, or move center x coord to the right." << endl;
			status = 1;
		}

		if (bufferRight < 0) 
		{
			cerr << "Vertibars don't fit on screen. Right hand buffer is negative. Make columns narrower, make fewer columns, or move center x coord to the left." << endl;
			status = 1;
		}

	}
	else
	{
		if (iCenterCoord < 0) 
		{
			useCenterCoord = scrHeight/2;
		}
		else
		{
			useCenterCoord = iCenterCoord;
		}

		if (iBarWidth == 0)
		{
			useBarWidth = scrHeight/nCols;
		}
		else
		{
			useBarWidth = iBarWidth;
		}

		bufferTop = useCenterCoord - nCols*useBarWidth/2;
		bufferBottom = scrHeight - bufferTop - nCols*useBarWidth;

		// Sanity check 
		if (bufferTop < 0) 
		{
			cerr << "Horizontabars don't fit on screen. Top buffer is negative. Make bars narrower, make fewer bars, or move center y coord downward (more positive)." << endl;
			status = 1;
		}

		if (bufferBottom < 0) 
		{
			cerr << "Horizontabars don't fit on screen. Bottom buffer is negative. Make bars narrower, make fewer bars, or move center y coord upward." << endl;
			status = 1;
		}
	}

	if (status) return status;


	// each bar has lut index 0, 1, 2, ..., nCols-1. Background lut index = nCols. 
	vsgSetDrawMode(vsgSOLIDFILL);
	vsgSetDrawPage(vsgVIDEOPAGE, 0, nCols);
	vsgSetDrawPage(vsgVIDEOPAGE, 1, nCols);

	if (f_bVertical)
	{
		if (f_verbose)
		{
			cout << "drawing vertibars with these parameters:" << endl;
			cout << "c = " << nCols << endl;
			cout << "bar width (pixels) = " << useBarWidth << endl;
			cout << "screen width, height = " << scrWidth << ", " << scrHeight << endl;
			cout << "left, right buffer = " << bufferLeft << ", " << bufferRight << endl;
			cout << "center x coord = " << useCenterCoord << endl;
			cout << "p = " << f_p << endl;
		}

		for (i=0; i<nCols; i++)
		{
			vsgSetPen1(i);
			vsgDrawRect(bufferLeft + i*useBarWidth, 0, useBarWidth, scrHeight);
		}
	}
	else
	{
		if (f_verbose)
		{
			cout << "drawing horizontabars with these parameters:" << endl;
			cout << "c = " << nCols << endl;
			cout << "bar width (pixels) = " << useBarWidth << endl;
			cout << "screen width, height = " << scrWidth << ", " << scrHeight << endl;
			cout << "top, bottom buffer = " << bufferTop << ", " << bufferBottom << endl;
			cout << "center y coord = " << useCenterCoord << endl;
			cout << "p = " << f_p << endl;
		}

		for (i=0; i<nCols; i++)
		{
			vsgSetPen1(i);
			vsgDrawRect(0, bufferTop + i*useBarWidth, scrWidth, useBarWidth);
		}
	}

	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);
	return status;
}

void init_triggers()
{
	triggers.addTrigger(new CallbackTrigger("s", 0x2, 0x2, 0x2, 0x2, callback));
	triggers.addTrigger(new CallbackTrigger("X", 0x2, 0x0, 0x2, 0x0, callback));
	triggers.addTrigger(new QuitTrigger("q", 0x80, 0x80, 0xff, 0x0, 0));

	// Dump triggers
	std::cout << "Triggers:" << std::endl;
	for (unsigned int i=0; i<triggers.size(); i++)
	{
		std::cout << "Trigger " << i << " " << *(triggers[i]) << std::endl;
	}
}

int callback(int &output, const CallbackTrigger* ptrig)
{
	int ival=0;
	string key = ptrig->getKey();
	if (key == "s")
	{
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);
		vsgSetCommand(vsgCYCLELUTENABLE);
	}
	else if (key == "X")
	{
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);
		vsgSetCommand(vsgCYCLELUTDISABLE);
	}

	return 0;		// this ensures that no vsgPresent is called (which would insert an extra trigger)
}


/*
 * populateLutBuffers
 * 
 * starting with nextTerm, populate the vsg lut buffers starting at nextBufferIndex. In all, fill
 * nTerms buffers. If MAXLUTBUFFERS is hit, circle around to 0. Return the next term in the sequence
 * after the nTerms are played out.
 * 
 */
int populateLutBuffers(int &nextTerm, int nTerms, int &nextBufferIndex)
{
	static VSGTRIVAL w = {1, 1, 1};
	static VSGTRIVAL b = {0, 0, 0};
	static VSGTRIVAL g = {0.5, 0.5, 0.5};
	int i, j, n;
	int bufferIndex;
	bufferIndex = nextBufferIndex;

	// This loop is for entire rows. The position in the msequence of the first column of the 
	// row is the same as nextTerm. Each time through we increment nextTerm, and set it to zero
	// when it hits the end of the sequence. 
	for (i=0; i<nTerms; i++)
	{
		for (j=0; j<f_nBars; j++)
		{
			// mseq term in the first column is always the same as the overall term in to be displayed.
			// The rest of the dots in the row are governed by  px + pcy. The y value is the term in the
			// first column, and x is the column index; c = f_nBars.
			n = (f_p * j + f_p * f_nBars * nextTerm) % (f_M-1);
			if (f_sMSequence[n]=='1')
			{
				f_buffer[j] = w;
			}
			else
			{
				f_buffer[j] = b;
			}
		}
		f_buffer[f_nBars] = g;
		vsgLUTBUFFERWrite(nextBufferIndex, &f_buffer);
		if (++nextBufferIndex == MAXLUTBUFFERS)
		{
			nextBufferIndex = 0;
		}
		if (nextTerm++ == f_M-1)
		{
			nextTerm = 0;
		}
	}
	return nextTerm;
}


void initLutBuffers()
{
	populateLutBuffers(f_nextSeqTerm, 1000, f_nextLutBufferIndex);
	cout << "After calling \"populateLutBuffers(f_nextSeqTerm, 2000, f_nextLutBufferIndex);\" f_nextSeqTerm=" << f_nextSeqTerm << ", f_nextLutBufferIndex=" << f_nextLutBufferIndex << endl;
	populateLutBuffers(f_nextSeqTerm, 1000, f_nextLutBufferIndex);
	cout << "After calling \"populateLutBuffers(f_nextSeqTerm, 2000, f_nextLutBufferIndex);\" f_nextSeqTerm=" << f_nextSeqTerm << ", f_nextLutBufferIndex=" << f_nextLutBufferIndex << endl;
	vsgLUTBUFFERCyclingSetup(0, f_iFramesPerTerm, 0, 1999, 1, 0, 0);
	return;
}

