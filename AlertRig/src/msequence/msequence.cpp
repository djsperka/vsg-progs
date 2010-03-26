#include "vsgv8.h"
#include <cmath>
#include <iostream>
#include "Alertlib.h"
#include "AlertUtil.h"
#pragma comment(lib, "vsgv8.lib")
#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

bool f_binaryTriggers = true;
int f_scrGridCenterX, f_scrGridCenterY;
int f_iDotSize;
int f_nRepeats;
int f_iFramesPerTerm;
bool f_verbose = false;
bool f_really_verbose = false;
int f_nRows = 16;
int f_nCols = 16;
int f_iFirst;
int f_iLast;
COLOR_TYPE f_background = { gray, {} };
TriggerVector triggers;
PageCyclingTrigger *f_ptrigCycling = NULL;

#define NUMBER_OF_TERMS 32767
char *f_sMSequence = NULL;
VSGCYCLEPAGEENTRY f_MPositions[32768];

void init_pages_and_cycling();
void init_triggers();
int callback(int &output, const CallbackTrigger* ptrig);
int args(int argc, char **argv);
void usage();


using namespace std;

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

	init_pages_and_cycling();

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


void init_pages_and_cycling()
{
	int ApertureX, ApertureY;
	double ScreenHeight,ScreenWidth;
	int Width,Height;
	int term, rindex, cindex, index;
	short row, column;
	int PixelWidth, PixelHeight;
	int count=0;


	ScreenHeight=vsgGetSystemAttribute(vsgSCREENHEIGHT);
	ScreenWidth=vsgGetSystemAttribute(vsgSCREENWIDTH);
	PixelWidth=f_iDotSize;
	PixelHeight=f_iDotSize/4;
	Width = f_nRows * PixelWidth;
	Height = f_nCols * PixelHeight;

	ApertureX=(int)ScreenWidth/2  + f_scrGridCenterX;
	ApertureY=(int)ScreenHeight/2 + f_scrGridCenterY;
	vsgSetDrawOrigin(0,0);
 	vsgSetVideoMode(vsgPANSCROLLMODE);
	vsgSetVideoMode(vsgZOOM4);


	//Setup the overlay page
	VSGLUTBUFFER Overlay;
	Overlay[1].a=.5; Overlay[1].b=.5; Overlay[1].c=.5;
	vsgSetCommand(vsgOVERLAYMASKMODE);

	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&Overlay, 0, 2);

	vsgSetDrawPage(vsgOVERLAYPAGE, 1, 1);
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1 );
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 1);
	vsgSetPen1(0);
	vsgDrawRect(ApertureX, ApertureY, Width, Height*4);

	//Draw the M msequence terms into video memory
	vsgSetPen2(0);
	vsgSetCommand(vsgVIDEOCLEAR);
	vsgSetDrawPage(vsgVIDEOPAGE,0,vsgNOCLEAR);
	
	vsgSetPen1(255);


	cout << "preparing video memory...." << endl;
	for (rindex=0; rindex<128*16+15; rindex++)
	{
		for (cindex=0; cindex<31; cindex++)
		{
			term=(16*128*rindex+128*cindex) % NUMBER_OF_TERMS;
			if (f_sMSequence[term]=='1') 
			{
				vsgDrawRect(PixelWidth*(cindex+.5),PixelHeight*(rindex+.5),PixelWidth,PixelHeight);
			}
		}		
	}

	//Create a VSGPAGECYCLE buffer to hold the different positions in video memory
	//corresponding to our stimulus. We can only display about 2000 of them at a time,
	//due to VSG limitations.

	cout << "preparing page cycling buffer......" << endl;
	for (index=f_iFirst-1; index < f_iLast; index++)
	{
		row=(index%128)*Height+PixelHeight*(index/(128*16));
		column=(short)((index/128)%16);
		f_MPositions[count].Page=0+vsgDUALPAGE+vsgTRIGGERPAGE ;
		f_MPositions[count].Xpos=-ApertureX+Width/2 +column*PixelWidth ;
		f_MPositions[count].Ypos=-ApertureY/4+Height/2+row;
		f_MPositions[count].Frames=f_iFramesPerTerm;
		f_MPositions[count].Stop=0;
		f_MPositions[count].ovPage=0;
		f_MPositions[count].ovXpos=0;
		f_MPositions[count].ovYpos=0;
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
	vsgPageCyclingSetup(count, &f_MPositions[0]);

	return;
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

	while ((c = getopt(argc, argv, "t:f:p:d:aR:hvVT:")) != -1)
	{
		switch (c) 
		{
		case 'a':
			f_binaryTriggers = false;
			break;
		case 'f':
			s.assign(optarg);
			if (arutil_load_mseq(&f_sMSequence, s))
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
			if (parse_integer(s, f_nRepeats))
			{
				cerr << "Cannot parse repeats (" << s << "): must be an integer." << endl;
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
	if (!have_d) 
	{
		cerr << "No dot size specified!" << endl; 
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
	if (!have_T)
	{
		f_iFirst = 1;
		f_iLast = NUMBER_OF_TERMS;
	}
	if (errflg) 
	{
		usage();
	}
	return errflg;
}

void usage()
{
	cerr << "Usage: msequence -f msequence_file -d dotsize -p gridCenterX,gridCenterY -t frames_per_term [-a] [-v] [-V]" << endl;
}




