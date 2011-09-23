#include "DualStimSet.h"
#include <iostream>
using namespace std;

static const int f_nlevels = 100;



std::string DualStimSet::toString() const
{
	std::ostringstream oss;
	oss << "DualStimSet NI.";
	return oss.str();
}

int NullDualStimSet::init(ARvsg& master, ARvsg& slave)
{
	cout << "NullDualStimSet::init()" << endl;
	return 0;
}

int NullDualStimSet::cleanup(ARvsg& master, ARvsg& slave)
{
	cout << "NullDualStimSet::cleanup()" << endl;
	return 0;
}


int NullDualStimSet::init_triggers(TriggerVector& triggers)
{
	cout << "NullDualStimSet::init_triggers" << endl;

	// quit trigger
	triggers.addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));

	return 0;
}

std::string NullDualStimSet::toString() const
{
	std::ostringstream oss;
	oss << "NULL Dual StimSet" << endl;
	oss << "  xxxxxxxxxxxxxx";
	return oss.str();
}


std::ostream& operator<<(std::ostream& out, const DualStimSet& dsset)
{
	out << "Dual Stim Set " << endl;

	return out;
}


std::ostream& operator<<(std::ostream& out, const StimSet& sset)
{
	out << sset.toString();
	return out;
}

#if 0
std::string StimSet::toString() const
{
	std::ostringstream oss;
	oss << "StimSet NI.";
	return oss.str();
}
#endif

int StimSet::setup_cycling(int firstpage, int secondpage, int lastpage)
{
	VSGCYCLEPAGEENTRY cycle[32768];
	int status = 0;

	memset(cycle, 0, 3*sizeof(VSGCYCLEPAGEENTRY));

	cycle[0].Frames = 1 + m_nframes_delay;
	cycle[0].Page = firstpage;
	cycle[0].Stop = 0;

	if (lastpage >= 0)
	{
		cycle[1].Frames = m_nframes_on;
		cycle[1].Page = secondpage + vsgTRIGGERPAGE;
		cycle[1].Stop = 0;

		cycle[2].Frames = 1;
		cycle[2].Page = lastpage + vsgTRIGGERPAGE;
		cycle[2].Stop = 1;

		status = vsgPageCyclingSetup(3, &cycle[0]);
		cout << "StimSet: Page cycling ready status=" << status << "  frames delay/on=" << m_nframes_delay << "/" << m_nframes_on << endl;
	}
	else
	{
		cycle[1].Frames = 1;
		cycle[1].Page = secondpage + vsgTRIGGERPAGE;
		cycle[1].Stop = 1;

		status = vsgPageCyclingSetup(2, &cycle[0]);
		cout << "StimSet: Page cycling ready status=" << status << "  frames delay=" << m_nframes_delay << endl;	
	}
	status = 0;
	return status;
}

int StimSet::setup_fixpt_cycling(int firstpage, int fixptpage)
{
	VSGCYCLEPAGEENTRY cycle[3];
	int status = 0;

	memset(cycle, 0, 3*sizeof(VSGCYCLEPAGEENTRY));

	cycle[0].Frames = 1 + m_nframes_fixpt_delay;
	cycle[0].Page = firstpage;
	cycle[0].Stop = 0;

	cycle[1].Frames = 1;
	cycle[1].Page = fixptpage + vsgTRIGGERPAGE;
	cycle[1].Stop = 1;

	status = vsgPageCyclingSetup(2, &cycle[0]);
	cout << "StimSet: Page cycling ready for fixpt, status=" << status << "  frames delay=" << m_nframes_fixpt_delay << endl;

	return status;
}

std::string NullStimSet::toString() const
{
	std::ostringstream oss;
	oss << "NULL StimSet" << endl;
	oss << "  xxxxxxxxxxxxxx" << endl;
	oss << "  xxxxxxxxxxxxxx" << endl;
	return oss.str();
}


std::string FixptGratingStimSet::toString() const
{
	std::ostringstream oss;
	oss << "FixptGrating StimSet" << endl;
	if (has_fixpt())
	{
		oss << "  fixation point: " << fixpt() << endl;
	}
	else
	{
		oss << "  fixation point: NONE" << endl;
	}
	if (m_bHaveGrating)
	{
		oss <<     "  grating       : " << m_grating << endl;
	}
	else
	{
		oss <<     "  grating       : NONE" << endl;
	}
	return oss.str();
}


std::string TFStimSet::toString() const
{
	unsigned int i;
	std::ostringstream oss;
	oss << "TF StimSet" << endl;
	if (has_fixpt())
	{
		oss << "  fixation point: " << fixpt() << endl;
	}
	else
	{
		oss << "  fixation point: NONE" << endl;
	}
	oss <<     "  grating base  : " << m_grating << endl;
	oss << "  temporal frequencies: ";
	for (i=0; i<m_temporal_frequencies.size(); i++)
	{
		if (i>0) oss << ", ";
		oss << m_temporal_frequencies[i];
	}
	oss << endl;
	return oss.str();
}


std::string ContrastStimSet::toString() const
{
	unsigned int i;
	std::ostringstream oss;
	oss << "Contrast StimSet" << endl;
	if (has_fixpt())
	{
		oss << "  fixation point: " << fixpt() << endl;
	}
	else
	{
		oss << "  fixation point: NONE" << endl;
	}
	oss <<     "  grating base  : " << m_grating << endl;
	oss << "  contrasts: ";
	for (i=0; i<m_contrasts.size(); i++)
	{
		if (i>0) oss << ", ";
		oss << m_contrasts[i];
	}
	oss << endl;
	return oss.str();
}

std::string SFStimSet::toString() const
{
	unsigned int i;
	std::ostringstream oss;
	oss << "SF StimSet" << endl;
	if (has_fixpt())
	{
		oss << "  fixation point: " << fixpt() << endl;
	}
	else
	{
		oss << "  fixation point: NONE" << endl;
	}
	oss <<     "  grating base  : " << m_grating << endl;
	oss << "  spatial frequencies: ";
	for (i=0; i<m_spatial_frequencies.size(); i++)
	{
		if (i>0) oss << ", ";
		oss << m_spatial_frequencies[i];
	}
	oss << endl;
	return oss.str();
}

std::string OrientationStimSet::toString() const
{
	unsigned int i;
	std::ostringstream oss;
	oss << "Orientation StimSet" << endl;
	if (has_fixpt())
	{
		oss << "  fixation point: " << fixpt() << endl;
	}
	else
	{
		oss << "  fixation point: NONE" << endl;
	}
	oss <<     "  grating base  : " << m_grating << endl;
	oss << "  orientations: ";
	for (i=0; i<m_orientations.size(); i++)
	{
		if (i>0) oss << ", ";
		oss << m_orientations[i];
	}
	oss << endl;
	return oss.str();
}

std::string AreaStimSet::toString() const
{
	unsigned int i;
	std::ostringstream oss;
	oss << "Area StimSet" << endl;
	if (has_fixpt())
	{
		oss << "  fixation point: " << fixpt() << endl;
	}
	else
	{
		oss << "  fixation point: NONE" << endl;
	}
	oss <<     "  grating base  : " << m_grating << endl;
	oss << "  diameters: ";
	for (i=0; i<m_diameters.size(); i++)
	{
		if (i>0) oss << ", ";
		oss << m_diameters[i];
	}
	oss << endl;
	return oss.str();
}

std::string DonutStimSet::toString() const
{
	unsigned int i;
	std::ostringstream oss;
	oss << "Area StimSet" << endl;
	if (has_fixpt())
	{
		oss << "  fixation point: " << fixpt() << endl;
	}
	else
	{
		oss << "  fixation point: NONE" << endl;
	}
	oss <<     "  donut         : " << m_grating << endl;
	oss << "  (outer, inner) diameters: ";
	for (i=0; i<m_diameters.size(); i++)
	{
		if (i>0) oss << ", ";
		oss << "(" << m_diameters[i].first << ", " << m_diameters[i].second << ")";
	}
	oss << endl;
	return oss.str();
}

// if present, xhair drawn on all pages.
// fixpt is drawn (100% contrast) on page 1 and 2. 
// page 0 is blank - that's where you go on X. 
// Page 2 has grating at contrast 0%. 

int FixptGratingStimSet::init(ARvsg& vsg)
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
		m_contrast = m_grating.contrast;
		m_grating.init(vsg, f_nlevels);
		m_grating.setContrast(0);
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
	if (has_grating())
	{
		m_grating.draw();
	}
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

int FixptGratingStimSet::handle_trigger(std::string &s)
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
		setup_fixpt_cycling(0, 1);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "S")
	{
		if (has_grating())
		{
			m_grating.select();
			vsgObjResetDriftPhase();
			//vsgObjSetSpatialPhase(get_spatial_phase());
			m_grating.setContrast(m_contrast);
		}
		setup_cycling(1, 2, 1);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "s")
	{
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "X")
	{
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		if (has_grating())
		{
			m_grating.setContrast(0);
		}
		status = 1;
	}
	return status;
}

int ContrastStimSet::init(ARvsg& vsg)
{
#if 0
	int status = 0;
	vsg.select();
	m_iterator = m_contrasts.begin();

	// get levels and set contrasts - don't draw yet
	m_grating.init(vsg, 40);
	m_grating.setContrast(0);
	if (has_fixpt())
	{
		fixpt().init(vsg, 2);
		fixpt().setContrast(0);
	}


	// page 2 has grating (contrast 0) and fixpt (contrast 100)
	vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgBACKGROUND);
	m_grating.draw();
	if (has_fixpt())
	{
		fixpt().draw();
	}

	// page 1 has fixpt (contrast 100) and no grating
	vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);
	if (has_fixpt())
	{
		fixpt().draw();
	}

	// page 0 is blank
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);

	// present (page 0 will be displayed)
	vsgPresent();

	// cycling prep
	setup_cycling(1, 2, 1);
	return status;
#else
	int status = 0;

	vsg.select();
	m_iterator = m_contrasts.begin();

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
		m_contrast = m_grating.contrast;
		m_grating.init(vsg, f_nlevels);
		m_grating.setContrast(0);

		// SUBCLASS set any parameters in grating that are needed. In classes 
		// where there is a list of parameters (e.g. TF, SF, ...) you'd set the
		// first one here. For contrast we only save the iterator value as the 
		// contrast to be set on "S"
		m_contrast = (int)*m_iterator;
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
	if (has_grating())
	{
		m_grating.draw();
	}
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
#endif
}	

int ContrastStimSet::handle_trigger(std::string& s)
#if 0
{
	int status = 0;
	if (s == "F")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		if (has_fixpt())
		{
			fixpt().setContrast(100);
			status = 1;
		}
	}
	else if (s == "S")
	{
		m_grating.select();
		vsgObjResetDriftPhase();
		vsgObjSetSpatialPhase(get_spatial_phase());
		m_grating.setContrast((int)*m_iterator);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "s")
	{
		m_grating.setContrast(0);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		status = 1;
	}
	else if (s == "a")
	{
		m_iterator++;
		if (m_iterator == m_contrasts.end())
		{
			cout << "at end of contrasts, back to beginning." << endl;
			m_iterator = m_contrasts.begin();
		}
		cout << "Contrast " << *m_iterator << endl;
		status = 1;
	}
	else if (s == "X")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		status = 1;
	}
	return status;
#else
{
	int status = 0;
	if (s == "F")
	{
#if 0
		// s/b draw page 1, so set contrast and present
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		if (has_fixpt())
		{
			fixpt().setContrast(100);
		}
		status = 1;
#endif
		// s/b draw page 1, so set contrast and present
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		if (has_fixpt())
		{
			fixpt().setContrast(100);
		}
		setup_fixpt_cycling(0, 1);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;

	}
	else if (s == "S")
	{
		if (has_grating())
		{
			m_grating.select();
			vsgObjResetDriftPhase();
			//vsgObjSetSpatialPhase(get_spatial_phase());
			m_grating.setContrast(m_contrast);
		}
		setup_cycling(1, 2, 1);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "s")
	{
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "X")
	{
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		if (has_grating())
		{
			m_grating.setContrast(0);
		}
		status = 1;
	}
	else if (s == "a")
	{
		// SUBCLASS must define what happens here. 
		// Redrawing of stim is done here if needed.
		m_iterator++;
		if (m_iterator == m_contrasts.end())
		{
			cout << "at end of contrasts, back to beginning." << endl;
			m_iterator = m_contrasts.begin();
		}
		cout << "Contrast " << *m_iterator << endl;
		m_contrast = (int)*m_iterator;
		status = 1;
	}
	return status;
}
#endif

int TFStimSet::init(ARvsg& vsg)
{
#if OLDCODE
	int status = 0;
	vsg.select();

	// init vsg obj (get levels) and set contrast to 0 (save original contrast value) - 
	// will draw invisible obj i.e. off.  Same for fixpt if applicable.
	m_grating.init(vsg, 40);

	m_iterator = m_temporal_frequencies.begin();

	// SUBCLASS define this operation - set stim params so correct grating is 
	// drawn first time around. Stim params will need to be set in the "a" 
	// trigger as well. 
	m_grating.setTemporalFrequency(*m_iterator);

	m_contrast = m_grating.contrast;
	m_grating.setContrast(0);
	if (has_fixpt())
	{
		fixpt().init(vsg, 2);
		fixpt().setContrast(0);
	}

	// page 2 : draw stim, then fixpt
	vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgBACKGROUND);
	m_grating.draw();
	if (has_fixpt())
	{
		fixpt().draw();
	}
	vsgPresent();


	// page 1 has fixpt just draw fixpt, not stim
	vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);
	if (has_fixpt())
	{
		fixpt().draw();
	}
	vsgPresent();


	// page 0 is blank. 
	// Call present() so this page is displayed
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	vsgPresent();

	// cycling prep
	setup_cycling(1, 2, 1);
	return status;
#else

	int status = 0;

	vsg.select();
	m_iterator = m_temporal_frequencies.begin();

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
		m_contrast = m_grating.contrast;
		m_grating.init(vsg, f_nlevels);
		m_grating.setContrast(0);

		// SUBCLASS set any parameters in grating that are needed. In classes 
		// where there is a list of parameters (e.g. TF, SF, ...) you'd set the
		// first one here. For contrast we only save the iterator value as the 
		// contrast to be set on "S"
		m_grating.setTemporalFrequency(*m_iterator);
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
	if (has_grating())
	{
		m_grating.draw();
	}
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
#endif
}

int TFStimSet::handle_trigger(std::string& s)
{
#if OLDCODE
	int status = 0;
	if (s == "F")
	{
		// Switch to page 1, turn on fixpt, return 1 (requests vsgPresent())
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		if (has_fixpt())
		{
			fixpt().setContrast(100);
			status = 1;
		}
	}
	else if (s == "S")
	{
		// Turn on stim. Select() it, get drift/spatial phase set,
		// vsgSetSynchronizedCommand tells vsg to start page cycling
		// on next present, which we request by returning 1. 
		m_grating.select();
		vsgObjResetDriftPhase();
		vsgObjSetSpatialPhase(get_spatial_phase());
		m_grating.setContrast(m_contrast);

		// SUBCLASS set parameter in stim
		m_grating.setTemporalFrequency(*m_iterator);

		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "s")
	{
		// If it hasn't happened already, stop cycling (vsgSetSynchronizedCommand - 
		// see "S" above). Also set stim contrast back to 0.
		m_grating.setContrast(0);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		status = 1;
	}
	else if (s == "a")
	{
		// advance to next parameter, cycle back if at end. 
		m_iterator++;
		if (m_iterator == m_temporal_frequencies.end())
		{
			cerr << "at end of temporal frequencies, back to beginning." << endl;
			m_iterator = m_temporal_frequencies.begin();
		}
		cerr << "TF " << *m_iterator << endl;

		// SUBCLASS set parameter. 
		// SUBCLASS decides if stim must be redrawn? 
		// TF and Contrast don't require it, 
		// SF, Area, Berliner, Donut, Orientation, ? DO require it. 
		// Maybe make m_reqireRedraw and .requireRedraw(bool) protected func.
		//m_grating.select();
		//m_grating.setTemporalFrequency(*m_iterator);

		// SUBCLASS set parameter. 
		// SUBCLASS decides if stim must be redrawn? 
		// TF and Contrast don't require it, 
		// SF, Area, Berliner, Donut, Orientation, ? DO require it. 
		// Maybe make m_reqireRedraw and .requireRedraw(bool) protected func.
		m_grating.select();
		m_grating.setTemporalFrequency(*m_iterator);

		int ipage = vsgGetZoneDisplayPage(vsgVIDEOPAGE);

		// page 2 : draw stim, then fixpt
		vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgBACKGROUND);
		m_grating.draw();
		if (has_fixpt())
		{
			fixpt().draw();
		}
		vsgPresent();
		vsgSetDrawPage(vsgVIDEOPAGE, ipage, vsgNOCLEAR);


		status = 1;
	}
	else if (s == "X")
	{
		// to page 0, disable cycling (see "S", "s")
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		status = 1;
	}
	return status;
#else
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
		if (has_grating())
		{
			m_grating.select();
			vsgObjResetDriftPhase();
			//vsgObjSetSpatialPhase(get_spatial_phase());
			m_grating.setContrast(m_contrast);
		}
		setup_cycling(1, 2, 1);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "s")
	{
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "X")
	{
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		if (has_grating())
		{
			m_grating.setContrast(0);
		}
		status = 1;
	}
	else if (s == "a")
	{
		// SUBCLASS must define what happens here. 
		// Redrawing of stim is done here if needed.
		m_iterator++;
		if (m_iterator == m_temporal_frequencies.end())
		{
			cout << "at end of TFs, back to beginning." << endl;
			m_iterator = m_temporal_frequencies.begin();
		}
		cout << "TF " << *m_iterator << endl;
		m_grating.select();
		m_grating.setTemporalFrequency(*m_iterator);
		status = 1;
	}
	return status;
#endif
}





int SFStimSet::init(ARvsg& vsg)
{
#if 0
	int status = 0;
	vsg.select();
	m_iterator = m_spatial_frequencies.begin();

	// init vsg obj (get levels)
	m_grating.init(vsg, 40);

	// SUBCLASS define this operation - set stim params so correct grating is 
	// drawn first time around. Stim params will need to be set in the "a" 
	// trigger as well. 
	m_grating.sf = *m_iterator;

	// Save contrast value, then set to 0
	// will draw invisible obj i.e. off.  Same for fixpt if applicable.	
	m_contrast = m_grating.contrast;
	m_grating.setContrast(0);
	if (has_fixpt())
	{
		fixpt().init(vsg, 2);
		fixpt().setContrast(0);
	}


	// page 2 : draw stim, then fixpt
	vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgBACKGROUND);
	m_grating.draw();
	if (has_fixpt())
	{
		fixpt().draw();
	}
	vsgPresent();

	// page 1 has fixpt just draw fixpt, not stim
	vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);
	if (has_fixpt())
	{
		fixpt().draw();
	}
	vsgPresent();

	// page 0 is blank. 
	// Call present() so this page is displayed
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	vsgPresent();

	// cycling prep
	setup_cycling(1, 2, 1);
	return status;
#else
	
	int status = 0;

	vsg.select();

	// SUBCLASS take care of resetting list
	m_iterator = m_spatial_frequencies.begin();

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
		m_contrast = m_grating.contrast;
		m_grating.init(vsg, f_nlevels);
		m_grating.setContrast(0);

		// SUBCLASS set any parameters in grating that are needed. In classes 
		// where there is a list of parameters (e.g. TF, SF, ...) you'd set the
		// first one here. For contrast we only save the iterator value as the 
		// contrast to be set on "S"
		m_grating.sf = *m_iterator;
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
	if (has_grating())
	{
		m_grating.draw();
	}
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

#endif
}

int SFStimSet::handle_trigger(std::string& s)
{
#if 0
	int status = 0;
	if (s == "F")
	{
		// Switch to page 1, turn on fixpt, return 1 (requests vsgPresent())
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		if (has_fixpt())
		{
			fixpt().setContrast(100);
			status = 1;
		}
	}
	else if (s == "S")
	{
		// Turn on stim. Select() it, get drift/spatial phase set,
		// vsgSetSynchronizedCommand tells vsg to start page cycling
		// on next present, which we request by returning 1. 
		m_grating.select();
		vsgObjResetDriftPhase();
		vsgObjSetSpatialPhase(get_spatial_phase());
		m_grating.setContrast(m_contrast);

		// SUBCLASS set parameter in stim  NOOOOOOOO! ONLY FOR TF?
		// m_grating.sf = *m_iterator;

		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "s")
	{
		// If it hasn't happened already, stop cycling (vsgSetSynchronizedCommand - 
		// see "S" above). Also set stim contrast back to 0.
		m_grating.setContrast(0);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		status = 1;
	}
	else if (s == "a")
	{
		// advance to next parameter, cycle back if at end. 
		m_iterator++;
		if (m_iterator == m_spatial_frequencies.end())
		{
			cerr << "at end of spatial frequencies, back to beginning." << endl;
			m_iterator = m_spatial_frequencies.begin();
		}
		cerr << "SF " << *m_iterator << endl;

		// SUBCLASS set parameter. 
		// SUBCLASS decides if stim must be redrawn? 
		// TF and Contrast don't require it, 
		// SF, Area, Berliner, Donut, Orientation, ? DO require it. 
		// Maybe make m_reqireRedraw and .requireRedraw(bool) protected func.
		m_grating.select();
		m_grating.sf = *m_iterator;

		int ipage = vsgGetZoneDisplayPage(vsgVIDEOPAGE);

		// page 2 : draw stim, then fixpt
		vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgBACKGROUND);
		m_grating.draw();
		if (has_fixpt())
		{
			fixpt().draw();
		}
		vsgPresent();
		vsgSetDrawPage(vsgVIDEOPAGE, ipage, vsgNOCLEAR);

		status = 1;
	}
	else if (s == "X")
	{
		// to page 0, disable cycling (see "S", "s")
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		status = 1;
	}
	return status;
#else
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
		if (has_grating())
		{
			m_grating.select();
			vsgObjResetDriftPhase();
			//vsgObjSetSpatialPhase(get_spatial_phase());
			m_grating.setContrast(m_contrast);
		}
		setup_cycling(1, 2, 1);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "s")
	{
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "X")
	{
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		if (has_grating())
		{
			m_grating.setContrast(0);
		}
		status = 1;
	}
	else if (s == "a")
	{
		// SUBCLASS must define what happens here. 
		// Redrawing of stim is done here if needed.
		m_iterator++;
		if (m_iterator == m_spatial_frequencies.end())
		{
			cout << "at end of SFs, back to beginning." << endl;
			m_iterator = m_spatial_frequencies.begin();
		}
		cout << "SF " << *m_iterator << endl;
		m_grating.select();
		m_grating.sf = *m_iterator;

		// SUBCLASS: redraw page 2 if necessary
		int ipage = vsgGetZoneDisplayPage(vsgVIDEOPAGE);
		vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgBACKGROUND);
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
			m_grating.draw();
		}
		vsgPresent();
		vsgSetDrawPage(vsgVIDEOPAGE, ipage, vsgNOCLEAR);
		status = 1;
	}
	return status;

#endif
}

int OrientationStimSet::init(ARvsg& vsg)
{
#if 0
	int status = 0;
	vsg.select();
	m_iterator = m_orientations.begin();

	// init vsg obj (get levels)
	m_grating.init(vsg, 40);

	// SUBCLASS define this operation - set stim params so correct grating is 
	// drawn first time around. Stim params will need to be set in the "a" 
	// trigger as well. 
	m_grating.orientation = *m_iterator;

	// Save contrast value, then set to 0
	// will draw invisible obj i.e. off.  Same for fixpt if applicable.	
	m_contrast = m_grating.contrast;
	m_grating.setContrast(0);
	if (has_fixpt())
	{
		fixpt().init(vsg, 2);
		fixpt().setContrast(0);
	}


	// page 2 : draw stim, then fixpt
	vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgBACKGROUND);
	m_grating.draw();
	if (has_fixpt())
	{
		fixpt().draw();
	}
	vsgPresent();

	// page 1 has fixpt just draw fixpt, not stim
	vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);
	if (has_fixpt())
	{
		fixpt().draw();
	}
	vsgPresent();

	// page 0 is blank. 
	// Call present() so this page is displayed
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	vsgPresent();

	// cycling prep
	setup_cycling(1, 2, 1);
	return status;
#else
		
	int status = 0;

	vsg.select();

	// SUBCLASS take care of resetting list
	m_iterator = m_orientations.begin();

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
		m_contrast = m_grating.contrast;
		m_grating.init(vsg, f_nlevels);
		m_grating.setContrast(0);

		// SUBCLASS set any parameters in grating that are needed. In classes 
		// where there is a list of parameters (e.g. TF, SF, ...) you'd set the
		// first one here. For contrast we only save the iterator value as the 
		// contrast to be set on "S"
		m_grating.orientation = *m_iterator;
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
	if (has_grating())
	{
		m_grating.draw();
	}
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


#endif
}

int OrientationStimSet::handle_trigger(std::string& s)
{
#if 0
	int status = 0;
	if (s == "F")
	{
		// Switch to page 1, turn on fixpt, return 1 (requests vsgPresent())
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		if (has_fixpt())
		{
			fixpt().setContrast(100);
			status = 1;
		}
	}
	else if (s == "S")
	{
		// Turn on stim. Select() it, get drift/spatial phase set,
		// vsgSetSynchronizedCommand tells vsg to start page cycling
		// on next present, which we request by returning 1. 
		m_grating.select();
		vsgObjResetDriftPhase();
		vsgObjSetSpatialPhase(get_spatial_phase());
		m_grating.setContrast(m_contrast);

		// SUBCLASS set parameter in stim  NOOOOOOOO! ONLY FOR TF?
		// m_grating.sf = *m_iterator;

		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "s")
	{
		// If it hasn't happened already, stop cycling (vsgSetSynchronizedCommand - 
		// see "S" above). Also set stim contrast back to 0.
		m_grating.setContrast(0);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		status = 1;
	}
	else if (s == "a")
	{
		// advance to next parameter, cycle back if at end. 
		m_iterator++;
		if (m_iterator == m_orientations.end())
		{
			cerr << "at end of orientations, back to beginning." << endl;
			m_iterator = m_orientations.begin();
		}
		cerr << "Orientation " << *m_iterator << endl;

		// SUBCLASS set parameter. 
		// SUBCLASS decides if stim must be redrawn? 
		// TF and Contrast don't require it, 
		// SF, Area, Berliner, Donut, Orientation, ? DO require it. 
		// Maybe make m_reqireRedraw and .requireRedraw(bool) protected func.
		m_grating.select();
		m_grating.orientation = *m_iterator;

		int ipage = vsgGetZoneDisplayPage(vsgVIDEOPAGE);

		// page 2 : draw stim, then fixpt
		vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgBACKGROUND);
		m_grating.draw();
		if (has_fixpt())
		{
			fixpt().draw();
		}
		vsgPresent();
		vsgSetDrawPage(vsgVIDEOPAGE, ipage, vsgNOCLEAR);

		status = 1;
	}
	else if (s == "X")
	{
		// to page 0, disable cycling (see "S", "s")
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		status = 1;
	}
	return status;
#else

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
		if (has_grating())
		{
			m_grating.select();
			vsgObjResetDriftPhase();
			//vsgObjSetSpatialPhase(get_spatial_phase());
			m_grating.setContrast(m_contrast);
		}
		setup_cycling(1, 2, 1);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "s")
	{
		if (has_grating())
		{
			m_grating.setContrast(0);
		}
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "X")
	{
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		if (has_grating())
		{
			m_grating.setContrast(0);
		}
		status = 1;
	}
	else if (s == "a")
	{
		// SUBCLASS must define what happens here. 
		// Redrawing of stim is done here if needed.
		m_iterator++;
		if (m_iterator == m_orientations.end())
		{
			cout << "at end of Orientations, back to beginning." << endl;
			m_iterator = m_orientations.begin();
		}
		cout << "Orientation " << *m_iterator << endl;
		m_grating.select();
		m_grating.orientation = *m_iterator;


		// SUBCLASS: redraw page 2 if necessary
		int ipage = vsgGetZoneDisplayPage(vsgVIDEOPAGE);
		vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgBACKGROUND);
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
			m_grating.draw();
		}
		vsgPresent();
		vsgSetDrawPage(vsgVIDEOPAGE, ipage, vsgNOCLEAR);
		status = 1;
	}
	return status;


#endif
}


int AreaStimSet::init(ARvsg& vsg)
{
#if 0
	int status = 0;
	vsg.select();
	m_iterator = m_diameters.begin();

	// init vsg obj (get levels)
	m_grating.init(vsg, 40);

	// SUBCLASS define this operation - set stim params so correct grating is 
	// drawn first time around. Stim params will need to be set in the "a" 
	// trigger as well. 
	m_grating.w = m_grating.h = *m_iterator;

	// Save contrast value, then set to 0
	// will draw invisible obj i.e. off.  Same for fixpt if applicable.	
	m_contrast = m_grating.contrast;
	m_grating.setContrast(0);
	if (has_fixpt())
	{
		fixpt().init(vsg, 2);
		fixpt().setContrast(0);
	}


	// page 2 : draw stim, then fixpt
	vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgBACKGROUND);
	m_grating.draw();
	if (has_fixpt())
	{
		fixpt().draw();
	}
	vsgPresent();

	// page 1 has fixpt just draw fixpt, not stim
	vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);
	if (has_fixpt())
	{
		fixpt().draw();
	}
	vsgPresent();

	// page 0 is blank. 
	// Call present() so this page is displayed
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	vsgPresent();

	// cycling prep
	setup_cycling(1, 2, 1);
	return status;

#else

	int status = 0;
	vsg.select();

	// SUBCLASS take care of resetting list
	m_iterator = m_diameters.begin();

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
		m_contrast = m_grating.contrast;
		m_grating.init(vsg, f_nlevels);
		m_grating.setContrast(0);

		// SUBCLASS set any parameters in grating that are needed. In classes 
		// where there is a list of parameters (e.g. TF, SF, ...) you'd set the
		// first one here. For contrast we only save the iterator value as the 
		// contrast to be set on "S"
		m_grating.w = m_grating.h = *m_iterator;
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
	if (has_grating())
	{
		m_grating.draw();
	}
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

#endif
}

int AreaStimSet::handle_trigger(std::string& s)
{
#if 0
	int status = 0;
	if (s == "F")
	{
		// Switch to page 1, turn on fixpt, return 1 (requests vsgPresent())
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		if (has_fixpt())
		{
			fixpt().setContrast(100);
			status = 1;
		}
	}
	else if (s == "S")
	{
		// Turn on stim. Select() it, get drift/spatial phase set,
		// vsgSetSynchronizedCommand tells vsg to start page cycling
		// on next present, which we request by returning 1. 
		m_grating.select();
		vsgObjResetDriftPhase();
		vsgObjSetSpatialPhase(get_spatial_phase());
		m_grating.setContrast(m_contrast);

		// SUBCLASS set parameter in stim  NOOOOOOOO! ONLY FOR TF?
		// m_grating.sf = *m_iterator;

		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "s")
	{
		// If it hasn't happened already, stop cycling (vsgSetSynchronizedCommand - 
		// see "S" above). Also set stim contrast back to 0.
		m_grating.setContrast(0);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		status = 1;
	}
	else if (s == "a")
	{
		// advance to next parameter, cycle back if at end. 
		m_iterator++;
		if (m_iterator == m_diameters.end())
		{
			cerr << "at end of diameters, back to beginning." << endl;
			m_iterator = m_diameters.begin();
		}
		cerr << "Diameter " << *m_iterator << endl;

		// SUBCLASS set parameter. 
		// SUBCLASS decides if stim must be redrawn? 
		// TF and Contrast don't require it, 
		// SF, Area, Berliner, Donut, Orientation, ? DO require it. 
		// Maybe make m_reqireRedraw and .requireRedraw(bool) protected func.
		m_grating.select();
		m_grating.w = m_grating.h = *m_iterator;

		int ipage = vsgGetZoneDisplayPage(vsgVIDEOPAGE);

		// page 2 : draw stim, then fixpt
		vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgBACKGROUND);
		m_grating.draw();
		if (has_fixpt())
		{
			fixpt().draw();
		}
		vsgPresent();
		vsgSetDrawPage(vsgVIDEOPAGE, ipage, vsgNOCLEAR);

		status = 1;
	}
	else if (s == "X")
	{
		// to page 0, disable cycling (see "S", "s")
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		status = 1;
	}
	return status;
#else

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
		if (has_grating())
		{
			m_grating.select();
			vsgObjResetDriftPhase();
			//vsgObjSetSpatialPhase(get_spatial_phase());
			m_grating.setContrast(m_contrast);
		}
		setup_cycling(1, 2, 1);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "s")
	{
		if (has_grating())
		{
			m_grating.setContrast(0);
		}
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "X")
	{
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		if (has_grating())
		{
			m_grating.setContrast(0);
		}
		status = 1;
	}
	else if (s == "a")
	{
		// SUBCLASS must define what happens here. 
		// Redrawing of stim is done here if needed.
		m_iterator++;
		if (m_iterator == m_diameters.end())
		{
			cout << "at end of diameters, back to beginning." << endl;
			m_iterator = m_diameters.begin();
		}
		cout << "Diameter " << *m_iterator << endl;
		m_grating.select();
		m_grating.w = m_grating.h = *m_iterator;

		// SUBCLASS: redraw if necessary
		int ipage = vsgGetZoneDisplayPage(vsgVIDEOPAGE);

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
		if (has_grating())
		{
			m_grating.draw();
		}
		vsgPresent();

		vsgSetDrawPage(vsgVIDEOPAGE, ipage, vsgNOCLEAR);
		status = 1;
	}
	return status;

#endif
}

int DonutStimSet::init_diameters(std::vector<double>diams)
{
	double d1, d2;
	std::vector<double>::const_iterator iter = diams.begin();
	for (; iter != diams.end(); iter++)
	{
		d1 = *iter;
		iter++;
		if (iter == diams.end())
		{
			cerr << "AHoleStimSet: ERROR! Must init with an even number of diameters!" << endl;
			return -1;
		}
		d2 = *iter;
		m_diameters.push_back(make_pair(d1, d2));
	}
	return 0;
}


int DonutStimSet::init(ARvsg& vsg)
{
	int status = 0;
	vsg.select();

	// SUBCLASS take care of resetting list
	m_iterator = m_diameters.begin();

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
		m_contrast = m_grating.contrast;
		m_grating.init(vsg, f_nlevels);
		m_grating.setContrast(0);

		// SUBCLASS set any parameters in grating that are needed. In classes 
		// where there is a list of parameters (e.g. TF, SF, ...) you'd set the
		// first one here. For contrast we only save the iterator value as the 
		// contrast to be set on "S"
		m_grating.w = m_grating.h = m_iterator->first;
		m_grating.wd = m_grating.hd = m_iterator->second;
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
	if (has_grating())
	{
		m_grating.draw();
	}
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

int DonutStimSet::handle_trigger(std::string& s)
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
		if (has_grating())
		{
			m_grating.select();
			vsgObjResetDriftPhase();
			//vsgObjSetSpatialPhase(get_spatial_phase());
			m_grating.setContrast(m_contrast);
		}
		setup_cycling(1, 2, 1);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "s")
	{
		if (has_grating())
		{
			m_grating.setContrast(0);
		}
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "X")
	{
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		if (has_grating())
		{
			m_grating.setContrast(0);
		}
		status = 1;
	}
	else if (s == "a")
	{
		// SUBCLASS must define what happens here. 
		// Redrawing of stim is done here if needed.
		m_iterator++;
		if (m_iterator == m_diameters.end())
		{
			cout << "at end of diameters, back to beginning." << endl;
			m_iterator = m_diameters.begin();
		}
		cerr << "Diameter " << m_iterator->first <<  " Inner " << m_iterator->second << endl;
		m_grating.select();
		m_grating.w = m_grating.h = m_iterator->first;
		m_grating.wd = m_grating.hd = m_iterator->second;

		// SUBCLASS: redraw if necessary
		int ipage = vsgGetZoneDisplayPage(vsgVIDEOPAGE);

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
		if (has_grating())
		{
			m_grating.draw();
		}
		vsgPresent();

		vsgSetDrawPage(vsgVIDEOPAGE, ipage, vsgNOCLEAR);
		status = 1;
	}
	return status;
}



