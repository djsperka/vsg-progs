#include "StimSetCRG.h"
#include <iostream>
using namespace std;

static const int f_nlevels = 100;
const string StimSequenceList::empty_sequence = "";


int StimSetCRG::init(ARvsg& vsg)
{
	
	int status = 0;
	vsg.select();

	if (has_xhair())
	{
		xhair().init(vsg, 16);
	}
	if (has_fixpt())
	{
		fixpt().init(vsg, 2);
	}
	if (has_grating())
	{
		grating().setTemporalFrequency(0);

		// This had better have a grating!!!
		// Grating0 will be the reversed grating - shown for "0" in the sequence. 
		// grating() will be shown for "1".
		m_grating1 = grating();


		// grating contrast is saved. Whenever grating is drawn it must
		// be drawn with contrast 0. That way it isn't seen until later, 
		// when we set the contrast to its intended value. 
		m_contrast = grating().contrast;

		grating().init(vsg, f_nlevels);
		m_grating1.init(vsg, f_nlevels);
		grating().setContrast(0);
		m_grating1.setContrast(0);
	}

	// page 2 - xhair, fixpt and stim. 
	vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgBACKGROUND);
	if (has_xhair())
	{
		xhair().draw();
	}
	if (has_fixpt())
	{
		fixpt().draw();
	}
	grating().draw();
	vsgPresent();

	// page 3 - xhair, fixpt and reversed stim. 
	vsgSetDrawPage(vsgVIDEOPAGE, 3, vsgBACKGROUND);
	if (has_xhair())
	{
		xhair().draw();
	}
	if (has_fixpt())
	{
		fixpt().draw();
	}
	m_grating1.draw();
	vsgPresent();

	// page 1 - xhair and fixpt
	vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);
	if (has_xhair())
	{
		xhair().draw();
	}
	if (has_fixpt())
	{
		fixpt().draw();
	}
	vsgPresent();

	// page 0 - xhair only
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
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
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		if (has_fixpt())
		{
			fixpt().setContrast(100);
		}
		status = 1;
	}
	else if (s == "S")
	{
		grating().select();
		vsgObjResetDriftPhase();
		grating().setContrast(m_contrast);
		m_grating1.select();
		vsgObjResetDriftPhase();
		m_grating1.setContrast(-1*m_contrast);
		setup_cycling(1, 2, 3, 1);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	// hack djs
	else if (s == "C") 
	{
		grating().setContrast(100);
		m_grating1.setContrast(-100);
		status = 1;
	}
	else if (s == "D") 
	{
		grating().setContrast(100);
		m_grating1.setContrast(-100);
		status = 0;
	}
	// end hack djs
	else if (s == "s")
	{
		grating().setContrast(0);
		m_grating1.setContrast(0);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "X")
	{
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		if (has_fixpt())
		{
			fixpt().setContrast(0);
		}
		grating().setContrast(0);
		m_grating1.setContrast(0);
		status = 1;
	}
	else if (s == "a")
	{	
		advance();
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

	cycle[0].Frames = 1 + get_frames_delay();
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


