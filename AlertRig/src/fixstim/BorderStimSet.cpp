#include "BorderStimSet.h"
#include "AlertUtil.h"
#define _USE_MATH_DEFINES
#include <math.h>

class BorderStimSet;
BorderStimSet * parseBorderStimSet(const std::string & s)
{
	// comma-separated
	std::vector<std::string> tokens;
	tokenize(s, tokens, ",");
	if (tokens.size() < 7)
	{
		cerr << "Error parsing BorderStimSet: must be at least 7 args" << endl;
		return nullptr;
	}

	double x, y;
	double s0, s1;
	COLOR_TYPE c0, c1;
	if (parse_double(tokens[0], x) || parse_double(tokens[1], y))
	{
		cerr << "parseBorderStimSet: bad x,y value(s) in args 0,1: " << tokens[0] << "," << tokens[1] << endl;
		return nullptr;
	}

	if (parse_double(tokens[2], s0) || s0 < .01 || parse_double(tokens[3], s1) || s1 < 0.01)
	{
		cerr << "parseBorderStimSet: bad size value(s) in args 2,3: " << tokens[2] << "," << tokens[3] << endl;
		return nullptr;
	}

	if (parse_color(tokens[4], c0) || parse_color(tokens[5], c1))
	{
		cerr << "parseBorderStimSet: bad color value(s) in args 4,5: " << tokens[4] << "," << tokens[5] << endl;
		return nullptr;
	}

	// parse remaining values as stim specs......
	std::vector<std::string> specs(tokens.cbegin() + 6, tokens.cend());
	std::vector<int> stim;
	if (parse_int_list(specs, stim))
	{
		cerr << "parseBorderStimSet: bad spec list: " << s << endl;
		return nullptr;
	}

	cerr << "Got specs: ";
	for (auto it = stim.cbegin(); it != stim.cend(); it++)
		cout << ' ' << std::hex << (unsigned int)(*it);
	cout << endl;

	return new BorderStimSet(x, y, s0, s1, c0, c1, stim);
}

BorderStimSet::BorderStimSet(double x, double y, double size0, double size1, COLOR_TYPE c0, COLOR_TYPE c1, vector<int>& stim)
	: m_xrf(x)
	, m_yrf(y)
	, m_size0(size0)
	, m_size1(size1)
	, m_color0(c0)
	, m_color1(c1)
	, m_vecStim(stim)
	, m_bHasFixpt(false)
{

}

BorderStimSet::BorderStimSet(const ARContrastFixationPointSpec& fixpt, double x, double y, double size0, double size1, COLOR_TYPE c0, COLOR_TYPE c1, vector<int>& stim)
	: m_fixpt(fixpt)
	, m_bHasFixpt(true)
	, m_xrf(x)
	, m_yrf(y)
	, m_size0(size0)
	, m_size1(size1)
	, m_color0(c0)
	, m_color1(c1)
	, m_vecStim(stim)
{
	cerr << "WARNING: fixpt not implemented!!!" << endl;
}



std::string BorderStimSet::toString() const
{
	std::string s("BorderStimSet");
	return s;
}


int BorderStimSet::init(std::vector<int> pages)
{
	// first page is for background only
	m_pageBackground = pages[0];
	m_pageStimulus = pages[1];
	m_pageStimulusFixpt = pages[2];

	// allocate a spot for the two colors we need.....
	ARvsg::instance().request_single(m_levelColor0);
	arutil_color_to_palette(m_color0, m_levelColor0);
	ARvsg::instance().request_single(m_levelColor1);
	arutil_color_to_palette(m_color1, m_levelColor1);

	// init the rect levels, but don't bother setting x,y, color, size until drawCurrent()
	m_rect.init(2);

	// background page will not change
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageBackground, vsgBACKGROUND);

	m_uiCurrentTrial = 0;
	drawCurrent();

	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	return 0;
}

// handle the trigger indicated by the string s. Do not call vsgPresent! return value of 
// 1 means vsgPresent() will be called. 
int BorderStimSet::handle_trigger(const std::string& s, const std::string&)
{
	int status = 0;
	if (s == "F")
	{
		cerr << "Fixpt not supported" << endl;
	}
	else if (s == "S")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageStimulus, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "a")
	{
		m_uiCurrentTrial++;
		if (m_uiCurrentTrial >= m_vecStim.size()) m_uiCurrentTrial = 0;
		drawCurrent();
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageBackground, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "X")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageBackground, vsgNOCLEAR);
		status = 1;
	}
	return status;
}


int BorderStimSet::drawCurrent()
{
	// background of page will depend on local color chosen. 
	// lc==0 means shape is color0, background is color1
	// lc==1 means shape is color1, background is color0
	// b_lc1 just reflects whether the LC_BIT is set, and whether the rectangle uses local color 1. 

	bool b_lc1 = (m_vecStim[m_uiCurrentTrial] & LC_BIT);
	PIXEL_LEVEL bkgdLevel = (b_lc1 ? m_levelColor0 : m_levelColor1);
	COLOR_TYPE bkgdColor = (b_lc1 ? m_color0 : m_color1);
	PIXEL_LEVEL rectLevel = (b_lc1 ? m_levelColor1 : m_levelColor0);
	COLOR_TYPE rectColor = (b_lc1 ? m_color1 : m_color0);
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageStimulus, bkgdLevel);

	m_rect.color = rectColor;
	m_rect.setContrast(100);
	m_rect.drawmode = 0;

	// lowest three bits are orientation of rect
	double ori = (m_vecStim[m_uiCurrentTrial] & 0x7) * 45;
	m_rect.orientation = ori;

	// size of rect
	double rectSize = (m_vecStim[m_uiCurrentTrial] & SZ_BIT ? m_size1 : m_size0);
	m_rect.w = m_rect.h = rectSize;

	// set center of rect
	m_rect.x = m_xrf + 0.5 * rectSize * cos(ori*M_PI / 180.0);
	m_rect.y = m_yrf + 0.5 * rectSize * sin(ori*M_PI / 180.0);

	// now draw
	m_rect.draw();

	cerr << "rect: " << m_rect << endl;
	return 0;
}
