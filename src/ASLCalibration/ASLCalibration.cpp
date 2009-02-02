// ASLCalibration.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ASLCalibration.h"
#include "ASLSerial.h"

#include "VSGV8.H"
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__
#include "alertlib.h"
#include "AlertUtil.h"


COLOR_TYPE m_background;
int m_screenDistanceMM=0;
bool m_verbose=false;
double m_dCalibrationOffset = 0.0;
ARFixationPointSpec m_afp;
PIXEL_LEVEL m_fixation_level;

int init_calibration();
void usage();
int init_pages();
void init_page(int ipage, void *unused);
int args(int argc, char **argv);




using namespace std;
using namespace alert;

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;


	// Check input arguments
	if (args(argc, argv))
	{
		return 1;
	}
	else
	{
		if (m_verbose)
		{
			cout << "Screen distance " << m_screenDistanceMM << endl;
			cout << "Fixation point " << m_afp << endl;
			cout << "Background color " << m_background << endl;
		}
	}


	// COM initialzation
	CoInitialize(NULL);

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		nRetCode = 1;
	}
	else
	{
		cout << "Connecting to ASL controller...." << endl;

		if (aslserial_connect())
		{
			cerr << "Failed to connect to ASL serial port." << endl;
			nRetCode = 1;
		}
		else 
		{
			int idot;
			int lastdot=-1;


			// vsg initialization here. 
			init_calibration();


			// need a while loop ... how do we know the calibration is complete? 

			while (true)
			{
				if (!aslserial_dotnumber(&idot))
				{
					if (idot != lastdot)
					{
						// change dot on screen here
						cout << "New dot number: " << idot << endl;
						lastdot = idot;

						if (idot >= 0 && idot <= 9)
						{
							vsgSetZoneDisplayPage(vsgVIDEOPAGE, idot);
						}
					}
				}
				Sleep(100);
			}

		}
		
		aslserial_disconnect();
	}

	return nRetCode;
}


int init_calibration()
{
	int status=0;

	// INit vsg
	if (ARvsg::instance().init(m_screenDistanceMM, m_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}

	VSGTRIVAL fixation_color;
	if (get_color(m_afp.color, fixation_color))
	{
		cerr << "Cannot get trival for fixation color " << m_afp.color << endl;
		return 2;
	}
	vsgSetFixationColour(&fixation_color);

	// initialize video pages
	if (ARvsg::instance().init_video_pages(init_page, NULL, NULL))
	{
		cerr << "VSG video initialization failed!" << endl;
		return 1;
	}
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);

	
	
	
	
	return status;
}


void init_page(int ipage, void *unused)
{
	double x, y;
	bool bDraw=false;
	switch (ipage)
	{
	case 0: break;
	case 1: 
		bDraw = true;
		x = -m_dCalibrationOffset;
		y = m_dCalibrationOffset;
		break;
	case 2:
		bDraw = true;
		x = 0;
		y = m_dCalibrationOffset;
		break;
	case 3:
		bDraw = true;
		x = m_dCalibrationOffset;
		y = m_dCalibrationOffset;
		break;
	case 4: 
		bDraw = true;
		x = -m_dCalibrationOffset;
		y = 0;
		break;
	case 5:
		bDraw = true;
		x = 0;
		y = 0;
		break;
	case 6:
		bDraw = true;
		x = m_dCalibrationOffset;
		y = 0;
		break;
	case 7: 
		bDraw = true;
		x = -m_dCalibrationOffset;
		y = -m_dCalibrationOffset;
		break;
	case 8:
		bDraw = true;
		x = 0;
		y = -m_dCalibrationOffset;
		break;
	case 9:
		bDraw = true;
		x = m_dCalibrationOffset;
		y = -m_dCalibrationOffset;
		break;
	}

	if (bDraw)
	{
		cout << "draw page " << ipage << " " << x << "," << y << " level " << m_fixation_level << endl;
		vsgSetPen1(vsgFIXATION);
		vsgDrawOval(x, y, m_afp.d, m_afp.d);
	}
		
	return;
}


int args(int argc, char **argv)
{	
	bool have_f=false;
	bool have_d=false;
	bool have_offset = false;
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "f:b:hd:vC:")) != -1)
	{
		switch (c) 
		{
		case 'v':
			m_verbose = true;
			break;
		case 'f': 
			s.assign(optarg);
			if (parse_fixation_point(s, m_afp)) errflg++;
			else have_f = true;
			break;
		case 'b': 
			s.assign(optarg);
			if (parse_color(s, m_background)) errflg++; 
			break;
		case 'd':
			s.assign(optarg);
			if (parse_distance(s, m_screenDistanceMM)) errflg++;
			else have_d=true;
			break;
		case 'C':
			s.assign(optarg);
			if (parse_double(s, m_dCalibrationOffset)) errflg++;
			else have_offset = true;
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

	if (!have_f) 
	{
		cerr << "Fixation point not specified!" << endl; 
		errflg++;
	}
	if (!have_d)
	{
		cerr << "Screen distance not specified!" << endl; 
		errflg++;
	}
	if (!have_offset)
	{
		cerr << "Calibration dots offset not specified!" << endl; 
		errflg++;
	}
	if (errflg) 
	{
		usage();
	}
	return errflg;
}

void usage()
{
	cerr << "usage: calibration -f x,y,d[,color] -d screen_distance_MM -b g|b|w -C dot_offset_degrees" << endl;
}
