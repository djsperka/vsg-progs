#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include "Alertlib.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")

using namespace std;
using namespace alert;



int main (int argc, char *argv[])
{
	string s;
	COLOR_TYPE background = gray;
	int dist = 555;

	ARvsg::instance().init(dist, gray);

	double dScrWidth = vsgGetScreenWidthPixels();
	double dScrHeight = vsgGetScreenHeightPixels();
	vsgUnit2Unit(vsgPIXELUNIT,dScrWidth,vsgDEGREEUNIT,&dScrWidth);
	vsgUnit2Unit(vsgPIXELUNIT,dScrHeight,vsgDEGREEUNIT,&dScrHeight);

	// Initialize and draw overlay page
	if (ARvsg::instance().init_overlay())
	{
		cerr << "VSG overlay initialization failed!" << endl;
		return 1;
	}

	// Set overlay page to all clear (level 0), then set level 1 to red, and draw a dot. 
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 0);


	VSGTRIVAL red;
	red.a=1; red.b = red.c = 0;
	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&red, 1, 1);
	vsgSetPen1(1);
	vsgDrawOval(0, 0, 1, 1);

	vsgSetPen1(0);
	vsgDrawOval(2,3,4,4);


	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	vsgSetDisplayPage(0);

	ARGratingSpec gr;
	ostringstream oss;
	oss << "0,0," << dScrWidth << "," << dScrHeight << ",100,3,3,30,b,s,e";
	if (parse_grating(oss.str(), gr))
	{
		cerr << "Error parsing gr string: " << oss.str() << endl;
	}

	// try draw page 1
	vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);
	vsgSetSpatialUnits(vsgPIXELUNIT);
	vsgSetCommand(vsgVIDEODRIFT + vsgPANSCROLLMODE);

	gr.init(40);
	gr.draw(true);
	vsgPresent();
	vsgSetDisplayPage(1);

	cout << "Enter move coords: "; 
	cin >> s;
	while (s != "q" && s!= "Q")
	{


		if (s == "m" || s=="M")
		{
			while (1)
			{
				double ax, ay;
				double mx, my;
				POINT pos;
				GetCursorPos(&pos);
				ax = pos.x / uScreenWidth;
				ay = pos.y / uScreenHeight;
//				mx = floor(1024 - ax*800);
//				my = floor(600 - ay*600);
				mx = floor(400 - ax*800);
				my = floor(300 - ay*600);
				if (mx < 0) mx = 0;
				if (my < 0) my = 0;
				cout << "xm=" << mx << " ym=" << my << endl;
				if (vsgMoveScreen(mx, my) < 0)
				{
					cerr << "Command failed" <<endl;
				}
			}
		}
		else
		{
			double x, y;
			if (!parse_xy(s, x, y))
			{
				cout << "Moving " << x << " " << y << endl;
				if (vsgMoveScreen(x, y) < 0)
				{
					cerr << "Command failed" <<endl;
				}
			}
		}
		cout << "Enter move dist or q: ";
		cin >> s;
	}

	ARvsg::instance().clear(0);
	
	return 0;
}
