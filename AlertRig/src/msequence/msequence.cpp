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
int f_scrGridCenterX, f_scrGridCenterY;
int f_iDotSize;
int f_iDotWidth;
int f_iDotHeight;
bool f_toobigOK = false;
bool f_narrowPages = false;
int f_nRepeats;
int f_iFramesPerTerm;
bool f_verbose = false;
bool f_really_verbose = false;
int f_nRows = 16;
int f_nCols = 16;
int f_iFirst;
int f_iLast;
int f_iZoom = 4;
int f_iOrder = 15;
string f_sFilename;
COLOR_TYPE f_background = { gray, {} };
TriggerVector triggers;
PageCyclingTrigger *f_ptrigCycling = NULL;
bool f_outline = false;
bool f_terms = false;	// testing only

#define NUMBER_OF_TERMS 32767
char *f_sMSequence = NULL;
VSGCYCLEPAGEENTRY f_MPositions[32768];

void init_pages_and_cycling();
int init_pages_and_cycling_2(long dotWidth, long dotHeight, int iZoom, int nRows, int nCols);
void init_triggers();
int callback(int &output, const CallbackTrigger* ptrig);
int args(int argc, char **argv);
void usage();



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

	if (init_pages_and_cycling_2(f_iDotWidth, f_iDotHeight, f_iZoom, f_nRows, f_nCols))
	{
		cerr << "Cannot draw msequence using given parameters." << endl;
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
			vsgSetCommand(vsgCYCLEPAGEDISABLE);
			vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 1);
			break;
		}
		else if (tf.present())
		{	
			last_output_trigger = tf.output_trigger();
			vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
			vsgPresent();
		}

		// Throttle cpu usage a little. No need to be in hyperspeed checking for triggers here. 
		Sleep(100);
	}



	return 0;
}


int init_pages_and_cycling_2(long dotWidth, long dotHeight, int iZoom, int nRows, int nCols)
{
	long pageHeight, pageWidth;		// video pages
	long scrHeight, scrWidth;		// screen resolution
	long pixelHeight;	            // height (as drawn) of a single square in the grid; depends on zoom factor
	long pixelWidth;				// width of a single square in the grid
	long gridHeight;
	long gridWidth;
	long xAperture, yAperture;
	int term, rindex, cindex, index;
	long row, column;
	int M, p;
	int nPages;
	int count=0;
	VSGTRIVAL bw[2] = { {1, 1, 1}, {0, 0, 0}};
	VSGLUTBUFFER buffer;
	int status = 0;

	// all drawing is done relative to upper left corner of video page
	vsgSetDrawOrigin(0,0);

	// pan scroll mode doubles the video page width and cuts number of pages in half
	if (!f_narrowPages)
	{
		cerr << "Setting pan/scroll mode." << endl;
	 	vsgSetVideoMode(vsgPANSCROLLMODE);
	}
	else
	{
		cerr << "Narrow pages requested. No pan/scroll mode." << endl;
	}

	// compute parameters used for drawing
	M = (int)pow(2.0f, (float)f_iOrder);
	p = M/nRows/nCols;
	pageHeight = vsgGetSystemAttribute(vsgPAGEHEIGHT);
	pageWidth = vsgGetSystemAttribute(vsgPAGEWIDTH);
	nPages = vsgGetSystemAttribute(vsgNUMVIDEOPAGES);
	scrHeight = vsgGetSystemAttribute(vsgSCREENHEIGHT);
	scrWidth = vsgGetSystemAttribute(vsgSCREENWIDTH);
	pixelHeight = dotHeight/iZoom;
	pixelWidth = dotWidth;
	gridHeight = nRows * pixelHeight;
	gridWidth = nCols * pixelWidth;
	xAperture = scrWidth/2  + f_scrGridCenterX;
	yAperture = scrHeight/2 + f_scrGridCenterY;

	// Test aperture position and adjust if necessary.
	if ((yAperture/iZoom)*iZoom != yAperture)
	{
		long y = (yAperture/iZoom)*iZoom;
		cout << "Adjust y aperture position from " << yAperture << " to " << y << endl;
		yAperture = y;
	}

	if (f_verbose)
	{
		cout << "drawing msequence with these parameters:" << endl;
		cout << "r,c = " << nRows << ", " << nCols << endl;
		cout << "order = " << f_iOrder << endl;
		cout << "#terms = " << M << endl;
		cout << "p = " << p << endl;
		cout << "dotsize (wxh)   = " << dotWidth << " x " << dotHeight << endl;
		cout << "dotsize (drawn) = " << pixelWidth << " x " << pixelHeight << endl;
		cout << "zoom factor = " << iZoom << endl;
		cout << "screen width, height = " << scrWidth << ", " << scrHeight << endl;
		cout << "vid pg width, height = " << pageWidth << ", " << pageHeight << endl;
		cout << "num vid pages = " << nPages << endl;
		cout << "aperture pos " << xAperture << ", " << yAperture << endl;
	}

	// Will the pattern fit be too tall for video memory?
	if (((p * nRows + (nRows-1))* pixelHeight) > (nPages * pageHeight))
	{
		cout << "ERROR: The mseq will not fit in video memory." << endl;
		cout << "       Total height required is (p * nRows + (nRows-1)) * dotHeight / iZoom = " << ((p*nRows+(nRows-1))*dotHeight/iZoom) << endl;
		cout << "       System has " << nPages << " video pages of height " << pageHeight << " pixels, for a total height of " << (nPages*pageHeight) << " pixels." << endl;
		cout << "       You can try reducing dot height (-d or -H) or the number of rows (r), using a larger zoom factor, or using a shorter msequence." << endl;
		status = 1;
	}

	// verify input parameters are kosher. Too wide?
	if ((2*nCols-1)*dotWidth > pageWidth)
	{
		cout << "ERROR: The mseq will not fit in video memory." << endl;
		cout << "       Total page width required is (2c-1)*dotWidth = " << (2*nCols-1)*dotWidth << endl;
		cout << "       System video pages have width " << pageWidth << " pixels" << endl;
		cout << "       You can try reducing dot width (-d or -W) or the number of columns (-c)." << endl;
		cout << "       Its also possible that by increasing the screen resolution the video page size will be increased, though that may make the video memory height too small." << endl;
		status = 1;
	}

	// will it fit on screen?
	if ((nRows * dotHeight) > scrHeight)
	{
		if (f_toobigOK)
		{
			cout << "WARNING: the pattern will not fit completely on the screen." << endl;
			cout << "         Screen height is " << scrHeight << " pixels, but the mseq requires at least r*dotHeight = " << (nRows*dotHeight) << " pixels." << endl;
			cout << "         You can try reducing the dot height (-d or -H) or the number of rows (-r)." << endl;
			cout << "         Since you specified the \"toobigOK\" flag (-y) we'll just let this go." << endl;
		}
		else
		{
			cout << "ERROR: The mseq will not fit on the screen. " << endl;
			cout << "       Screen height is " << scrHeight << " pixels, but the mseq requires at least r*dotHeight = " << (nRows*dotHeight) << " pixels." << endl;
			cout << "       You can try reducing the dot height (-d or -H) or the number of rows (-r)." << endl;
			status = 1;
		}
	}

	if ((nCols * dotWidth) > scrWidth)
	{
		if (f_toobigOK)
		{
			cout << "WARNING: The pattern will not fit on the screen. " << endl;
			cout << "         Screen width is " << scrWidth << " pixels, but the mseq requires at least c*dotWidth = " << (nCols*dotWidth) << " pixels." << endl;
			cout << "       You can try reducing the dot width (-d or -W) or the number of columns (-c)." << endl;
		}
		else
		{
			cout << "ERROR: The mseq will not fit on the screen. " << endl;
			cout << "       Screen width is " << scrWidth << " pixels, but the mseq requires at least c*dotWidth = " << (nCols*dotWidth) << " pixels." << endl;
			cout << "       You can try reducing the dot width (-d or -W) or the number of columns (-c)." << endl;
			status = 1;
		}
	}

	if (status) return status;


	switch (iZoom)
	{
	case 1: 
		vsgSetVideoMode(vsgZOOM1);
		break;
	case 2: 
		vsgSetVideoMode(vsgZOOM2);
		break;
	case 4: 
		vsgSetVideoMode(vsgZOOM4);
		break;
	case 8: 
		vsgSetVideoMode(vsgZOOM8);
		break;
	case 16: 
		vsgSetVideoMode(vsgZOOM16);
		break;
	default:
		cerr << "ERROR: bad zoom value (" << iZoom << ")." << endl;
		return -1;
		break;
	}



	//Setup the overlay page
	vsgSetCommand(vsgOVERLAYMASKMODE);
	buffer[0].a = buffer[0].b = buffer[0].c = .5;
	buffer[1].a = 1; buffer[1].b = buffer[1].c = 0;
	vsgPaletteWriteOverlayCols(&buffer, 1, 2);
	vsgSetPen1(0);
	vsgSetPen2(1);
	vsgSetCommand(vsgOVERLAYCLEAR);				// clear all overlay mem to pen2's level
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 1);

	if (!f_outline)
	{
		vsgSetDrawPage(vsgOVERLAYPAGE, 0, vsgNOCLEAR);
		vsgDrawRect(xAperture, yAperture, gridWidth, gridHeight*iZoom);
	}
	else
	{
		vsgSetDrawPage(vsgOVERLAYPAGE, 0, 0);	// clear
		vsgSetPen1(2);
		vsgSetDrawMode(vsgCENTREXY+vsgPIXELPEN);
		vsgDrawRect(xAperture, yAperture, gridWidth, gridHeight*iZoom);
		vsgSetPen1(0);
		vsgSetDrawMode(vsgCENTREXY+vsgSOLIDFILL);
	}
	if (f_verbose)
	{
		cout << "Aperture grid at " << xAperture << ", " << yAperture << " size " << gridWidth << " x " << gridHeight*iZoom << endl;
	}


	// Clear video memory to black, get white into palette
	cout << "preparing video memory...." << endl;
	// hackvsgPaletteSet(0, 1, &bw[0]);
	// Have black-white linear ramp. Set pen1 to level 0 (black) and clear memory. 
//	vsgSetPen(0);	// white
//	vsgSetPen2(1);	// black
	vsgSetPen2(0);
	vsgSetCommand(vsgVIDEOCLEAR);	// clear all video to pen2 (black). 
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	vsgSetPen1(255);

	for (rindex=0; rindex < (p * nRows + (nRows-1)); rindex++)
	{
		for (cindex=0; cindex < 2*nCols-1; cindex++)
		{
			// term = px + pcy; p==128 when n==15, r=c=16
			//term=(16*128*rindex+128*cindex) % NUMBER_OF_TERMS;
			term = (p * cindex + p * nCols * rindex) % (M-1);
			if (f_sMSequence[term]=='1') 
			{
				vsgDrawRect(pixelWidth*(cindex+.5), pixelHeight*(rindex+.5), pixelWidth, pixelHeight);
			}
		}		
	}

	//Create a VSGPAGECYCLE buffer to hold the different positions in video memory
	//corresponding to our stimulus. We can only display about 2000 of them at a time,
	//due to VSG limitations.

	cout << "preparing (*) page cycling buffer......" << endl;
	for (index=f_iFirst-1; index < f_iLast; index++)
	{
		row=(index%p)*gridHeight + pixelHeight*(index/(p*nCols));
		column=(short)((index/p)%nCols);
		f_MPositions[count].Page=0+vsgDUALPAGE+vsgTRIGGERPAGE ;
		f_MPositions[count].Xpos=(short)(-xAperture + gridWidth/2 + column*pixelWidth);
		f_MPositions[count].Ypos=(short)(-yAperture/iZoom + gridHeight/2 + row);
		f_MPositions[count].Frames=f_iFramesPerTerm;
		f_MPositions[count].Stop=0;
		f_MPositions[count].ovPage=0;
		f_MPositions[count].ovXpos=0;
		f_MPositions[count].ovYpos=0;
		//cout << index << " " << f_MPositions[count].Xpos << " " << f_MPositions[count].Ypos << endl;
		count++;
	}
	f_MPositions[count].Page=0+vsgDUALPAGE+vsgTRIGGERPAGE;
	f_MPositions[count].Xpos=0;
	f_MPositions[count].Ypos=0;
	f_MPositions[count].Frames=1;
	f_MPositions[count].ovPage=1;
	f_MPositions[count].ovXpos=0;
	f_MPositions[count].ovYpos=0;
	f_MPositions[count].Stop=1;
	count++;

	cout << "page cycling buffer has " << count << " terms." << endl;
	//hack vsgSetCommand(vsgVIDEODRIFT+vsgOVERLAYDRIFT);
	vsgPageCyclingSetup(count, &f_MPositions[0]);

	if (f_verbose)
	{
		cout << "First term Xpos, Ypos = " << f_MPositions[0].Xpos << ", " << f_MPositions[0].Ypos << endl;
	}



	// check for testing flag
	if (f_terms)
	{
		string stmp;
		vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 0);
		vsgSetCommand(vsgVIDEODRIFT);
		cout << "Enter term or q: ";
		cin >> stmp;
		while (stmp != "q")
		{
			short xpos, ypos;
			if (stmp != "x" && stmp != "y")
			{
				index = atoi(stmp.c_str());
				row=(index%p)*gridHeight + pixelHeight*(index/(p*nCols));
				column=(short)((index/p)%nCols);
				xpos=(short)(-xAperture + gridWidth/2 + column*pixelWidth);
				ypos=(short)(-yAperture/iZoom + gridHeight/2 + row);
				cout << "x = (short)(-xAperture + gridWidth/2 + column*pixelWidth) = " <<
					-xAperture << " + " << gridWidth/2 << " + " << column*pixelWidth << " = " << xpos << endl;
				cout << "y = (short)(-yAperture/iZoom + gridHeight/2 + row) = " << 
					-yAperture/iZoom << " + " << gridHeight/2 << " + " << row << " = " << ypos << endl;
#if 0
		row=(index%p)*gridHeight + pixelHeight*(index/(p*nCols));
		column=(short)((index/p)%nCols);
		f_MPositions[count].Page=0+vsgDUALPAGE+vsgTRIGGERPAGE ;
		f_MPositions[count].Xpos=(short)(-xAperture + gridWidth/2 + column*pixelWidth);
		f_MPositions[count].Ypos=(short)(-yAperture/iZoom + gridHeight/2 + row);
#endif

			}
			else
			{
				cout << "Enter x: ";
				cin >> xpos;
				cout << "Enter y: ";
				cin >> ypos;
			}
			cout << "moving to x, y = " << xpos << ", " << ypos <<endl;
			vsgMoveScreen(xpos, ypos);
			cout << "Enter term or q: ";
			cin >> stmp;
		}
	}

	return 0;
}




void init_triggers()
{
	triggers.addTrigger(new CallbackTrigger("s", 0x2, 0x2, 0x2, 0x2, callback));
	triggers.addTrigger(new CallbackTrigger("X", 0x2, 0x0, 0x2, 0x0, callback));
	f_ptrigCycling = new PageCyclingTrigger("C", f_nRepeats);
	triggers.addTrigger(f_ptrigCycling);
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
		vsgResetTimer();
		vsgSetCommand(vsgVIDEODRIFT+vsgOVERLAYDRIFT);
		vsgSetCommand(vsgCYCLEPAGEENABLE);
		f_ptrigCycling->started();
	}
	else if (key == "X")
	{
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		f_ptrigCycling->stopped();
		vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 1);
	}

	return 0;		// this ensures that no vsgPresent is called (which would insert an extra trigger)
}

int args(int argc, char **argv)
{	
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	bool have_p = false;
	bool have_d = false;
	bool have_t = false;
	bool have_f = false;
	bool have_T = false;
	bool have_o = false;
	bool have_w = false;
	bool have_h = false;

	while ((c = getopt(argc, argv, "t:f:p:d:aR:hvVT:z:r:c:o:OKW:H:yN")) != -1)
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
		case 'W':
			s.assign(optarg);
			if (parse_integer(s, f_iDotWidth))
			{
				cerr << "Cannot parse dot width (" << s << "): must be an integer." << endl;
				errflg++;
			}
			else
			{
				have_w = true;
			}
			break;
		case 'H':
			s.assign(optarg);
			if (parse_integer(s, f_iDotHeight))
			{
				cerr << "Cannot parse dot height (" << s << "): must be an integer." << endl;
				errflg++;
			}
			else
			{
				have_h = true;
			}
			break;
		case 'y':
			f_toobigOK = true;
			break;
		case 'N':
			f_narrowPages = true;
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
		case 'R':
			s.assign(optarg);
			if (parse_integer(s, f_nRepeats))
			{
				cerr << "Cannot parse repeats (" << s << "): must be an integer." << endl;
				errflg++;
			}
			break;
		case 'z':
			s.assign(optarg);
			if (parse_integer(s, f_iZoom))
			{
				cerr << "Cannot parse zoom factor (" << s << "): must be an integer." << endl;
				errflg++;
			}
			else if (f_iZoom != 1 && f_iZoom != 2 && f_iZoom != 4 && f_iZoom != 8 && f_iZoom != 16)
			{
				cerr << "Zoom factor must be one of (1, 2, 4, 8, 16)." << endl;
				errflg++;
			}
			break;
		case 'v':
			f_verbose = true;
			break;
		case 'V':
			f_verbose = true;
			f_really_verbose = true;
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
		case 'T':
			s.assign(optarg);
			if (parse_sequence_pair(s, f_iFirst, f_iLast))
			{
				cerr << "Cannot parse subsequence pair (" << s << ")." << endl;
				errflg++;
			}
			else
			{
				have_T = true;
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
	if (have_d)
	{
		f_iDotHeight = f_iDotWidth = f_iDotSize;
		if (have_w || have_h)
		{
			cerr << "Cannot specify both dot size (-d) and either dot height (-H) or width (-W)." << endl;
			errflg++;
		}
		else
		{
			if (f_iDotSize % f_iZoom != 0 || f_iDotSize < f_iZoom) 
			{
				cerr << "Dot size (" << f_iDotSize << ") must be a multiple of zoom factor (" << f_iZoom << ")." << endl;
				errflg++;
			}
		}
	}
	else
	{
		if (!have_h || !have_w)
		{
			cerr << "You did not specify dot size (-d), so you must specify both dot height (-H) and width (-W)." << endl;
			errflg++;
		}
		else
		{
			if (f_iDotHeight % f_iZoom != 0 || f_iDotHeight < f_iZoom) 
			{
				cerr << "Dot Height (" << f_iDotHeight << ") must be a multiple of zoom factor (" << f_iZoom << ")." << endl;
				errflg++;
			}
		}
	}
	if (!have_t)
	{
		cerr << "No frames_per_term value specified!" << endl;
		errflg++;
	}
	if (!have_p)
	{
		cerr << "No position (-p) specified!" << endl;
		errflg++;
	}
	if (!have_T)
	{
		f_iFirst = 1;
		f_iLast = NUMBER_OF_TERMS;
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
	}

	return errflg;
}

void usage()
{
	cerr << "Usage: msequence -f msequence_file -d dotsize -p gridCenterX,gridCenterY -t frames_per_term [-a] [-v] [-V]" << endl;
}




