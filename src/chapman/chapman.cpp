#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__

#include "VSGV8.H"
#include "alertlib.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")


// Useful consts for drawing
#define BACKGROUND_LEVEL 0
#define BLACK_LEVEL 1
#define WHITE_LEVEL 2
#define BACKGROUND_PAGE 0
#define BLACK_PAGE 1
#define WHITE_PAGE 2


int args(int argc, char **argv);
static void usage();
int init_vsg();
void do_tests();
void do_stim();

using namespace std;

int m_dScreenDistanceMM=0;	// screen dist in MM. Required! 
int m_nBlocks=0;			// Number of [blank-(b/w repeat)] cycles to run
int m_nRepeats=0;			// Number of (b/w repeat)'s in a block. 
double m_dPeriod=3;			// Period for b/w cycle. Also length of gray screen at start of block
double m_dDiameter=1;		// Diameter of circle, degrees. 
bool m_verbose=false;
bool m_testing=false;

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
			cout << "Screen distance " << m_dScreenDistanceMM << endl;
			cout << "# blocks: " << m_nBlocks << " # b/w repeats: " << m_nRepeats << endl;
			cout << "Period: " << m_dPeriod << endl;
			cout << "Diameter: " << m_dDiameter << endl;
		}
	}


	if (init_vsg())
	{
		return 1;
	}


	if (m_testing) 
		do_tests();
	else
		do_stim();

	return 0;
}



void do_stim()
{
	// Look for ON signal on digital input line 0x2
	// Look for OFF signal on digital input line 0x4
	vsgSetCommand(vsgCYCLEPAGEENABLE);

}



void do_tests()
{
	
	string s;
	cout << "Enter page: ";
	cin >> s;
	while (s != "q" && s!= "Q")
	{
		if (s == "c" || s=="C")
		{
			vsgSetCommand(vsgCYCLEPAGEENABLE);
		}
		else if (s=="d" || s=="D") 
		{
			vsgSetCommand(vsgCYCLEPAGEDISABLE);
		}
		else
		{
			istringstream iss(s);
			int i;
			iss >> i;
			if (i>=0 && i<3) 
			{
				vsgSetZoneDisplayPage(vsgVIDEOPAGE, i);
			}
		}
		cout << "Enter page: ";
		cin >> s;
	}

}




int init_vsg()
{
	int status=0;
	VSGTRIVAL gray = {0.5, 0.5, 0.5};
	VSGTRIVAL black = {0.0, 0.0, 0.0};
	VSGTRIVAL white = {1.0, 1.0, 1.0};

	status = vsgInit("");
	vsgSetSpatialUnits(vsgDEGREEUNIT);
	vsgSetViewDistMM(m_dScreenDistanceMM);
	vsgPaletteWrite((VSGLUTBUFFER*)(&gray), BACKGROUND_LEVEL, 1);
	vsgPaletteWrite((VSGLUTBUFFER*)(&black), BLACK_LEVEL, 1);
	vsgPaletteWrite((VSGLUTBUFFER*)(&white), WHITE_LEVEL, 1);

	vsgSetDrawPage(vsgVIDEOPAGE, BACKGROUND_PAGE, BACKGROUND_LEVEL);

	vsgSetDrawPage(vsgVIDEOPAGE, BLACK_PAGE, BACKGROUND_LEVEL);
	vsgSetPen1(BLACK_LEVEL);
	vsgDrawOval(0, 0, m_dDiameter, m_dDiameter);
	
	vsgSetDrawPage(vsgVIDEOPAGE, WHITE_PAGE, BACKGROUND_LEVEL);
	vsgSetPen1(WHITE_LEVEL);
	vsgDrawOval(0, 0, m_dDiameter, m_dDiameter);

	//Create a VSGPAGECYCLE buffer to hold the different positions in video memory
	//corresponding to our stimulus. We can only display about 2000 of them at a time,
	//due to VSG limitations.

	int nperiod = (int)(m_dPeriod*1000000/(double)vsgGetSystemAttribute(vsgFRAMETIME));
	int n_half_period = nperiod/2;
	int npages = 1+2*m_nRepeats;
	VSGCYCLEPAGEENTRY *pe = new VSGCYCLEPAGEENTRY[npages];
	int i;

	// background page
	pe[0].Page = BACKGROUND_PAGE + vsgTRIGGERPAGE;
	pe[0].Xpos=0;
	pe[0].Ypos=0;
	pe[0].Frames = nperiod;
	pe[0].Stop = 0;

	for (i=0; i<m_nRepeats; i++)
	{
		pe[1+i*2].Page = WHITE_PAGE + vsgTRIGGERPAGE;
		pe[1+i*2].Xpos=0;
		pe[1+i*2].Ypos=0;
		pe[1+i*2].Frames = n_half_period;
		pe[1+i*2].Stop = 0;

		pe[1+i*2+1].Page = BLACK_PAGE + vsgTRIGGERPAGE;
		pe[1+i*2+1].Xpos=0;
		pe[1+i*2+1].Ypos=0;
		pe[1+i*2+1].Frames = n_half_period;
		pe[1+i*2+1].Stop = 0;
	}

	vsgPageCyclingSetup(npages, pe);
	
	return status;
}

	
int args(int argc, char **argv)
{	
	bool have_d=false;
	bool have_s=false;
	bool have_n=false;
	bool have_m=false;
	bool have_T=false;
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;

	while ((c = getopt(argc, argv, "vd:hn:m:T:s:t")) != -1)
	{
		switch (c) 
		{
		case 'v':
			m_verbose = true;
			break;
		case 't':
			m_testing = true;
			break;
		case 'd':
			s.assign(optarg);
			if (parse_double(s, m_dDiameter)) errflg++;
			else have_d=true;
			break;
		case 's':
			s.assign(optarg);
			if (parse_integer(s, m_dScreenDistanceMM)) errflg++;
			else have_s=true;
			break;
		case 'n':
			s.assign(optarg);
			if (parse_integer(s, m_nBlocks)) errflg++;
			else have_n=true;
			break;
		case 'm':
			s.assign(optarg);
			if (parse_integer(s, m_nRepeats)) errflg++;
			else have_m=true;
			break;
		case 'T':
			s.assign(optarg);
			if (parse_double(s, m_dPeriod)) errflg++;
			else have_T=true;
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

	if (!have_d) 
	{
		cerr << "Diameter (-d) not specified!" << endl; 
		errflg++;
	}
	if (!have_s)
	{
		cerr << "Screen distance (-s) not specified!" << endl; 
		errflg++;
	}
	if (!have_n)
	{
		cerr << "Number of blocks (-n) not specified!" << endl;
		errflg++;
	}
	if (!have_m)
	{
		cerr << "Number of repeats (-m) not specified!" << endl;
		errflg++;
	}
	if (!have_T)
	{
		cerr << "Period (-T) not specified!" << endl;
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
	cerr << "usage: onoff -n <#blocks> -m <#repeats> -T <period(s)> -s <screen_dist_mm> -d <circle_diam_degrees> [-h] [-v]" << endl;
	return;
}
