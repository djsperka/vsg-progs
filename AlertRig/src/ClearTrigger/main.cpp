// $Id: main.cpp,v 1.1 2005-09-20 00:45:51 dan Exp $
//
//
#include "VSGEX2.H"
#include "Alertlib.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

using namespace alert;
using namespace std;

int main(int argc, char **argv)
{
	COLOR_TYPE background = gray;
	double dist=999;

	// INit vsg
	if (ARvsg::instance().init(dist, background))
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

	return 0;
}