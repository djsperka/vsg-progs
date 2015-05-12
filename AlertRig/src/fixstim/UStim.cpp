#include "UStim.h"
#include "alertlib.h"
#include <iostream>

using namespace std;
using namespace alert;

bool UStim::parses(const std::string& args)
{
	int argc;
	char *argv[100] = {0};

	// Generate array of args for getopt
	make_argv(args, argc, argv);

	return parse(argc, argv);
}


int UStim::initialize(alert::ARvsg& vsg, int iScreenDistanceMM, const COLOR_TYPE& background)
{
	int status = vsg.init(iScreenDistanceMM, background, false);
	if (status != 0)
	{
		cerr << "VSG init failed!!" << endl;
	}
	else
	{
		// Initialize page 0 and leave display there. The stim set may safely assume that the current 
		// page is a blank background page and it may return to it (no drawing there!). 
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);
	}
	return status;
}