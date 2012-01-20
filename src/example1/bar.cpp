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

COLOR_TYPE f_background = {black, { 0, 0, 0}};
int f_iDistanceToScreenMM = 1000;

int main(int argc, char **argv)
{

	if (ARvsg::instance().init(f_iDistanceToScreenMM, f_background))
//	if (ARvsg::instance().init(0, f_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}

	// Initialize page 0 and leave display there. The stim set may safely assume that the current 
	// page is a blank background page and it may return to it (no drawing there!). 
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);

	ARContrastRectangleSpec rect;
	rect.x = rect.y = 0;
	rect.w = 10;
	rect.h = 2;
	rect.color.type = red;
	rect.color.color.a = 0; rect.color.color.b = rect.color.color.c = 0;
	rect.orientation = 0;
	rect.init(1);
	rect.setContrast(100);
	rect.draw();
	vsgPresent();

	string s;
	cout << "Hit key to set contrast to 0." << endl;
	cin >> s;
	rect.setContrast(0);
	vsgPresent();
	cout << "Hit key to exit." << endl;
	cin >> s;
	return 0;
}