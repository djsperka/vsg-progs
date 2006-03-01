#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__

#include "VSGV8.H"
#include "util.h"


#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")


int args(int argc, char **argv);
static void usage();
int init();

using namespace std;
int m_screenDistanceMM=0;
bool m_verbose=true;
typedef struct colorflash_struct {
	double r, g, b;
	double secs;
} CFStruct;

vector<CFStruct> cfVecPages;

int main (int argc, char *argv[])
{

	// Check input arguments
	if (args(argc, argv))
	{
		return 1;
	}
	else
	{
		if (m_verbose)
		{
			cout << "colorflash - " << cfVecPages.size() << " flash pages." << endl;
			for (int i=0; i<cfVecPages.size(); i++)
			{
				cout << "Page " << i+1 << " (r,g,b) = (" << cfVecPages[i].r << "," << cfVecPages[i].g << "," << cfVecPages[i].b << ") seconds=" << cfVecPages[i].secs << endl;
			}
		}
	}

	if (init()) 
	{
		cerr << "Error in vsg init and/or page cycling setup." << endl;
	}


	char c[10];
	cout << "Hit any key to start." << endl;
	cin.getline(c, 10);

#if 0
	for (int i=0; i<cfVecPages.size(); i++)
	{
		cout << " Hit a key to show page " << i+1 << endl;
		cin.getline(c, 10);
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, i+1);
	}
#endif

	vsgSetCommand(vsgCYCLEPAGEENABLE);

	cout << "Hit any key to stop." << endl;
	cin.getline(c, 10);

	vsgSetCommand(vsgCYCLEPAGEDISABLE);

	return 0;
}


int init()
{
	int istatus=0;
	VSGTRIVAL *c;
	int npages;
	int nframerate;
	int i;

	if (0 != (istatus = vsgInit(""))) return istatus;

	npages = vsgGetSystemAttribute(vsgNUMVIDEOPAGES);
	nframerate = vsgGetSystemAttribute(vsgFRAMERATE);

	if (npages < cfVecPages.size())
	{
		cerr << "Error - vsg has only " << npages << " video pages, but you requested " << cfVecPages.size() << " pages." << endl;
		return 1;
	}

	// set colors in palette. Color for page 'i' will be at LUT index 'i' (skip index 0!)
	vsgSetCommand(vsgPALETTECLEAR);
	c = new VSGTRIVAL[cfVecPages.size()];
	for (i=0; i<cfVecPages.size(); i++)
	{
		c[i].a = cfVecPages[i].r;
		c[i].b = cfVecPages[i].g;
		c[i].c = cfVecPages[i].b;
	}
	vsgPaletteWrite((VSGLUTBUFFER*)(c), 1, cfVecPages.size());
	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);

	// set up pages
	for (i=0; i<cfVecPages.size(); i++)
	{
		vsgSetDrawPage(vsgVIDEOPAGE, i+1, i+1);
	}

	//Create a VSGPAGECYCLE buffer to hold the different positions in video memory
	//corresponding to our stimulus. We can only display about 2000 of them at a time,
	//due to VSG limitations.

	VSGCYCLEPAGEENTRY *pe = new VSGCYCLEPAGEENTRY[cfVecPages.size()];
	for (i=0; i<cfVecPages.size(); i++)
	{
		pe[i].Page = (i + 1) + vsgTRIGGERPAGE;
		pe[i].Frames = nframerate*cfVecPages[i].secs;
		pe[i].Xpos=0;
		pe[i].Ypos=0;
		pe[i].Stop = 0;
	}

	vsgPageCyclingSetup(cfVecPages.size(), pe);
	
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

	while ((c = getopt(argc, argv, "p:")) != -1)
	{
		switch (c) 
		{
		case 'p':
		{
			vector<string> tokens;
			s.assign(optarg);
			mytokenize(s, tokens, ",");
			cout << "Parsed " << tokens.size() << " tokens." << endl;
			if (tokens.size()%4 != 0) 
			{
				cout << "Error - pages arg must have a multiple of 4 parts (r,g,b,seconds[,r,g,b,seconds[...]]" << endl;
				errflg++;
			}
			else
			{
				for (int i=0; i<tokens.size()/4; i++)
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
					if (errflg) break;
					else cfVecPages.push_back(cfs);
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
	cerr << "usage: colorflash -p r1,g1,b1,seconds_1[,r2,g2,b2,seconds_2[...]]" << endl;
	cerr << "       pages (-p) arg must be in groups of 4. Cannot exceed number of video pages available." << endl;
	return;
}
