#include "alertlib.h"
#include <iostream>

#pragma comment(lib, "vsgv8.lib")

using namespace std;
using namespace alert;

int main(int argc, char **argv)
{
	// Init vsg
	if (ARvsg::instance().init(500, gray))
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
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	

	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0xff, 0);
	vsgPresent();
	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x00, 0);
	vsgPresent();


	cout << "Ready" << endl;

	long inputs=0;
	long last_inputs=0;
	while (1)
	{
		inputs = vsgIOReadDigitalIn() & 0xff;
		if (inputs != last_inputs)
		{
			last_inputs = inputs;
			vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, inputs, 0);
			vsgPresent();
		}
	}


	return 0;
}
