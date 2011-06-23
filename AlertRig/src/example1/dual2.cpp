// Variation of dual.cpp. 
// this version creates two fixation points using contrast to turn them on/off. 
// simple keyboard input turns them on/off - trying to find a way around the slooooooow
// response of application to vsgPresent() on slave VSG. 

#include <iostream>
#include <string>
#include "VSGV8.h"
#pragma comment (lib, "vsgv8.lib")
using namespace std;

int main(int argc, char **argv)
{
	int master_device, slave_device;
	VSGTRIVAL bg, color1, slavecolor2, mastercolor2;
	VSGOBJHANDLE master_obj, slave_obj;
	// background is gray
	bg.a = bg.b = bg.c = 0.5;

	// fixpt cotrast 0 color is gray
	color1.a = color1.b = color1.c = 0.5;

	// endpoint of slave fixpt color vector is red
	slavecolor2.a = 1; slavecolor2.b = slavecolor2.c = 0;

	// endpoint of slave master color vector is green
	mastercolor2.b = 1; mastercolor2.a = mastercolor2.c = 0;

	// CHECK COMMAND LINE ARGS
	if (argc != 3)
	{
		cerr << "give names of config files (slave then master) on command line." << endl;
		return -1;
	}
	else
	{
		cerr << "Slave: " << argv[1] << " Master: " << argv[2] << endl;
	}


	// init slave vsg
	slave_device = vsgAdvancedInit(argv[1], vsgADVINITSLAVEVIDEO);
	if (slave_device < 0)
	{
		cerr << "Error initializing slave video \"" << argv[1] << "\"(" << slave_device << ")" << endl;
		return -1;
	}

	// draw slave fixpt. Set contrast to 0 so its initially NOT seen
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	slave_obj = vsgObjCreate();
	vsgObjSetPixelLevels(1, 2);
	vsgSetBackgroundColour(&bg);
	vsgObjSetColourVector(&color1, &slavecolor2, vsgUNIPOLAR);
	vsgObjSetContrast(0);

	vsgSetPen1(1);
	vsgSetPen2(2);

	vsgDrawOval(0, 0, 100, 100);

	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_TOGGLEMODE, 0, 0);

	// Init second card. This is the master. 
	master_device = vsgInit(argv[2]);
	if (master_device < 0)
	{
		cerr << "Error initializing master \"" << argv[2] << "\"(" << master_device << ")" << endl;
		return -1;
	}

	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	master_obj = vsgObjCreate();
	vsgObjSetPixelLevels(1, 2);
	vsgSetBackgroundColour(&bg);
	vsgObjSetColourVector(&color1, &mastercolor2, vsgUNIPOLAR);
	vsgObjSetContrast(0);

	vsgSetPen1(1);
	vsgSetPen2(2);

	vsgDrawOval(0, 0, 100, 100);

	// present stuff on master
	vsgPresent();

	// select slave and present
	vsgInitSelectDevice(slave_device);
	vsgPresent();

	// input loop
	string s;
	while (s != "q")
	{
		cout << "Enter command: ";
		cin >> s;
		if (s=="M")
		{
			vsgInitSelectDevice(master_device);
			vsgObjSelect(master_obj);
			vsgObjSetContrast(100);
			cout << "master present...";
			vsgPresent();
			cout << "done." << endl;
		}
		else if (s == "m")
		{
			vsgInitSelectDevice(master_device);
			vsgObjSelect(master_obj);
			vsgObjSetContrast(0);
			cout << "master present...";
			vsgPresent();
			cout << "done." << endl;
		}
		else if (s=="S")
		{
			vsgInitSelectDevice(slave_device);
			vsgObjSelect(slave_obj);
			vsgObjSetContrast(100);
			cout << "slave present...";
			vsgPresent();
			cout << "done." << endl;
		}
		else if (s == "s")
		{
			vsgInitSelectDevice(slave_device);
			vsgObjSelect(slave_obj);
			vsgObjSetContrast(0);
			cout << "slave present...";
			vsgPresent();
			cout << "done." << endl;
		}
		else
		{
			cout << "Unknown command \"" << s << "\"" << endl;
		}
	}

	vsgInitSelectDevice(master_device);
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	vsgPresent();
	vsgInitSelectDevice(slave_device);
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	vsgPresent();

	return 0;
}