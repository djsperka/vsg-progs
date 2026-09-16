/* $Id: CMouseUStim.cpp,v 1.3 2016-05-18 19:36:46 devel Exp $*/

#include "CMouseUStim.h"
#include "RegHelper.h"
#include <iostream>
#include <fstream>
#include <conio.h>
#include "argp.h"

using namespace std;

const string CMouseUStim::m_allowedArgs("ab:d:f:g:p:qvADS:r:j:");

// for argp
error_t parse_opt(int key, char* carg, struct argp_state* state);
static struct argp_option options[] = {
	{"ascii", 'a', 0, 0, "Use ascii triggers (user-entered)"},
	{"verbose",  'v', 0, 0, "Produce verbose output" },
	{"background", 'b', "COLOR", 0, "background color"},
	{"distance-to-screen", 'd', "DIST_MM", 0, "screen distance in MM"},
	{"fixpt", 'f', "FIXPT_SPEC", 0, "fixation point"},
	{"grating", 'g', "GRATING", 0, "Output to FILE instead of standard output" },
	{"ready-pulse", 'p', "BITPATTERN", 0, "Ready pulse issued when startup is complete"},
	{"alert", 'A', 0, 0, "Using fixpt triggers for alert usage"},
	{"allow-quit", 'q', 0, 0, "Allow key 'q' to exit (default is to ignore 'q')"},
	{"prompt-for-distance", 'D', 0, 0, "Prompt user for screen distance in MM"},
	{"sleep-time", 'S', "SLEEP_MS", 0, "Time to sleep (ms) each loop awaiting key input"},
	{"registry-dump-file", 'r', "FILENAME", 0, "Filename to dump grating spec to (for remote systems only - unused presently"},
	{"use-images", 301, 0, 0, "image list"},
	{ 0 }
};
static struct argp f_argp = { options, parse_opt, 0, "cmouse -- mouse control of grating stimulus" };


CMouseUStim::CMouseUStim()
: UStim()
, m_arguments()
, m_gratingPage(1)
, m_overlayPage(1)
, m_bFixationDisabled(false)
, m_bStimulusDisabled(false)
{
};


void CMouseUStim::flip_draw_grating()
{
	m_gratingPage = 1 - m_gratingPage;
	cout << "flip_draw_grating, disabled? " << m_bStimulusDisabled << " page " << m_gratingPage << ": " << m_arguments.grating << endl;
	if (m_bStimulusDisabled)
	{
		vsgSetDrawPage(vsgVIDEOPAGE, m_gratingPage, vsgBACKGROUND);
	}
	else
	{
		arutil_draw_grating_fullscreen(m_arguments.grating, m_gratingPage);
	}
}


int CMouseUStim::init_pages()
{
	int status=0;
	int islice=50;

	//vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);

	m_arguments.grating.init(islice);
	flip_draw_grating();
	vsgPresent();

	//if (!m_arguments.bUseImages)
	//{
	//	m_arguments.grating.init(islice);
	//	flip_draw_grating();
	//	vsgPresent();
	//}
	//else
	//{
	//	// images - 
	//	m_imageSpec.init(128, false);
	//}

	// initialize overlay pages
	init_overlay_pages();
	if (m_arguments.bHaveFixpt)
		arutil_color_to_overlay_palette(m_arguments.fixpt, 3);
	arutil_draw_aperture(m_arguments.grating, 0);
	arutil_draw_aperture(m_arguments.grating, 1);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 0);

	return 0;
}

void CMouseUStim::init_overlay_pages()
{
	vsgSetCommand(vsgOVERLAYMASKMODE);
	arutil_color_to_overlay_palette(ARvsg::instance().background_color(), 1);
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1);
	vsgSetDrawPage(vsgOVERLAYPAGE, 1, 1);
}


int CMouseUStim::callback(int &output, const FunctorCallbackTrigger* ptrig, const std::string&)
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

	// clear all dig outputs
	vsgIOWriteDigitalOut(0, 0xff);
	vsgPresent();

	// initialize triggers
	TSpecificFunctor<CMouseUStim> functor(this, &CMouseUStim::callback);
	init_triggers(&functor);
	if (m_arguments.verbose)
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
	cout << "Issuing ready pulse " << m_arguments.pulse << endl;
	vsg.ready_pulse(100, m_arguments.pulse);

	// There used to be code here to run with a client on another machine. The client would send JSON here with mouse position info. 

	doMouseKBLoop();

	// Have to turn off OVERLAYMASKMODE before leaving! The call to ARvsg::init_overlay enables
	// mask mode, this will turn it off.
	vsgSetCommand(vsgOVERLAYDISABLE);

	return;
}

void CMouseUStim::doMouseKBLoop()
{
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
		if (m_arguments.sleepMS > 0)
			Sleep(m_arguments.sleepMS);

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
		if (m_arguments.isAlert && !bUseManualTriggers)
		{
			int din = vsgIOReadDigitalIn();
			TriggerFunc	tf = std::for_each(triggers().begin(), triggers().end(), TriggerFunc(din, last_output_trigger));

			if (tf.quit()) bQuit = true;
			else if (tf.present())
			{	
				// The use of vsgIODigitalWriteOut here means that the output triggers appear as-is at the 
				// spike2 end. When we use vsgObjSetTriggers the bits are shifted left by one because the 
				// VSG takes the lowest order output bit for itself, and when we output bit 0x1 it is sent
				// on DOUT1 (not DOUT0).
				last_output_trigger = tf.output_trigger();
				vsgIOWriteDigitalOut(tf.output_trigger()<<1, 0xff);
			}
		}

		// flip overlay page, then draw aperture (and fixpt if needed).
		flip_draw_overlay(m_arguments.bHaveFixpt && m_bFixationOn && !m_bFixationDisabled, m_arguments.fixpt.x, m_arguments.fixpt.y, m_arguments.fixpt.d, degVSGMouseX, degVSGMouseY, m_arguments.grating.w);

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
							m_arguments.grating.w = m_arguments.grating.h = a;
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
							c.setCustom(luminance);
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
							m_arguments.grating.sf = s;
							flip_draw_grating();
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
							m_arguments.grating.setTemporalFrequency(d);
							m_tfPrevious = 0;
							flip_draw_grating();
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
					double tmp = m_arguments.grating.tf;
					m_arguments.grating.setTemporalFrequency(m_tfPrevious);
					m_tfPrevious = tmp;
					flip_draw_grating();
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
							m_arguments.grating.setContrast(c);
							flip_draw_grating();
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
							m_arguments.grating.orientation = oo;
							flip_draw_grating();
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
			case 'z': 
				{
					m_bStimulusDisabled = !m_bStimulusDisabled;
					cerr << "Stim disabled? " << m_bStimulusDisabled << endl;
					flip_draw_grating();
					vsgPresent();
					break;
				}
			case 'x':
				{
					m_bFixationDisabled = !m_bFixationDisabled;
					cerr << "fixpt disabled? " << m_bFixationDisabled << endl;
					break;
				}
			case 'p':
			case 'v':
				{
					cout << "X, Y (pixels)             = " << pixVSGMouseX << ", " << -pixVSGMouseY << endl;
					cout << "X, Y (degrees)            = " << degVSGMouseX << ", " << -degVSGMouseY << endl;
					cout << "Diameter (deg)            = " << m_arguments.grating.w << endl;
					cout << "Orientation(deg)          = " << m_arguments.grating.orientation << endl;
					cout << "Spatial Freq(cycles/deg)  = " << m_arguments.grating.sf << endl;
					cout << "Temporal Freq(cycles/sec) = " << m_arguments.grating.tf << endl;
					cout << "Contrast(%)               = " << m_arguments.grating.contrast << endl;
					break;
				}
			case 27:
			case 'q':
				{
					if (m_arguments.allowQuit)
					{
						COLOR_TYPE c;
						bQuit = true;

						// in case background luminance was changed, set it back to mean gray.
						c.setType(gray);
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
					c.setType(gray);
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
					xtemp = m_arguments.grating.x;
					ytemp = m_arguments.grating.y;
					m_arguments.grating.x = degVSGMouseX;
					m_arguments.grating.y = -degVSGMouseY;
					oss << m_arguments.grating;
					m_arguments.grating.x = xtemp;
					m_arguments.grating.y = ytemp;

					// Save to registry
					cout << "Saving stimulus string to registry: " << oss.str() << endl;
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
					cout << "Visibility controls:" << endl;
					cout << "---------------------------" << endl;
					cout << "<z> toggle stimulus on/off" << endl;
					cout << "<x> toggle fixpt on/off" << endl << endl;
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

void CMouseUStim::flip_draw_overlay(bool bFixationOn, double fixX, double fixY, double fixD, double apertureX, double apertureY, double apertureDiameter)
{
	m_overlayPage = 1 - m_overlayPage;
	vsgSetDrawPage(vsgOVERLAYPAGE, m_overlayPage, 1);

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
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, m_overlayPage);
}

void CMouseUStim::init_triggers(TSpecificFunctor<CMouseUStim>* pfunctor)
{
	triggers().clear();
	triggers().addTrigger(new FunctorCallbackTrigger("F", 0x2, 0x2, 0x2, 0x2, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("F", 0x2, 0x0, 0x2, 0x0, pfunctor));

	// quit trigger
	triggers().addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));

	return;
}


error_t parse_opt(int key, char* carg, struct argp_state* state)
{
	error_t ret = 0;
	struct cmouse_arguments* arguments = (struct cmouse_arguments*)state->input;
	std::string sarg;
	if (carg) sarg = carg;
	switch (key)
	{
		case 'S':
			if (parse_integer(sarg, arguments->sleepMS))
			{
				cerr << "Error in sleepMS arg, must be integer: " << sarg << endl;
				ret = EINVAL;
			}
			break;
		case 'A':
			arguments->isAlert = true;
			break;
		case 'q':
			arguments->allowQuit = true;
			break;
		case 'a':
			arguments->useBinaryTriggers = false;
			break;
		case 'b':
			if (parse_color(sarg, arguments->bkgdColor))
				ret = EINVAL;
			break;
		case 'p':
			if (parse_integer(sarg, arguments->pulse))
			{
				cerr << "Error in pulse arg: must be integer (0-7)." << endl;
				ret = EINVAL;
			}
			break;
		case 'v':
			arguments->verbose = true;
			break;
		case 'D':
			arguments->bPromptForDistance = true;
			break;
		case 'd':
			if (parse_distance(sarg, arguments->screenDistanceMM))
				ret = EINVAL;
			break;
		case 'f':
			if (parse_fixation_point(sarg, arguments->fixpt))
				ret = EINVAL;
			else
				arguments->bHaveFixpt = true;
			break;
		case 'g':
			if (parse_grating(sarg, arguments->grating))
			{
				ret = EINVAL;
			}
			else
				arguments->bHaveGrating = true;
			break;
		case 'r':
			arguments->sRegDumpFile = sarg;
			arguments->useRegDump = true;
			break;
		case 301:
			cout << "image list" << endl;
			arguments->bUseImages = true;
			break;
		case 0:
		{
			if (arguments->bPromptForDistance)
			{
				cout << "Enter screen distance in MM: ";
				cin >> arguments->screenDistanceMM;
				if (!cin)
				{
					cerr << "Bad data entered. Try again." << endl;
					ret = EINVAL;
				}
			}
			else if (arguments->screenDistanceMM == 0)
			{
				cerr << "No screen distance supplied - checking registry..." << endl;
				if (GetRegScreenDistance(arguments->screenDistanceMM))
				{
					cerr << "Got registry value for screen distance = " << arguments->screenDistanceMM << endl;
				}
				else
				{
					cerr << "Screen distance not supplied (-d) and registry value not found." << endl;
					ret = EINVAL;
				}
			}

			if (arguments->isAlert && !arguments->bHaveFixpt)
			{
				cerr << "No fixpt specs supplied (-f)." << endl;
				ret = EINVAL;
			}

			if (!arguments->bHaveGrating)
			{
				cerr << "No grating specs supplied (-g): starting with default values." << endl;
				arguments->grating.x = arguments->grating.y = 0;
				arguments->grating.w = arguments->grating.h = 5;
				arguments->grating.sf = 3;
				arguments->grating.tf = 4;
				arguments->grating.orientation = 45;
				arguments->grating.contrast = 100;
				arguments->grating.swt = sinewave;
				arguments->grating.twt = sinewave;
				arguments->grating.aperture = ellipse;
				arguments->grating.cv.setType(b_w);
				arguments->grating.ttf = 0;
			}
			break;
		}
		default:
		{
			ret = ARGP_ERR_UNKNOWN;
			break;
		}
	}
	return ret;
}

bool CMouseUStim::parse(int argc, char** argv)
{
	return !argp_parse(&f_argp, argc, argv, 0, 0, &m_arguments);
}

void CMouseUStim::joystickPositionToVSGDrawDegrees(double joyX, double joyY, double *pvsgDegX, double *pvsgDegY)
{
	double vsgPixelsX = joyX / 100.0 * m_vsgWidthPixels / 2;
	double vsgPixelsY = joyY / 100.0 * m_vsgHeightPixels / 2;
	vsgPixelsToVSGDrawDegrees((int)vsgPixelsX, (int)vsgPixelsY, pvsgDegX, pvsgDegY);
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

