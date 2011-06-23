#include <iostream>
#include "VSGV8.h"
#pragma comment (lib, "vsgv8.lib")
using namespace std;

int main(int argc, char **argv)
{
	int device1, device2;
	int scrWidth, scrHeight;
	VSGTRIVAL bg, color1, color2;

	bg.a = bg.b = bg.c = 0.5;
	color1.a = color1.b = color1.c = 1;
	color2.a = color2.b = color2.c = 0;

	if (argc != 3)
	{
		cerr << "give names of config files (slave then master) on command line." << endl;
		return -1;
	}
	else
	{
		cerr << "Slave: " << argv[1] << " Master: " << argv[2] << endl;
	}


	device1 = vsgAdvancedInit(argv[1], vsgADVINITSLAVEVIDEO);
	if (device1 < 0)
	{
		cerr << "Error initializing slave video \"" << argv[1] << "\"(" << device1 << ")" << endl;
		return -1;
	}

	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	vsgObjCreate();
	vsgObjSetPixelLevels(1, 2);
	vsgSetBackgroundColour(&bg);
	vsgObjSetColourVector(&color1, &color2, vsgBIPOLAR);

	vsgSetPen1(1);
	vsgSetPen2(2);

	scrWidth = vsgGetScreenWidthPixels();
	scrHeight = vsgGetScreenHeightPixels();

	vsgDrawChessboard(0, 0, scrWidth, scrHeight, scrWidth/2, scrHeight/2, 0);

	// temp freq and triggers
	vsgObjSetTableSize(vsgTWTABLE, 140);
	vsgObjTableSquareWave(vsgTWTABLE, 0, 70);
	vsgObjSetTemporalFrequency(1);
	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0xff, 0xff);

	// Init second card. This is the master. 
	device2 = vsgInit(argv[2]);
	if (device2 < 0)
	{
		cerr << "Error initializing master \"" << argv[2] << "\"(" << device2 << ")" << endl;
		return -1;
	}

	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	vsgObjCreate();
	vsgObjSetPixelLevels(1, 2);
	vsgSetBackgroundColour(&bg);
	vsgObjSetColourVector(&color1, &color2, vsgBIPOLAR);

	vsgSetPen1(1);
	vsgSetPen2(2);

	scrWidth = vsgGetScreenWidthPixels();
	scrHeight = vsgGetScreenHeightPixels();

	vsgDrawChessboard(0, 0, scrWidth, scrHeight, scrWidth/2, scrHeight/2, 0);

	// temp freq and triggers
	vsgObjSetTableSize(vsgTWTABLE, 140);
	vsgObjTableSquareWave(vsgTWTABLE, 0, 70);
	vsgObjSetTemporalFrequency(1);
	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0XFF, 0xff);

	// present stuff on master
	vsgPresent();

	// select slave and present
	vsgInitSelectDevice(device1);
	vsgPresent();

	return 0;
}