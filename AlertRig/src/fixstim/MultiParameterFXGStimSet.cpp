#include "MultiParameterFXGStimSet.h"
#include "FXGStimParameterList.h"

using namespace boost;
using namespace alert;
using namespace std;

static const int f_nlevels = 100;

int MultiParameterFXGStimSet::init(ARvsg& vsg, std::vector<int> pages)
{
	int status = 0;
	m_page[0] = pages[0];
	m_page[1] = pages[1];
	m_current_page = 0;
	m_fixpt_page = pages[2];
	m_bUseCycling = false;
	m_iCyclingDelay = -1;

	if (has_xhair())
	{
		xhair().init(vsg, 16);
	}
	if (has_grating())
	{
		grating().init(vsg, f_nlevels);
		grating().setContrast(0);
	}
	if (has_fixpt())
	{
		fixpt().init(vsg, 2);
	}

	// Set initial params in grating(s)
	set_initial_parameters();
	if (m_bUseCycling)
	{
		setup_cycling();
	}

	// When cycling is used, we'll need this fixpt (and xhair) only page. 
	vsgSetDrawPage(vsgVIDEOPAGE, m_fixpt_page, vsgBACKGROUND);
	if (has_xhair())
	{
		xhair().draw();
	}
	if (has_fixpt())
	{
		fixpt().setContrast(0);
		fixpt().draw();
	}

	// page 2 - xhair, fixpt and stim. 
	vsgSetDrawPage(vsgVIDEOPAGE, m_page[m_current_page], vsgBACKGROUND);
	if (has_xhair())
	{
		xhair().draw();
	}
	if (has_grating())
	{
		grating().setContrast(0);
		grating().draw();
	}
	if (has_fixpt())
	{
		fixpt().setContrast(0);
		fixpt().draw();
	}
	vsgPresent();

	return status;
}

int MultiParameterFXGStimSet::handle_trigger(std::string& s)
{
	int status = 0;
	if (s == "F")
	{
		if (has_fixpt())
		{
			fixpt().setContrast(100);
			status = 1;
		}
	}
	else if (s == "S")
	{
		grating().select();
		vsgObjResetDriftPhase();
		grating().setContrast(contrast());
		if (m_bUseCycling)
		{
			vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		}
		status = 1;
	}
	else if (s == "s")
	{
		grating().setContrast(0);
		status = 1;
	}
	else if (s == "a")
	{
		// Note that after this trigger is completed, the current draw page
		// is the NEXT stimulus page to be shown. 

		vsgSetDrawPage(vsgVIDEOPAGE, m_page[1-m_current_page], vsgBACKGROUND);
		m_current_page = 1-m_current_page;

		// unset cycling delay, if any. One of the parameter lists called in advance() must enable it. 
		setCyclingDelay(-1);
		advance();
		if (m_bUseCycling)
		{
			setup_cycling();
		}

		if (has_xhair())
		{
			xhair().draw();
		}
		if (has_fixpt())
		{
			fixpt().draw();
		}
		if (has_grating())
		{
			grating().setContrast(0);
			grating().draw();
		}
		// make sure to draw fixpt last!
		if (has_fixpt())
		{
			fixpt().draw();
		}


		status = 0;
	}
	else if (s == "X")
	{
		if (has_fixpt()) fixpt().setContrast(0);
		if (has_grating()) grating().setContrast(0);
		if (m_bUseCycling)
			vsgSetCommand(vsgCYCLEPAGEDISABLE);
		status = 1;
	}
	return status;
}

void MultiParameterFXGStimSet::setCyclingDelay(int ndelay)
{
	if (ndelay < 0)
	{
		m_bUseCycling = false;
		m_iCyclingDelay = -1;
	}
	else
	{
		m_bUseCycling = true;
		m_iCyclingDelay = ndelay;
	}
	return;
}


void MultiParameterFXGStimSet::setup_cycling()
{
	VSGCYCLEPAGEENTRY cycle[2];
	int status = 0;

	memset(cycle, 0, sizeof(cycle));

	cerr << "setup_cycling: delay=" << m_iCyclingDelay << endl;

	cycle[0].Frames = 1 + (m_iCyclingDelay>0 ? m_iCyclingDelay : 0);
	cycle[0].Page = m_fixpt_page;
	cycle[0].Stop = 0;

	cycle[1].Frames = 0;
	cycle[1].Page = m_page[m_current_page] + vsgTRIGGERPAGE;
	cycle[1].Stop = 1;

	status = vsgPageCyclingSetup(2, &cycle[0]);
}


string MultiParameterFXGStimSet::toString() const
{
	string s;
	return s;
}

void MultiParameterFXGStimSet::set_initial_parameters()
{
	for (ptr_vector<FXGStimParameterList>::iterator it = begin(); it != end(); it++)
	{
		it->set_current_parameter(this);
	}
	return;
}

void MultiParameterFXGStimSet::advance()
{
	for (ptr_vector<FXGStimParameterList>::iterator it = begin(); it != end(); it++)
	{
		it->advance(this);
	}
	return;
}


