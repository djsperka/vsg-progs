#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__

#include "VSGV8.H"
//#include "util.h"
#include "alertlib.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")


int args(int argc, char **argv);
static void usage();
int init();

using namespace alert;
using namespace std;
bool f_verbose=true;
typedef struct colorflash_struct {
	double r, g, b;
	double secs;
} CFStruct;
bool f_bAutoStart = false;
double f_cycletimeS = 0;	// total cycle time in sec
int f_cycleRepeats = 1;
bool f_bUseLockFile = true;

vector<CFStruct> f_cfVecPages;

int main (int argc, char *argv[])
{

	// Check input arguments
	if (args(argc, argv))
	{
		return 1;
	}
	else
	{
		if (f_verbose)
		{
			cout << "colorflash - " << f_cfVecPages.size() << " flash pages." << endl;
			for (unsigned int i=0; i<f_cfVecPages.size(); i++)
			{
				cout << "Page " << i+1 << " (r,g,b) = (" << f_cfVecPages[i].r << "," << f_cfVecPages[i].g << "," << f_cfVecPages[i].b << ") seconds=" << f_cfVecPages[i].secs << endl;
			}
		}
	}

	if (init()) 
	{
		cerr << "Error in vsg init and/or page cycling setup." << endl;
		return -1;
	}


	if (!f_bAutoStart)
	{

		char c[10];
		cout << "Hit any key to start." << endl;
		cin.getline(c, 10);

		vsgSetCommand(vsgCYCLEPAGEENABLE);

		cout << "Hit any key to stop." << endl;
		cin.getline(c, 10);

		vsgSetCommand(vsgCYCLEPAGEDISABLE);
	}
	else
	{
		vsgSetCommand(vsgCYCLEPAGEENABLE);
		long endTimeUS = vsgGetTimer() + (long)(1000000*f_cycletimeS*f_cycleRepeats + 100000); // .1s buffer to make sure

		// Wait until the entire cycle is complete
		while (vsgGetTimer() < endTimeUS)
		{
			Sleep(1000);
		}
		vsgSetCommand(vsgCYCLEPAGEDISABLE);


		vsgSetCommand(vsgCYCLEPAGEDISABLE);
	}

	return 0;
}


int init()
{
	int istatus=0;
	VSGTRIVAL *c;
	int npages;
	int nframerate;
	unsigned int i;
	COLOR_TYPE g = {gray, {0.5,0.5,0.5}};

	//if (0 != (istatus = vsgInit(""))) return istatus;
	if (ARvsg::instance().init(0, g, f_bUseLockFile))
	{
		cerr << "Error initializing VSG." << endl;
		return -1;
	}

	npages = vsgGetSystemAttribute(vsgNUMVIDEOPAGES);
	nframerate = vsgGetSystemAttribute(vsgFRAMERATE);

	if (npages < (int)f_cfVecPages.size())
	{
		cerr << "Error - vsg has only " << npages << " video pages, but you requested " << f_cfVecPages.size() << " pages." << endl;
		return 1;
	}

	// set colors in palette. Color for page 'i' will be at LUT index 'i' (skip index 0!)
	vsgSetCommand(vsgPALETTECLEAR);
	c = new VSGTRIVAL[f_cfVecPages.size()];
	for (i=0; i<f_cfVecPages.size(); i++)
	{
		c[i].a = f_cfVecPages[i].r;
		c[i].b = f_cfVecPages[i].g;
		c[i].c = f_cfVecPages[i].b;
	}
	vsgPaletteWrite((VSGLUTBUFFER*)(c), 1, f_cfVecPages.size());
	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);

	// set up pages
	for (i=0; i<f_cfVecPages.size(); i++)
	{
		vsgSetDrawPage(vsgVIDEOPAGE, i+1, i+1);
	}

	//Create a VSGPAGECYCLE buffer to hold the different positions in video memory
	//corresponding to our stimulus. We can only display about 2000 of them at a time,
	//due to VSG limitations.

	VSGCYCLEPAGEENTRY *pe = new VSGCYCLEPAGEENTRY[f_cfVecPages.size()];
	for (i=0; i<f_cfVecPages.size(); i++)
	{
		pe[i].Page = (i + 1) + vsgTRIGGERPAGE;
		pe[i].Frames = (WORD)(nframerate*f_cfVecPages[i].secs);
		pe[i].Xpos=0;
		pe[i].Ypos=0;
		pe[i].Stop = 0;
	}

	vsgPageCyclingSetup(f_cfVecPages.size(), pe);
	
	return istatus;
}
	
int args(int argc, char **argv)
{	
	bool have_d=false;
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;

	while ((c = getopt(argc, argv, "p:Ar:L")) != -1)
	{
		switch (c) 
		{
		case 'A':
			{
				f_bAutoStart = true;
				break;
			}
		case 'L':
			{
				f_bUseLockFile = false;
				break;
			}
		case 'r':
			{
				s.assign(optarg);
				if (parse_integer(s, f_cycleRepeats))
				{
					cerr << "Error - cannot parse repeats (-r): must be integer.";
					errflg++;
				}
				break;
			}
		case 'p':
			{
				vector<string> tokens;
				s.assign(optarg);
				tokenize(s, tokens, ",");
				cout << "Parsed " << tokens.size() << " tokens." << endl;
				if (tokens.size()%4 != 0) 
				{
					cout << "Error - pages arg must have a multiple of 4 parts (r,g,b,seconds[,r,g,b,seconds[...]]" << endl;
					errflg++;
				}
				else
				{
					for (unsigned int i=0; i<tokens.size()/4; i++)
					{
						CFStruct cfs;
						if (parse_double(tokens[i*4], cfs.r) != 0)
						{
							cerr << "Parse error, token " << tokens[i*4] << endl;
							errflg++;
						}
						if (parse_double(tokens[i*4+1], cfs.g) != 0)
						{
							cerr << "Parse error, token " << tokens[i*4+1] << endl;
							errflg++;
						}
						if (parse_double(tokens[i*4+2], cfs.b) != 0)
						{
							cerr << "Parse error, token " << tokens[i*4+2] << endl;
							errflg++;
						}
						if (parse_double(tokens[i*4+3], cfs.secs) != 0)
						{
							cerr << "Parse error, token " << tokens[i*4+3] << endl;
							errflg++;
						}
						else
							f_cycletimeS += cfs.secs;
						if (errflg) break;
						else f_cfVecPages.push_back(cfs);
					}
				}
				break;
			}
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

	if (errflg) 
	{
		usage();
	}
	return errflg;
}


void usage()
{
	cerr << "usage: colorflash [-A] [-n] -p r1,g1,b1,seconds_1[,r2,g2,b2,seconds_2[...]]" << endl;
	cerr << "       pages (-p) arg must be in groups of 4. Cannot exceed number of video pages available." << endl;
	cerr << "       0<=rgb<=1." << endl;
	return;
}
