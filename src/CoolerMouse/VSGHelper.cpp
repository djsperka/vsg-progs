#include "VSGHelper.h"
#include <iostream>
#include <sstream>

using namespace std;
using namespace alert;

VSGHelper::VSGHelper(bool useVSG, int distToScreen, COLOR_TYPE bg) : m_bUsingVSG(false), m_bHaveFP(false), m_bHaveStim(false), m_bDirtyStim(false), m_bDirtyAperture(false), m_iOverlayPage(0), m_iPage(0)
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

};

void VSGHelper::stop() {};

void VSGHelper::setStimXY(double x, double y)
{
	m_stim.x = x;
	m_stim.y = y;
	m_bDirtyAperture = true;
}

void VSGHelper::setFixationPointXY(double x, double y)
{
	m_fp.x = x;
	m_fp.y = y;
}

// Set values in stimulus, set dirty flag
void VSGHelper::setApertureProperties(double x, double y, double diam, APERTURE_TYPE a)
{
	m_stim.x = x;
	m_stim.y = y;
	m_stim.w = m_stim.h = diam;
	m_stim.aperture = a;
	m_bDirtyAperture = true;
}

void VSGHelper::setStimProperties(double sf, double tf, int contrast, double orient, PATTERN_TYPE p, COLOR_VECTOR_TYPE cv)
{
	m_stim.sf = sf;
	m_stim.tf = tf;
	m_stim.contrast = contrast;
	m_stim.orientation = orient;
	m_stim.pattern = p;
	m_stim.cv = cv;
	m_bDirtyStim = true;
	update();
}




void VSGHelper::update()
{
	// m_dirty indicates the overlay must be redrawn. That means either the stim diameter
	// changed, stim position changed, fixation point properties changed. 
	if (m_bDirtyAperture)
	{
		m_iOverlayPage = 1-m_iOverlayPage;
		vsgSetDrawPage(vsgOVERLAYPAGE, m_iOverlayPage, 1);
		m_stim.drawOverlay();
	}

	if (m_bDirtyStim)
	{
		m_iPage = 1 - m_iPage;
		vsgSetDrawPage(vsgVIDEOPAGE, m_iPage, vsgBACKGROUND);
		m_stim.draw();
	}

	if (m_bDirtyStim || m_bDirtyAperture)
	{
		vsgPresent();
		m_bDirtyStim = false;
		m_bDirtyAperture = false;
	}
}