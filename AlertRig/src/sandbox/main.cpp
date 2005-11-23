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

#pragma comment(lib, "vsgv8.lib")

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

#if 0
	if (ARvsg::instance().init_overlay())
	{
		cerr << "VSG overlay init failed!" << endl;
	}
#endif
	if (ARvsg::instance().init_video())
	{
		cerr << "VSG video init failed!" << endl;
	}

	init_pages();
//	vsgSetDrawPage(vsgOVERLAYPAGE, 0, vsgNOCLEAR);
	vsgPresent();


	string s;
	cout << "Enter 0/1/q: ";
	cin >> s;
	while (s != "q" && s!= "Q")
	{
		if (s=="0")
		{
//			vsgSetDrawPage(vsgOVERLAYPAGE, 0, vsgNOCLEAR);
			vsgPresent();
		}
		else if (s=="1")
		{
//			vsgSetDrawPage(vsgOVERLAYPAGE, 1, vsgNOCLEAR);
			vsgPresent();
		}

		cout << "Enter 0/1/q: ";
		cin >> s;
	}

	ARvsg::instance().clear(0);

	return 0;
}



void init_pages()
{
	vsgSetCommand(vsgPALETTERAMP);
	vsgSetDrawPage(vsgVIDEOPAGE, 0, 1);

//	VSGLUTBUFFER OverlayBuffer;

//	OverlayBuffer[0].a=1; OverlayBuffer[0].b=0; OverlayBuffer[0].c=0;
//	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&OverlayBuffer[0], 2, 1);
//	vsgSetDrawPage(vsgOVERLAYPAGE, 1, 2);

}