#include "AttentionStimSet.h"


int parse_attcues(const string& s, int nstim, vector<AttentionCue>& vecCues)
{
	COLOR_TYPE color;
	double rdiff;
	unsigned int i;
	vector<string> tokens;
	istringstream iss;
	tokenize(s, tokens, ",");
	if (tokens.size() % (nstim*2) == 0)
	{
		for (i=0; i<tokens.size()/2; i++)
		{
			if (parse_double(tokens[i*2], rdiff) || parse_color(tokens[i*2+1], color))
			{
				cerr << "Error reading attention cues (" << s << ")" << endl;
				return 1;
			}
			vecCues.push_back(AttentionCue(rdiff, color));
		}
	}
	else
	{
		cerr << "Error reading attention cues (" << s << ") Expecting " << nstim*2 << " args, 2 for each stim." << endl;
		return 1;
	}
	return 0;
}




// helper function for loading params from a comma-separated string
int parse_attparams(const string& s, int nstim, vector<struct AttParams>& vecTrialParams, double& tCC)
{
	int i;
	int status = 0;
	struct AttParams params;
	vector<string> tokens;
	tokenize(s, tokens, ",");
	if (tokens.size() % (4+nstim*2) == 1)
	{
		istringstream iss;
		vector<string>::const_iterator it = tokens.begin();
		if (parse_double(*it, tCC))
		{
			cerr << "Error reading tCC - time to contrast change (" << *it << ")" << endl;
			return 1;
		}
		it++;

		while (it != tokens.end())
		{
			// each pass through this loop will pickup the parameters for a single trial. 
			// The color, init phase, time to cc, and off bits are followed by a contrast 
			// pair for each stim.

			if (parse_color(*it, params.color))
			{
				cerr << "Error reading color: " << *it << endl;
				status = 1;
				break;
			}
			it++;
			iss.str(*it);
			if (parse_double(*it, params.dInitialPhase))
			{
				cerr << "Error reading initial phase: " << *it << endl;
				status = 1;
				break;
			}
			it++;
			if (parse_double(*it, params.dTimeToCC))
			{
				cerr << "Error reading time to CC: " << *it;
				status = 1;
				break;
			}
			it++;
			if (parse_integer(*it, params.iOffBits))
			{
				cerr << "Error reading off bits: " << *it;
				status = 1;
				break;
			}
			it++;

			// Pick up contrast pairs. Clear the vector that holds the pairs first, so we
			// don't just append the current trials' pairs to last trials' pairs. 
			params.contrastPairs.clear();
			for (i=0; i<nstim; i++)
			{
				int iBase, iChg;
				if (parse_integer(*it, iBase))
				{
					cerr << "Error reading base contrast: " << *it << endl;
					status = 1;
					return 1;
				}
				it++;
				if (parse_integer(*it, iChg))
				{
					cerr << "Error reading chg contrast: " << *it << endl;
					status = 1;
					return 1;
				}
				it++;
				params.contrastPairs.push_back(std::pair<int, int>(iBase, iChg));
			}
			vecTrialParams.push_back(params);
		}
	}
	else
	{
		cerr << "Error reading attention parameters. Check command line args." << endl;
		status = 1;
	}
	return status;
}

AttentionStimSet::AttentionStimSet(ARContrastFixationPointSpec& fixpt, double tMax, std::vector<alert::ARGratingSpec>& vecGratings, vector<AttParams>& params)
: m_fixpt(fixpt)
, m_tMax(tMax)
, m_vecGratings(vecGratings)
, m_vecGratingsCC(vecGratings)
, m_vecParams(params)
, m_current(0)
{};


AttentionStimSet::AttentionStimSet(ARContrastFixationPointSpec& fixpt, double tMax, vector<alert::ARGratingSpec>& vecGratings, vector<AttentionCue>& vecCuePairs, vector<AttParams>& params)
: m_fixpt(fixpt)
, m_tMax(tMax)
, m_vecGratings(vecGratings)
, m_vecGratingsCC(vecGratings)
, m_vecParams(params)
, m_current(0)
{
	for (unsigned int i=0; i<vecCuePairs.size(); i++)
	{
		ARContrastCircleSpec circle;
		int indGrating = i % m_vecGratings.size();
		circle.x = m_vecGratings[indGrating].x;
		circle.y = m_vecGratings[indGrating].y;
		circle.d = m_vecGratings[indGrating].w + vecCuePairs[i].first*2;
		circle.color = vecCuePairs[i].second;
		m_vecCues.push_back(circle);
	}
};

int AttentionStimSet::init(ARvsg& vsg, std::vector<int> pages)
{
	int status = 0;
	int nlevels;
	m_pageBlank = pages[0];
	m_pageFixpt = pages[1];
	m_pageStim = pages[2];
	m_pageChg = pages[3];

	// divvy up levels. There are only about 250 levels available but fixpt takes 2...
	m_fixpt.init(vsg, 2);
	m_fixpt.setContrast(100);

	if (m_vecGratings.size() < 4) nlevels = 40;
	else
	{
		nlevels = (247 - m_vecCues.size()*2)/m_vecGratings.size()/2;
	}
	cerr << "Number of levels per stim" << nlevels << endl;
	for (unsigned int i=0; i<m_vecCues.size(); i++)
	{
		m_vecCues[i].init(vsg, 2);
	}
	for (unsigned int i=0; i<m_vecGratings.size(); i++)
	{
		m_vecGratings[i].init(vsg, nlevels);
		m_vecGratingsCC[i].init(vsg, nlevels);
	}

	status = drawCurrent();

	return status;
}

int AttentionStimSet::drawCurrent()
{
	int status = 0;
	int page = vsgGetZoneDisplayPage(vsgVIDEOPAGE);
	if (m_current >= m_vecParams.size())
		return 1;

	// Set color of fixpt...
	m_fixpt.color = m_vecParams[m_current].color;

	// Stim page
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageStim, vsgBACKGROUND);
	for (unsigned int i=0; i<m_vecGratings.size(); i++)
	{
		// Check if this stim has an off bit set.
		if (m_vecParams[m_current].iOffBits & (1 << i))
		{
			m_vecGratings[i].setContrast(0);
		}
		else
		{
			m_vecGratings[i].setContrast(m_vecParams[m_current].contrastPairs[i].first);
		}
		m_vecGratings[i].setSpatialPhase(m_vecParams[m_current].dInitialPhase);
		m_vecGratings[i].select();
		vsgObjResetDriftPhase();
		m_vecGratings[i].draw();
	}

	// Draw cue circles.
	// One for each grating, but the set of cues used are taken from 
	// (iOffBits & 0xff00) >> 8

	int iCueBase = (m_vecParams[m_current].iOffBits & 0xff00) >> 8;

	for (unsigned int i=0; i<m_vecGratings.size(); i++)
	{
		// Check if this stim has an off bit set.
		if (m_vecParams[m_current].iOffBits & (1 << i))
		{
			// do nothing
		}
		else
		{
			m_vecCues[iCueBase*m_vecGratings.size() + i].draw();
		}
	}
	m_fixpt.draw();

	// Stim page, this one with contrast change
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageChg, vsgBACKGROUND);
	for (unsigned int i=0; i<m_vecGratingsCC.size(); i++)
	{
		// Check if this stim has an off bit set.
		if (m_vecParams[m_current].iOffBits & (1 << i))
		{
			m_vecGratingsCC[i].setContrast(0);
		}
		else
		{
			m_vecGratingsCC[i].setContrast(m_vecParams[m_current].contrastPairs[i].second);
		}
		m_vecGratingsCC[i].setSpatialPhase(m_vecParams[m_current].dInitialPhase);
		m_vecGratings[i].select();
		vsgObjResetDriftPhase();
		m_vecGratingsCC[i].draw();
	}
	for (unsigned int i=0; i<m_vecGratings.size(); i++)
	{
		// Check if this stim has an off bit set.
		if (m_vecParams[m_current].iOffBits & (1 << i))
		{
			// do nothing
		}
		else
		{
			m_vecCues[iCueBase*m_vecGratings.size() + i].draw();
		}
	}
	m_fixpt.draw();

	// plain fixpt page
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixpt, vsgBACKGROUND);
	for (unsigned int i=0; i<m_vecGratings.size(); i++)
	{
		// Check if this stim has an off bit set.
		if (m_vecParams[m_current].iOffBits & (1 << i))
		{
			// do nothing
		}
		else
		{
			m_vecCues[iCueBase*m_vecGratings.size() + i].draw();
		}
	}
	m_fixpt.draw();

	// blank page
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, vsgBACKGROUND);
	vsgPresent();

	// Setup page cycling
	VSGCYCLEPAGEENTRY cycle[32767];
	cycle[0].Frames = (WORD)(m_vecParams[m_current].dTimeToCC * 1000000.0 /vsgGetSystemAttribute(vsgFRAMETIME));
	cycle[0].Page = m_pageStim + vsgTRIGGERPAGE;
	cycle[0].Xpos = cycle[0].Ypos = 0;
	cycle[0].Stop = 0;
	cycle[1].Frames = (WORD)(m_tMax * 1000000.0 /vsgGetSystemAttribute(vsgFRAMETIME));
	cycle[1].Page = m_pageChg + vsgTRIGGERPAGE;
	cycle[1].Xpos = cycle[1].Ypos = 0;
	cycle[1].Stop = 0;
	cycle[2].Frames = 1;
	cycle[2].Page = 0 + vsgTRIGGERPAGE;
	cycle[2].Xpos = cycle[2].Ypos = 0;
	cycle[2].Stop = 1;
	vsgPageCyclingSetup(3, &cycle[0]);

	return status;
}

int AttentionStimSet::handle_trigger(std::string& s)
{
	int status = 0;

	if (s == "F")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixpt, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "S")
	{
		for (unsigned int i=0; i<m_vecGratings.size(); i++)
		{
			m_vecGratings[i].select();
			vsgObjSetSpatialPhase(m_vecParams[m_current].dInitialPhase);
			vsgObjResetDriftPhase();
			m_vecGratingsCC[i].select();
			vsgObjSetSpatialPhase(m_vecParams[m_current].dInitialPhase);
			vsgObjResetDriftPhase();
		}
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "a")
	{
		m_current++;
		if (m_current == m_vecParams.size()) m_current = 0;
		drawCurrent();
	}
	else if (s == "X")
	{
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, vsgNOCLEAR);
		status = 1;
	}
	return status;
}


std::string AttentionStimSet::toString() const
{
	return string("not implemented");
}
