#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__

#include "vsgv8.h"
#include "Alertlib.h"
#include "AlertUtil.h"
using namespace std;
using namespace alert;

COLOR_TYPE f_background(black);
int f_iDistanceToScreenMM = 1000;

int main(int argc, char **argv)
{

	if (ARvsg::instance().init(f_iDistanceToScreenMM, f_background, false))
//	if (ARvsg::instance().init(0, f_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}

	// Initialize page 0 and leave display there. The stim set may safely assume that the current 
	// page is a blank background page and it may return to it (no drawing there!). 
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);

	vsgSetDrawMode(0);
	vsgSetSpatialUnits(vsgPIXELUNIT);
	//vsgSetDrawOrigin(0, 0);

	ARContrastRectangleSpec rect;
	rect.x = rect.y = 0;
	rect.w = 1024;
	rect.h = 384;
	rect.color = red;
	rect.orientation = 0;
	rect.init(1);
	rect.setContrast(100);
	rect.draw();
	vsgPresent();

	string s;

	cout << "Hit key [cpq]: ";
	cin >> s;
	while (s != "q")
	{
		if (s == "c")
		{
			rect.setContrast(0);
			vsgPresent();
		}
		else if (s == "p")
		{
			uint8_t pix[1024];
			int i = 0;
			int status;
			cout << "Enter pixel line: ";
			cin >> i;
			if (i < 0 || i >= vsgGetScreenHeightPixels())
			{
				i = 0;
				cout << "Err input - using line 0" << endl;
			}

			status = vsgReadPixelLine(0, i, pix, 1024);
			cout << "status " << status << endl;
			for (int j = 0; j < 1024; j++)
			{
				cout << (unsigned int)pix[j] << ",";
			}
			cout << endl;
		}
		else if (s == "x")
		{
			uint8_t pix[1024];
			int i = 0;
			int x = 0;
			int status;
			cout << "Enter pixel line: ";
			cin >> i;
			if (i < 0 || i >= vsgGetScreenHeightPixels())
			{
				i = 0;
				cout << "Err input - using line 0" << endl;
			}


			cout << "Enter x: ";
			cin >> x;



			status = vsgReadPixelLine(x, i, pix, 1024);
			cout << "status " << status << endl;
			for (int j = 0; j < 1024; j++)
			{
				cout << (unsigned int)pix[j] << ",";
			}
			cout << endl;
		}
		cout << "Hit key [cpq]: ";
		cin >> s;
	}
	return 0;
}