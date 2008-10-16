// $Id: main.cpp,v 1.3 2008-10-16 23:42:39 devel Exp $
//
//
#include "vsgv8.h"
#include "Alertlib.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")

using namespace alert;
using namespace std;

int main(int argc, char **argv)
{
	COLOR_TYPE background = {gray, {0, 0, 0}};
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