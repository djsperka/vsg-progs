// $Id: main.cpp,v 1.6 2011-08-30 01:16:43 djsperka Exp $
//
//
#include "vsgv8.h"
#include "Alertlib.h"
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__


#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")

bool f_bUseLock = true;
int args(int argc, char **argv);

using namespace alert;
using namespace std;

int main(int argc, char **argv)
{
	COLOR_TYPE background = {gray, {0, 0, 0}};
	int dist=999;

	if (args(argc, argv))
	{
		cerr << " Bad command line args." << endl;
		return -1;
	}

	// INit vsg
	if (ARvsg::instance().init(dist, background, f_bUseLock))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}


	// initialize video pages
	if (ARvsg::instance().init_video())
	{
		cerr << "VSG video initialization failed!" << endl;
		return 1;
	}
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);

	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0xFF, 0);
	vsgPresent();

	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x0, 0);
	vsgPresent();

	vsgObjSetTriggers(vsgTRIG_ONPRESENT, 0, 0);
	vsgPresent();
	vsgObjSetTriggers(vsgTRIG_ONPRESENT, 0, 0);
	vsgPresent();
	vsgObjSetTriggers(vsgTRIG_ONPRESENT, 0, 0);
	vsgPresent();

	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0xFE, 0);
	vsgPresent();

	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x0, 0);
	vsgPresent();

	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0xFC, 0);
	vsgPresent();

	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x0, 0);
	vsgPresent();

	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x1, 0);
	vsgPresent();
	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x2, 0);
	vsgPresent();
	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x4, 0);
	vsgPresent();
	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x8, 0);
	vsgPresent();
	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x10, 0);
	vsgPresent();
	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x20, 0);
	vsgPresent();
	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x40, 0);
	vsgPresent();
	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x80, 0);
	vsgPresent();
	vsgObjSetTriggers(vsgTRIG_ONPRESENT, 0, 0);
	vsgPresent();
	vsgObjSetTriggers(vsgTRIG_ONPRESENT, 0, 0);
	vsgPresent();

	return 0;
}



int args(int argc, char **argv)
{	
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "n")) != -1)
	{
		switch(c)
		{
			case 'n':
				f_bUseLock = false;
				break;
			default:
				cout << "Unknown arg: " << c << endl;
				break;
		}
	}
	return errflg;
}
