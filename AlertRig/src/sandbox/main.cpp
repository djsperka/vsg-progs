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

#define BACKGROUND_PAGE 0
#define FIXATION_PAGE 1
#define STIMULUS_PAGE 2

int m_screenDistanceMM = 555;
COLOR_TYPE m_background = gray;
VSGOBJHANDLE m_handle0;
VSGOBJHANDLE m_handle1;
ARGratingSpec m_stim;
ARContrastFixationPointSpec m_fp;
double m_dCurrentOri = 0;
int m_ipage=0;

void init_pages();
void initfunc(int ipage, void *data);

int main(int argc, char **argv)
{
	
	// Init vsg
	if (ARvsg::instance().init(m_screenDistanceMM, m_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}


	parse_grating(string("-3,0,2,2,100,2,2,0,b,s,e"), m_stim);
	parse_fixation_point(string("0,0,2,red"), m_fp);

	init_pages();

//	m_fp1.init(1);
//	m_fp1.drawOverlay();
//	m_stim1.drawOverlay();

//	vsgSetDrawPage(vsgOVERLAYPAGE, 0, vsgNOCLEAR);
//	m_fp0.drawOverlay();
//	m_stim0.drawOverlay();



	string s;
	cout << "Enter f/F/s/S/a/q: ";
	cin >> s;
	while (s != "q" && s!= "Q")
	{
		if (s=="f")
		{
			m_fp.setContrast(0);
			vsgPresent();
		}
		else if (s=="F")
		{
			m_fp.setContrast(100);
			vsgPresent();
		}
		else if (s=="s")
		{
			m_stim.setContrast(0);
			vsgPresent();
		}
		else if (s=="S")
		{
			m_stim.setContrast(100);
			vsgPresent();
		}
		else if (s=="a")
		{
			m_dCurrentOri += 30;
			m_stim.orientation = m_dCurrentOri;
			m_stim.redraw(true);
			vsgPresent();
		}


		cout << "Enter f/F/s/S/a/q: ";
		cin >> s;
	}

	ARvsg::instance().clear(0);

	return 0;
}



void init_pages()
{
	// initialize video pages
	if (ARvsg::instance().init_video())
	{
		cerr << "VSG video initialization failed!" << endl;
	}

	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);

	m_fp.init(2);
	m_fp.draw();

	m_stim.init(50);
	m_stim.drawOnce();
	vsgPresent();

}