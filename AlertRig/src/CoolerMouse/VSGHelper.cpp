#include "VSGHelper.h"
#include <iostream>
#include <sstream>

using namespace std;

VSGHelper::VSGHelper(bool useVSG, int distToScreen, COLOR_TYPE bg) : m_bUsingVSG(false), m_bHaveFP(false), m_bHaveStim(false)
{
	m_bUsingVSG = useVSG;

	if (m_bUsingVSG)
	{
		// Initialize vsg
		if (alert::ARvsg::instance().init(distToScreen, bg))
		{
			cerr << "VSG init failed!" << endl;
		}
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

void VSGHelper::start() {};
void VSGHelper::stop() {};

void VSGHelper::setStimXY(double x, double y)
{
	m_stim.x = x;
	m_stim.y = y;
}

void VSGHelper::setFixationPointXY(double x, double y)
{
	m_fp.x = x;
	m_fp.y = y;
}


