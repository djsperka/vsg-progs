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
	string s, s2;
	COLOR_TYPE background = gray;
	int dist = 555;

	ARvsg::instance().init(dist, background);

	double dScrWidth = vsgGetScreenWidthPixels();
	double dScrHeight = vsgGetScreenHeightPixels();
	vsgUnit2Unit(vsgPIXELUNIT,dScrWidth,vsgDEGREEUNIT,&dScrWidth);
	vsgUnit2Unit(vsgPIXELUNIT,dScrHeight,vsgDEGREEUNIT,&dScrHeight);


	ARContrastFixationPointSpec fixpt;
	ARGratingSpec gr;
	ostringstream oss;
	oss << "0,0,100,100,100,3,3,30,b,s,e";
	if (parse_grating(oss.str(), gr))
	{
		cerr << "Error parsing gr string: " << oss.str() << endl;
	}
	oss << "0,0,1,green";
	if (parse_fixation_point("0,0,1,green", fixpt))
	{
		cerr << "Error parsing fixpt string" << endl;
	}
	fixpt.init(2, false);


	vsgSetCommand(vsgOVERLAYMASKMODE);

	// Set overlay page to all clear (level 0), then set level 1 to red, and draw a dot. 
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 0);
	fixpt.drawOverlay(1);

	//VSGTRIVAL red;
	//red.a=1; red.b = red.c = 0;
	//vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&red, 1, 1);
	//vsgSetPen1(1);
	//vsgDrawOval(0, 0, 1, 1);

	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	vsgSetDisplayPage(0);



	// try draw page 1
	vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);
	vsgSetSpatialUnits(vsgPIXELUNIT);
	vsgSetCommand(vsgVIDEODRIFT + vsgPANSCROLLMODE);

	gr.init(40);
	gr.draw(true);
	vsgPresent();
	vsgSetDisplayPage(1);

	cout << "Enter m(move fixpt),q,d,o: "; 
	cin >> s;
	while (s != "q" && s!= "Q")
	{
		if (s == "m")
		{
			cout << "Enter fixpt spec: ";
			cin >> s2;
			if (parse_fixation_point(s2, fixpt))
			{
				cerr << "Error parsing fixpt string, try again: ";
			}
			else
			{
				cout << "Moving fixpt to " << fixpt << endl;
				vsgSetCommand(vsgOVERLAYMASKMODE);
				vsgSetDrawPage(vsgOVERLAYPAGE, 0, 0);
				fixpt.drawOverlay(1);
			}
		}
		else if (s == "d")
		{
			cout << "vsgSetCommand(vsgOVERLAYDISABLE);" << endl;
			vsgSetCommand(vsgOVERLAYDISABLE);
		}
		else if (s == "o")
		{
			cout << "vsgSetCommand(vsgOVERLAYMASKMODE);" << endl;
			vsgSetCommand(vsgOVERLAYMASKMODE);
		}
		cout << "Enter m(move fixpt),q,d,o: ";
		cin >> s;
	}

	ARvsg::instance().clear(0);
	
	return 0;
}
