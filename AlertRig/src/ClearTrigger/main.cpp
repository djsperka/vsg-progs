// $Id: main.cpp,v 1.7 2012-01-27 23:01:26 devel Exp $
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
void do_trig(int t);

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

	vsgSetPen2(vsgBACKGROUND);
	vsgSetCommand(vsgVIDEOCLEAR);

	do_trig(0xff);
	vsgPresent();

	do_trig(0x0);
	vsgPresent();

	do_trig(0x0);
	vsgPresent();
	do_trig(0x0);
	vsgPresent();
	do_trig(0x0);
	vsgPresent();

	do_trig(0xfe);
	vsgPresent();

	do_trig(0x0);
	vsgPresent();

	do_trig(0xfc);
	vsgPresent();

	do_trig(0x0);
	vsgPresent();

	do_trig(0x1);
	vsgPresent();
	do_trig(0x2);
	vsgPresent();
	do_trig(0x4);
	vsgPresent();
	do_trig(0x8);
	vsgPresent();
	do_trig(0x10);
	vsgPresent();
	do_trig(0x20);
	vsgPresent();
	do_trig(0x40);
	vsgPresent();
	do_trig(0x80);
	vsgPresent();
	do_trig(0x0);
	vsgPresent();
	do_trig(0x0);
	vsgPresent();

	return 0;
}


void do_trig(int t)
{
	if (IS_VISAGE)
	{
		vsgSetTriggerOptions(vsgTRIGOPT_PRESENT, 0, vsgTRIG_OUTPUTMARKER, 0.5, 0, t << 1, 0x1FE);
	}
	else
	{
		vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, t, 0);
	}
	return;
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
