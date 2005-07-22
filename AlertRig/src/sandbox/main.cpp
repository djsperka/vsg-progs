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
ARGratingSpec m_stim0;
ARGratingSpec m_stim1;
ARFixationPointSpec m_fp1;
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


	parse_grating(string("-3,0,2,2,100,2,2,0,b,s,e"), m_stim0);
	parse_grating(string("-3,0,2,2,100,2,2,0,b,s,e"), m_stim1);
	parse_fixation_point(string("0,0,2,red"), m_fp1);
//	parse_grating(string("3,0,2,2,100,2,2,90,b,s,e"), m_stim1);

//	init_pages();

	if (ARvsg::instance().init_overlay())
	{
		cerr << "VSG overlay initialization failed!" << endl;
		return 1;
	}
	vsgSetDrawPage(vsgOVERLAYPAGE, 1, vsgNOCLEAR);
//	m_fp1.init(1);
	m_fp1.drawOverlay();
	m_stim1.drawOverlay();

	vsgSetDrawPage(vsgOVERLAYPAGE, 0, vsgNOCLEAR);
	m_stim0.drawOverlay();


	// initialize video pages
	if (ARvsg::instance().init_video_pages(NULL, initfunc, NULL))
	{
		cerr << "VSG video initialization failed!" << endl;
		return 1;
	}

	string s;
	cout << "Enter 0/1: ";
	cin >> s;
	while (s != "q" && s!= "Q")
	{
		if (s=="0")
		{
			vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
			vsgPresent();
		}
		else if (s=="1")
		{
			vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
			vsgPresent();
		}
		else if (s=="a")
		{
			m_dCurrentOri += 30;
			m_stim0.orientation = m_dCurrentOri;
			m_stim0.drawOnce();
			vsgPresent();
		}
		else if (s=="s")
		{
			// swap pages
			m_ipage = 1-m_ipage;
			vsgSetDrawPage(vsgVIDEOPAGE, m_ipage, vsgNOCLEAR);
			vsgPresent();
		}
		else if (s=="A")
		{
			m_ipage = 1-m_ipage;
			vsgSetDrawPage(vsgOVERLAYPAGE, m_ipage, vsgNOCLEAR);
			vsgSetDrawPage(vsgVIDEOPAGE, m_ipage, vsgNOCLEAR);
			m_dCurrentOri += 30;
			if (m_ipage == 0)
			{
				m_stim0.orientation = m_dCurrentOri;
				m_stim0.draw();
			}
			else if (m_ipage = 1)
			{
				m_stim1.orientation = m_dCurrentOri;
				m_stim1.draw();
			}
			vsgPresent();
		}
		else if (s=="B")
		{
//			m_ipage = 1-m_ipage;
//			vsgSetDrawPage(vsgVIDEOPAGE, m_ipage, vsgNOCLEAR);
			m_dCurrentOri += 30;
			m_stim0.orientation = m_dCurrentOri;
			m_stim0.draw();
			vsgPresent();
		}


		cout << "Enter 0/1/a/s: ";
		cin >> s;
	}

	ARvsg::instance().clear(0);

	return 0;
}


void initfunc(int ipage, void *data)
{
	if (ipage==0)
	{
		m_stim0.init(50);
		m_stim0.draw();
	}
	else if (ipage==1)
	{
		m_stim1.init(50);
		m_stim1.draw();
	}
}


void init_pages()
{
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	m_stim0.init(50);
	m_stim0.drawOnce();
	vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
	m_stim1.init(50);
	m_stim1.drawOnce();
///	vsgPresent();
}