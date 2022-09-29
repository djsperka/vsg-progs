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


int UStim::initialize(alert::ARvsg& vsg, int iScreenDistanceMM, const COLOR_TYPE& background, const std::string& gammaFile)
{
	int status = vsg.init(iScreenDistanceMM, background, false, false, gammaFile);
	if (status != 0)
	{
		cerr << "VSG init failed!!" << endl;
	}
	else
	{
		double xdeg, ydeg;
		vsgUnit2Unit(vsgPIXELUNIT, vsgGetSystemAttribute(vsgSCREENWIDTH), vsgDEGREEUNIT, &xdeg);
		vsgUnit2Unit(vsgPIXELUNIT, vsgGetSystemAttribute(vsgSCREENHEIGHT), vsgDEGREEUNIT, &ydeg);

		// dump some useful diagnostics
		cerr << "vsgDEVICESERIALNUMBER: " << std::hex << vsgGetSystemAttribute(vsgDEVICESERIALNUMBER) << std::dec << endl;
		cerr << "vsgFRAMERATE(Hz): " << vsgGetSystemAttribute(vsgFRAMERATE) << endl;
		cerr << "vsgFRAMETIME(us): " << vsgGetSystemAttribute(vsgFRAMETIME) << endl;
		cerr << "vsgSCREENWIDTH: " << vsgGetSystemAttribute(vsgSCREENWIDTH) << " (" << xdeg << " deg., screen dist " << iScreenDistanceMM << "mm)" << endl;
		cerr << "vsgSCREENHEIGHT: " << vsgGetSystemAttribute(vsgSCREENHEIGHT) << " (" << ydeg << " deg., screen dist " << iScreenDistanceMM << "mm)" << endl;
		cerr << "vsgNUMVIDEOPAGES: " << vsgGetSystemAttribute(vsgNUMVIDEOPAGES) << endl;

		// Initialize page 0 and leave display there. The stim set may safely assume that the current 
		// page is a blank background page and it may return to it (no drawing there!). 
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);
	}
	return status;
}

