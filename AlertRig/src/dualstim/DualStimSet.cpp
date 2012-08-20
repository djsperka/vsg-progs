#include "DualStimSet.h"
#include <iostream>
using namespace std;

int StimSetBase::get_frames_on()
{ 
	// m_nframes_on default value is 1. Lazy initialize it...
	if (m_nframes_on<2 && m_dstimtime_sec > 0)
	{
		// vsgFRAMETIME is in usec, so convert stimtime to usecs to get nframes 
		m_nframes_on = (int)(m_dstimtime_sec * 1000000 /(double)vsgGetSystemAttribute(vsgFRAMETIME));
	}
	return m_nframes_on; 
};

void StimSetBase::set_fixpt(const ARFixationPointSpec& fixpt, double xoffset, double yoffset)
{
	m_have_fixpt = true;
	m_fixpt = fixpt;
	m_fixpt.x += xoffset;
	m_fixpt.y += yoffset;
	return;
}

void StimSetBase::set_xhair(const ARXhairSpec& xhair, double xoffset, double yoffset)
{
	m_have_xhair = true;
	m_xhair = xhair;
	m_xhair.x += xoffset;
	m_xhair.y += yoffset;
	return;
}

void StimSetSingleGrating::set_grating(const ARGratingSpec& grating, double xoffset, double yoffset)
{
	m_have_grating = true;
	m_grating = grating;
	m_grating.x += xoffset;
	m_grating.y += yoffset;
	return;
}

void StimSetMultipleGrating::set_grating(const ARGratingSpec& grating, double xoffset, double yoffset)
{
	ARGratingSpec *temp = new ARGratingSpec(grating);
	m_have_grating = true;
	temp->x += xoffset;
	temp->y += yoffset;
	m_gratings.push_back(temp);
	m_contrasts.push_back(temp->contrast);
	return;
}
