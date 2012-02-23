/* $Id: tmouse.cpp,v 1.1 2012-02-23 18:56:44 devel Exp $ */

#include <windows.h>
#include <process.h>
#include <conio.h>
#include <iostream>
#define _USE_MATH_DEFINES
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__

#include "vsgv8.h"
#include "Alertlib.h"
#include "AlertUtil.h"
#include "RegHelper.h"
using namespace std;
using namespace alert;

// libs

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif
#pragma comment (lib, "vsgv8.lib")


// globals
bool f_bBinaryTriggers = true;
bool f_bVerbose = false;
COLOR_TYPE f_colorBackground = { gray, {0.5, 0.5, 0.5}};
ARContrastFixationPointSpec f_fixpt;
ARGratingSpec f_grating;
int f_iDistanceToScreenMM = -1;
double f_tfPrevious = 0;		// save last temporal freq when freezing grating
TriggerVector f_vecTriggers;
int f_iPulse = 0x40;
bool f_bAlert = false;
bool f_bUseLockFile = true;
bool f_bQuit = false;
bool f_bMouseOn = true;
int f_iStepSize = 5;
double f_cvec[2];		// draw position of stim in pixels from ULH corner

// screen parameters
double f_vsgWidthPixels;
double f_vsgHeightPixels;
double f_vsgWidthDegrees;
double f_vsgHeightDegrees;
double f_monWidthPixels;
double f_monHeightPixels;

// events, threads n stuff
HANDLE f_threadMouse;
HANDLE f_threadUpdate;
HANDLE f_threadInput;
HANDLE f_mutexPosn;			// get this before using f_pixVSGMouseX,Y
HANDLE f_eventUpdate;		// position updated, redraw
HANDLE f_mutexGrating;
HANDLE f_eventGrating;
// mouse position
double f_pixVSGMouseX, f_pixVSGMouseY;
double f_degVSGDrawX, f_degVSGDrawY;
double f_pixVSGDrawX, f_pixVSGDrawY;

// prototypes
int prargs_callback(int c, string& arg);
int init_screen_params();
int init_pages();
void do_mouse(void *unused);
void do_update(void *unused);
void do_input(void *unused);
void mousePixelsToVSGPixels(int pixMouseX, int pixMouseY, double* pvsgPixelsX, double* pvsgPixelsY);
void vsgPixelsToVSGDrawCoords(double vsgPixX, double vsgPixY, double *pvsgDrawPixX, double *pvsgDrawPixY, double *pvsgDrawDegX, double *pvsgDrawDegY);
//void mousePosToVSGDrawDegrees(int pixMouseX, int pixMouseY, double* pvsgDegX, double* pvsgDegY);
//void vsgDrawDegreesToMousePos(double vsgDegX, double vsgDegY, long* pixMouseX, long* pixMouseY);



int main(int argc, char **argv)
{
	int status;

	// Check input arguments
	status = prargs(argc, argv, prargs_callback, "Dd:avg:f:Ab:np:", 'F');
	if (status)
	{
		return -1;
	}

	// Get lock and init vsg
	if (ARvsg::instance().init(f_iDistanceToScreenMM, f_colorBackground, f_bUseLockFile))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}
	vsgSetPageWidth(2048);

	// initialize screen parameters
	init_screen_params();

	// Set position of stim. 
	double gx, gy;
	vsgUnitToUnit(vsgPIXELUNIT, f_vsgWidthPixels/2, vsgDEGREEUNIT, &gx);
	vsgUnitToUnit(vsgPIXELUNIT, f_vsgHeightPixels/2, vsgDEGREEUNIT, &gy);
	f_grating.x += gx;
	f_grating.y -= gy;
	cout << "Grating " << f_grating << endl;
	f_cvec[0] = f_vsgWidthPixels;
	f_cvec[1] = f_vsgHeightPixels;

	// Set up pages
	init_pages();
	
	// Create mutex, launch mouse thread. 
	f_mutexPosn = (HANDLE)CreateMutex(NULL, FALSE, NULL);
	f_threadMouse = (HANDLE)_beginthread(do_mouse, 0, NULL);
	f_threadUpdate = (HANDLE)_beginthread(do_update, 0, NULL);
	f_threadInput = (HANDLE)_beginthread(do_input, 0, NULL);
	f_eventUpdate = CreateEvent(NULL, 
								FALSE,	// auto-reset; resets to nonsignaled after waiting thread is released
								FALSE,	// non-signaled initial state
								NULL);
	f_mutexGrating = (HANDLE)CreateMutex(NULL, FALSE, NULL);
	f_eventGrating = CreateEvent(NULL, 
								FALSE,	// auto-reset; resets to nonsignaled after waiting thread is released
								FALSE,	// non-signaled initial state
								NULL);

	// wait for threads
	HANDLE handles[5];
	handles[0] = f_threadMouse;
	handles[1] = f_threadUpdate;
	handles[2] = f_threadInput;
	WaitForMultipleObjects(3, handles, TRUE, INFINITE);

	ARvsg::instance().clear(0);

	return 0;
}


// Input loop. Looks for keystrokes. May update grating and/or position, but does not draw. 
// May make vsg calls (indirectly through grating updates, e.g. contrast). 

void do_input(void *unused)
{
	while (!f_bQuit)
	{
		int key = _getch();	// keystroke will end prog
		switch (key)
		{
		case 'q':
		case 27:
			{
				if (!f_bAlert)
				{
					f_bQuit = true;
				}
				else
				{
					cerr << "Stop the Spike2 script to stop this application." << endl;
				}
				break;
			}
		case 'a':
			{
				// get aperture size. It will be updated next time through.
				float a = -1;
				cout << "Aperture Diameter (degrees) = ";
				cin >> a;
				if (!cin.fail())
				{
					if ( a>0 )
					{
						WaitForSingleObject(f_mutexGrating, INFINITE);
						f_grating.w = f_grating.h = a;
						ReleaseMutex(f_mutexGrating);
						SetEvent(f_eventGrating);
					}
					else 
					{
						cerr << "Error in input: Aperture diam must be a number>0." << endl;
					}
				}
				else
				{
					cerr << "Error in input: Must be numeric." << endl;
					cin.clear(); cin.ignore(INT_MAX, '\n');
				}
				break;
			}
		case 'b':
			{
				VSGTRIVAL c;
				double luminance = -1;
				cout << "Background luminance = ";
				cin >> luminance;
				if (!cin.fail())
				{
					if (luminance >= 0 && luminance <= 1)
					{
						c.a = c.b = c.c = luminance;
						WaitForSingleObject(f_mutexGrating, INFINITE);
						vsgSetBackgroundColour(&c);				// grating doesn't need be protected, but still force redraw
						ReleaseMutex(f_mutexGrating);
						SetEvent(f_eventGrating);
					}
					else
					{
						cerr << "Error in input: luminance value must be between 0 and 1!" << endl;
					}
				}
				else
				{
					cerr << "Error in input: must be numeric value between 0 and 1." << endl;
					cin.clear(); cin.ignore(INT_MAX, '\n');
				}
				break;
			}
		case 's':
			{
				double s;
				cout << "Spatial frequency [cycles/degree]= ";
				cin >> s;
				if (!cin.fail())
				{
					if (s>0.005 && s<100)
					{
						WaitForSingleObject(f_mutexGrating, INFINITE);
						f_grating.sf = s;
						ReleaseMutex(f_mutexGrating);
						SetEvent(f_eventGrating);
					}
					else
					{
						cout << "Error in input: spatial freq must be a number between 0.005 and 100." << endl;
					}
				}
				else
				{
					cerr << "Error in input: must be numeric." << endl;
					cin.clear(); cin.ignore(INT_MAX, '\n');
				}
				break;
			}
		case 'c':
			{
				int c;
				cout << "Contrast = ";
				cin >> c;
				if (!cin.fail())
				{
					if (c>=0 && c<=100)
					{
						WaitForSingleObject(f_mutexGrating, INFINITE);
						f_grating.setContrast(c);
						ReleaseMutex(f_mutexGrating);
						SetEvent(f_eventGrating);
					}
					else
					{
						cout << "Error in input: Contrast must be between 0 and 100." << endl;
					}
				}
				else 
				{
					cerr << "Error in input: Contrast must be a number." << endl;
					cin.clear(); cin.ignore(INT_MAX, '\n');
				}
				break;
			}
		case 'o':
			{
				double oo = 999;
				cout <<"Orientation = ";
				cin >> oo;
				if (!cin.fail())
				{
					if (oo >=0 && oo <=360)
					{
						WaitForSingleObject(f_mutexGrating, INFINITE);
						f_grating.orientation = oo;
						ReleaseMutex(f_mutexGrating);
						SetEvent(f_eventGrating);

					}
					else
					{
						cout << "Error in input: Orientation must be a number between 0 and 360" << endl;
					}
				}
				else
				{
					cerr << "Error in input: must be numeric." << endl;
					cin.clear(); cin.ignore(INT_MAX, '\n');
				}
				break;
			}
		case 't':
			{
				double d = -1;
				cout << "Temporal Frequency = ";
				cin >> d;
				if (!cin.fail())
				{
					if (d>=0 && d<50)
					{
						WaitForSingleObject(f_mutexGrating, INFINITE);
						f_grating.setTemporalFrequency(d);
						f_tfPrevious = 0;
						ReleaseMutex(f_mutexGrating);
						SetEvent(f_eventGrating);
					}
					else 
					{
						cout << "Error in input: Temporal freq must be between 0 and 50." << endl;
					}
				}
				else
				{
					cerr << "Error in input: Temporal freq must be numeric." << endl;
					cin.clear(); cin.ignore(INT_MAX, '\n');
				}
				break;
			}
		case ' ':
			{
				double tmp = f_grating.tf;
				WaitForSingleObject(f_mutexGrating, INFINITE);
				f_grating.setTemporalFrequency(f_tfPrevious);
				f_tfPrevious = tmp;
				ReleaseMutex(f_mutexGrating);
				SetEvent(f_eventGrating);
				break;
			}
		case 'm':
			{
				if (f_bMouseOn)
				{
					cout << "Mouse movement OFF. Use keypad 8462 to move. Step size is " << f_iStepSize << " pixels (use \"S\" to change)" << endl;
					f_bMouseOn = false;
				}
				else
				{
					cout << "Mouse movement ON. Move stim with cursor." << endl;
					f_bMouseOn = true;
				}
				break;
			}
		case 'S':
			{
				int i;
				cout << "Step size (pixels) = ";
				cin >> i;
				if (!cin.fail())
				{
					if (i > 0)
					{
						f_iStepSize = i;
					}
					else
					{
						cout << "Error in input: step size must be positive" << endl;
					}
				}
				else
				{
					cerr << "Error in input: step size must be a positive integer." << endl;
					cin.clear();
					cin.ignore(INT_MAX, '\n');
				}
				break;
			}
		case '8':
			{
				if (!f_bMouseOn)
				{
					WaitForSingleObject(f_mutexPosn, INFINITE);
					f_pixVSGMouseY -= f_iStepSize;
					vsgPixelsToVSGDrawCoords(f_pixVSGMouseX, f_pixVSGMouseY, &f_pixVSGDrawX, &f_pixVSGDrawY, &f_degVSGDrawX, &f_degVSGDrawY);
					ReleaseMutex(f_mutexPosn);
					SetEvent(f_eventUpdate);
				}
				else
				{
					cout << "Mouse movement is ON - toggle on/off with 'm'" << endl;
				}
				break;
			}
		case '2':
			{
				if (!f_bMouseOn)
				{
					WaitForSingleObject(f_mutexPosn, INFINITE);
					f_pixVSGMouseY += f_iStepSize;
					vsgPixelsToVSGDrawCoords(f_pixVSGMouseX, f_pixVSGMouseY, &f_pixVSGDrawX, &f_pixVSGDrawY, &f_degVSGDrawX, &f_degVSGDrawY);
					ReleaseMutex(f_mutexPosn);
					SetEvent(f_eventUpdate);
				}
				else
				{
					cout << "Mouse movement is ON - turn off with 'f'" << endl;
				}
				break;
			}
		case '4':
			{
				if (!f_bMouseOn)
				{
					WaitForSingleObject(f_mutexPosn, INFINITE);
					f_pixVSGMouseX -= f_iStepSize;
					vsgPixelsToVSGDrawCoords(f_pixVSGMouseX, f_pixVSGMouseY, &f_pixVSGDrawX, &f_pixVSGDrawY, &f_degVSGDrawX, &f_degVSGDrawY);
					ReleaseMutex(f_mutexPosn);
					SetEvent(f_eventUpdate);
				}
				else
				{
					cout << "Mouse movement is ON - turn off with 'f'" << endl;
				}
				break;
			}
		case '6':
			{
				if (!f_bMouseOn)
				{
					WaitForSingleObject(f_mutexPosn, INFINITE);
					f_pixVSGMouseX += f_iStepSize;
					vsgPixelsToVSGDrawCoords(f_pixVSGMouseX, f_pixVSGMouseY, &f_pixVSGDrawX, &f_pixVSGDrawY, &f_degVSGDrawX, &f_degVSGDrawY);
					ReleaseMutex(f_mutexPosn);
					SetEvent(f_eventUpdate);
				}
				else
				{
					cout << "Mouse movement is ON - turn off with 'f'" << endl;
				}
				break;
			}
		case 'p':
		case 'v':
			{
				cout << "X, Y (pixels)             = " << f_pixVSGDrawX << ", " << f_pixVSGDrawY << endl;
				cout << "X, Y (degrees)            = " << f_degVSGDrawX << ", " << f_degVSGDrawY << endl;
				cout << "Diameter (deg)            = " << f_grating.w << endl;
				cout << "Orientation(deg)          = " << f_grating.orientation << endl;
				cout << "Spatial Freq(cycles/deg)  = " << f_grating.sf << endl;
				cout << "Temporal Freq(cycles/sec) = " << f_grating.tf << endl;
				cout << "Contrast(%)               = " << f_grating.contrast << endl;
				break;
			}
		default:
			{
				cerr << "Key not handled (" << string(1, (char)key) << ")" << endl;
				break;
			}
		}

		//Sleep(500);
	}
}


// Mouse thread. Checks mouse position, transforms to vsg coords, updates 
// position vars and sets event (f_eventUpdate) to trigger screen pos update. 
void do_mouse(void *unused)
{
	POINT mousePos;
	DWORD dwWaitResult;

	while (!f_bQuit)
	{
		if (f_bMouseOn)
		{
			// Fetch current mouse position
			GetCursorPos(&mousePos);
			// With dual screens the x coord can be wider than the screen itself. 
			if (mousePos.x > f_monWidthPixels) mousePos.x = (long)f_monWidthPixels;

			// Get mutex before updating mouse position
			dwWaitResult = WaitForSingleObject(f_mutexPosn, 250);
			switch (dwWaitResult)
			{
			case WAIT_OBJECT_0:
				// We got it!
				mousePixelsToVSGPixels(mousePos.x, mousePos.y, &f_pixVSGMouseX, &f_pixVSGMouseY);
				vsgPixelsToVSGDrawCoords(f_pixVSGMouseX, f_pixVSGMouseY, &f_pixVSGDrawX, &f_pixVSGDrawY, &f_degVSGDrawX, &f_degVSGDrawY);
				ReleaseMutex(f_mutexPosn);
				SetEvent(f_eventUpdate);
				break;
			case WAIT_ABANDONED:
				cerr << "Mutex WAIT_ABANDONED error " << endl;
				break;
			default:
				cerr << "Mutex wait failed." << endl;
			}
		}
		else
		{
			Sleep(250);
		}
	}
	return;
}


// update loop. Will do drawing and screen pos updates here. 
// f_eventGrating indicates that grating must be redrawn. 
// f_eventUpdate indicates that screen position has been changed. 

void do_update(void *unused)
{
	double x, y;
	while (!f_bQuit)
	{
		// grating event firing says we gotta redraw the grating. 
		// Don't wait around for it though. 
		if (!WaitForSingleObject(f_eventGrating, 0))
		{
			WaitForSingleObject(f_mutexGrating, INFINITE);
			f_grating.draw();
			vsgPresent();
			ReleaseMutex(f_mutexGrating);
		}

		WaitForSingleObject(f_eventUpdate, INFINITE);

		// Get mutex before using mouse posn
		WaitForSingleObject(f_mutexPosn, INFINITE);

		// Update screen position
		vsgUnitToUnit(vsgPIXELUNIT, f_cvec[0] - f_pixVSGMouseX, vsgDEGREEUNIT, &x);
		vsgUnitToUnit(vsgPIXELUNIT, f_cvec[1] - f_pixVSGMouseY, vsgDEGREEUNIT, &y);
		vsgMoveScreen(x, y);
		ReleaseMutex(f_mutexPosn);
	}
	return;
}

// thread to check digitial inputs and check triggers
void do_digin(void *unused)
{
	while (!f_bQuit)
	{
		if (f_bAlert && !f_bUseManualTriggers)
		{
			TriggerFunc	tf = std::for_each(f_triggers.begin(), f_triggers.end(), TriggerFunc(vsgIOReadDigitalIn(), last_output_trigger));

			if (tf.quit()) f_bQuit = true;
			else if (tf.present())
			{	
				// The use of vsgIODigitalWriteOut here means that the output triggers appear as-is at the 
				// spike2 end. When we use vsgObjSetTriggers the bits are shifted left by one because the 
				// VSG takes the lowest order output bit for itself, and when we output bit 0x1 it is sent
				// on DOUT1 (not DOUT0).
				last_output_trigger = tf.output_trigger();
				vsgIOWriteDigitalOut(tf.output_trigger(), 0xff);
			}
		}
	}
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

// Scale mouse position to VSG position. Mouse position (0,0) is upper left hand corner, positive down.
void mousePixelsToVSGPixels(int pixMouseX, int pixMouseY, double* pvsgPixelsX, double* pvsgPixelsY)
{
	*pvsgPixelsX = pixMouseX * f_vsgWidthPixels/f_monWidthPixels;
	*pvsgPixelsY = pixMouseY * f_vsgHeightPixels/f_monHeightPixels;
}

// Assume input vsg pixels are ordered from upper let hand corner with positive down. 
// Convert to center-screen positive up pixels, then to degrees. 
void vsgPixelsToVSGDrawCoords(double vsgPixX, double vsgPixY, double *pvsgDrawPixX, double *pvsgDrawPixY, double *pvsgDrawDegX, double *pvsgDrawDegY)
{
	*pvsgDrawPixX = vsgPixX - f_vsgWidthPixels/2;
	*pvsgDrawPixY = f_vsgHeightPixels/2 - vsgPixY;
	vsgUnit2Unit(vsgPIXELUNIT, *pvsgDrawPixX, vsgDEGREEUNIT, pvsgDrawDegX);
	vsgUnit2Unit(vsgPIXELUNIT, *pvsgDrawPixY, vsgDEGREEUNIT, pvsgDrawDegY);
}

int init_pages()
{
	int status=0;
	int islice=50;

	vsgSetPen2(vsgBACKGROUND);
	vsgSetCommand(vsgVIDEOCLEAR);
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	f_grating.init(islice);
	f_grating.draw();
	vsgPresent();
#if 0
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

	// Initialize triggers

	// Fixation point trigger
	triggers.addTrigger(new CallbackTrigger("F", 0x2, 0x2, 0x4, 0x4, callback));
	triggers.addTrigger(new CallbackTrigger("f", 0x2, 0x0, 0x4, 0x0, callback));

	// quit trigger
	triggers.addTrigger(new QuitTrigger("q", 0x80, 0x80, 0xff, 0x0, 0));
#endif

	return 0;
}


int prargs_callback(int c, string& arg)
{	
	static int errflg = 0;
	static bool have_d=false;
	static bool have_fixpt = false;
	static bool have_grating = false;
	string s;

	switch (c) 
	{
	case 'A':
		f_bAlert = true;
		break;
	case 'n':
		f_bUseLockFile = false;
		break;
	case 'a':
		f_bBinaryTriggers = false;
		break;
	case 'b':
		s.assign(optarg);
		if (parse_color(s, f_colorBackground)) errflg++; 
		break;
	case 'p':
		s.assign(optarg);
		if (parse_integer(s, f_iPulse))
		{
			cerr << "Error in pulse arg: must be integer (0-7)." << endl;
			errflg++;
		}
		break;
	case 'v':
		f_bVerbose = true;
		break;
	case 'd':
		s.assign(optarg);
		if (parse_distance(s, f_iDistanceToScreenMM)) errflg++;
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
	case 0:
		if (!have_d)
		{
			cerr << "No screen distance supplied - checking registry..." << endl;
			if (GetRegScreenDistance(f_iDistanceToScreenMM))
			{
				cerr << "Got registry value for screen distance = " << f_iDistanceToScreenMM << endl;
			}
			else
			{
				cerr << "Screen distance not supplied (-d) and registry value not found." << endl;
				errflg++;
			}
		}

		if (f_bAlert && !have_fixpt)
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
		break;
	default:
		errflg++;
		break;
	}

	return errflg;
}