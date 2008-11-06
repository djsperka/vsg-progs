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
void do_init_video_pages();
void init_vp_before(int ipage, void *data);
void init_vp_after(int ipage, void *data);
void do_move_screen();
void do_overlay_coolmouse_test();
void init_coolmouse_test_after(int ipage);
void init_coolmouse_test_before(int ipage);


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
	else if (s=="O" || s=="o")
	{
		do_init_video_pages();
	}
	else if (s=="N" || s=="n")
	{
		do_no_overlay();
	}
	else if (s=="M" || s == "m")
	{
		do_move_screen();
	}
	else if (s=="C" || s=="c")
	{
		do_overlay_coolmouse_test();
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
	COLOR_TYPE background = { gray, {0,0,0}};
	int screenDistanceMM = 555;
	string stmp;
	int iPage;


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
	iPage = 0;
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
		else if (s == "C")
		{
			int cc;
			cout << "contrast: ";
			cin >> cc;
			gr.contrast = cc;
			gr.setContrast(cc);
			vsgPresent();
		}
		else if (s == "D")
		{
			double dd;
			cout << "diam: ";
			cin >> dd;
			gr.h = gr.w = dd;
			vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1);
			gr.drawOverlay();
			vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
			gr.draw();
			vsgPresent();
		}
		else if (s == "V")
		{
			double vv;
			cout << "drift vel: ";
			cin >> vv;
			gr.tf = vv;
			vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
			gr.draw();
			vsgPresent();
		}
		else if (s == "O")
		{
			double vv;
			cout << "orientation: ";
			cin >> vv;
			gr.orientation = vv;

			// swap pages...
//			iPage = 1 - iPage;
//			vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);
			gr.draw();
			vsgPresent();
		}
		else if (s == "L")
		{
			gr.cv.type =  l_cone;

			// swap pages...
//			iPage = 1 - iPage;
//			vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);
			gr.draw();
			vsgPresent();
		}
		else if (s == "S")
		{
			double ss;
			cout << "sf: ";
			cin >> ss;
			gr.sf = ss;

			// swap pages...
//			iPage = 1 - iPage;
//			vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);
			gr.draw();
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
	COLOR_TYPE background = { gray, {0,0,0}};
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


	// circle object
	ARContrastCircleSpec circle;
	string cspec="2.0,2.0,2.1,r";
	if (parse_fixation_point(cspec, circle))
	{
		cerr << "Error parsing circle string: " << cspec << endl;
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


	circle.init(2);
	circle.draw();
/*
	get_color(background, from);
	to.a = 1; to.b = to.c = 0;
	circle.select();
	vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);
	vsgSetDrawMode(vsgCENTREXY | vsgPIXELPEN);
	vsgDrawOval(gr.x, -gr.y, gr.w+.5, gr.h+.5);
	circle.setContrast(100);
*/

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
			circle.select();
			vsgObjSetContrast(100);
			vsgPresent();
		}
		else if (s == "d")
		{
			fp.select();
			vsgObjSetContrast(0);
			circle.select();
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


void drawCircle(const ARGratingSpec& gr)
{
	
}


void do_init_video_pages()
{
	COLOR_TYPE background = { gray, {0,0,0}};
	int screenDistanceMM = 555;

	// INit vsg
	if (ARvsg::instance().init(screenDistanceMM, background))
	{
		cerr << "VSG init failed!" << endl;
		return;
	}


	VSGTRIVAL color;
	color.a = 1;
	color.b = 1;
	color.c = 0;
	vsgPaletteWrite((VSGLUTBUFFER*)&color, 2, 1);

	// initialize video pages
	if (ARvsg::instance().init_video_pages(init_vp_before, init_vp_after, NULL))
	{
		cerr << "VSG video initialization failed!" << endl;
		return;
	}

	vsgSetFixationColour(&color);

	string s;
	cout << "Enter page: ";
	cin >> s;
	while (s != "q" && s!= "Q")
	{
		istringstream iss(s);
		int i;
		iss >> i;
		if (i>=0 && i<=9) 
		{
			vsgSetDrawPage(vsgVIDEOPAGE, i, vsgNOCLEAR);
			vsgPresent();
		}
		cout << "Enter page or q: ";
		cin >> s;

	}



}

void init_vp_before(int ipage, void *data)
{
	cout << "Init before - page " << ipage << endl;
	if (ipage < 10)
	{
//		vsgSetPen1(ipage * 20 + 10);
		vsgSetPen1(vsgFIXATION);
		vsgDrawRect(0, 0, 5, 5);
	}
	return;
}

void init_vp_after(int ipage, void *data)
{
	cout << "Init after - page " << ipage << endl;
	return;
}



void do_move_screen()
{
	COLOR_TYPE background = { gray, {0,0,0}};
	int screenDistanceMM = 555;
	string stmp;


	ARContrastFixationPointSpec fp;
	string fpspec="0.0,0.0,1.0,r";
	if (parse_fixation_point(fpspec, fp))
	{
		cerr << "Error parsing fp string: " << fpspec << endl;
	}


	ARGratingSpec gr;
	string gspec="0,0,2,2,100,3,0,30,b,s,e";
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
	

	string s;
	cout << "Enter move coords: "; 
	cin >> s;
	while (s != "q" && s!= "Q")
	{
		double x, y;
		if (!parse_xy(s, x, y))
		{
			cout << "Moving " << x << " " << y << endl;
			if (vsgMoveScreen(x, y) < 0)
			{
				cerr << "Command failed" <<endl;
			}
//		vsgPresent();
		}
		cout << "Enter move dist or q: ";
		cin >> s;
	}

	ARvsg::instance().clear(0);
	

	return;
}





void init_coolmouse_test_before(int ipage, void *pgrating)
{
	if (ipage == 0)
	{
		cout << "Init before - page " << ipage << endl;
		((ARGratingSpec *)pgrating)->drawOverlay();
	}
	return;
}

void init_coolmouse_test_after(int ipage, void *pgrating)
{
	if (ipage == 0)
	{
		cout << "Init after - page " << ipage << endl;
		((ARGratingSpec *)pgrating)->init(40);
		((ARGratingSpec *)pgrating)->draw(true);
	}
	return;
}




void do_overlay_coolmouse_test()
{
	COLOR_TYPE background = { gray, {0,0,0}};
	int screenDistanceMM = 555;
	string stmp;


	ARGratingSpec gr;
	string gspec="2,2,2,2,100,3,2,30,b,s,e";
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

	// Set overlay page to all clear (level 0), then set level 1 to red, and draw a dot. 
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 0);

	VSGTRIVAL red;
	red.a=1; red.b = red.c = 0;
	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&red, 1, 1);
	vsgSetPen1(1);
	vsgDrawOval(0, 0, 1, 1);



	// initialize video pages
	if (ARvsg::instance().init_video_pages(NULL, init_coolmouse_test_after, &gr))
	{
		cerr << "VSG video initialization failed!" << endl;
		return;
	}
//	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
//	gr.init(40);
//	gr.drawOnce();
	vsgPresent();
	

	string s;
	cout << "Enter move coords: "; 
	cin >> s;
	while (s != "q" && s!= "Q")
	{
		double x, y;
		if (!parse_xy(s, x, y))
		{
			cout << "Moving " << x << " " << y << endl;
			if (vsgMoveScreen(x, y) < 0)
			{
				cerr << "Command failed" <<endl;
			}
//		vsgPresent();
		}
		cout << "Enter move dist or q: ";
		cin >> s;
	}

	ARvsg::instance().clear(0);
	

	return;
}
