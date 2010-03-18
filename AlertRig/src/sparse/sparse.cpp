#include "alertlib.h"
#include "VSGV8.H"
#include <stdio.h>
#include <math.h>
using namespace alert;
using namespace std;


#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")

int f_screenDistanceMM = 0;
COLOR_TYPE f_background = { gray, {} };
int f_iDotSize = 8;
int f_nRows = 16;
int f_nCols = 16;
int f_pixBoxX, f_pixBoxY;		// pixel coord of the single box drawn. For the white (black) box its relative to page 0 (2). 
int f_scrGridCenterX;
int f_scrGridCenterY;
int f_scrGridX, f_scrGridY;		// screen coord of origin (upper left corner) of grid
int f_scxGridX, f_scxGridY;		// screen pixel coord (measured from upper left corner, positive down and right) of grid origin.
int f_scrBoxX, f_scrBoxY;
int f_iFramesPerTerm;
bool f_verbose = false;
bool f_really_verbose = false;
bool f_bTesting = false;
int f_nrepeats=1;
short *f_pterms = NULL;
int f_nterms = 0;
double f_tbefore = 1.0;
double f_tafter = 1.0;
bool f_binaryTriggers = true;
TriggerVector triggers;

void usage();
int args(int argc, char **argv);
int load_stimulus(const string& sfilename);
void do_testing();
void prepare_page_cycling();
void init_triggers();
int callback(int &output, const CallbackTrigger* ptrig);


int main(int argc, char **argv)
{

	// Check input args
	if (args(argc, argv))
	{
		return 1;
	}

	// INit vsg
	if (ARvsg::instance().init(f_screenDistanceMM, f_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}


	// Pan/scroll mode makes video pages twice as wide as usual.
	vsgSetVideoMode(vsgPANSCROLLMODE);

	// clear all draw modes. In particular, this turns off vsgCENTREXY, so that primitives (like Rects) are 
	// drawn with their origins at upper left corner
	vsgSetDrawMode(0);

	// PALETTERAMP means black is color 0, white is 255, ramp between them. 
	vsgSetCommand(vsgPALETTERAMP);

	// clear video pages 0-3 to mean gray
	vsgSetDrawPage(vsgVIDEOPAGE, 0, 127);
	vsgSetDrawPage(vsgVIDEOPAGE, 1, 127);
	vsgSetDrawPage(vsgVIDEOPAGE, 2, 127);
	vsgSetDrawPage(vsgVIDEOPAGE, 3, 127);

	// Determine where to draw the boxes. 
	f_pixBoxX = vsgGetSystemAttribute(vsgPAGEWIDTH)/2;
	f_pixBoxY = vsgGetSystemAttribute(vsgPAGEHEIGHT);
	f_scrBoxX = f_pixBoxX - vsgGetScreenWidthPixels()/2;
	f_scrBoxY = f_pixBoxY - vsgGetScreenHeightPixels()/2;

	// Now draw the boxes
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	vsgSetPen1(255);
	vsgDrawRect(f_scrBoxX, f_scrBoxY, f_iDotSize, f_iDotSize);
	vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgNOCLEAR);
	vsgSetPen1(0);
	vsgDrawRect(f_scrBoxX, f_scrBoxY, f_iDotSize, f_iDotSize);
	if (f_verbose)
	{
		cerr << "Page dim wxh = " << vsgGetSystemAttribute(vsgPAGEWIDTH) << " x " << vsgGetSystemAttribute(vsgPAGEHEIGHT) << endl;
		cerr << "screen dim wxh = " << vsgGetScreenWidthPixels() << " x " << vsgGetScreenHeightPixels() << endl;
		cerr << "Box drawn at " << f_scrBoxX << ", " << f_scrBoxY << endl;
	}

	// compute other important numbers
	f_scrGridX = f_scrGridCenterX - f_nCols/2*f_iDotSize;
	f_scrGridY = f_scrGridCenterY + f_nRows/2*f_iDotSize;
	f_scxGridX = vsgGetScreenWidthPixels()/2 + f_scrGridX;
	f_scxGridY = vsgGetScreenHeightPixels()/2 - f_scrGridY;

	// get page cycling ready
	prepare_page_cycling();

	// prepare triggers
	init_triggers();

	// Issue "ready" triggers to spike2.
	// These commands pulse spike2 port 6. 
	Sleep(500);
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
	 	
		if (tf.quit()) break;
		else if (tf.present())
		{	
			last_output_trigger = tf.output_trigger();
			vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
			vsgPresent();
		}
	}






	ARvsg::instance().release_lock();
	return 0;
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
		vsgResetTimer();
		vsgSetCommand(vsgVIDEODRIFT);			// allows us to move the offset of video memory
		vsgSetCommand(vsgCYCLEPAGEENABLE);
	}
	else if (key == "X")
	{
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);
	}

	return 0;		// this ensures that no vsgPresent is called (which would insert an extra trigger)
}


void prepare_page_cycling()
{
	int i, iterm;
	int term;
	int ix, iy;
	int x, y;
	int count=0;
	int ipage=0;
	int iStimLengthUS;
	string stmp;
	VSGCYCLEPAGEENTRY *pcycle;

	iStimLengthUS = (f_nrepeats * f_nterms * f_iFramesPerTerm) * vsgGetSystemAttribute(vsgFRAMETIME);
	if (f_verbose)
	{
		cerr << "Stimulus has " << f_nterms << " terms, with " << f_nrepeats << "repeat(s)." << endl;
		cerr << "Total stim run time approx " << (double)iStimLengthUS/1000000.0 << " sec." << endl;
	}

	pcycle = new VSGCYCLEPAGEENTRY[f_nterms];
	for (i=0; i<f_nterms; i++)
	{
		iterm = i+1;
		term = f_pterms[iterm]%(2*16*16);
		if (term % 2) 
			ipage=0;				// white dot
		else 
			ipage=2;				// black dot

		iy = term/(2*16);							// TODO hard coded for 16 x 16
		ix = (term %(2*16))/2;

		x = f_pixBoxX - f_scxGridX - ix*f_iDotSize;
		y = f_pixBoxY - f_scxGridY - iy*f_iDotSize;
		pcycle[count].Frames = f_iFramesPerTerm;
		pcycle[count].Page = ipage + vsgTRIGGERPAGE;
		pcycle[count].Xpos = x;
		pcycle[count].Ypos = y;
		count++;
	}

	vsgPageCyclingSetup(count, &pcycle[0]);

	return;
}

int load_stimulus(const string& sfilename)
{
	int status = 0;
	short norder;
	int basepoly;
	ifstream ifs;

	ifs.open(sfilename.c_str(), ios::binary);
	if (!ifs.is_open())
	{
		cerr << "Cannot open stimulus file (" << sfilename << ")" << endl;
		status = -1;
	}
	else
	{
		if (!ifs.read(reinterpret_cast<char *>(&norder), sizeof(short)) ||
			!ifs.read(reinterpret_cast<char *>(&basepoly), sizeof(int)))
		{
			cerr << "Error reading order/basepoly from stimulus file (" << sfilename << ")" << endl;
			status = -1;
		}
		else
		{
			f_nterms = (int)pow(2.0f, norder);
			cout << "Order is " << norder << ", expecting " << f_nterms << " terms." <<endl;
			f_pterms = new short[f_nterms];
			if (!ifs.read(reinterpret_cast<char *>(f_pterms), f_nterms * sizeof(short)))
			{
				cerr << "Error reading terms from stimulus file (" << sfilename << ")" << endl;
				status = -1;
				delete[] f_pterms;
			}
			else if (f_really_verbose)
			{
				int i;
				for (i=0; i<f_nterms; i++) cerr << i << ": " << f_pterms[i] << endl;
			}
		}
		ifs.close();
	}
	return status;
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
	bool have_r = false;
	bool have_c = false;
	bool have_t = false;
	bool have_f = false;

	while ((c = getopt(argc, argv, "d:p:r:c:t:hvVf:TR:a")) != -1)
	{
		switch (c) 
		{
		case 'a':
			f_binaryTriggers = false;
			break;
		case 'T':
			f_bTesting = true;
			break;
		case 'f':
			s.assign(optarg);
			if (load_stimulus(s))
			{
				cerr << "Error loading stimulus file (" << s << ")." << endl;
				errflg++;
			}
			else
			{
				have_f = true;
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
		case 'R':
			s.assign(optarg);
			if (parse_integer(s, f_nrepeats))
			{
				cerr << "Cannot parse repeats (" << s << "): must be an integer." << endl;
				errflg++;
			}
			break;
		case 'r':
			s.assign(optarg);
			if (parse_integer(s, f_nRows))
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
			if (parse_integer(s, f_nCols))
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

	if (!f_bTesting && !have_f)
	{
		cerr << "No stim file specified (-f), and not testing (-T). Need either a stim file or the -T flag." << endl;
		errflg++;
	}
	if (!have_d) 
	{
		cerr << "No dot size specified!" << endl; 
		errflg++;
	}
	if (!have_r || !have_c)
	{
		cerr << "Both rows and columns must be specified!" << endl; 
		errflg++;
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
	if (errflg) 
	{
		usage();
	}
	return errflg;
}

void usage()
{
	cerr << "Usage: sparse -d dotsize -p gridCenterX,gridCenterY -r nrows -c ncolumns -t frames_per_term" << endl;
}

void do_testing()
{
	bool bQuit = false;
	string s;
	
	cout << "Enter command(qpm): ";
	cin >> s;
	while (!bQuit)
	{
		if (s=="q")
		{
			bQuit = true;
		}
		else if (s=="p")
		{
			int page;
			cout << "Enter page: ";
			cin >> s;
			if (!parse_integer(s, page))
			{
				vsgSetZoneDisplayPage(vsgVIDEOPAGE, page);
			}
		}
		else if (s=="m")
		{
			int ix, iy, i;
			cout << "Enter coords for move: ";
			cin >> s;
			if (!parse_int_pair(s, ix, iy))
			{
				i = vsgMoveScreen((double)ix, (double)iy);
				printf("MoveScreen returned %d\n", i);
			}
			else printf("Error in input\n");
		}
		else if (s=="t")
		{
			int ix, iy;
			int x, y;
			cout << "Enter row, column: ";
			cin >> s;
			if (!parse_int_pair(s, ix, iy))
			{
				x = f_pixBoxX - f_scxGridX - ix*f_iDotSize;
				y = f_pixBoxY - f_scxGridY - iy*f_iDotSize;
				vsgMoveScreen(x, y);
				cout << "x,y = " << x << ", " << y << endl;
			}
		}
		else if (s=="o")
		{
			// animation along outline of grid
			int i, j;
			int x, y;
			int count=0;
			string stmp;
			VSGCYCLEPAGEENTRY cycle[32768];

			j=0;
			for (i=0; i<f_nCols; i++)
			{
				x = f_pixBoxX - f_scxGridX - i*f_iDotSize;
				y = f_pixBoxY - f_scxGridY - j*f_iDotSize;
				cycle[count].Frames = f_iFramesPerTerm;
				cycle[count].Page = 0;
				cycle[count].Xpos = x;
				cycle[count].Ypos = y;
				count++;
			}
			i = 15;
			for (j=1; j<f_nCols; j++)
			{
				x = f_pixBoxX - f_scxGridX - i*f_iDotSize;
				y = f_pixBoxY - f_scxGridY - j*f_iDotSize;
				cycle[count].Frames = f_iFramesPerTerm;
				cycle[count].Page = 0;
				cycle[count].Xpos = x;
				cycle[count].Ypos = y;
				count++;
			}
			j = 15;
			for (i=f_nCols-1; i>=0; i--)
			{
				x = f_pixBoxX - f_scxGridX - i*f_iDotSize;
				y = f_pixBoxY - f_scxGridY - j*f_iDotSize;
				cycle[count].Frames = f_iFramesPerTerm;
				cycle[count].Page = 0;
				cycle[count].Xpos = x;
				cycle[count].Ypos = y;
				count++;
			}
			i = 0;
			for (j=f_nCols-1; j>=0; j--)
			{
				x = f_pixBoxX - f_scxGridX - i*f_iDotSize;
				y = f_pixBoxY - f_scxGridY - j*f_iDotSize;
				cycle[count].Frames = f_iFramesPerTerm;
				cycle[count].Page = 0;
				cycle[count].Xpos = x;
				cycle[count].Ypos = y;
				count++;
			}
			j=0;
			for (i=1; i<f_nCols; i++)
			{
				x = f_pixBoxX - f_scxGridX - i*f_iDotSize;
				y = f_pixBoxY - f_scxGridY - j*f_iDotSize;
				cycle[count].Frames = f_iFramesPerTerm;
				cycle[count].Page = 2;
				cycle[count].Xpos = x;
				cycle[count].Ypos = y;
				count++;
			}
			i = 15;
			for (j=1; j<f_nCols; j++)
			{
				x = f_pixBoxX - f_scxGridX - i*f_iDotSize;
				y = f_pixBoxY - f_scxGridY - j*f_iDotSize;
				cycle[count].Frames = f_iFramesPerTerm;
				cycle[count].Page = 2;
				cycle[count].Xpos = x;
				cycle[count].Ypos = y;
				count++;
			}
			j = 15;
			for (i=f_nCols-1; i>=0; i--)
			{
				x = f_pixBoxX - f_scxGridX - i*f_iDotSize;
				y = f_pixBoxY - f_scxGridY - j*f_iDotSize;
				cycle[count].Frames = f_iFramesPerTerm;
				cycle[count].Page = 2;
				cycle[count].Xpos = x;
				cycle[count].Ypos = y;
				count++;
			}
			i = 0;
			for (j=f_nCols-1; j>=0; j--)
			{
				x = f_pixBoxX - f_scxGridX - i*f_iDotSize;
				y = f_pixBoxY - f_scxGridY - j*f_iDotSize;
				cycle[count].Frames = f_iFramesPerTerm;
				cycle[count].Page = 2;
				cycle[count].Xpos = x;
				cycle[count].Ypos = y;
				count++;
			}
			vsgPageCyclingSetup(count, &cycle[0]);
			cout << "Hit any key to start." << endl;
			cin >> stmp;
			vsgSetCommand(vsgCYCLEPAGEENABLE);
			cout << "Hit any key to stop." << endl;
			cin >> stmp;
			vsgSetCommand(vsgCYCLEPAGEDISABLE);
		}
		else
		{
			printf("Unknown input.\n");
			break;
		}
		if (bQuit) break;
		cout << "Enter command(qpm): ";
		cin >> s;
	}

}