#include "EQStimSet.h"
#include <iostream>
using namespace std;

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
// test.bmp,0,1,1.5,2,3,50,100,50,25
int parse_eqparams(const string& s, int nstim, struct EQParams& params)
{
	int i, inext;
	int status = 1;
	int nexpect = 8+2*nstim;
	vector<string> tokens;
	tokenize(s, tokens, "\t");
	if (tokens.size() != nexpect)
	{
		cerr << "Error in eq param: expecting " << nexpect << " tokens, got " << tokens.size() << ": " << s << endl;
		return 1;
	}

	if (parse_double(tokens[0], params.tQ0) ||
		parse_double(tokens[1], params.tQ1) ||
		parse_double(tokens[2], params.tS) ||
		parse_double(tokens[3], params.tCC) ||
		parse_double(tokens[4], params.tE))
	{
		cerr << "Error in eq param: expecting double for params 0-4: " << s << endl;
		return 1;
	}

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
			params.contrastPairs.push_back(pair<int, int>(c0, c1));
		}
		i++;
	}

	if (parse_integer(tokens[inext], params.iAL) || parse_integer(tokens[inext+1], params.iCR))
	{
		cerr << "Error in eq param: expect integer for attend location and correct response: " << s << endl;
		return 1;
	}
	
	parse_string(tokens[inext+2], params.cueFile);
	
	return 0;
}



EQStimSet::EQStimSet(ARContrastFixationPointSpec& fixpt, std::vector<alert::ARGratingSpec>& vecGratings, vector<EQParams>& params)
: FXMultiGStimSet(fixpt)
, m_vecParams(params)
, m_current(0)
{
	vector<alert::ARGratingSpec>::iterator it = vecGratings.begin();
	while (it != vecGratings.end())
	{
		set_grating(*it);
		set_grating(*it);	// contrast changed partner
		it++;
	}
};

int EQStimSet::init(ARvsg& vsg, std::vector<int> pages)
{
	int status = 0;
	int nlevels = 40; // default up to 6 gratings

	m_pageBlank = pages[0];
	m_pageFixpt = pages[1];
	m_pageFixptCue = pages[2];
	m_pageFixptCueStim = pages[3];
	m_pageFixptCueStimCC = pages[4];
	m_pageFixptStim = pages[5];
	m_pageFixptStimCC = pages[6];

	// divvy up levels. There are only about 250 levels available but fixpt takes 2...
	// test - snag 40 levels for loaded images.......
	PIXEL_LEVEL level;
	int i;
	vsg.request_range(40, m_levelImage);

	fixpt().init(vsg, 2);
	fixpt().setContrast(100);

	if (count() > 6) nlevels = 200/count();
	cerr << "Number of levels per grating " << nlevels << endl;
	for (int i=0; i<count(); i++)
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
	int nstim = count()/2;
	int status = 0;
	int page = vsgGetZoneDisplayPage(vsgVIDEOPAGE);
	if (m_current >= m_vecParams.size())
		return 1;

	// vsgDrawImage needs a char* not const char*
	char filename[1024];
	strncpy(filename, m_vecParams.at(m_current).cueFile.c_str(), sizeof(filename));
	filename[1023] = 0;

	// load palette
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
		default: cerr << "get palette error: " << i << endl;
		}
	}
	vsgPaletteWrite((VSGLUTBUFFER*)f_palImage, 0, 32);
	
	for (i=0; i<nstim; i++)
	{
		grating(i * nstim).setContrast(m_vecParams[m_current].contrastPairs[i].first);
		grating(i * nstim + 1).setContrast(m_vecParams[m_current].contrastPairs[i].second);
	}

	// fixpt page
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixpt, vsgBACKGROUND);
	fixpt().draw();

	// fixpt + cue page
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixptCue, vsgBACKGROUND);
	vsgDrawImage(vsgBMPPICTURE, 0, 0, filename);
	fixpt().draw();

	// fixpt + cue + stim
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixptCueStim, vsgBACKGROUND);
	vsgDrawImage(vsgBMPPICTURE, 0, 0, filename);
	for (i=0; i<nstim; i++)
	{
		grating(i*nstim).draw();	// stim, not CC partners
	}
	fixpt().draw();

	// fixpt + cue + stimCC
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixptCueStimCC, vsgBACKGROUND);
	vsgDrawImage(vsgBMPPICTURE, 0, 0, filename);
	for (i=0; i<nstim; i++)
	{
		grating(i*nstim + 1).draw();	// stim, not CC partners
	}
	fixpt().draw();

	// fixpt + stim
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixptStim, vsgBACKGROUND);
	for (i=0; i<nstim; i++)
	{
		grating(i*nstim).draw();	// stim, not CC partners
	}
	fixpt().draw();

	// fixpt + stimCC
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixptStimCC, vsgBACKGROUND);
	for (i=0; i<nstim; i++)
	{
		grating(i*nstim + 1).draw();	// stim, not CC partners
	}
	fixpt().draw();

	// blank page
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, vsgBACKGROUND);



	// Setup page cycling
	int nQ0 = m_vecParams.at(m_current).tQ0 * 1000000.0 /vsgGetSystemAttribute(vsgFRAMETIME);
	int nQ1 = m_vecParams.at(m_current).tQ1 * 1000000.0 /vsgGetSystemAttribute(vsgFRAMETIME);
	int nS = m_vecParams.at(m_current).tS * 1000000.0 /vsgGetSystemAttribute(vsgFRAMETIME);
	int nCC = m_vecParams.at(m_current).tCC * 1000000.0 /vsgGetSystemAttribute(vsgFRAMETIME);
	int nE = m_vecParams.at(m_current).tE * 1000000.0 /vsgGetSystemAttribute(vsgFRAMETIME);
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

		// fixpt/stimCC on
		cycle[iCycle].Frames = nE - nCC;
		cycle[iCycle].Page = m_pageFixptStimCC + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

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

		// fixpt/stimCC on
		cycle[iCycle].Frames = nE - nCC;
		cycle[iCycle].Page = m_pageFixptStimCC + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

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

		// fixpt/stimCC on
		cycle[iCycle].Frames = nE - nCC;
		cycle[iCycle].Page = m_pageFixptStimCC + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

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


