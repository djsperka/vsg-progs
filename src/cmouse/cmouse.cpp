#include <iostream>
#include <string>
#include <conio.h>
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__

#include "vsgv8.h"
#include "Alertlib.h"
#include "AlertUtil.h"
#include "RegHelper.h"

using namespace std;
using namespace alert;


#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment (lib, "vsgv8.lib")

int args(int argc, char **argv);
int init_pages();
int init_triggers();
int init_screen_params();
void UpdateOverlay(bool bFixationOn, double fixX, double fixY, double fixD, double apertureX, double apertureY, double apertureDiameter);
void mousePosToVSGDrawDegrees(int pixMouseX, int pixMouseY, double* pvsgDegX, double* pvsgDegY);
void vsgDrawDegreesToMousePos(double vsgDegX, double vsgDegY, long* pixMouseX, long* pixMouseY);


int f_screenDistanceMM = -1;
COLOR_TYPE f_background = { gray, {0.5, 0.5, 0.5}};
bool f_binaryTriggers = false;
bool f_verbose = false;
ARGratingSpec f_grating;
ARContrastFixationPointSpec f_fixpt;
bool f_alert = false;

double f_vsgWidthPixels;
double f_vsgHeightPixels;
double f_vsgWidthDegrees;
double f_vsgHeightDegrees;
double f_monWidthPixels;
double f_monHeightPixels;
double f_tfPrevious = 0;



int main(int argc, char **argv)
{
	// Check input arguments
	if (args(argc, argv))
	{
		return 1;
	}

	// Get lock and init vsg
	if (ARvsg::instance().init(f_screenDistanceMM, f_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}

	// initialize screen parameters
	init_screen_params();

	// Set up pages
	init_pages();


	bool bUseManualTriggers = false;
	bool bSendTrigger = false;
	bool bFixationOn = false;
	bool bMouseOn = true;
	bool bQuit = false;
	int key;
	int iStepSize = 10;
	POINT mousePos;
	double degMouseX, degMouseY;
	int iPage = 1;
	long lDigitalIO=0;
	long lDigitalIOLast=0;

	// loop forever until 'q' is hit on keyboard
	while(!bQuit)
	{
		// get cursor position if mouse is on.
		// Convert to degrees outside the if(bMouseOn) because arrow keys may have moved position.
		if (bMouseOn)
		{
			GetCursorPos(&mousePos);
		}
		if (mousePos.x > f_monWidthPixels) mousePos.x = (long)f_monWidthPixels;
		mousePosToVSGDrawDegrees(mousePos.x, mousePos.y, &degMouseX, &degMouseY);

		// read vsg io for fixation pt signal
		if (f_alert && !bUseManualTriggers)
		{
			long ltemp = vsgIOReadDigitalIn();
			lDigitalIO = ltemp & vsgDIG1;
			bFixationOn =  lDigitalIO != 0;
			bSendTrigger = lDigitalIO!=lDigitalIOLast;
			lDigitalIOLast = lDigitalIO;

			bQuit = ((ltemp & vsgDIG6) != 0);
		}



		// flip overlay page, then draw aperture (and fixpt if needed).
		iPage = 1 - iPage;
		vsgSetDrawPage(vsgOVERLAYPAGE, iPage, 1);
		UpdateOverlay(bFixationOn, f_fixpt.x, f_fixpt.y, f_fixpt.d, degMouseX, degMouseY, f_grating.w);

		// put freshly drawn overlay page up
		if (!bSendTrigger)
		{
			vsgSetZoneDisplayPage(vsgOVERLAYPAGE, iPage);
		}
		else
		{
			if (bFixationOn)
			{
				vsgIOWriteDigitalOut(0x2, 0x2);
			}
			else 
			{
				vsgIOWriteDigitalOut(0x0, 0x2);
			}
			vsgSetZoneDisplayPage(vsgOVERLAYPAGE, iPage + vsgTRIGGERPAGE);
		}

		while (_kbhit() && !bQuit)
		{
			key = _getch();
			switch(key)
			{
			case 'a':
				{
					// get aperture size. It will be updated next time through.
					float a = -1;
					cout << "Aperture Diameter (degrees) = ";
					cin >> a;
					if ( a>0 )
					{
						f_grating.w = f_grating.h = a;
					}
					else 
					{
						cerr << "Error in input: Aperture diam must be a number>0." << endl;
					}
					break;
				}
			case 'b':
				{
					COLOR_TYPE c;
					double luminance = -1;
					cout << "Background luminance = ";
					cin >> luminance;
					if (luminance >= 0 && luminance <= 1)
					{
						c.type = custom;
						c.color.a = c.color.b = c.color.c = luminance;
						arutil_color_to_overlay_palette(c, 1);
					}
					else
					{
						cerr << "Error in input: luminance value must be between 0 and 1!" << endl;
					}
					break;
				}
			case 's':
				{
					double s;
					cout << "Spatial frequency [cycles/degree]= ";
					cin >> s;
					if (s>0.005 && s<100)
					{
						f_grating.sf = s;
						arutil_draw_grating_fullscreen(f_grating, 0);
						vsgPresent();
					}
					else
					{
						cout << "Error in input: spatial freq must be a number between 0.005 and 100." << endl;
					}
					break;
				}
			case 't':
			{
				double d = -1;
				cout << "Temporal Frequency = ";
				cin >> d;
				if (d>=0 && d<50)
				{
					f_grating.setTemporalFrequency(d);
					f_tfPrevious = 0;
					vsgPresent();
				}
				else 
				{
					cout << "Error in input: Temporal freq must be between 0 and 50." << endl;
				}
				break;
			}
			case ' ':
				{
					double tmp = f_grating.tf;
					f_grating.setTemporalFrequency(f_tfPrevious);
					f_tfPrevious = tmp;
					vsgPresent();
					break;
				}
			case 'c':
				{
					int c;
					cout << "Contrast = ";
					cin >> c;
					if (c>=0 && c<=100)
					{
						f_grating.setContrast(c);
						vsgPresent();
					}
					else
					{
						cout << "Error in input: Contrast must be between 0 and 100." << endl;
					}
					break;
				}
			case 'o':
				{
					double oo = 999;
					cout <<"Orientation = ";
					cin >> oo;
					if (oo >=0 && oo <=360)
					{
						f_grating.orientation = oo;
						arutil_draw_grating_fullscreen(f_grating, 0);
						vsgPresent();
					}
					else
					{
						cout << "Error in input: Orientation must be a number between 0 and 360" << endl;
					}
					break;
				}
			case 'p':
			case 'v':
				{
					cout << "X, Y (pixels)             = " << mousePos.x << ", " << mousePos.y << endl;
					cout << "X, Y (degrees)            = " << degMouseX << ", " << -degMouseY << endl;
					cout << "Diameter (deg)            = " << f_grating.w << endl;
					cout << "Orientation(deg)          = " << f_grating.orientation << endl;
					cout << "Spatial Freq(cycles/deg)  = " << f_grating.sf << endl;
					cout << "Temporal Freq(cycles/sec) = " << f_grating.tf << endl;
					cout << "Contrast(%)               = " << f_grating.contrast << endl;
					break;
				}
			case 27:
			case 'q':
				{
					if (!f_alert)
					{
						COLOR_TYPE c;
						bQuit = true;

						// in case background luminance was changed, set it back to mean gray.
						c.type = gray;
						arutil_color_to_overlay_palette(c, 1);
					}
					else
					{
						cerr << "Stop the Spike2 script to stop this application." << endl;
					}
					break;
				}
			case 'Q':
				{
					COLOR_TYPE c;
					bQuit = true;

					// in case background luminance was changed, set it back to mean gray.
					c.type = gray;
					arutil_color_to_overlay_palette(c, 1);
					break;
				}
			case 'm':
				{
					if (bMouseOn)
					{
						cout << "Mouse movement OFF" << endl;
						bMouseOn = false;
					}
					else
					{
						cout << "Mouse movement ON" << endl;
						bMouseOn = true;
					}
					break;
				}
			case 'S':
				{
					int i;
					cout << "Step size (pixels) = ";
					cin >> i;
					if (i > 0)
					{
						iStepSize = i;
					}
					else
					{
						cout << "Error in input: step size must be positive" << endl;
					}
					break;
				}
			case '8':
				{
					if (!bMouseOn)
					{
						mousePos.y -= iStepSize;
					}
					else
					{
						cout << "Mouse movement is ON - turn off with 'f'" << endl;
					}
					break;
				}
			case '2':
				{
					if (!bMouseOn)
					{
						mousePos.y += iStepSize;
					}
					else
					{
						cout << "Mouse movement is ON - turn off with 'f'" << endl;
					}
					break;
				}
			case '4':
				{
					if (!bMouseOn)
					{
						mousePos.x -= iStepSize;
					}
					else
					{
						cout << "Mouse movement is ON - turn off with 'f'" << endl;
					}
					break;
				}
			case '6':
				{
					if (!bMouseOn)
					{
						mousePos.x += iStepSize;
					}
					else
					{
						cout << "Mouse movement is ON - turn off with 'f'" << endl;
					}
					break;
				}
			case '1':
				{
					if (bUseManualTriggers)
					{
						cout << "Fixation point ON" << endl;
						bFixationOn = true;
					}
					else
					{
						cout << "Manual triggers are OFF: use <M> first" << endl;
					}
					break;
				}
			case '0':
				{
					if (bUseManualTriggers)
					{
						cout << "Fixation point OFF" << endl;
						bFixationOn = false;
					}
					else
					{
						cout << "Manual triggers are OFF: use <M> first" << endl;
					}
					break;
				}
			case 'M':
				{
					if (bUseManualTriggers)
					{
						cout << "Manual triggers OFF" << endl;
						bUseManualTriggers = false;
					}
					else
					{
						cout << "Manual triggers ON" << endl;
						bUseManualTriggers = true;
					}
					break;
				}
			case 13:
				{
					std::ostringstream oss;
					oss << f_grating;
					cout << "reg string=" <<  oss.str() << endl;
					SaveRegStimulus(oss.str());
					break;
				}
			case 'h':
			default:
				{
					cout << "~~~~~~~~~HELP MENU~~~~~~~~~" << endl << endl;
					cout << "Grating Parameter controls:" << endl;
					cout << "---------------------------" << endl;
					cout << "<a> aperture size" << endl;
					cout << "<o> orientation" << endl;
					cout << "<s> spatial frequency" << endl;
					cout << "<t> temporal frequency" << endl;
					cout << "<c> contrast [0-100]" << endl << endl;
					cout << "<space bar> toggles between drifting and static" << endl;
					cout << "Background luminance control:" << endl;
					cout << "-----------------------------" << endl;
					cout << "<b> background luminance [0-1]" << endl << endl;
					cout << "Mouse controls:" << endl;
					cout << "---------------" << endl;
					cout << "<m> toggle mouse movement on/off" << endl;
					cout << "<8>,<6>,<2>,<4> move aperture with arrows (when mouse movenment off)" << endl;
					cout << "<S> step size in pixels" << endl << endl;
					cout << "Status information:" << endl;
					cout << "---------------" << endl;
					cout << "<p> position information" << endl;
					cout << "<v> current spatiotemporal values" << endl;
					cout << "<M> use manual triggers for fixation point (<1>/<0>)" << endl;
					cout << "<Enter> save current stim parameters in registry" << endl;
					cout << "<esc>,<q> end program" << endl;
					cout << endl << endl;
					break;
				}
			}
		}
	}

	return 0;
}

void UpdateOverlay(bool bFixationOn, double fixX, double fixY, double fixD, double apertureX, double apertureY, double apertureDiameter)
{
	// draw aperture (color 0 is clear)
	vsgSetPen1(0);
	vsgDrawOval(apertureX, apertureY, apertureDiameter, apertureDiameter);

	if (bFixationOn) 
	{
		vsgSetPen1(3);

		// Draw the fixation point. Note that the y coord is reversed -- this is because the VSG draws everything with 
		// the y axis positive-downward, whereas we always use y-axis positive upwards. Note that the aperture doesn't 
		// require this change because its location is derived from the mouse position, which is already 
		// positive-y-downward. 
		vsgDrawOval(fixX, -fixY, fixD, fixD);
	}
}


void mousePosToVSGDrawDegrees(int pixMouseX, int pixMouseY, double* pvsgDegX, double* pvsgDegY)
{
	vsgUnit2Unit(vsgPIXELUNIT, pixMouseX * f_vsgWidthPixels/f_monWidthPixels - f_vsgWidthPixels/2, vsgDEGREEUNIT, pvsgDegX);
	vsgUnit2Unit(vsgPIXELUNIT, pixMouseY * f_vsgHeightPixels/f_monHeightPixels - f_vsgHeightPixels/2, vsgDEGREEUNIT, pvsgDegY);
}

void vsgDrawDegreesToMousePos(double vsgDegX, double vsgDegY, long* pixMouseX, long* pixMouseY)
{
	double x, y;
	vsgUnit2Unit(vsgDEGREEUNIT, vsgDegX, vsgPIXELUNIT, &x);
	vsgUnit2Unit(vsgDEGREEUNIT, vsgDegY, vsgPIXELUNIT, &y);
	*pixMouseX = (long)((x + f_vsgWidthPixels/2) * f_monWidthPixels/f_vsgWidthPixels);
	*pixMouseY = (long)((y + f_vsgHeightPixels/2) * f_monHeightPixels/f_vsgHeightPixels);
}

int init_screen_params()
{
	// Initilize globals for cnversions
	f_vsgWidthPixels = vsgGetScreenWidthPixels();
	f_vsgHeightPixels = vsgGetScreenHeightPixels();
	f_monWidthPixels = GetSystemMetrics(SM_CXSCREEN);
	f_monHeightPixels = GetSystemMetrics(SM_CYSCREEN);
	vsgUnit2Unit(vsgPIXELUNIT, f_vsgWidthPixels, vsgDEGREEUNIT, &f_vsgWidthDegrees);
	vsgUnit2Unit(vsgPIXELUNIT, f_vsgHeightPixels, vsgDEGREEUNIT, &f_vsgHeightDegrees);

	return 0;
}

int init_pages()
{
	int status=0;
	int islice=50;

	// initialize video pages
	if (ARvsg::instance().init_video())
	{
		cerr << "VSG video initialization failed!" << endl;
		return 1;
	}

	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	f_grating.init(islice);
	arutil_draw_grating_fullscreen(f_grating, 0);
	vsgPresent();

	// initialize overlay pages
	if (ARvsg::instance().init_overlay())
	{
		cerr << "VSG overlay initialization failed!" << endl;
		return 1;
	}
	arutil_color_to_overlay_palette(f_fixpt, 3);
	arutil_draw_aperture(f_grating, 0);
	arutil_draw_aperture(f_grating, 1);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 0);

	return 0;
}

int args(int argc, char **argv)
{	
	bool have_d=false;
	bool have_fixpt = false;
	bool have_grating = false;
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "d:avg:f:Ab:")) != -1)
	{
		switch (c) 
		{
		case 'A':
			f_alert = true;
			break;
		case 'a':
			f_binaryTriggers = false;
			break;
		case 'b':
			s.assign(optarg);
			if (parse_color(s, f_background)) errflg++; 
			break;
		case 'v':
			f_verbose = true;
			break;
		case 'd':
			s.assign(optarg);
			if (parse_distance(s, f_screenDistanceMM)) errflg++;
			else have_d=true;
			break;
		case 'f':
			s.assign(optarg);
			if (parse_fixation_point(s, f_fixpt)) errflg++;
			else have_fixpt = true;
			break;

		case 'g':
			s.assign(optarg);
			if (parse_grating(s, f_grating))
			{
				errflg++;
			}
			else have_grating = true;
			break;
		case 'h':
			errflg++;
			break;
		case '?':
            errflg++;
			break;
		default:
			errflg++;
			break;
		}
	}

	if (!have_d)
	{
		cerr << "No screen distance supplied!" << endl;
		errflg++;
	}

	if (f_alert && !have_fixpt)
	{
		cerr << "No fixpt specs supplied (-f): using default values." << endl;
		f_fixpt.x = f_fixpt.y = 0;
		f_fixpt.d = 0.5;
		f_fixpt.color.type = red;
	}

	if (!have_grating)
	{
		cerr << "No grating specs supplied (-g): starting with default values." << endl;
		f_grating.x = f_grating.y = 0;
		f_grating.w = f_grating.h = 5;
		f_grating.sf = 3;
		f_grating.tf = 4;
		f_grating.orientation = 45;
		f_grating.contrast = 100;
		f_grating.pattern = sinewave;
		f_grating.aperture = ellipse;
		f_grating.cv.type = b_w;
	}


	if (errflg) return 1;
	return 0;
}