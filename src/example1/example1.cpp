#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include "Alertlib.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

using namespace std;
using namespace alert;

void do_no_overlay();
void do_overlay();



int main (int argc, char *argv[])
{
	string s;


	cout << "Overlay? ";
	cin >> s;

	if (s == "Y" || s=="y") 
	{
		cout << "Drawing using overlay...." << endl;
		do_overlay();
	}
	else
	{
		do_no_overlay();
	}
	return 0;
}



// Draw a fixation point and grating using an overlay page. 
// Overlay page 0 is used for the overlay. Video page 0 is used for the 
// fixation point and grating. 
// Note that the overlay pages must be drawn first, before any video pages. This is 
// because the library uses the VSG Object system for drawing dots (fixation points)
// and gratings, and all non-object-system drawing must be completed before the first
// object is created. 
// So the sequence to follow for drawing using an overlay page is this:
//
//	1. call ARvsg::instance().init(screenDistanceMM, background) to do the basic vsg init. 
//  2. call ARvsg::instance().init_overlay(). This clears overlay page 0 to the background
//     color, and writes that color into pixel level 1 of the overlay palette. (Pixel level
//     zero in the overlay palette is reserved as the "clear" color). After this call, 
//     overlay page 0 is the current draw page.  
//  3. call drawOverlay() method for any objects that are to be drawn on the overlay page
//  4) call ARvsg::instance().init_video(). This clears all video pages to the background color
//     and leaves video page 0 as the current draw page. 
//  5) call init(#levels) for each object that is to be drawn. This creates the vsg objects and 
//     assigns their pixel levels.
//  6) call draw() for each object to be drawn
//  7) call vsgPresent() to display. 
// 
//	At this point you can change the contrast on any of your objects, call vsgPresent() and see
//  the change reflected on the screen. You can make all objects disappear by changing video pages.

void do_overlay()
{
	COLOR_TYPE background = gray;
	int screenDistanceMM = 555;
	string stmp;


	ARContrastFixationPointSpec fp;
	string fpspec="0.0,0.0,1.0,r";
	if (parse_fixation_point(fpspec, fp))
	{
		cerr << "Error parsing fp string: " << fpspec << endl;
	}


	ARGratingSpec gr;
	string gspec="2,2,2,2,100,3,0,30,b,s,e";
	if (parse_grating(gspec, gr))
	{
		cerr << "Error parsing gr string: " << gspec << endl;
	}



	// Init vsg
	if (ARvsg::instance().init(screenDistanceMM, background))
	{
		cerr << "VSG init failed!" << endl;
		return;
	}

	// Initialize and draw overlay page
	if (ARvsg::instance().init_overlay())
	{
		cerr << "VSG overlay initialization failed!" << endl;
		return;
	}
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1);

	// Now draw overlay stuff BEFORE any of the vsg object system is in use. 
	fp.drawOverlay();
	gr.drawOverlay();

	// initialize video pages
	if (ARvsg::instance().init_video())
	{
		cerr << "VSG video initialization failed!" << endl;
		return;
	}
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);

	// Now put a simple fixation point on the screen. 
	// The parse_fixation_point call parses a string that specifies (in order) x,y,diam,color. 
	// The ARContrastFixationPointSpec object, derived from ARFixationPointSpec, gets the 
	// specs from the string. 
	fp.init(2);
	fp.draw();
	gr.init(40);
	gr.draw();
	vsgPresent();
	
	// Now a demo of how to turn the spot on/off. There's two ways. You can change to a page
	// which has nothing drawn on it (enter a number 0-4). You can also change the contrast
	// of the spot to 0 (to make it disappear) or 100(to make it reappear), provided you are 
	// on the same page on which it is drawn. 
	string s;
	cout << "Enter page/q/c/d/e/f: ";
	cin >> s;
	while (s != "q" && s!= "Q")
	{
		if (s=="c")
		{
			fp.select();
			vsgObjSetContrast(100);
			vsgPresent();
		}
		else if (s == "d")
		{
			fp.select();
			vsgObjSetContrast(0);
			vsgPresent();
		}
		else if (s=="e")
		{
			gr.select();
			vsgObjSetContrast(100);
			vsgPresent();
		}
		else if (s == "f")
		{
			gr.select();
			vsgObjSetContrast(0);
			vsgPresent();
		}
		else
		{
			istringstream iss(s);
			int i;
			iss >> i;
			if (i>=0 && i<=4) 
			{
				vsgSetDrawPage(vsgVIDEOPAGE, i, vsgNOCLEAR);
				vsgPresent();
			}
		}
		cout << "Enter page or q: ";
		cin >> s;
	}

	ARvsg::instance().clear(0);
	

	return;
}



// Drawing fixation points and gratings without overlay is pretty simple. 
//	1. call ARvsg::instance().init(screenDistanceMM, background) to do the basic vsg init. 
//  2) call ARvsg::instance().init_video(). This clears all video pages to the background color
//     and leaves video page 0 as the current draw page. 
//  3) call init(#levels) for each object that is to be drawn. This creates the vsg objects and 
//     assigns their pixel levels.
//  4) call draw() for each object to be drawn. Gratings should be drawn with drawOnce(). 
//  5) call vsgPresent() to display. 



void do_no_overlay()
{
	COLOR_TYPE background = gray;
	int screenDistanceMM = 555;
	string stmp;


	ARContrastFixationPointSpec fp;
	string fpspec="0.0,0.0,1.0,r";
	if (parse_fixation_point(fpspec, fp))
	{
		cerr << "Error parsing fp string: " << fpspec << endl;
	}


	ARGratingSpec gr;
	string gspec="2,2,2,2,100,3,0,30,b,s,e";
	if (parse_grating(gspec, gr))
	{
		cerr << "Error parsing gr string: " << gspec << endl;
	}



	// INit vsg
	if (ARvsg::instance().init(screenDistanceMM, background))
	{
		cerr << "VSG init failed!" << endl;
		return;
	}


	// initialize video pages
	if (ARvsg::instance().init_video())
	{
		cerr << "VSG video initialization failed!" << endl;
		return;
	}
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);

	// Now put a simple fixation point on the screen. 
	// The parse_fixation_point call parses a string that specifies (in order) x,y,diam,color. 
	// The ARContrastFixationPointSpec object, derived from ARFixationPointSpec, gets the 
	// specs from the string. 

	fp.init(2);
	fp.draw();
	gr.init(40);
	gr.drawOnce();
	vsgPresent();
	
	// Now a demo of how to turn the spot on/off. There's two ways. You can change to a page
	// which has nothing drawn on it (enter a number 0-4). You can also change the contrast
	// of the spot to 0 (to make it disappear) or 100(to make it reappear), provided you are 
	// on the same page on which it is drawn. 

	string s;
	cout << "Enter page/q/c/d/e/f: ";
	cin >> s;
	while (s != "q" && s!= "Q")
	{
		if (s=="c")
		{
			fp.select();
			vsgObjSetContrast(100);
			vsgPresent();
		}
		else if (s == "d")
		{
			fp.select();
			vsgObjSetContrast(0);
			vsgPresent();
		}
		else if (s=="e")
		{
			gr.select();
			vsgObjSetContrast(100);
			vsgPresent();
		}
		else if (s == "f")
		{
			gr.select();
			vsgObjSetContrast(0);
			vsgPresent();
		}
		else
		{
			istringstream iss(s);
			int i;
			iss >> i;
			if (i>=0 && i<=4) 
			{
				vsgSetDrawPage(vsgVIDEOPAGE, i, vsgNOCLEAR);
				vsgPresent();
			}
		}
		cout << "Enter page or q: ";
		cin >> s;

	}

	ARvsg::instance().clear(0);
	

	return;
}

