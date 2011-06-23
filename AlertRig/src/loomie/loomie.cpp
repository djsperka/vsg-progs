/* $Id: loomie.cpp,v 1.2 2011-06-23 02:09:34 djsperka Exp $ */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__

#include "vsgv8.h"
#include "Alertlib.h"
#include "AlertUtil.h"
using namespace std;
using namespace alert;

bool f_bMaster = false;
bool f_bSlave = false;
bool f_bDefault = true;
int f_iDistanceToScreenMM = 500;
COLOR_TYPE f_background = { gray, {0.5, 0.5, 0.5}};

// libs

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif
#pragma comment (lib, "vsgv8.lib")


// function prototypes

int args(int argc, char **argv);
void usage();

int main (int argc, char *argv[])
{

	// Check input arguments
	if (args(argc, argv))
	{
		return 1;
	}

	// Initialize vsg
	if (f_bMaster)
	{
		if (ARvsg::master().init(f_iDistanceToScreenMM, f_background))
		{
			cerr << "VSG init for master failed!" << endl;
			return -1;
		}
		ARvsg::master().select();
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);
	}
	else if (f_bSlave)
	{
		if (ARvsg::slave().init(f_iDistanceToScreenMM, f_background))
		{
			cerr << "VSG init for slave failed!" << endl;
			return -1;
		}
		ARvsg::slave().select();
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);
	}
	else
	{
		if (ARvsg::instance().init(f_iDistanceToScreenMM, f_background))
		{
			cerr << "VSG init for slave failed!" << endl;
			return -1;
		}
		ARvsg::instance().select();
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);
	}


	/* 
	 * Loop, wait for input. 
	 */

	float f;
	string s;
	bool bQuit = false;
	VSGTRIVAL c;
	do 
	{
		cout << "Enter lum value (0-1) or q to quit: ";
		if (!(cin >> f))
		{
			// Bad input. Clear cin and check if its a q
			cin.clear();
			cin >> s;
			bQuit = (s == "q");
		}
		else if (f < 0 || f > 1)
		{
			cout << "Enter a value between 0 and 1." << endl;
		}
		else
		{
			c.a = c.b = c.c = f;
			vsgSetBackgroundColour(&c);
			vsgPresent();
		}
	} 
	while (!bQuit);
	cout << "Quitting." << endl;
	return 0;
}

int args(int argc, char **argv)
{	
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "MSD")) != -1)
	{
		switch (c) 
		{
		case 'S':
			f_bMaster = false;
			f_bSlave = true;
			f_bDefault = false;
			break;
		case 'M':
			f_bMaster = true;
			f_bSlave = false;
			f_bDefault = false;
			break;
		case 'D':
			f_bMaster = false;
			f_bSlave = false;
			f_bDefault = true;
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

	if (errflg) 
	{
		usage();
	}
	return errflg;
}

void usage()
{
	cerr << "usage: loomie [-D|M|S]" << endl;
}

