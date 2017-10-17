#include "ClientNetwork.h"
#include "EQStimSet.h"
#include <iostream>
#include <boost/algorithm/string.hpp>
using namespace std;
using namespace boost;

VSGLUTBUFFER f_palImage;	// loaded from image
VSGLUTBUFFER f_palHW;	// loaded from hardware

void clearPalImage()
{
	for (int i=0; i<256; i++) f_palImage[i].a = f_palImage[i].b = f_palImage[i].c = 0;
}

void palDiff()
{
	vsgPaletteRead(&f_palHW);
	cerr << "DIFF ";
	for (int i=0; i<40; i++)
		if (f_palImage[i].a != f_palHW[i].a || f_palImage[i].b != f_palHW[i].b || f_palImage[i].c != f_palHW[i].c) cerr << i << " ";
	cerr << endl;
}

void dumpPal()
{
	VSGLUTBUFFER palVSG;
	vsgPaletteRead(&palVSG);
	cerr << "palVSG[0] " << palVSG[0].a << ", " << palVSG[0].b << ", " << palVSG[0].c << endl;
	cerr << "palVSG[1] " << palVSG[1].a << ", " << palVSG[1].b << ", " << palVSG[1].c << endl;
}




// parse string 
int parse_eqparams(const string& s, int nstim, struct EQParams& params)
{
	int i, inext;
	int status = 1;
	int nexpect = 8+2*nstim;
	int nexpectC3 = 10 + 4 * nstim;
	vector<string> piped;
	vector<string> tokens;

	tokenize(s, piped, "|");
	params.fixpt = (ARContrastFixationPointSpec *)NULL;
	params.g0 = (ARGratingSpec *)NULL;
	params.g1 = (ARGratingSpec *)NULL;
	params.tC2 = params.tC3 = 0;


	// Parse first part of arg. This was the original arg, and is everything before the first "|"

	tokenize(piped[0], tokens, "\t");
	if (tokens.size() == nexpect)
	{
		if (parse_double(tokens[0], params.tQ0) ||
			parse_double(tokens[1], params.tQ1) ||
			parse_double(tokens[2], params.tS) ||
			parse_double(tokens[3], params.tCC) ||
			parse_double(tokens[4], params.tE))
		{
			cerr << "Error in eq param: expecting double for params 0-4: " << s << endl;
			return 1;
		}
		params.tC2 = params.tC3 = 0;
		i = 0;
		inext = 5;
		while (i<nstim)
		{
			int c0, c1;
			if (parse_integer(tokens[inext++], c0) ||
				parse_integer(tokens[inext++], c1))
			{
				cerr << "Error in eq param: contrasts must be int: " << s << endl;
				return 1;
			}
			else
			{
				//params.contrastPairs.push_back(pair<int, int>(c0, c1));
				params.contrastTuples.push_back(boost::make_tuple(c0, c1, 0, 0));
			}
			i++;
		}

		if (parse_integer(tokens[inext], params.iAL) || parse_integer(tokens[inext + 1], params.iCR))
		{
			cerr << "Error in eq param: expect integer for attend location and correct response: " << s << endl;
			return 1;
		}

		parse_string(tokens[inext + 2], params.cueFile);

	}
	else if (tokens.size() == nexpectC3)
	{
		if (parse_double(tokens[0], params.tQ0) ||
			parse_double(tokens[1], params.tQ1) ||
			parse_double(tokens[2], params.tS) ||
			parse_double(tokens[3], params.tCC) ||
			parse_double(tokens[4], params.tC2) ||
			parse_double(tokens[5], params.tC3) ||
			parse_double(tokens[6], params.tE))
		{
			cerr << "Error in eq param: expecting double for params 0-7: " << s << endl;
			return 1;
		}
		i = 0;
		inext = 7;
		while (i<nstim)
		{
			int c0, c1, c2, c3;
			if (parse_integer(tokens[inext++], c0) ||
				parse_integer(tokens[inext++], c1) ||
				parse_integer(tokens[inext++], c2) ||
				parse_integer(tokens[inext++], c3))
			{
				cerr << "Error in eq param: contrasts must be int: " << s << endl;
				return 1;
			}
			else
			{
				params.contrastTuples.push_back(boost::make_tuple(c0, c1, c2, c3));
			}
			i++;
		}

		if (parse_integer(tokens[inext], params.iAL) || parse_integer(tokens[inext + 1], params.iCR))
		{
			cerr << "Error in eq param: expect integer for attend location and correct response: " << s << endl;
			return 1;
		}

		parse_string(tokens[inext + 2], params.cueFile);

	}
	else
	{
		cerr << "Bad parameter line. Expected " << nexpect << " or " << nexpectC3 << " parameters." << endl;
		return 1;
	}



	// Now parse each of the remaining items in piped
	if (piped.size() > 1)
	{
		ARContrastFixationPointSpec fixpt;
		string s = trim_copy(piped[1]);
		if (s.length() > 0)
		{
			if (parse_fixation_point(s, fixpt))
			{
				cerr << "Error in fixation point arg:" << s << ":" << endl;
				return 1;
			}
			else 
			{
				params.fixpt = new ARContrastFixationPointSpec(fixpt);
			}
		}
	}

	if (piped.size() > 2)
	{
		ARGratingSpec grating;
		string s = trim_copy(piped[2]);
		if (s.length() > 0)
		{
			if (parse_grating(s, grating))
			{
				cerr << "Error in first grating arg: " << s <<  ":" << endl;
				return 1;
			}
			else 
			{
				params.g0 = new ARGratingSpec(grating);
			}
		}
	}

	if (piped.size() > 3)
	{
		ARGratingSpec grating;
		string s = trim_copy(piped[3]);
		if (s.length() > 0)
		{
			if (parse_grating(s, grating))
			{
				cerr << "Error in second grating arg: " << s <<  ":" << endl;
				return 1;
			}
			else 
			{
				params.g1 = new ARGratingSpec(grating);
			}
		}
	}
	return 0;
}

void sendEEGSignal(int ival, ClientNetwork& client)
{
	char msg[48];
	int len;
	len = sprintf_s(msg, 48, "<TRIGGER>%d</TRIGGER>", ival);
	client.sendMessage(msg, len);
	return;
}

EQStimSet::EQStimSet(ARContrastFixationPointSpec& fixpt, std::vector<alert::ARGratingSpec>& vecGratings, vector<EQParams>& params, const char *pIPPort)
: FXMultiGStimSet(fixpt)
, m_vecParams(params)
, m_current(0)
, m_pEEGClient(NULL)
{
	vector<alert::ARGratingSpec>::iterator it = vecGratings.begin();
	while (it != vecGratings.end())
	{
		add_grating(*it);
		add_grating(*it);	// contrast changed partner
		add_grating(*it);	// C2 grating
		add_grating(*it);	// C3 grating
		it++;
	}

	if (pIPPort)
	{
		vector<string> strs;
		char ip[16];
		char port[8];
		boost::split(strs, pIPPort, boost::is_any_of(":"));
		if (strs.size() == 2)
		{
			strcpy_s(ip, 16, strs[0].c_str());
			strcpy_s(port, 8, strs[1].c_str());
			m_pEEGClient = new ClientNetwork(ip, port);
		}
	}
};

EQStimSet::~EQStimSet()
{
	if (m_pEEGClient)
		delete m_pEEGClient;
}

int EQStimSet::init(ARvsg& vsg, std::vector<int> pages)
{
	int status = 0;
	size_t nlevels = 40; // default up to 6 gratings

	m_pageBlank = pages[0];
	m_pageFixpt = pages[1];
	m_pageFixptCue = pages[2];
	m_pageFixptCueStim = pages[3];
	m_pageFixptCueStimCC = pages[4];
	m_pageFixptStim = pages[5];
	m_pageFixptStimCC = pages[6];
	m_pageFixptStimC2 = pages[7];
	m_pageFixptStimC3 = pages[8];

	// divvy up levels. There are only about 250 levels available but fixpt takes 2...
	// test - snag 40 levels for loaded images.......
	vsg.request_range(40, m_levelImage);

	fixpt().init(vsg, 2);
	fixpt().setContrast(100);

	if (count() > 6) nlevels = 200/count();
	cerr << "Number of levels per grating " << nlevels << endl;
	for (unsigned int i=0; i<count(); i++)
	{
		grating(i).init(vsg, nlevels);
	}

	status = drawCurrent();
	//vsgPresent();

	return status;
}

int EQStimSet::drawCurrent()
{
	int i;
	size_t nstim = count()/2;
	int status = 0;
	ARContrastFixationPointSpec *pfixpt;
	ARGratingSpec *pg0Before, *pg0After, *pg0C2, *pg0C3;
	ARGratingSpec *pg1Before, *pg1After, *pg1C2, *pg1C3;
	ARGratingSpec g0Before, g0After, g0C2, g0C3;
	ARGratingSpec g1Before, g1After, g1C2, g1C3;
	const struct EQParams * pparams;		// pointer to current of current params, convenience

	int page = vsgGetZoneDisplayPage(vsgVIDEOPAGE);
	if (m_current >= m_vecParams.size())
		return 1;
	else
		pparams = &m_vecParams.at(m_current);

	// vsgDrawImage needs a char* not const char*
	// Note that if the length is 0 then no image is drawn. In spec file use "none" (case does not matter)
	char filename[1024];
	strncpy_s(filename, 1024, pparams->cueFile.c_str(), sizeof(filename));
	filename[1023] = 0;

	// load palette
	if (strlen(filename))
	{
		clearPalImage();
		i = vsgImageGetPalette(0, filename, &f_palImage);
		if (i)
		{
			switch (i)
			{
			case vsgerrorERRORREADINGFILE: cerr << "get palette vsgerrorERRORREADINGFILE" << endl; break;
			case vsgerrorUNSUPPORTEDIMAGETYPE: cerr << "get palette vsgerrorUNSUPPORTEDIMAGETYPE" << endl; break;
			case vsgerrorUNSUPPORTEDBITMAPFORMAT: cerr << "get palette vsgerrorUNSUPPORTEDBITMAPFORMAT" << endl; break;
			case vsgerrorOUTOFPCMEMORY: cerr << "get palette vsgerrorOUTOFPCMEMORY" << endl; break;
			case vsgerrorIMAGEHASNOPALETTE: cerr << "get palette vsgerrorIMAGEHASNOPALETTE" << endl; break;
			default: cerr << "get palette error: " << i << " filename " << pparams->cueFile << " len " << strlen(filename) << endl; break;
			}
		}
		vsgPaletteWrite((VSGLUTBUFFER*)f_palImage, 0, 32);
	}

	// Decide what stuff to draw
	if (pparams->fixpt)
	{
		pfixpt = pparams->fixpt;
		pfixpt->init(fixpt());
	}
	else
	{
		pfixpt = &fixpt();
	}

	if (pparams->g0)
	{
		g0Before = *pparams->g0;
		g0Before.init(grating(0));
		pg0Before = &g0Before;
		g0After = *pparams->g0;
		g0After.init(grating(1));
		pg0After = &g0After;
		if (pparams->hasC2C3())
		{
			g0C2 = *pparams->g0;
			g0C2.init(grating(2));
			pg0C2 = &g0C2;
			g0C3 = *pparams->g0;
			g0C3.init(grating(3));
			pg0C3 = &g0C3;
		}
	}
	else
	{
		pg0Before = &grating(0);
		pg0After = &grating(1);
		if (pparams->hasC2C3())
		{
			pg0C2 = &grating(2);
			pg0C3 = &grating(3);
		}
	}

	if (pparams->g1)
	{
		g1Before = *pparams->g1;
		g1Before.init(grating(4));
		pg1Before = &g1Before;
		g1After = *pparams->g1;
		g1After.init(grating(5));
		pg1After = &g1After;
		if (pparams->hasC2C3())
		{
			g1C2 = *pparams->g1;
			g1C2.init(grating(6));
			pg1C2 = &g1C2;
			g1C3 = *pparams->g1;
			g1C3.init(grating(7));
			pg1C3 = &g1C3;
		}
	}
	else
	{
		pg1Before = &grating(4);
		pg1After = &grating(5);
		if (pparams->hasC2C3())
		{
			pg1C2 = &grating(6);
			pg1C3 = &grating(7);
		}
	}

	// Set contrasts
#if MORE_THAN_2_GRATINGS
	for (i=0; i<nstim; i++)
	{
		grating(i * nstim).setContrast(pparams->contrastPairs[i].first);
		grating(i * nstim + 1).setContrast(pparams->contrastPairs[i].second);
	}
#else
//tuple	pg0Before->setContrast(pparams->contrastPairs[0].first);
//tuple	pg0After->setContrast(pparams->contrastPairs[0].second);
//tuple	pg1Before->setContrast(pparams->contrastPairs[1].first);
//tuple	pg1After->setContrast(pparams->contrastPairs[1].second);
	pg0Before->setContrast(boost::get<0>(pparams->contrastTuples[0]));
	pg0After->setContrast(boost::get<1>(pparams->contrastTuples[0]));
	pg1Before->setContrast(boost::get<0>(pparams->contrastTuples[1]));
	pg1After->setContrast(boost::get<1>(pparams->contrastTuples[1]));
	if (pparams->hasC2C3())
	{
		pg0C2->setContrast(boost::get<2>(pparams->contrastTuples[0]));
		pg0C3->setContrast(boost::get<3>(pparams->contrastTuples[0]));
		pg1C2->setContrast(boost::get<2>(pparams->contrastTuples[1]));
		pg1C3->setContrast(boost::get<3>(pparams->contrastTuples[1]));
	}
#endif




	// fixpt page
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixpt, vsgBACKGROUND);
	pfixpt->draw();

	// fixpt + cue page
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixptCue, vsgBACKGROUND);
	if (strlen(filename)) vsgDrawImage(vsgBMPPICTURE, pfixpt->x, pfixpt->y, filename);
	pfixpt->draw();

	// fixpt + cue + stim
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixptCueStim, vsgBACKGROUND);
	if (strlen(filename)) vsgDrawImage(vsgBMPPICTURE, pfixpt->x, pfixpt->y, filename);
#if MORE_THAN_2_GRATINGS
	for (i=0; i<nstim; i++)
	{
		grating(i*nstim).draw();	// stim, not CC partners
	}
#else
	pg0Before->draw();
	pg1Before->draw();
#endif
	pfixpt->draw();

	// fixpt + cue + stimCC
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixptCueStimCC, vsgBACKGROUND);
	if (strlen(filename)) vsgDrawImage(vsgBMPPICTURE, pfixpt->x, pfixpt->y, filename);
#if MORE_THAN_2_GRATINGS
	for (i=0; i<nstim; i++)
	{
		grating(i*nstim + 1).draw();	// stim, not CC partners
	}
#else
	pg0After->draw();
	pg1After->draw();
#endif
	pfixpt->draw();

	// fixpt + stim
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixptStim, vsgBACKGROUND);
#if MORE_THAN_2_GRATINGS
	for (i=0; i<nstim; i++)
	{
		grating(i*nstim).draw();	// stim, not CC partners
	}
#else
	pg0Before->draw();
	pg1Before->draw();
#endif
	pfixpt->draw();

	// fixpt + stimCC
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixptStimCC, vsgBACKGROUND);
#if MORE_THAN_2_GRATINGS
	for (i=0; i<nstim; i++)
	{
		grating(i*nstim + 1).draw();	// stim, not CC partners
	}
#else
	pg0After->draw();
	pg1After->draw();
#endif
	pfixpt->draw();

	// C2 and C3 pages are drawn only if tCC < tC2 < tC3 < tE
	if (pparams->hasC2C3())
	{
		// fixpt + stimC2
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixptStimC2, vsgBACKGROUND);
		pg0C2->draw();
		pg1C2->draw();
		pfixpt->draw();

		vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixptStimC3, vsgBACKGROUND);
		pg0C3->draw();
		pg1C3->draw();
		pfixpt->draw();
	}

	// blank page
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, vsgBACKGROUND);


	// Setup page cycling
	int nQ0 = (int)(pparams->tQ0 * 1000000.0 /vsgGetSystemAttribute(vsgFRAMETIME));
	int nQ1 = (int)(pparams->tQ1 * 1000000.0 /vsgGetSystemAttribute(vsgFRAMETIME));
	int nS = (int)(pparams->tS * 1000000.0 /vsgGetSystemAttribute(vsgFRAMETIME));
	int nCC = (int)(pparams->tCC * 1000000.0 /vsgGetSystemAttribute(vsgFRAMETIME));
	int nC2 = (int)(pparams->tC2 * 1000000.0 / vsgGetSystemAttribute(vsgFRAMETIME));
	int nC3 = (int)(pparams->tC3 * 1000000.0 / vsgGetSystemAttribute(vsgFRAMETIME));
	int nE = (int)(pparams->tE * 1000000.0 /vsgGetSystemAttribute(vsgFRAMETIME));
	int iCycle = 0;	// keeps position during setup below
	VSGCYCLEPAGEENTRY cycle[32767];

	// If cue comes on after frame 0, then we must show fixpt first.
	// Also catch the case where stim comes on before cue
	if (min(nQ0, nS) > 0)
	{
		cycle[iCycle].Frames = min(nQ0, nS);
		cycle[iCycle].Page = m_pageFixpt + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;
	}

	// Now now long will cue stay on, and what will it switch to? 
	// The cases are 
	// nQ1 < nS  - cue turns off, just fixpt shown, then stim comes on
	// nQ1 == nS - cue turns off, stim comes on, no intervening "fixpt only" time
	// nQ1 > nS - stim comes on before cue turns off

	if (nQ1 < nS)
	{
		// fixpt/cue on
		cycle[iCycle].Frames = nQ1 - nQ0;
		cycle[iCycle].Page = m_pageFixptCue + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

		// back to fixpt only
		cycle[iCycle].Frames = nS - nQ1;
		cycle[iCycle].Page = m_pageFixpt + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

		// fixpt/stim on
		cycle[iCycle].Frames = nCC - nS;
		cycle[iCycle].Page = m_pageFixptStim + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

		if (!pparams->hasC2C3())
		{
			// fixpt/stimCC on
			cycle[iCycle].Frames = nE - nCC;
			cycle[iCycle].Page = m_pageFixptStimCC + vsgTRIGGERPAGE;
			cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
			cycle[iCycle].Stop = 0;
			iCycle++;
		}
		else
		{
			// fixpt/stimCC on
			cycle[iCycle].Frames = nC2 - nCC;
			cycle[iCycle].Page = m_pageFixptStimCC + vsgTRIGGERPAGE;
			cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
			cycle[iCycle].Stop = 0;
			iCycle++;

			// fixpt/stimC2 on
			cycle[iCycle].Frames = nC3 - nC2;
			cycle[iCycle].Page = m_pageFixptStimC2 + vsgTRIGGERPAGE;
			cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
			cycle[iCycle].Stop = 0;
			iCycle++;

			// fixpt/stimC3 on
			cycle[iCycle].Frames = nE - nC3;
			cycle[iCycle].Page = m_pageFixptStimC3 + vsgTRIGGERPAGE;
			cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
			cycle[iCycle].Stop = 0;
			iCycle++;
		}

		// back to blank and end
		cycle[iCycle].Frames = 1;
		cycle[iCycle].Page = m_pageBlank + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 1;
		iCycle++;
	}
	else if (nQ1 == nS)
	{
		// fixpt/cue on
		cycle[iCycle].Frames = nQ1 - nQ0;
		cycle[iCycle].Page = m_pageFixptCue + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

		// fixpt/stim on
		cycle[iCycle].Frames = nCC - nQ1;
		cycle[iCycle].Page = m_pageFixptStim + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

		if (!pparams->hasC2C3())
		{
			// fixpt/stimCC on
			cycle[iCycle].Frames = nE - nCC;
			cycle[iCycle].Page = m_pageFixptStimCC + vsgTRIGGERPAGE;
			cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
			cycle[iCycle].Stop = 0;
			iCycle++;
		}
		else
		{
			// fixpt/stimCC on
			cycle[iCycle].Frames = nC2 - nCC;
			cycle[iCycle].Page = m_pageFixptStimCC + vsgTRIGGERPAGE;
			cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
			cycle[iCycle].Stop = 0;
			iCycle++;

			// fixpt/stimC2 on
			cycle[iCycle].Frames = nC3 - nC2;
			cycle[iCycle].Page = m_pageFixptStimC2 + vsgTRIGGERPAGE;
			cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
			cycle[iCycle].Stop = 0;
			iCycle++;

			// fixpt/stimC3 on
			cycle[iCycle].Frames = nE - nC3;
			cycle[iCycle].Page = m_pageFixptStimC3 + vsgTRIGGERPAGE;
			cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
			cycle[iCycle].Stop = 0;
			iCycle++;
		}

		// back to blank and end
		cycle[iCycle].Frames = 1;
		cycle[iCycle].Page = m_pageBlank + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 1;
		iCycle++;
	}
	else
	{
		// fixpt/stim on
		cycle[iCycle].Frames = nS - nQ0;
		cycle[iCycle].Page = m_pageFixptStim + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

		// fixpt/cue/stim on
		cycle[iCycle].Frames = nQ1 - nS;
		cycle[iCycle].Page = m_pageFixptCueStim + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

		// back to fixpt/stim only
		cycle[iCycle].Frames = nCC - nQ1;
		cycle[iCycle].Page = m_pageFixptStim + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

		if (!pparams->hasC2C3())
		{
			// fixpt/stimCC on
			cycle[iCycle].Frames = nE - nCC;
			cycle[iCycle].Page = m_pageFixptStimCC + vsgTRIGGERPAGE;
			cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
			cycle[iCycle].Stop = 0;
			iCycle++;
		}
		else
		{
			// fixpt/stimCC on
			cycle[iCycle].Frames = nC2 - nCC;
			cycle[iCycle].Page = m_pageFixptStimCC + vsgTRIGGERPAGE;
			cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
			cycle[iCycle].Stop = 0;
			iCycle++;

			// fixpt/stimC2 on
			cycle[iCycle].Frames = nC3 - nC2;
			cycle[iCycle].Page = m_pageFixptStimC2 + vsgTRIGGERPAGE;
			cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
			cycle[iCycle].Stop = 0;
			iCycle++;

			// fixpt/stimC3 on
			cycle[iCycle].Frames = nE - nC3;
			cycle[iCycle].Page = m_pageFixptStimC3 + vsgTRIGGERPAGE;
			cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
			cycle[iCycle].Stop = 0;
			iCycle++;
		}

		// back to blank and end
		cycle[iCycle].Frames = 1;
		cycle[iCycle].Page = m_pageBlank + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 1;
		iCycle++;
	}

	vsgPageCyclingSetup(iCycle, &cycle[0]);

	return status;
}

int EQStimSet::handle_trigger(std::string& s)
{
	int status = 0;
	if (s == "F")
	{
		if (m_pEEGClient)
			sendEEGSignal(1, *m_pEEGClient);
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixpt, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "S")
	{
		for (unsigned int i=0; i<count(); i++)
		{
			grating(i).select();
			vsgObjSetSpatialPhase(grating(i).phase);
			vsgObjResetDriftPhase();
		}
		if (m_pEEGClient)
			sendEEGSignal(2, *m_pEEGClient);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "a")
	{
//		dumpPal();
//		palDiff();
		m_current++;
		if (m_current == m_vecParams.size()) m_current = 0;
		drawCurrent();
		status = 1; // ???
	}
	else if (s == "X")
	{
//		dumpPal();
//		palDiff();
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, vsgNOCLEAR);
		if (m_pEEGClient)
			sendEEGSignal(3, *m_pEEGClient);
		status = 1;
	}
	else if (s.at(0) > '0' && s.at(0) < '8')
	{
		vsgSetDrawPage(vsgVIDEOPAGE, (int)(s.at(0) - '0'), vsgNOCLEAR);
		status = 1;
	}
	return status;
}


std::string EQStimSet::toString() const
{
	return string("not implemented");
}



