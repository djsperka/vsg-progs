/* $Id: CMouseUStim.cpp,v 1.1 2015-05-12 17:26:59 devel Exp $*/

#include "CMouseUStim.h"
#include "RegHelper.h"
#include <iostream>
#include <fstream>
#include <conio.h>
using namespace std;

const string CMouseUStim::m_allowedArgs("ab:d:f:g:p:vADS:r:");



CMouseUStim::CMouseUStim()
: UStim()
, m_screenDistanceMM(-1)
, m_binaryTriggers(true)
, m_verbose(false)
, m_alert(false)
, m_pulse(0x40)
, m_sleepMS(100)
, m_bFixationOn(false)
, m_bUseRegDump(false)
{
	m_background.type = gray;
	m_background.color.a = m_background.color.b = m_background.color.c = 0.5;
};


int CMouseUStim::init_pages()
{
	int status=0;
	int islice=50;

	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	m_grating.init(islice);
	arutil_draw_grating_fullscreen(m_grating, 0);
	vsgPresent();

	// initialize overlay pages
	if (ARvsg::instance().init_overlay())
	{
		cerr << "VSG overlay initialization failed!" << endl;
		return 1;
	}
	arutil_color_to_overlay_palette(m_fixpt, 3);
	arutil_draw_aperture(m_grating, 0);
	arutil_draw_aperture(m_grating, 1);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 0);

	return 0;
}



int CMouseUStim::callback(int &output, const FunctorCallbackTrigger* ptrig)
{
	int ival=0;
	string key = ptrig->getKey();
	if (key == "F")
	{
		m_bFixationOn = true;
	}
	else if (key == "f")
	{
		m_bFixationOn = false;
	}
	return 1;
}



void CMouseUStim::run_stim(alert::ARvsg& vsg)
{
	cout << "CMouseUStim: running." << endl;

	// set screen distance
	vsg.setViewDistMM(m_screenDistanceMM);

	// clear all dig outputs
	vsgIOWriteDigitalOut(0, 0xff);
	vsgPresent();

	// initialize triggers
	TSpecificFunctor<CMouseUStim> functor(this, &CMouseUStim::callback);
	init_triggers(&functor);
	if (m_verbose)
	{
		for (unsigned int i=0; i<triggers().size(); i++)
		{
			std::cout << "Trigger " << i << " " << *(triggers().at(i)) << std::endl;
		}
	}

	// Initializations
	init_screen_params();

	// Set up pages
	init_pages();

	// reset triggers
	triggers().reset(vsgIOReadDigitalIn());

	// ready pulse
	cout << "Issuing ready pulse " << m_pulse << endl;
	vsg.ready_pulse(100, m_pulse);


	bool bUseManualTriggers = false;
	bool bSendTrigger = false;
	bool bMouseOn = true;
	bool bQuit = false;
	int key;
	int iStepSize = 10;
	POINT mousePos;
	double degVSGMouseX, degVSGMouseY;
	double pixVSGMouseX, pixVSGMouseY;
	int iPage = 1;
	long lDigitalIO=0;
	long lDigitalIOLast=0;
	long last_output_trigger = 0;

	// loop forever until 'q' is hit on keyboard or a quit signal is received on digition IO lines
	while(!bQuit)
	{
		Sleep(m_sleepMS);

		// get cursor position if mouse is on.
		// Convert to degrees outside the if(bMouseOn) because arrow keys may have moved position.
		if (bMouseOn)
		{
			GetCursorPos(&mousePos);

			// With dual screens the x coord can be wider than the screen itself. 
			if (mousePos.x > m_monWidthPixels) mousePos.x = (long)m_monWidthPixels;
			mousePixelsToVSGPixels(mousePos.x, mousePos.y, &pixVSGMouseX, &pixVSGMouseY);
		}
		vsgPixelsToVSGDrawDegrees((int)pixVSGMouseX, (int)pixVSGMouseY, &degVSGMouseX, &degVSGMouseY);

		// read vsg io for fixation pt signal
		if (m_alert && !bUseManualTriggers)
		{
			TriggerFunc	tf = std::for_each(triggers().begin(), triggers().end(), TriggerFunc(vsgIOReadDigitalIn(), last_output_trigger));

			if (tf.quit()) bQuit = true;
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

		// flip overlay page, then draw aperture (and fixpt if needed).
		iPage = 1 - iPage;
		vsgSetDrawPage(vsgOVERLAYPAGE, iPage, 1);
		overlay(m_bFixationOn, m_fixpt.x, m_fixpt.y, m_fixpt.d, degVSGMouseX, degVSGMouseY, m_grating.w);
		vsgSetZoneDisplayPage(vsgOVERLAYPAGE, iPage);

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
					if (!cin.fail())
					{
						if (a > 0)
						{
							m_grating.w = m_grating.h = a;
						}
						else 
						{
							cerr << "Error in input: Aperture diam must be a number>0." << endl;
						}
					}
					else
					{
						cerr << "Bad format - must be a number." << endl;
						cin.clear(); cin.ignore(INT_MAX, '\n');
					}
					break;
				}
			case 'b':
				{
					COLOR_TYPE c;
					double luminance = -1;
					cout << "Background luminance = ";
					cin >> luminance;
					if (!cin.fail())
					{
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
					}
					else
					{
						cerr << "Bad format - must be a number." << endl;
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
							m_grating.sf = s;
							arutil_draw_grating_fullscreen(m_grating, 0);
							vsgPresent();
						}
						else
						{
							cout << "Error in input: spatial freq must be a number between 0.005 and 100." << endl;
						}
					}
					else
					{
						cerr << "Bad format - must be a number." << endl;
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
							m_grating.setTemporalFrequency(d);
							m_tfPrevious = 0;
							arutil_draw_grating_fullscreen(m_grating, 0);
							vsgPresent();
						}
						else 
						{
							cout << "Error in input: Temporal freq must be between 0 and 50." << endl;
						}
					}
					else
					{
						cerr << "Bad format - must be a number." << endl;
						cin.clear(); cin.ignore(INT_MAX, '\n');
					}
					break;
				}
			case ' ':
				{
					double tmp = m_grating.tf;
					m_grating.setTemporalFrequency(m_tfPrevious);
					m_tfPrevious = tmp;
					arutil_draw_grating_fullscreen(m_grating, 0);
					vsgPresent();
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
							m_grating.setContrast(c);
							arutil_draw_grating_fullscreen(m_grating, 0);
							vsgPresent();
						}
						else
						{
							cout << "Error in input: Contrast must be between 0 and 100." << endl;
						}
					}
					else
					{
						cerr << "Bad format - must be an integer." << endl;
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
							m_grating.orientation = oo;
							arutil_draw_grating_fullscreen(m_grating, 0);
							vsgPresent();
						}
						else
						{
							cout << "Error in input: Orientation must be a number between 0 and 360" << endl;
						}
					}
					else
					{
						cerr << "Bad format - must be a number." << endl;
						cin.clear(); cin.ignore(INT_MAX, '\n');
					}
					break;
				}
			case 'p':
			case 'v':
				{
					cout << "X, Y (pixels)             = " << pixVSGMouseX << ", " << -pixVSGMouseY << endl;
					cout << "X, Y (degrees)            = " << degVSGMouseX << ", " << -degVSGMouseY << endl;
					cout << "Diameter (deg)            = " << m_grating.w << endl;
					cout << "Orientation(deg)          = " << m_grating.orientation << endl;
					cout << "Spatial Freq(cycles/deg)  = " << m_grating.sf << endl;
					cout << "Temporal Freq(cycles/sec) = " << m_grating.tf << endl;
					cout << "Contrast(%)               = " << m_grating.contrast << endl;
					break;
				}
			case 27:
			case 'q':
				{
					if (!m_alert)
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
					if (!cin.fail())
					{
						if (i > 0)
						{
							iStepSize = i;
						}
						else
						{
							cout << "Error in input: step size must be positive" << endl;
						}
					}
					else
					{
						cerr << "Bad format - must be an integer." << endl;
						cin.clear(); cin.ignore(INT_MAX, '\n');
					}
					break;
				}
			case '8':
				{
					if (!bMouseOn)
					{
						pixVSGMouseY -= iStepSize;
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
						pixVSGMouseY += iStepSize;
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
						pixVSGMouseX -= iStepSize;
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
						pixVSGMouseX += iStepSize;
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
						m_bFixationOn = true;
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
						m_bFixationOn = false;
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
					double xtemp, ytemp;
					xtemp = m_grating.x;
					ytemp = m_grating.y;
					m_grating.x = degVSGMouseX;
					m_grating.y = -degVSGMouseY;
					oss << m_grating;
					m_grating.x = xtemp;
					m_grating.y = ytemp;
					cout << "reg string=" <<  oss.str() << endl;
					if (!m_bUseRegDump)
					{
						// Save to registry
						SaveRegStimulus(oss.str());
					}
					else
					{
						// Dump grating string to reg dump file. This is designed for use when running on a remote system. 
						// I assume that in that case there is a drive mapped from one of those systems to another, and 
						// that this server can create a file and write to it on that mapped drive.
						std::ofstream out(m_sRegDumpFile.c_str(), ios_base::out, ios_base::trunc);
						if (!out)
						{
							cerr << "Cannot write to registry save file \"" << m_sRegDumpFile << "\", grating parameters NOT saved!" << endl;
						}
						else 
						{
							out << oss.str();
							out.close();
						}
					}
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

	// Have to turn off OVERLAYMASKMODE before leaving! The call to ARvsg::init_overlay enables
	// mask mode, this will turn it off.
	vsgSetCommand(vsgOVERLAYDISABLE);

	return;
}



void CMouseUStim::overlay(bool bFixationOn, double fixX, double fixY, double fixD, double apertureX, double apertureY, double apertureDiameter)
{
	// draw aperture (color 0 is clear)
	vsgSetPen1(0);
	vsgDrawOval(apertureX, apertureY, apertureDiameter, apertureDiameter);

	if (m_bFixationOn) 
	{
		vsgSetPen1(3);

		// Draw the fixation point. Note that the y coord is reversed -- this is because the VSG draws everything with 
		// the y axis positive-downward, whereas we always use y-axis positive upwards. Note that the aperture doesn't 
		// require this change because its location is derived from the mouse position, which is already 
		// positive-y-downward. 
		vsgDrawOval(fixX, -fixY, fixD, fixD);
	}
}





void CMouseUStim::init_triggers(TSpecificFunctor<CMouseUStim>* pfunctor)
{
	triggers().clear();

	// Fixation point trigger
	triggers().addTrigger(new FunctorCallbackTrigger("F", 0x2, 0x2, 0x4, 0x4, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("f", 0x2, 0x0, 0x4, 0x0, pfunctor));

	// quit trigger
	triggers().addTrigger(new QuitTrigger("q", 0x80, 0x80, 0xff, 0x0, 0));

	return;
}


bool CMouseUStim::parse(int argc, char **argv)
{
	bool b = false;
	int status;
	status = prargs(argc, argv, (process_args_func)NULL, m_allowedArgs.c_str(), 'F', this);
	if (!status)
	{
		b = true;
	}
	return b;
}

int CMouseUStim::process_arg(int c, std::string& arg)
{
	static bool have_d=false;
	static bool have_D = false;
	static bool have_fixpt = false;
	static bool have_grating = false;
	static int errflg = 0;
	
	switch (c) 
	{
		case 'S':
			if (parse_integer(arg, m_sleepMS))
			{
				cerr << "Error in sleepMS arg, must be integer: " << arg << endl;
				errflg++;
			}
			break;
		case 'A':
			m_alert = true;
			break;
		case 'a':
			m_binaryTriggers = false;
			break;
		case 'b':
			if (parse_color(arg, m_background)) errflg++; 
			break;
		case 'p':
			if (parse_integer(arg, m_pulse))
			{
				cerr << "Error in pulse arg: must be integer (0-7)." << endl;
				errflg++;
			}
			break;
		case 'v':
			m_verbose = true;
			break;
		case 'D':
			have_D = true;
			break;
		case 'd':
			if (parse_distance(arg, m_screenDistanceMM)) errflg++;
			else have_d=true;
			break;
		case 'f':
			if (parse_fixation_point(arg, m_fixpt)) errflg++;
			else have_fixpt = true;
			break;
		case 'g':
			if (parse_grating(arg, m_grating))
			{
				errflg++;
			}
			else have_grating = true;
			break;
		case 'r':
			m_sRegDumpFile = arg;
			m_bUseRegDump = true;
			break;
		case 0:
			{
				if (have_D)
				{
					cout << "Enter screen distance in MM: ";
					cin >> m_screenDistanceMM;
					if (!cin)
					{
						cerr << "Bad data entered. Try again." << endl;
						errflg++;
					}
				}
				else if (!have_d)
				{
					cerr << "No screen distance supplied - checking registry..." << endl;
					if (GetRegScreenDistance(m_screenDistanceMM))
					{
						cerr << "Got registry value for screen distance = " << m_screenDistanceMM << endl;
					}
					else
					{
						cerr << "Screen distance not supplied (-d) and registry value not found." << endl;
						errflg++;
					}
				}

				if (m_alert && !have_fixpt)
				{
					cerr << "No fixpt specs supplied (-f): using default values." << endl;
					m_fixpt.x = m_fixpt.y = 0;
					m_fixpt.d = 0.5;
					m_fixpt.color.type = red;
				}

				if (!have_grating)
				{
					cerr << "No grating specs supplied (-g): starting with default values." << endl;
					m_grating.x = m_grating.y = 0;
					m_grating.w = m_grating.h = 5;
					m_grating.sf = 3;
					m_grating.tf = 4;
					m_grating.orientation = 45;
					m_grating.contrast = 100;
					m_grating.pattern = sinewave;
					m_grating.aperture = ellipse;
					m_grating.cv.type = b_w;
				}
				break;
			}
		default:
			{
				cerr << "Unknown option - " << (char)c << endl;
				errflg++;
				break;
			}
	}
	return errflg;
}



void CMouseUStim::mousePixelsToVSGPixels(int pixMouseX, int pixMouseY, double* pvsgPixelsX, double* pvsgPixelsY)
{
	*pvsgPixelsX = pixMouseX * m_vsgWidthPixels/m_monWidthPixels - m_vsgWidthPixels/2;
	*pvsgPixelsY = pixMouseY * m_vsgHeightPixels/m_monHeightPixels - m_vsgHeightPixels/2;
}

void CMouseUStim::vsgPixelsToVSGDrawDegrees(int vsgPixX, int vsgPixY, double *pvsgDegX, double *pvsgDegY)
{
	vsgUnit2Unit(vsgPIXELUNIT, vsgPixX, vsgDEGREEUNIT, pvsgDegX);
	vsgUnit2Unit(vsgPIXELUNIT, vsgPixY, vsgDEGREEUNIT, pvsgDegY);
}


void CMouseUStim::mousePosToVSGDrawDegrees(int pixMouseX, int pixMouseY, double* pvsgDegX, double* pvsgDegY)
{
	double vsgPixelsX, vsgPixelsY;
	mousePixelsToVSGPixels(pixMouseX, pixMouseY, &vsgPixelsX, &vsgPixelsY);
	vsgUnit2Unit(vsgPIXELUNIT, vsgPixelsX, vsgDEGREEUNIT, pvsgDegX);
	vsgUnit2Unit(vsgPIXELUNIT, vsgPixelsY, vsgDEGREEUNIT, pvsgDegY);
}

void CMouseUStim::vsgDrawDegreesToMousePos(double vsgDegX, double vsgDegY, long* pixMouseX, long* pixMouseY)
{
	double x, y;
	vsgUnit2Unit(vsgDEGREEUNIT, vsgDegX, vsgPIXELUNIT, &x);
	vsgUnit2Unit(vsgDEGREEUNIT, vsgDegY, vsgPIXELUNIT, &y);
	*pixMouseX = (long)((x + m_vsgWidthPixels/2) * m_monWidthPixels/m_vsgWidthPixels);
	*pixMouseY = (long)((y + m_vsgHeightPixels/2) * m_monHeightPixels/m_vsgHeightPixels);
}

int CMouseUStim::init_screen_params()
{
	// Initilize globals for cnversions
	m_vsgWidthPixels = vsgGetScreenWidthPixels();
	m_vsgHeightPixels = vsgGetScreenHeightPixels();
	m_monWidthPixels = GetSystemMetrics(SM_CXSCREEN);
	m_monHeightPixels = GetSystemMetrics(SM_CYSCREEN);
	vsgUnit2Unit(vsgPIXELUNIT, m_vsgWidthPixels, vsgDEGREEUNIT, &m_vsgWidthDegrees);
	vsgUnit2Unit(vsgPIXELUNIT, m_vsgHeightPixels, vsgDEGREEUNIT, &m_vsgHeightDegrees);

	return 0;
}



