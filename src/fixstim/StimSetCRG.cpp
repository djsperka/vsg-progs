#include "StimSetCRG.h"
#include <iostream>
using namespace std;

static const int f_nlevels = 100;
const string StimSequenceList::empty_sequence = "";



void StimSetCRG::initialize(ARGratingSpec& g)
{
	// grating(0) will be the reversed contrast, grating(1) is the contrast supplied.
	int c = g.contrast;
	g.contrast = -1*c;
	g.tf = 0;
	set_grating(g);
	g.contrast = c;
	set_grating(g);
}

int StimSetCRG::init(ARvsg& vsg, std::vector<int> pages)
{
	
	int status = 0;

	m_page_x = pages[0];
	m_page_f = pages[1];
	m_page_1 = pages[2];
	m_page_0 = pages[3];
	if (has_xhair())
	{
		xhair().init(vsg, 16);
	}
	if (has_fixpt())
	{
		fixpt().init(vsg, 2);
	}

	grating(1).init(vsg, f_nlevels);
	grating(0).init(vsg, f_nlevels);
	grating(1).setContrast(0);
	grating(0).setContrast(0);

	// "1" page - shown for "1" in sequence 
	vsgSetDrawPage(vsgVIDEOPAGE, m_page_1, vsgBACKGROUND);
	if (has_xhair())
	{
		xhair().draw();
	}
	if (has_fixpt())
	{
		fixpt().draw();
	}
	grating(1).draw();
	vsgPresent();

	// "0" page - xhair, fixpt and reversed stim. 
	vsgSetDrawPage(vsgVIDEOPAGE, m_page_0, vsgBACKGROUND);
	if (has_xhair())
	{
		xhair().draw();
	}
	if (has_fixpt())
	{
		fixpt().draw();
	}
	grating(0).draw();
	vsgPresent();

	// fixpt page - xhair and fixpt
	vsgSetDrawPage(vsgVIDEOPAGE, m_page_f, vsgBACKGROUND);
	if (has_xhair())
	{
		xhair().draw();
	}
	if (has_fixpt())
	{
		fixpt().draw();
	}
	vsgPresent();

	// xhair page - xhair only (if it exists)
	vsgSetDrawPage(vsgVIDEOPAGE, m_page_x, vsgBACKGROUND);
	if (has_xhair())
	{
		xhair().draw();
	}
	vsgPresent();

	return status;
}

int StimSetCRG::handle_trigger(std::string& s)
{
	int status = 0;
	if (s == "F")
	{
		// s/b draw page 1, so set contrast and present
		vsgSetDrawPage(vsgVIDEOPAGE, m_page_f, vsgNOCLEAR);
		if (has_fixpt())
		{
			fixpt().setContrast(100);
		}
		status = 1;
	}
	else if (s == "S")
	{
		grating(1).select();
		vsgObjResetDriftPhase();
		grating(1).setContrast(contrast(1));

		grating(0).select();
		vsgObjResetDriftPhase();
		grating(0).setContrast(contrast(0));
		setup_cycling(m_page_f, m_page_1, m_page_0, m_page_f);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "s")
	{
		grating(0).setContrast(0);
		grating(1).setContrast(0);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		vsgSetDrawPage(vsgVIDEOPAGE, m_page_f, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "X")
	{
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		vsgSetDrawPage(vsgVIDEOPAGE, m_page_x, vsgNOCLEAR);
		if (has_fixpt())
		{
			fixpt().setContrast(0);
		}
		grating(0).setContrast(0);
		grating(1).setContrast(0);
		status = 1;
	}
	else if (s == "a")
	{	
		advance();
		cout << "Stim index " << std::dec << this->index() << endl;
		status = 1;
	}
	return status;
}



int StimSetCRG::setup_cycling(int firstpage, int stim1page, int stim0page, int lastpage)
{
	VSGCYCLEPAGEENTRY cycle[32768];
	int status = 0;
	unsigned int i;
	string s;
	unsigned int count = 0;

	memset(cycle, 0, sizeof(cycle));

	s = get_current_sequence();


	// A note about triggers. 
	// There will be a start and end trig for each term. 
	// There is also a trigger (coincident with the stim trigger) before the terms - this 
	// is at the vsgPresent() that kicks off the cycling sequence. You should see a delay
	// between that trigger and the first onset trigger of the sequence. This delay differs 
	// for the master and slave - the difference allows the two stim to be presented 
	// simultaneously. 

//	cycle[0].Frames = 1 + get_frames_delay();
	cycle[0].Frames = 1;
	cycle[0].Page = firstpage;
	cycle[0].Stop = 0;

	// triggers here at onset of each term. 
	for (i=0; i<s.length(); i++)
	{
		count += m_fpt;
		cycle[i+1].Frames = m_fpt;
		if (s[i] == '0') cycle[i+1].Page = stim0page + vsgTRIGGERPAGE;
		else cycle[i+1].Page = stim1page + vsgTRIGGERPAGE;
		cycle[i+1].Stop = 0;
	}
	// one last trigger to close the last term. 
	cycle[s.length()+1].Frames = 1;
	cycle[s.length()+1].Page = lastpage + vsgTRIGGERPAGE;
	cycle[s.length()+1].Stop = 1;

	status = vsgPageCyclingSetup(s.length()+2, &cycle[0]);
	//cout << "StimSetCRG: Page cycling ready status=" << status << " seq length = " << s.length() << "  frames delay/on=" << get_frames_delay() << "/" << count << endl;
	return status;
}


