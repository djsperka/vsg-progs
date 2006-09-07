#include "VSGV8.H"
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__
#include <iostream>
#include <cstdio>
#include <cmath>
#include "alertlib.h"

using namespace std;
using namespace alert;

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")

int f_iZoom = 4;



int main(int argc, char **argv)
{
	int istatus=0;
	string s;
	string prompt;
	VSGLUTBUFFER overlayLUT;
	int i, j;

	// Init vsg card
	istatus = vsgInit("");


	// setup vsg
	vsgSetDrawOrigin(0,0);
	vsgSetDrawMode(vsgSOLIDFILL);			// default is vsgCENTREXY! This makes it top-left-hand-corner origin
	vsgSetCommand(vsgVIDEOCLEAR);
	vsgSetCommand(vsgPALETTERAMP);
	vsgSetVideoMode(vsgPANSCROLLMODE);
	vsgSetCommand(vsgOVERLAYMASKMODE);		// makes overlay pages visible
	vsgSetCommand(vsgVIDEODRIFT+vsgOVERLAYDRIFT);			// allows us to move the offset of video memory



	cout << "System has " << vsgGetSystemAttribute(vsgNUMOVERLAYPAGES) << " overlay pages." << endl;
	cout << "System has " << vsgGetSystemAttribute(vsgNUMVIDEOPAGES) << " video pages." << endl;

	// setup overlay lut
	overlayLUT[1].a=1; overlayLUT[1].b=overlayLUT[1].c=0;
	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&overlayLUT[1], 1, 1);

	// set up overlay pages
	vsgSetDrawPage(vsgOVERLAYPAGE, 1, 1);	// page 1 is red
	vsgSetPen1(0);
	vsgDrawRect(0, 0, 16, 16);				// clear rect
	
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 0);	// page 0 is clear



	vsgSetDrawPage(vsgVIDEOPAGE,0,0);
	vsgSetPen1(255);
	vsgSetPen2(0);


	for (i=40; i<800; i++)
	{
		vsgDrawPixel(i, 0);		// y=0; top line in video memory
		vsgDrawPixel(i, 599);	// y=599; bottom line; screen height is 600
//		vsgDrawPixel(i, 600);
//		vsgDrawPixel(i, 1);
	}

	for (j=40; j<600; j++)
	{
		vsgDrawPixel(0, j);		// x=0; left hand side
		vsgDrawPixel(799, j);	// x=799; right hand side; screen width is 800
		vsgDrawPixel(803, j);
		vsgDrawPixel(807, j);
		vsgDrawPixel(811, j);
	}


	// draw a checkerboard of 4x4 squares in upper l.h. corner
	for (i=0; i<16; i+=8)
		for (j=0; j<16; j+=8)
			vsgDrawRect(i,j,4,4);
	for (i=4; i<20; i+=8)
		for (j=4; j<20; j+=8)
			vsgDrawRect(i,j,4,4);


	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);



	prompt = "m/o/d/q: ";

	cout << prompt;
	cin >> s;

	while (s != "q")
	{
		if (s == "m")
		{
			int x,y;
			cout << "Enter x y for move screen: ";
			cin >> x >> y;
			cout << "x=" << x << " y=" << y << endl;
			vsgMoveScreen(x, y);
		}
		else if (s == "o")
		{
			int p;
			cout << "Enter overlay page (0-1): ";
			cin >> p;
			vsgSetZoneDisplayPage(vsgOVERLAYPAGE, p);
		}
		else if (s == "d")
		{
			cout << "Overlay, Video, Both? ";
			cin >> s;
			if (s == "O" || s == "o")
			{
				vsgSetCommand(vsgOVERLAYDRIFT);
			}
			else if (s == "V" || s == "v")
			{
				vsgSetCommand(vsgVIDEODRIFT);
			}
			else
			{
				vsgSetCommand(vsgVIDEODRIFT+vsgOVERLAYDRIFT);
			}
		}

		cout << prompt;
		cin >> s;
	}



	return 0;
}