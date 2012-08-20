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
#include "stopwatch.h"

using namespace std;
using namespace alert;

#pragma comment (lib, "Winmm.lib")

COLOR_TYPE f_background = {gray, { 0.5, 0.5, 0.5}};
int f_iDistanceToScreenMM = 1000;


void pages(char *str)
{
	string s;
	VSGPAGEDESCRIPTOR desc;
	desc._StructSize = sizeof(VSGPAGEDESCRIPTOR);
	s.assign(str);
	cout << s << endl;
	cout << "video page " << vsgGetZoneDisplayPage(vsgVIDEOPAGE) << endl;
	vsgGetCurrentDrawPage(&desc);
	cout << "draw page " << (desc.PageZone==vsgVIDEOPAGE ? "VIDEOPAGE" : "OTHER???" ) << ", " << desc.Page << endl;
	return;
}

int main(int argc, char **argv)
{

	if (ARvsg::instance().init(f_iDistanceToScreenMM, f_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}

	// Initialize page 0 and leave display there. The stim set may safely assume that the current 
	// page is a blank background page and it may return to it (no drawing there!). 
	ARGratingSpec g0, g1;
	parse_grating(string("-5.0,0.0,5.0,5.0,0.0,0.0,100,0.10,1.00,90.0,0.0,b,s,e"), g0);
	parse_grating(string("-5.0,0.0,5.0,5.0,3.0,3.0,99,0.10,1.00,40.0,0.0,b,s,e"), g1);
	g0.init(30);
	g1.init(30);

	ARRandomGridSpec grid;
	grid.x = 5; 
	grid.y = 0;
	grid.w = grid.h = 6;
	grid.nc = grid.nr = 8;
	grid.init(5);

	ARXhairSpec xhair;
	xhair.x = 5;
	xhair.y = 0;
	xhair.ro = 3;
	xhair.rm = 2;
	xhair.ri = 1.5;
	xhair.r1 = 1;
	xhair.r2 = 1.4;
	xhair.nc = 32;
	xhair.init(6);

	ARRandomGridSpec grid2;
	grid2.x = 5; 
	grid2.y = 0;
	grid2.w = grid2.h = 6;
	grid2.nc = grid2.nr = 8;
	grid2.init(6);

	grid.setContrast(100);
	vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);
	grid.draw();
	//vsgPresent();
	g1.draw();
	vsgPresent();
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	g0.draw();
	//grid2.draw();
	xhair.draw();
	vsgPresent();

	string s;
	int ipage=0;
	int contrast = 100;
	cout << "Enter t,c,q: ";
	getline(cin, s);
	while (s != "q")
	{
		if (s == "c")
		{
			pages("Before setContrast");
			contrast -= 25;
			if (contrast < 0) contrast = 100;
			//grid.select();
			grid.setContrast(contrast);
			xhair.setContrast(contrast);
			vsgPresent();
			pages("After setContrast and Present()");
		}
		else if (s == "t")
		{
			ipage = 1-ipage;
			pages("before ZoneDisplayPage()");
			vsgSetZoneDisplayPage(vsgVIDEOPAGE, ipage);
			pages("after ZoneDisplayPage()");
		}
		cout << "Enter t,c,q: ";
		getline(cin, s);
	}

	return 0;
}