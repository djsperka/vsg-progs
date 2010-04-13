#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <conio.h>
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__

#include "vsgv8.h"
#include "Alertlib.h"
#include "AlertUtil.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment (lib, "vsgv8.lib")

int args(int argc, char **argv);

using namespace std;
using namespace alert;

ARGratingSpec m_grating;
int m_screenDistanceMM=0;
bool m_verbose=false;
COLOR_TYPE m_background = {gray, {0.5, 0.5, 0.5}};

int args(int argc, char **argv);
void usage();

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
			cout << "Screen distance " << m_screenDistanceMM << endl;
			cout << "Background color " << m_background << endl;
			cout << "Grating " << m_grating << endl;
		}
	}

	// INit vsg
	if (ARvsg::instance().init(m_screenDistanceMM, m_background,false))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}

	// draw grating fullscreen
	cout << "Hit S to start stimuli ...." << endl;
	while (true)
	{
		if (_kbhit() &&	('S' == _getch())) break;
		Sleep(100);
	}


	m_grating.init(50);
	arutil_draw_grating_fullscreen(m_grating, 0);
	vsgObjSetTriggers(vsgTRIG_DRIFTVEL, 0, 0);
	vsgPresent();


	cout << "Hit q to stop ...." << endl;
	while (true)
	{
		if (_kbhit() &&	('q' == _getch())) break;
		Sleep(100);
	}

	vsgObjDestroy(m_grating.handle());
	vsgSetDrawPage(vsgVIDEOPAGE, 0, 0);
	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);

	return 0;
}


int args(int argc, char **argv)
{	
	bool have_d=false;
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "s:d:")) != -1)
	{
		switch (c) 
		{
		case 'd':
			s.assign(optarg);
			if (parse_distance(s, m_screenDistanceMM)) errflg++;
			else have_d=true;
			break;
		case 's':
			s.assign(optarg);
			if (parse_grating(s, m_grating)) errflg++;
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
	cerr << "usage: meadrift -d screen_distance_MM -s x,y,w,h,contrast%,sf,tf,orientation,color_vector,s|q,r|e" << endl;
}
