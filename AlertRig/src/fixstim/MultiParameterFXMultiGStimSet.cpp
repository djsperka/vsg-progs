#include "MultiParameterFXMultiGStimSet.h"
#include "FXGStimParameterList.h"

using namespace boost;
using namespace alert;
using namespace std;

int MultiParameterFXMultiGStimSet::init(ARvsg& vsg, std::vector<int> pages)
{
	int status = 0;
	int levels = 0;
	const int max_levels = 40;
	m_page[0] = pages[0];
	m_page[1] = pages[1];
	m_current_page = 0;
	m_fixpt_page = pages[2];
	m_bUseCycling = false;
	m_iCyclingDelay = 0;
	m_iStimDuration = 0;

	if (has_xhair())
	{
		xhair().init(vsg, 16);
	}
	if (has_grating() || has_distractor())
	{
		levels = min(240/(count() + distractor_count()), max_levels);
		for (unsigned int i = 0; i < count(); i++)
		{
			grating(i).init(vsg, levels);
			grating(i).setContrast(0);
		}
		for (unsigned int i = 0; i < distractor_count(); i++)
		{
			distractor(i).init(vsg, levels);
			distractor(i).setContrast(0);
		}
	}
	if (has_fixpt())
	{
		fixpt().init(vsg, 2);
	}

	if (has_dot())
	{
		for (unsigned int i = 0; i < dot_count(); i++)
		{
			dot(i).init(vsg, 1);
		}
	}

	// Set initial params in grating(s)
	set_initial_parameters();
	if (m_bUseCycling)
	{
		setup_cycling();
	}

	// When cycling is used, we'll need this page with fixpt, xhair (if present), distractors(if present) 
	vsgSetDrawPage(vsgVIDEOPAGE, m_fixpt_page, vsgBACKGROUND);
	if (has_xhair())
	{
		xhair().draw();
	}
	if (has_distractor())
	{
		for (unsigned int i = 0; i < distractor_count(); i++)
		{
			distractor(i).setContrast(0);
			distractor(i).draw();
		}
	}
	if (has_fixpt())
	{
		fixpt().setContrast(0);
		fixpt().draw();
	}

	// page 2 - xhair, fixpt and stim. 
	// update - include any dot() on this page
	vsgSetDrawPage(vsgVIDEOPAGE, m_page[m_current_page], vsgBACKGROUND);
	if (has_xhair())
	{
		xhair().draw();
	}
	if (has_distractor())
	{
		for (unsigned int i = 0; i < distractor_count(); i++)
		{
			distractor(i).setContrast(0);
			distractor(i).draw();
		}
	}
	if (has_grating())
	{
		for (unsigned int i = 0; i < count(); i++)
		{
			grating(i).setContrast(0);
			grating(i).draw();
		}
	}
	if (has_fixpt())
	{
		fixpt().setContrast(0);
		fixpt().draw();
	}
	if (has_dot())
	{
		for (unsigned int i = 0; i < dot_count(); i++)
		{
			dot(i).draw();
		}
	}
	vsgPresent();

	return status;
}

int MultiParameterFXMultiGStimSet::handle_trigger(std::string& s)
{
	int status = 0;
	if (s == "F")
	{
		for (unsigned int i = 0; i < distractor_count(); i++)
		{
			cerr << "F distractor " << i << " set contrast " << distractor_contrast(i) << endl;
			distractor(i).select();
			vsgObjResetDriftPhase();
			distractor(i).setContrast(distractor_contrast(i));
			status = 1;
		}
		if (has_fixpt())
		{
			fixpt().setContrast(100);
			status = 1;
		}
	}
	else if (s == "S")
	{
		for (unsigned int i = 0; i < count(); i++)
		{
			grating(i).select();
			vsgObjResetDriftPhase();
			grating(i).setContrast(contrast(i));
		}
		if (m_bUseCycling)
		{
			vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		}
		status = 1;
	}
	else if (s == "s")
	{
		for (unsigned int i = 0; i < count(); i++)
		{
			grating(i).setContrast(0);
		}
		status = 1;
	}
	else if (s == "a")
	{
		// Note that after this trigger is completed, the current draw page
		// is the NEXT stimulus page to be shown. 

		vsgSetDrawPage(vsgVIDEOPAGE, m_page[1 - m_current_page], vsgBACKGROUND);
		m_current_page = 1 - m_current_page;

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
		if (has_distractor())
		{
			for (unsigned int i = 0; i < distractor_count(); i++)
			{
				distractor(i).setContrast(0);
				distractor(i).draw();
			}
		}
		if (has_grating())
		{
			for (unsigned int i = 0; i < count(); i++)
			{
				grating(i).setContrast(0);
				grating(i).draw();
			}
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
		if (has_distractor())
		{
			for (unsigned int i = 0; i < distractor_count(); i++)
			{
				distractor(i).setContrast(0);
			}
		}
		if (has_grating())
		{
			for (unsigned int i = 0; i < count(); i++)
			{
				grating(i).setContrast(0);
			}
		}
		if (m_bUseCycling)
			vsgSetCommand(vsgCYCLEPAGEDISABLE);
		status = 1;
	}
	return status;
}

void MultiParameterFXMultiGStimSet::setCyclingDelay(int ndelay)
{
	if (ndelay < 0)
	{
		m_bUseCycling = false;
		m_iCyclingDelay = 0;
	}
	else
	{
		m_bUseCycling = true;
		m_iCyclingDelay = ndelay;
	}
	return;
}

void MultiParameterFXMultiGStimSet::setStimDuration(double seconds)
{
	if (seconds < 0)
	{
		m_bUseCycling = false;
		m_iStimDuration = 0;
	}
	else
	{
		// Convert stim duration to frames. Note frame time returned is in us.
		m_bUseCycling = true;
		m_iStimDuration = seconds * 1000000.0 / vsgGetSystemAttribute(vsgFRAMETIME);
	}
	return;
}


void MultiParameterFXMultiGStimSet::setup_cycling()
{
	VSGCYCLEPAGEENTRY cycle[12];	// warning! No check on usage. You have been warned. 
	int status = 0;
	int count = 0;

	memset(cycle, 0, sizeof(cycle));

	cerr << "setup_cycling: delay=" << m_iCyclingDelay << " duration=" << m_iStimDuration << endl;

	if (m_iCyclingDelay > 0)
	{
		cycle[count].Frames = 1 + (m_iCyclingDelay > 0 ? m_iCyclingDelay : 0);
		cycle[count].Page = m_fixpt_page;
		cycle[count].Stop = 0;
		count++;
	}

	cycle[count].Page = m_page[m_current_page] + vsgTRIGGERPAGE;
	if (m_iStimDuration <= 0)
	{
		cycle[count].Frames = 0;
		cycle[count].Stop = 1;
	}
	else
	{
		cycle[count].Frames = m_iStimDuration;
		cycle[count].Stop = 0;
		count++;

		cycle[count].Frames = 0;
		cycle[count].Page = 0 + vsgTRIGGERPAGE;
		cycle[count].Stop = 1;
		count++;
	}

	status = vsgPageCyclingSetup(count, &cycle[0]);
}


string MultiParameterFXMultiGStimSet::toString() const
{
	string s;
	cerr << "MultiParameterFXMultiGStimSet: " << endl;
	if (has_fixpt())
		cerr << " Fixpt: " << fixpt() << endl;
	else
		cerr << " Fixpt: NONE" << endl;
	if (has_xhair())
		cerr << " Xhair: " << xhair() << endl;
	else
		cerr << " Xhair: NONE" << endl;
	if (has_grating())
	{
		for (unsigned int i=0; i<count(); i++)
			cerr << " Grating " << i << ": " << grating(i) << endl;
	}
	else
		cerr << " Grating: NONE" << endl;
	if (has_distractor())
	{
		for (unsigned int i = 0; i<distractor_count(); i++)
			cerr << " Distractor " << i << ": " << distractor(i) << endl;
	}
	else
		cerr << " Distractor: NONE" << endl;
	if (has_dot())
	{
		for (unsigned int i = 0; i < dot_count(); i++)
			cerr << " Dot " << i << ": " << dot(i) << endl;
	}
	else
		cerr << " Dot: NONE" << endl;
	return s;
}

void MultiParameterFXMultiGStimSet::set_initial_parameters()
{
	for (ptr_vector<FXGStimParameterList>::iterator it = begin(); it != end(); it++)
	{
		it->set_current_parameter(this);
	}
	return;
}

void MultiParameterFXMultiGStimSet::advance()
{
	for (ptr_vector<FXGStimParameterList>::iterator it = begin(); it != end(); it++)
	{
		it->advance(this);
	}
	return;
}


