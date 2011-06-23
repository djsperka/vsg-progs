#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include "Alertlib.h"
#include "Alertutil.h"
#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

using namespace std;
using namespace alert;

long f_screenDistanceMM = 500;
COLOR_TYPE f_background = {gray, {}};
ARXhairSpec f_xhair;

int init_pages();
int draw_pie(int nc, PIXEL_LEVEL first, PIXEL_LEVEL second, double r, double astep);

int main(int argc, char **argv)
{
	string s;
	if (ARvsg::instance().init(500, f_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}

	f_xhair.x = f_xhair.y = 0;
	f_xhair.ro = 8;
	f_xhair.rm = 7;
	f_xhair.ri = 6;
	f_xhair.r1 = 1;
	f_xhair.r2 = 5.9;
	f_xhair.nc = 32;
	f_xhair.init(32);

	init_pages();

	cout << "Enter contrast or q: ";
	cin >> s;
	while (s != "q")
	{
		int contrast;
		contrast = atoi(s.c_str());
		f_xhair.setContrast(contrast);
		vsgPresent();
		cout << "Enter contrast or q: ";
		cin >> s;
	}
	return 0;
}

int init_pages()
{
	VSGLUTBUFFER buffer;
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	f_xhair.draw();
	f_xhair.setContrast(100);

	vsgPaletteRead(&buffer);
	cout << "buffer[" << f_xhair.getFirstLevel() << "]=" << 
		buffer[f_xhair.getFirstLevel()].a << "," << buffer[f_xhair.getFirstLevel()].b << "," << buffer[f_xhair.getFirstLevel()].c << endl; 
	cout << "buffer[" << f_xhair.getFirstLevel() + f_xhair.getNumLevels() - 1 << "]=" << 
		buffer[f_xhair.getFirstLevel() + f_xhair.getNumLevels() - 1].a << "," << buffer[f_xhair.getFirstLevel() + f_xhair.getNumLevels() - 1].b << "," << buffer[f_xhair.getFirstLevel() + f_xhair.getNumLevels() - 1].c  << endl;

	vsgPresent();

	vsgPaletteRead(&buffer);
	cout << "buffer[" << f_xhair.getFirstLevel() << "]=" << 
		buffer[f_xhair.getFirstLevel()].a << "," << buffer[f_xhair.getFirstLevel()].b << "," << buffer[f_xhair.getFirstLevel()].c << endl; 
	cout << "buffer[" << f_xhair.getFirstLevel() + f_xhair.getNumLevels() - 1 << "]=" << 
		buffer[f_xhair.getFirstLevel() + f_xhair.getNumLevels() - 1].a << "," << buffer[f_xhair.getFirstLevel() + f_xhair.getNumLevels() - 1].b << "," << buffer[f_xhair.getFirstLevel() + f_xhair.getNumLevels() - 1].c  << endl;
	vsgPresent();

	return 0;
}

#if 0

int main (int argc, char *argv[])
{

	// INit vsg
	vsgInit("");
	vsgSetViewDistMM(500);
	vsgSetSpatialUnits(vsgDEGREEUNIT);
	arutil_color_to_palette(f_background, 99);
	vsgSetDrawPage(vsgVIDEOPAGE, 0, 99);

//	if (ARvsg::instance().init(f_screenDistanceMM, f_background))
//	{
//		cerr << "VSG init failed!" << endl;
//		return -1;
//	}

	if (init_pages())
	{
		cerr << "Error initializing vsg stimuli." << endl;
		return -1;
	}

	return 0;
}

int init_pages()
{
	double ri=7;
	double ro = 9;
	double rm = 8;
	double r1 = 4;
	double r2 = 6;
	int nc = 32;
	double astep;
	COLOR_TYPE w = { white, {}};
	COLOR_TYPE b = { black, {}};
	COLOR_TYPE c = { custom, {.25, .25, .25}};


	astep = 360.0f/nc;
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);

	// level 100 = white, 101=black
	arutil_color_to_palette(w, 100);
	arutil_color_to_palette(b, 101);
	arutil_color_to_palette(c, 102);
	draw_pie(32, 100, 101, ro, astep);
	draw_pie(32, 101, 100, rm, astep);
	vsgSetPen1(99);
	vsgDrawOval(0, 0, 2*ri, 2*ri);
	vsgSetPen1(102);
	vsgSetDrawMode(vsgSOLIDPEN);
	vsgSetPenSize(4, 4);
	vsgDrawLine(r1, 0, r2, 0);
	vsgDrawLine(0, r1, 0, r2);
	vsgDrawLine(-r1, 0, -r2, 0);
	vsgDrawLine(0, -r1, 0, -r2);
	vsgSetSpatialUnits(vsgPIXELUNIT);
	vsgImageExport(vsgBMPPICTURE, -400, -300, 800, 600, "c:\\Documents and Settings\\Lab\\Desktop\\check.bmp");

	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);
	return 0;
}


int draw_pie(int nc, PIXEL_LEVEL first, PIXEL_LEVEL second, double r, double astep)
{
	int i;
	for (i=0; i<nc; i++)
	{
		if (i % 2 == 0)
		{
			vsgSetPen1(first);
			vsgSetPen2(second);
		}
		else
		{
			vsgSetPen1(second);
			vsgSetPen2(first);
		}
		vsgDrawPieArc(0, 0, 2*r, 2*r, i*astep, astep*1.5);
	}
	vsgSetPen1(first);
	vsgSetPen2(second);
	vsgDrawPieArc(0, 0, 2*r, 2*r, 0, astep);
	return 0;
}
#endif
