#include "VSGHelper.h"
#include <iostream>
#include <sstream>

using namespace std;
using namespace alert;

VSGHelper::VSGHelper(bool useVSG, int distToScreen, COLOR_TYPE bg) : m_bUsingVSG(false), m_bHaveFP(false), m_bHaveStim(false), m_bDirtyStim(false), m_bDirtyAperture(false), m_iOverlayPage(0), m_iPage(0), m_bStimFixed(true), m_bFPFixed(true)
{
	m_bUsingVSG = useVSG;
	long lScreenWidthPixels=0;
	long lScreenHeightPixels=0;

	if (m_bUsingVSG)
	{
		// Initialize vsg
		if (alert::ARvsg::instance().init(distToScreen, bg))
		{
			cerr << "VSG init failed!" << endl;
		}

		lScreenWidthPixels = vsgGetSystemAttribute(vsgSCREENWIDTH);
		lScreenHeightPixels = vsgGetSystemAttribute(vsgSCREENHEIGHT);
		vsgUnit2Unit(vsgPIXELUNIT, (double)lScreenWidthPixels, vsgDEGREEUNIT, &m_dScreenWidthDegrees);
		vsgUnit2Unit(vsgPIXELUNIT, (double)lScreenHeightPixels, vsgDEGREEUNIT, &m_dScreenHeightDegrees);
	}
}

void VSGHelper::setStimString(std::string s)
{
	if (parse_grating(s, m_stim))
	{
		cerr << "cannot parse stim string!" << endl;
	}

}

std::string VSGHelper::getStimString()
{
	std::ostringstream oss;
	oss << m_stim;
	return oss.str();
}


void VSGHelper::setFixationPointString(std::string s)
{
	if (parse_fixation_point(s, m_fp))
	{
		cerr << "cannot parse fixpt string!" << endl;
	}
}
	
std::string VSGHelper::getFixationPointString()
{
	std::ostringstream oss;
	oss << m_fp;
	return oss.str();
}

void VSGHelper::start() 
{
	// Initialize and draw overlay page
	if (ARvsg::instance().init_overlay())
	{
		cerr << "VSG overlay initialization failed!" << endl;
		return;
	}
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1);

	// Now draw overlay stuff BEFORE any of the vsg object system is in use. 
	m_fp.drawOverlay();
	m_stim.drawOverlay();
	m_iOverlayPage = 0;

	// initialize video pages
	if (ARvsg::instance().init_video())
	{
		cerr << "VSG video initialization failed!" << endl;
		return;
	}
	vsgSetDrawPage(vsgVIDEOPAGE, m_iPage, vsgNOCLEAR);

	m_stim.init(40);
	m_stim.draw();
	vsgPresent();


	// send ready signal on dig line 6
	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x20, 0);
	vsgPresent();

	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x00, 0);
	vsgPresent();


};

void VSGHelper::stop() 
{
	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x20, 0);
	vsgPresent();
};

void VSGHelper::drawOverlay(BOOL bpresent)
{
//	m_iOverlayPage = 1 - m_iOverlayPage;
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1);
	m_fp.drawOverlay();
	m_stim.drawOverlay();
	if (bpresent) vsgPresent();
}


void VSGHelper::setStimXY(double x, double y)
{
	m_stim.x = x;
	m_stim.y = y;
	drawOverlay(FALSE);
}


void VSGHelper::setStimSF(double sf)
{
	m_stim.sf = sf;
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	m_stim.draw();
	vsgPresent();
}


void VSGHelper::setStimTF(double tf)
{
	m_stim.tf = tf;
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	m_stim.draw();
	vsgPresent();
}

void VSGHelper::setStimOrientation(long orientation)
{
	m_stim.orientation = orientation;
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	m_stim.draw();
	vsgPresent();
}

void VSGHelper::setStimContrast(int contrast)
{
	m_stim.contrast = contrast;
	m_stim.select();
	m_stim.setContrast(contrast);
	vsgPresent();
}

void VSGHelper::setStimColorVector(int cv)
{
	switch(cv)
	{
	case 0:
		m_stim.cv = b_w;
		break;
	case 1:
		m_stim.cv = l_cone;
		break;
	case 2:
		m_stim.cv = m_cone;
		break;
	case 3:
		m_stim.cv = s_cone;
		break;
	}
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	m_stim.draw();
	vsgPresent();

}

void VSGHelper::setStimPattern(int pattern)
{
	switch(pattern)
	{
	case 0:
		m_stim.pattern = sinewave;
		break;
	case 1:
		m_stim.pattern = squarewave;
		break;
	}
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	m_stim.draw();
	vsgPresent();
}

void VSGHelper::setStimAperture(int aperture)
{
	switch(aperture)
	{
	case 0:
		m_stim.aperture = ellipse;
		break;
	case 1:
		m_stim.aperture = rectangle;
		break;
	}
	drawOverlay(FALSE);

	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	m_stim.draw();
	vsgPresent();
}


void VSGHelper::setStimSize(double diameter)
{
	m_stim.h = m_stim.w = diameter;
	drawOverlay(FALSE);
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	m_stim.draw();
	vsgPresent();
}


void VSGHelper::setFixationPointXY(double x, double y)
{
	m_fp.x = x;
	m_fp.y = y;
	drawOverlay(TRUE);
}


void VSGHelper::setFixationColor(int color)
{
	switch(color)
	{
	case 0:
		m_fp.color = red;
		break;
	case 1:
		m_fp.color = green;
		break;
	case 2:
		m_fp.color = blue;
		break;
	}
	drawOverlay(FALSE);
}

void VSGHelper::setFixationDiameter(double diameter)
{
	m_fp.d = diameter;
	drawOverlay(TRUE);
}


