#include "PlaidStimSet.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>

PlaidStimSet::PlaidStimSet(const ARGratingSpec& g1, const ARGratingSpec& g2, const std::vector<int> vecContrast, const std::vector<double> vecSF, const std::vector<double> vecTF, const std::vector<double>& oris)
	: StimSet()
	, m_plaid()
	, m_vecContrast(vecContrast)
	, m_vecSF(vecSF)
	, m_vecTF(vecTF)
	, m_vecOriDegrees(oris)
{
	alert::ARPlaidSubGr gr1(g1.x, g1.y, g1.w, g1.h, g1.contrast, g1.sf, g1.tf, g1.orientation);
	alert::ARPlaidSubGr gr2(g2.x, g2.y, g2.w, g2.h, g2.contrast, g2.sf, g2.tf, g2.orientation);
	m_plaid.setG1(gr1);
	m_plaid.setG2(gr2);
}

PlaidStimSet::PlaidStimSet(const ARGratingSpec& g1, const ARGratingSpec& g2, int contrast, double sf, double tf, double ori)
	: StimSet()
	, m_plaid()
{
	alert::ARPlaidSubGr gr1(g1.x, g1.y, g1.w, g1.h, g1.contrast, g1.sf, g1.tf, g1.orientation);
	alert::ARPlaidSubGr gr2(g2.x, g2.y, g2.w, g2.h, g2.contrast, g2.sf, g2.tf, g2.orientation);
	m_plaid.setG1(gr1);
	m_plaid.setG2(gr2);
	m_vecContrast.push_back(contrast);
	m_vecSF.push_back(sf);
	m_vecTF.push_back(tf);
	m_vecOriDegrees.push_back(ori);
}


int PlaidStimSet::init(ARvsg& vsg, std::vector<int> pages)
{
	m_pages[0] = pages[0];
	m_pages[1] = pages[1];
	m_pageIndex = 0;

	// set pan scroll mode - will need wider pages for drifting the plaid
	vsgSetVideoMode(vsgPANSCROLLMODE);
	vsgSetCommand(vsgVIDEODRIFT);
	vsgSetDrawPage(vsgVIDEOPAGE, m_pages[m_pageIndex], vsgBACKGROUND);
	vsgSetSpatialUnits(vsgPIXELUNIT);

	// arbitrarily taking many levels. 
	cerr << "draw plaid" << endl;
	m_plaid.init(240);
	m_current = 0;
	m_plaid.setContrast(0);
	m_plaid.setOriDegrees(m_vecOriDegrees[m_current % m_vecOriDegrees.size()]);

	double ppd;
	double xpos, ypos;
	vsgUnit2Unit(vsgDEGREEUNIT, 1, vsgPIXELUNIT, &ppd);
	getDriftPos(1 / m_vecTF[m_current % m_vecTF.size()], ppd, m_vecTF[m_current % m_vecTF.size()], m_plaid.g1(), m_plaid.g2(), xpos, ypos);
	cerr << "drift max " << xpos << "," << ypos << endl;

	m_plaid.drawDriftingPlaid(xpos, ypos);
	setupCycling(ppd, m_vecTF[m_current % m_vecTF.size()], m_plaid.g1(), m_plaid.g2(), xpos, ypos);
	vsgPresent();

	return 0;
}

int PlaidStimSet::handle_trigger(std::string& s)
{
	int status = 0;
	if (s == "F")
	{
		status = 1;
	}
	else if (s == "S")
	{
		m_plaid.select();
		m_plaid.setContrast(m_vecContrast[m_current % m_vecContrast.size()]);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "s")
	{
		m_plaid.setContrast(0);
		status = 1;
	}
	else if (s == "a")
	{
		// Note that after this trigger is completed, the current draw page
		// is the NEXT stimulus page to be shown. 

		vsgSetDrawPage(vsgVIDEOPAGE, m_pages[1 - m_pageIndex], vsgBACKGROUND);
		m_pageIndex = 1 - m_pageIndex;

		m_current++;

		cerr << m_vecContrast.size() << " " << m_vecSF.size() << " " << m_vecTF.size() << " " << m_vecOriDegrees.size() << " " << endl;
		m_plaid.setOriDegrees(m_vecOriDegrees[m_current % m_vecOriDegrees.size()]);

		m_plaid.setContrast(0);

		double ppd;
		double xpos, ypos;
		vsgUnit2Unit(vsgDEGREEUNIT, 1, vsgPIXELUNIT, &ppd);
		getDriftPos(1/m_vecTF[m_current % m_vecTF.size()], ppd, m_vecTF[m_current % m_vecTF.size()], m_plaid.g1(), m_plaid.g2(), xpos, ypos);
		cerr << "drift max " << xpos << "," << ypos << endl;

		m_plaid.drawDriftingPlaid(xpos, ypos);
		setupCycling(ppd, m_vecTF[m_current % m_vecTF.size()], m_plaid.g1(), m_plaid.g2(), xpos, ypos);

		status = 0;
	}
	else if (s == "X")
	{
		m_plaid.setContrast(0);
		status = 1;
	}
	else if (s == "A")
	{
		int c;
		string stmp;
		cerr << "Enter contrast: ";
		cin >> stmp;

		// if s is a number, treat it as a contrast.
		istringstream iss(stmp);
		iss >> c;
		if (iss)
		{
			m_plaid.setContrast(c);
			status = 1;
		}
		else
		{
			cerr << "Error reading input." << endl;
			status = 0;
		}
	}
	return status;
}

std::string PlaidStimSet::toString() const
{
	return std::string("PlaidStimSet");
}

void PlaidStimSet::getDriftPos(double t, double ppd, double tf, const alert::ARPlaidSubGr& gr1, const alert::ARPlaidSubGr& gr2, double& xpos, double& ypos)
{
	// The values returned assume a coord system where x, y are pos right, up. 
	// When using values from this method to use in vsg cycling setup, remember that the coord system 
	// for that call is y pos down! The args for Xpos, Ypos specify the movement of the viewing window relative
	// to the unmoved case. A value of Xpos > 0 moves viewing window to the right, and it appears that the stim moves left. 
	// A value of Ypos > 0 moves viewing window down, and the stim appears to move UP. 
	xpos = t * tf * ppd / (gr1.sf * gr2.sf) * (gr1.sf * gr1.salpha() - gr2.sf * gr2.salpha()) / (gr1.salpha() * gr2.calpha() - gr1.calpha() * gr2.salpha());
	ypos = t * tf * ppd / (gr1.sf * gr2.sf) * (gr2.sf * gr2.calpha() - gr1.sf * gr1.calpha()) / (gr1.salpha() * gr2.calpha() - gr1.calpha() * gr2.salpha());
}

void PlaidStimSet::setupCycling(double ppd, double tf, const alert::ARPlaidSubGr& gr1, const alert::ARPlaidSubGr& gr2, double endXDrift, double endYDrift)
{
	int status = 0;
	unsigned int frameUS;	// microseconds per frame
	VSGCYCLEPAGEENTRY cycle[32768];	// warning! No check on usage. You have been warned. 

	frameUS = vsgGetSystemAttribute(vsgFRAMETIME);

	// drift viewing window for one full cycle, set last frame of cycle on repeat.
	unsigned int nFramesPerCycle = 1 / tf * 1 / frameUS * 1.0e6;

	// check that frames per cycle is do-able
	if (nFramesPerCycle < vsgGetSystemAttribute(vsgPAGECYCLEARRAYSIZE))
	{
		unsigned int ui;
		double xpos, ypos;

		for (ui = 0; ui < nFramesPerCycle; ui++)
		{
			cycle[ui].Page = m_pages[m_pageIndex];
			getDriftPos(ui * frameUS / 1.0e6, ppd, tf, gr1, gr2, xpos, ypos);
			if (endXDrift < 0) 
				cycle[ui].Xpos = -xpos;
			else
				cycle[ui].Xpos = -xpos + endXDrift;
			if (endYDrift > 0)
				cycle[ui].Ypos = ypos;
			else
				cycle[ui].Ypos = ypos - endYDrift;

			// SPECIAL CASE k1=k2, u1=u2
			//cycle[ui].Xpos = (-1) * u / k * (s1 - s2) / (s1 * c2 - c1 * s2) * ui * frameUS / 1.0e6;
			//cycle[ui].Ypos = u / k * (c2 - c1) / (s1 * c2 - c1 * s2) * ui * frameUS / 1.0e6;
			if (ui % 64 == 0) cerr << ui << " " << cycle[ui].Xpos << ", " << cycle[ui].Ypos << endl;
			// These lines test what values of Xpos,Ypos mean.
			// Result is apparent drift along diagonal, from lower right towards upper left.
			// This is explained by 1) y is pos DOWN, 2) arg is movement of screen origin. 
			// As x is increased, screen "aperture" moves to left in video memory
			// as Y is increased, screen "aperture" moves down in video memory.
			//cycle[ui].Xpos = -ui;
			//cycle[ui].Ypos = ui;
			cycle[ui].ovPage = 0;
			cycle[ui].ovXpos = cycle[ui].ovYpos = 0;
			cycle[ui].Frames = 1;
			cycle[ui].Stop = 0;
		}
		vsgPageCyclingSetup(nFramesPerCycle, &cycle[0]);
	}
}


