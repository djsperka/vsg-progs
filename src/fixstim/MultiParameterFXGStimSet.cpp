#include "MultiParameterFXGStimSet.h"

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
		status = 1;
	}
	else if (s == "s")
	{
		grating().setContrast(0);
		status = 1;
	}
	else if (s == "a")
	{
		advance();
		vsgSetDrawPage(vsgVIDEOPAGE, m_page[1-m_current_page], vsgBACKGROUND);
		m_current_page = 1-m_current_page;
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
//		vsgPresent();
		status = 0;
	}
	else if (s == "X")
	{
		if (has_fixpt()) fixpt().setContrast(0);
		if (has_grating()) grating().setContrast(0);
		status = 1;
	}
	return status;
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
