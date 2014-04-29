#include "StimSet.h"
#include <iostream>
#define _USE_MATH_DEFINES
#include <cmath>

using namespace std;

std::ostream& operator<<(std::ostream& out, const StimSet& sset)
{
	out << sset.toString();
	return out;
}


std::string StimSet::toString() const
{
	std::ostringstream oss;
	oss << "StimSet NI.";
	return oss.str();
}

std::string DanishStimSet::toString() const
{
	unsigned int i;
	std::ostringstream oss;
	oss << "Danish StimSet" << endl;
	if (m_bHaveFixpt)
	{
		oss << "  fixation point: " << m_fixpt << endl;
	}
	else
	{
		oss << "  fixation point: NONE" << endl;
	}
	if (m_bHaveHole)
	{
		oss << "  grating hole  : " << m_hole << endl;
	}
	else
	{
		oss << "  grating hole  : NONE" << endl;
	}
	oss <<     "  grating donut : " << m_grating << endl;
	oss << "  donut outer diameters: ";
	for (i=0; i<m_ods.size(); i++)
	{
		if (i>0) oss << ", ";
		oss << m_ods[i];
	}
	oss << endl;
	return oss.str();
}

std::string TFStimSet::toString() const
{
	unsigned int i;
	std::ostringstream oss;
	oss << "TF StimSet" << endl;
	if (m_bHaveFixpt)
	{
		oss << "  fixation point: " << m_fixpt << endl;
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

std::string NullStimSet::toString() const
{
	std::ostringstream oss;
	oss << "NULL StimSet" << endl;
	oss << "  xxxxxxxxxxxxxx" << endl;
	oss << "  xxxxxxxxxxxxxx" << endl;
	return oss.str();
}

std::string GratingStimSet::toString() const
{
	std::ostringstream oss;
	oss << "Grating StimSet" << endl;
	oss <<     "  grating base  : " << m_grating << endl;
	return oss.str();
}

std::string FixptGratingStimSet::toString() const
{
	std::ostringstream oss;
	oss << "FixptGrating StimSet" << endl;
	oss << "  fixation point: " << m_fixpt << endl;
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

std::string ContrastStimSet::toString() const
{
	unsigned int i;
	std::ostringstream oss;
	oss << "Contrast StimSet" << endl;
	if (m_bHaveFixpt)
	{
		oss << "  fixation point: " << m_fixpt << endl;
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
	if (m_bHaveFixpt)
	{
		oss << "  fixation point: " << m_fixpt << endl;
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
	if (m_bHaveFixpt)
	{
		oss << "  fixation point: " << m_fixpt << endl;
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
	if (m_bHaveFixpt)
	{
		oss << "  fixation point: " << m_fixpt << endl;
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

std::string CounterphaseStimSet::toString() const
{
	unsigned int i;
	std::ostringstream oss;
	oss << "Counterphase StimSet" << endl;
	if (m_bHaveFixpt)
	{
		oss << "  fixation point: " << m_fixpt << endl;
	}
	else
	{
		oss << "  fixation point: NONE" << endl;
	}
	oss <<     "  grating base  : " << m_grating << endl;
	oss << "  phases: ";
	for (i=0; i<m_phases.size(); i++)
	{
		if (i>0) oss << ", ";
		oss << m_phases[i];
	}
	oss << endl;
	return oss.str();
}

std::string CRGStimSet::toString() const
{
	std::ostringstream oss;
	oss << "CRG StimSet" << endl;
	if (m_bHaveFixpt)
	{
		oss << "  fixation point: " << m_fixpt << endl;
	}
	else
	{
		oss << "  fixation point: NONE" << endl;
	}
	oss <<     "  grating 0     : " << m_grating0 << endl;
	oss <<     "  grating 1     : " << m_grating1 << endl;
	oss << "  Number of terms: " << m_seq.length() << " Balanced? " << (m_balanced ? "Y" : "N") << "Frames per term: " << m_fpt << endl;
	return oss.str();
}

std::string PositionStimSet::toString() const
{
	std::ostringstream oss;
	oss << "Position StimSet" << endl;
	if (m_bHaveFixpt)
	{
		oss << "  fixation point: " << m_fixpt << endl;
	}
	else
	{
		oss << "  fixation point: NONE" << endl;
	}
	oss <<     "  grating       : " << m_grating << endl;
	oss << "  Number of positions: " << m_positions.size()/2 << endl;
	return oss.str();
}

int StimSet::init(std::vector<int> pages)
{
	return init(ARvsg::instance(), pages);
}


int GratingStimSet::num_pages()
{
	return 1;
}

int GratingStimSet::num_overlay_pages()
{
	return 0;
}

int GratingStimSet::init(ARvsg& vsg, std::vector<int> pages)
{
	int status = 0;
	m_page = pages[0];
	m_contrast = m_grating.contrast;
	vsgSetDrawPage(vsgVIDEOPAGE, m_page, vsgBACKGROUND);
	m_grating.init(vsg, 40);
	m_grating.setContrast(0);
	m_grating.draw();
	vsgPresent();
	return status;
}

int GratingStimSet::handle_trigger(std::string &s)
{
	int status = 0;
	if (s == "S")
	{
		m_grating.select();
		vsgObjResetDriftPhase();
		m_grating.setContrast(m_contrast);
		status = 1;
	}
	else if (s == "s")
	{
		m_grating.setContrast(0);
		status = 1;
	}
	else if (s == "X")
	{
		m_grating.setContrast(0);
		status = 1;
	}
	return status;
}

int FixptGratingStimSet::num_pages()
{
	return 1;
}

int FixptGratingStimSet::num_overlay_pages()
{
	return 0;
}

int FixptGratingStimSet::init(ARvsg& vsg, std::vector<int> pages)
{
	int status = 0;
	m_page = pages[0];
	vsgSetDrawPage(vsgVIDEOPAGE, m_page, vsgBACKGROUND);
	if (m_bHaveGrating)
	{
		m_contrast = m_grating.contrast;
		m_grating.init(vsg, 40);
		m_grating.setContrast(0);
		m_grating.draw();
	}
	m_fixpt.init(vsg, 2);
	m_fixpt.setContrast(0);
	m_fixpt.draw();
	vsgPresent();
	return status;
}

int FixptGratingStimSet::handle_trigger(std::string &s)
{
	int status = 0;
	if (s == "F")
	{
		m_fixpt.setContrast(100);
		status = 1;
	}
	else if (s == "S")
	{
		if (m_bHaveGrating)
		{
			m_grating.select();
			vsgObjResetDriftPhase();
			m_grating.setContrast(m_contrast);
		}
		status = 1;
	}
	else if (s == "s")
	{
		if (m_bHaveGrating)
		{
			m_grating.setContrast(0);
		}
		status = 1;
	}
	else if (s == "X")
	{
		m_fixpt.setContrast(0);
		if (m_bHaveGrating)
		{
			m_grating.setContrast(0);
		}
		status = 1;
	}
	return status;
}

int ContrastStimSet::init(ARvsg& vsg, std::vector<int> pages)
{
	int status = 0;
	m_page = pages[0];
	m_iterator = m_contrasts.begin();
	vsgSetDrawPage(vsgVIDEOPAGE, m_page, vsgBACKGROUND);
	m_grating.init(vsg, 40);
	m_grating.setContrast(0);
	m_grating.draw();
	if (m_bHaveFixpt)
	{
		m_fixpt.init(vsg, 2);
		m_fixpt.setContrast(0);
		m_fixpt.draw();
	}

	vsgPresent();
	return status;
}	

int ContrastStimSet::handle_trigger(std::string& s)
{
	int status = 0;
	if (s == "F")
	{
		if (m_bHaveFixpt)
		{
			m_fixpt.setContrast(100);
			status = 1;
		}
	}
	else if (s == "S")
	{
		m_grating.select();
		vsgObjResetDriftPhase();
		m_grating.setContrast((int)*m_iterator);
		status = 1;
	}
	else if (s == "s")
	{
		m_grating.setContrast(0);
		status = 1;
	}
	else if (s == "a")
	{
		m_iterator++;
		if (m_iterator == m_contrasts.end())
		{
			cerr << "at end of contrasts, back to beginning." << endl;
			m_iterator = m_contrasts.begin();
		}
		cerr << "Contrast " << *m_iterator << endl;
		status = 1;
	}
	else if (s == "X")
	{
		m_fixpt.setContrast(0);
		m_grating.setContrast(0);
		status = 1;
	}
	return status;
}



int TFStimSet::init(ARvsg& vsg, std::vector<int> pages)
{
	int status = 0;
	m_page = pages[0];
	m_contrast = m_grating.contrast;
	m_iterator = m_temporal_frequencies.begin();
	vsgSetDrawPage(vsgVIDEOPAGE, m_page, vsgBACKGROUND);
	m_grating.init(vsg, 40);
	m_grating.setContrast(0);
	m_grating.setTemporalFrequency(*m_iterator);
	m_grating.draw();
	if (m_bHaveFixpt)
	{
		m_fixpt.init(vsg, 2);
		m_fixpt.setContrast(0);
		m_fixpt.draw();
	}
	vsgPresent();
	return status;
}

int TFStimSet::handle_trigger(std::string& s)
{
	int status = 0;
	if (s == "F")
	{
		if (m_bHaveFixpt)
		{
			m_fixpt.setContrast(100);
			status = 1;
		}
	}
	else if (s == "S")
	{
		m_grating.select();
		vsgObjResetDriftPhase();
		m_grating.setContrast(m_contrast);
		m_grating.setTemporalFrequency(*m_iterator);
		status = 1;
	}
	else if (s == "s")
	{
		m_grating.setContrast(0);
		status = 1;
	}
	else if (s == "a")
	{
		m_iterator++;
		if (m_iterator == m_temporal_frequencies.end())
		{
			cerr << "at end of temporal frequencies, back to beginning." << endl;
			m_iterator = m_temporal_frequencies.begin();
		}
		cerr << "TF " << *m_iterator << endl;
		status = 1;
	}
	else if (s == "X")
	{
		m_fixpt.setContrast(0);
		m_grating.setContrast(0);
		status = 1;
	}
	return status;
}





int SFStimSet::init(ARvsg& vsg, std::vector<int> pages)
{
	int status = 0;
	m_pages[0] = pages[0];
	m_pages[1] = pages[1];
	cout << "Pages " << m_pages[0] << ", " << m_pages[1] << endl;
	m_contrast = m_grating.contrast;
	m_iterator = m_spatial_frequencies.begin();
	vsgSetDrawPage(vsgVIDEOPAGE, m_pages[1], vsgBACKGROUND);
	vsgSetDrawPage(vsgVIDEOPAGE, m_pages[0], vsgBACKGROUND);
	m_current_page = 0;
	m_grating.init(vsg, 40);
	m_grating.setContrast(0);
	m_grating.sf = *m_iterator;
	m_grating.draw();
	if (m_bHaveFixpt)
	{
		m_fixpt.init(vsg, 2);
		m_fixpt.setContrast(0);
		m_fixpt.draw();
	}
	vsgPresent();
	return status;
}

int SFStimSet::handle_trigger(std::string& s)
{
	int status = 0;
	if (s == "F")
	{
		if (m_bHaveFixpt)
		{
			m_fixpt.setContrast(100);
			status = 1;
		}
	}
	else if (s == "S")
	{
		m_grating.select();
		vsgObjResetDriftPhase();
		m_grating.setContrast(m_contrast);
		status = 1;
	}
	else if (s == "s")
	{
		m_grating.setContrast(0);
		status = 1;
	}
	else if (s == "a")
	{
		// increment iterator, reset to beginning if at end
		m_iterator++;
		if (m_iterator == m_spatial_frequencies.end())
		{
			cerr << "at end of spatial frequencies, back to beginning." << endl;
			m_iterator = m_spatial_frequencies.begin();
		}
		cerr << "SF " << *m_iterator << endl;

		// Change current page and clear it. Note that this is not the page currently in view.
		// Set the sf and draw the grating, then draw the fixpt. 
		m_current_page = 1 - m_current_page;
		vsgSetDrawPage(vsgVIDEOPAGE, m_pages[m_current_page], vsgBACKGROUND);
		m_grating.sf = *m_iterator;
		m_grating.draw();
		if (m_bHaveFixpt)
		{
			//m_fixpt.setContrast(0);
			m_fixpt.draw();
		}
		status = 1;
	}
	else if (s == "X")
	{
		m_fixpt.setContrast(0);
		m_grating.setContrast(0);
		status = 1;
	}
	return status;
}

int OrientationStimSet::init(ARvsg& vsg, std::vector<int> pages)
{
	int status = 0;
	m_pages[0] = pages[0];
	m_pages[1] = pages[1];
	m_contrast = m_grating.contrast;
	m_iterator = m_orientations.begin();
	vsgSetDrawPage(vsgVIDEOPAGE, m_pages[1], vsgBACKGROUND);
	vsgSetDrawPage(vsgVIDEOPAGE, m_pages[0], vsgBACKGROUND);
	m_current_page = 0;
	m_grating.init(vsg, 40);
	m_grating.setContrast(0);
	m_grating.orientation = *m_iterator;
	m_grating.draw();
	if (m_bHaveFixpt)
	{
		m_fixpt.init(vsg, 2);
		m_fixpt.setContrast(0);
		m_fixpt.draw();
	}
	vsgPresent();
	return status;
}

int OrientationStimSet::handle_trigger(std::string& s)
{
	int status = 0;
	if (s == "F")
	{
		if (m_bHaveFixpt)
		{
			m_fixpt.setContrast(100);
			status = 1;
		}
	}
	else if (s == "S")
	{
		m_grating.select();
		vsgObjResetDriftPhase();
		m_grating.setContrast(m_contrast);
		status = 1;
	}
	else if (s == "s")
	{
		m_grating.setContrast(0);
		status = 1;
	}
	else if (s == "a")
	{
		// increment iterator, reset to beginning if at end
		m_iterator++;
		if (m_iterator == m_orientations.end())
		{
			cerr << "at end of orientations, back to beginning." << endl;
			m_iterator = m_orientations.begin();
		}
		cerr << "Orientation " << *m_iterator << endl;

		// Change current page and clear it. Note that this is not the page currently in view.
		// Set the sf and draw the grating, then draw the fixpt. 
		m_current_page = 1 - m_current_page;
		vsgSetDrawPage(vsgVIDEOPAGE, m_pages[m_current_page], vsgBACKGROUND);
		m_grating.orientation = *m_iterator;
		m_grating.draw();
		if (m_bHaveFixpt)
		{
			m_fixpt.draw();
		}
		status = 1;
	}
	else if (s == "X")
	{
		m_fixpt.setContrast(0);
		m_grating.setContrast(0);
		status = 1;
	}
	return status;
}


int AreaStimSet::init(ARvsg& vsg, std::vector<int> pages)
{
	int status = 0;
	m_pages[0] = pages[0];
	m_pages[1] = pages[1];
	m_contrast = m_grating.contrast;
	m_iterator = m_diameters.begin();
	vsgSetDrawPage(vsgVIDEOPAGE, m_pages[1], vsgBACKGROUND);
	vsgSetDrawPage(vsgVIDEOPAGE, m_pages[0], vsgBACKGROUND);
	m_current_page = 0;
	m_grating.init(vsg, 40);
	m_grating.setContrast(0);
	m_grating.w = m_grating.h = *m_iterator;
	m_grating.draw();
	if (m_bHaveFixpt)
	{
		m_fixpt.init(vsg, 2);
		m_fixpt.setContrast(0);
		m_fixpt.draw();
	}
	vsgPresent();
	return status;
}

int AreaStimSet::handle_trigger(std::string& s)
{
	int status = 0;
	if (s == "F")
	{
		if (m_bHaveFixpt)
		{
			m_fixpt.setContrast(100);
			status = 1;
		}
	}
	else if (s == "S")
	{
		m_grating.select();
		vsgObjResetDriftPhase();
		m_grating.setContrast(m_contrast);
		status = 1;
	}
	else if (s == "s")
	{
		m_grating.setContrast(0);
		status = 1;
	}
	else if (s == "a")
	{
		// increment iterator, reset to beginning if at end
		m_iterator++;
		if (m_iterator == m_diameters.end())
		{
			cerr << "at end of orientations, back to beginning." << endl;
			m_iterator = m_diameters.begin();
		}
		cerr << "Diameter " << *m_iterator << endl;

		// Change current page and clear it. Note that this is not the page currently in view.
		// Set the sf and draw the grating, then draw the fixpt. 
		m_current_page = 1 - m_current_page;
		vsgSetDrawPage(vsgVIDEOPAGE, m_pages[m_current_page], vsgBACKGROUND);
		m_grating.w = m_grating.h = *m_iterator;
		m_grating.draw();
		if (m_bHaveFixpt)
		{
			m_fixpt.draw();
		}
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, m_pages[m_current_page]);
		status = 0;
	}
	else if (s == "X")
	{
		m_fixpt.setContrast(0);
		m_grating.setContrast(0);
		status = 1;
	}
	return status;
}



int PositionStimSet::init(ARvsg& vsg, std::vector<int> pages)
{
	int status = 0;
	m_pages[0] = pages[0];
	m_pages[1] = pages[1];
	m_contrast = m_grating.contrast;
	m_iterator = m_positions.begin();
	vsgSetDrawPage(vsgVIDEOPAGE, m_pages[1], vsgBACKGROUND);
	vsgSetDrawPage(vsgVIDEOPAGE, m_pages[0], vsgBACKGROUND);
	m_current_page = 0;
	m_grating.init(vsg, 40);
	m_grating.setContrast(0);
	m_grating.x = *m_iterator++;
	m_grating.y = *m_iterator;
	m_grating.draw();
	if (m_bHaveFixpt)
	{
		m_fixpt.init(vsg, 2);
		m_fixpt.setContrast(0);
		m_fixpt.draw();
	}
	vsgPresent();
	return status;
}

int PositionStimSet::handle_trigger(std::string& s)
{
	int status = 0;
	if (s == "F")
	{
		if (m_bHaveFixpt)
		{
			m_fixpt.setContrast(100);
			status = 1;
		}
	}
	else if (s == "S")
	{
		m_grating.select();
		vsgObjResetDriftPhase();
		m_grating.setContrast(m_contrast);
		status = 1;
	}
	else if (s == "s")
	{
		m_grating.setContrast(0);
		status = 1;
	}
	else if (s == "a")
	{
		// increment iterator, reset to beginning if at end
		m_iterator++;
		if (m_iterator == m_positions.end())
		{
			cerr << "at end of positions, back to beginning." << endl;
			m_iterator = m_positions.begin();
		}

		// Change current page and clear it. Note that this is not the page currently in view.
		// Set the sf and draw the grating, then draw the fixpt. 
		m_current_page = 1 - m_current_page;
		vsgSetDrawPage(vsgVIDEOPAGE, m_pages[m_current_page], vsgBACKGROUND);
		m_grating.x = *m_iterator++;
		m_grating.y = *m_iterator;
		cerr << "Position " << m_grating.x << ", " << m_grating.y << endl;
		m_grating.draw();
		if (m_bHaveFixpt)
		{
			m_fixpt.draw();
		}
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, m_pages[m_current_page]);
		status = 0;
	}
	else if (s == "X")
	{
		m_fixpt.setContrast(0);
		m_grating.setContrast(0);
		status = 1;
	}
	return status;
}


int DanishStimSet::init(ARvsg& vsg, std::vector<int> pages)
{
	int status = 0;
	m_pages[0] = pages[0];
	m_pages[1] = pages[1];
	cout << "Pages " << m_pages[0] << ", " << m_pages[1] << endl;
	m_contrast = m_grating.contrast;
	m_iterator = m_ods.begin();

	// INit hole first, then donut
	// DRawing must be in reverse order - draw donut first, then hole. 
	if (m_bHaveHole) m_hole.init(vsg, 40);
	m_grating.init(vsg, 40);

	vsgSetDrawPage(vsgVIDEOPAGE, m_pages[1], vsgBACKGROUND);
	vsgSetDrawPage(vsgVIDEOPAGE, m_pages[0], vsgBACKGROUND);
	m_current_page = 0;
	m_grating.setContrast(0);
	m_grating.w = m_grating.h = *m_iterator;
	m_grating.draw();
	if (m_bHaveHole)
	{
		m_holeContrast = m_hole.contrast;
		m_hole.setContrast(0);
		m_hole.draw();
	}
	if (m_bHaveFixpt)
	{
		m_fixpt.init(vsg, 2);
		m_fixpt.setContrast(0);
		m_fixpt.draw();
	}
	vsgPresent();
	return status;
}

int DanishStimSet::handle_trigger(std::string& s)
{
	int status = 0;
	if (s == "F")
	{
		if (m_bHaveFixpt)
		{
			m_fixpt.setContrast(100);
			status = 1;
		}
	}
	else if (s == "S")
	{
		m_grating.select();
		vsgObjResetDriftPhase();
		if (m_bHaveHole)
		{
			m_hole.select();
			vsgObjResetDriftPhase();
		}
		m_grating.setContrast(m_contrast);
		m_hole.setContrast(m_holeContrast);
		status = 1;
	}
	else if (s == "s")
	{
		// toggle donut contrast
		// djs Also toggle hole contrast
		if (m_grating.contrast == 0) m_grating.setContrast(m_contrast);
		else m_grating.setContrast(0);
		if (m_hole.contrast == 0) m_hole.setContrast(m_holeContrast);
		else m_hole.setContrast(0);
		status = 1;
	}
	else if (s == "a")
	{
		// increment iterator, reset to beginning if at end
		m_iterator++;
		if (m_iterator == m_ods.end())
		{
			cerr << "at end of diameters, back to beginning." << endl;
			m_iterator = m_ods.begin();
		}
		cerr << "DIAMETER " << *m_iterator << endl;

		// Change current page and clear it. Note that this is not the page currently in view.
		// Set the sf and draw the grating, then draw the fixpt. 
		m_current_page = 1 - m_current_page;
		vsgSetDrawPage(vsgVIDEOPAGE, m_pages[m_current_page], vsgBACKGROUND);
		m_grating.w = m_grating.h = *m_iterator;
		m_grating.draw();
		if (m_bHaveHole) m_hole.draw();
		if (m_bHaveFixpt)
		{
			//m_fixpt.setContrast(0);
			m_fixpt.draw();
		}
		status = 1;
	}
	else if (s == "X")
	{
		m_fixpt.setContrast(0);
		m_grating.setContrast(0);
		if (m_bHaveHole) m_hole.setContrast(0);
		status = 1;
	}
	return status;
}






CRGStimSet::CRGStimSet(alert::ARContrastFixationPointSpec& f, alert::ARGratingSpec& g, int frames_per_term, const std::string& sequence, bool balanced) : m_grating1(g), m_grating0(g), m_fixpt(f), m_bHaveFixpt(true), m_fpt(frames_per_term), m_balanced(balanced)
{ 
	m_seq.assign(sequence); 
	m_contrasts.push_back(m_grating1.contrast);
	m_iterator = m_contrasts.begin();
	//m_contrast = m_grating1.contrast;
}

CRGStimSet::CRGStimSet(alert::ARGratingSpec& g, int frames_per_term, const std::string& sequence, bool balanced) : m_grating1(g), m_grating0(g), m_bHaveFixpt(false), m_fpt(frames_per_term), m_balanced(balanced)
{ 
	m_seq.assign(sequence); 
	m_contrasts.push_back(m_grating1.contrast);
	m_iterator = m_contrasts.begin();
	//m_contrast = m_grating1.contrast;
}

CRGStimSet::CRGStimSet(alert::ARContrastFixationPointSpec& f, alert::ARGratingSpec& g, int frames_per_term, const std::string& sequence, std::vector<double> contrasts, bool balanced) : m_grating1(g), m_grating0(g), m_fixpt(f), m_bHaveFixpt(true), m_fpt(frames_per_term), m_contrasts(contrasts), m_balanced(balanced)
{ 
	m_seq.assign(sequence); 
	m_iterator = m_contrasts.begin();
}

CRGStimSet::CRGStimSet(alert::ARGratingSpec& g, int frames_per_term, const std::string& sequence, std::vector<double> contrasts, bool balanced) : m_grating1(g), m_grating0(g), m_bHaveFixpt(false), m_fpt(frames_per_term), m_contrasts(contrasts), m_balanced(balanced)
{ 
	m_seq.assign(sequence); 
	m_iterator = m_contrasts.begin();
	//m_contrast = m_grating1.contrast;
}

int CRGStimSet::init(ARvsg& vsg, std::vector<int> pages)
{
	VSGCYCLEPAGEENTRY cycle[32768];
	int status = 0;
	unsigned int index;
	int pageBlank = vsgGetZoneDisplayPage(vsgVIDEOPAGE);

	m_page0 = pages[0];
	m_page1 = pages[1];

	// clear page 0, draw grating0, fixpt (if needed)
	vsgSetDrawPage(vsgVIDEOPAGE, m_page0, vsgBACKGROUND);
	m_grating0.init(vsg, 40);
	m_grating0.setContrast(0);
	m_grating0.setTemporalFrequency(0);
	m_grating0.draw();
	if (m_bHaveFixpt)
	{
		m_fixpt.init(vsg, 2);
		m_fixpt.setContrast(0);
		m_fixpt.draw();
	}

	// clear page 1, draw grating1, fixpt (if needed)
	vsgSetDrawPage(vsgVIDEOPAGE, m_page1, vsgBACKGROUND);
	m_grating1.init(vsg, 40);
	m_grating1.setContrast(0);
	m_grating1.setTemporalFrequency(0);
	m_grating1.draw();
	if (m_bHaveFixpt)
	{
		// do not init - the fixpt was init'd above
		m_fixpt.select();
		m_fixpt.draw();
	}

	vsgPresent();

	// Prepare page cycling
	for (index = 0; index < m_seq.length(); index++)
	{
		cycle[index].Frames = m_fpt;
		cycle[index].Page = (m_seq[index] == '0' ? m_page0 : m_page1) + vsgTRIGGERPAGE;
		cycle[index].Stop = 0;
	}

	if (m_balanced)
	{
		unsigned int len = m_seq.length();
		for (index = 0; index < m_seq.length(); index++)
		{
			cycle[len + index].Frames = m_fpt;
			cycle[len + index].Page = (m_seq[index] == '0' ? m_page1 : m_page0) + vsgTRIGGERPAGE;
			cycle[len + index].Stop = 0;
		}
		index = len*2;
	}
	cycle[index].Frames = m_fpt;
	cycle[index].Page = pageBlank + vsgTRIGGERPAGE;
	cycle[index].Stop = 0;

	cycle[index+1].Page = pageBlank + vsgTRIGGERPAGE;
	cycle[index+1].Stop = 1;


	vsgPageCyclingSetup(index + 2, &cycle[0]);
	//cerr << "Page cycling ready, " << index << " terms" << endl;

	return status;
}

int CRGStimSet::handle_trigger(std::string& s)
{
	static int page = m_page0;
	int status = 0;
	if (s == "F")
	{
		if (m_bHaveFixpt)
		{
			m_fixpt.setContrast(100);
			status = 1;
		}
	}
	else if (s == "S")
	{
		int contrast = (int)(*m_iterator);
		m_grating0.setContrast(-1*contrast);
		m_grating0.select();
		m_grating1.setContrast(contrast);
		m_grating1.select();
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "a")
	{
		m_iterator++;
		if (m_iterator == m_contrasts.end()) m_iterator = m_contrasts.begin();
	}
	else if (s == "X")
	{
		if (m_bHaveFixpt)
		{
			m_fixpt.setContrast(0);
		}
		m_grating0.setContrast(0);
		m_grating1.setContrast(0);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		status = 1;
	}
	return status;
}


int CBarStimSet::init(ARvsg& vsg, std::vector<int> pages)
{
	int status = 0;
	m_pageBackground = pages[0];
	m_pageStim = pages[1];
	cout << "background page " << m_pageBackground << " stim " << m_pageStim << endl;

	vsgSetPageWidth(2048);
	vsgSetPen2(vsgBACKGROUND);
	vsgSetCommand(vsgVIDEOCLEAR);
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageStim, vsgBACKGROUND);

	// check that bar width does not exceed maximum
	long w, h;
	double m;
	double md;
	w = vsgGetScreenWidthPixels();
	h = vsgGetScreenHeightPixels();
	m = sqrt((double)(w*w) + (double)(h*h)) * m_barMaxWidthKludge;
	vsgUnitToUnit(vsgPIXELUNIT, m, vsgDEGREEUNIT, &md);
	cerr << "Max bar pixels " << m << " degrees " << md << " (includes kludge factor)" << endl;
	if (md < m_barWidth)
	{
		cerr << "Specified bar width (" << m_barWidth << ") too large. Reducing to max degrees (" << md << ")" << endl;
		m_barWidth = md;
	}

	vsgUnitToUnit(vsgPIXELUNIT, m_barOffsetXPixels, vsgDEGREEUNIT, &m_barOffsetXDegrees);
	vsgUnitToUnit(vsgPIXELUNIT, m_barOffsetYPixels, vsgDEGREEUNIT, &m_barOffsetYDegrees);
	m_rect.x = m_barOffsetXDegrees;
	m_rect.y = m_barOffsetYDegrees;
	m_rect.w = m_barWidth;
	m_rect.h = m_barHeight;
	m_rect.orientation = *m_iterator;
	m_rect.init(1);

	// draw bar
	m_rect.draw();
	vsgPresent();

	prepareCycling(*m_iterator);

	return 0;
}

int CBarStimSet::handle_trigger(std::string& s)
{
	int status = 0;
	if (s == "F")
	{
	}
	else if (s == "S")
	{
		m_rect.setContrast(100);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "a")
	{
		// The bar may span multiple pages, so lets just clear all of video memory.
		vsgSetPen2(vsgBACKGROUND);
		vsgSetCommand(vsgVIDEOCLEAR);
		// OK now set draw page
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageStim, vsgBACKGROUND);
		m_iterator++;
		if (m_iterator == m_orientations.end()) m_iterator = m_orientations.begin();
		m_rect.orientation = *m_iterator;
		cout << "Ori " << m_rect.orientation << endl;
		m_rect.setContrast(0);
		m_rect.draw();
		prepareCycling(*m_iterator);
	}
	else if (s == "X")
	{
		m_rect.setContrast(0);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		status = 1;
	}
	return status;
}

std::string CBarStimSet::toString() const
{
	return string("CBar TEST StimSet");
}

void CBarStimSet::prepareCycling(double ori)
{
	VSGCYCLEPAGEENTRY cycle[32768];
	float thetadeg = (float)(*m_iterator);
	int i;
	double uhat[2];
	double vhat[2];
	double d;		// travel distance for center of box
	double pixperframe;
	double pixperdegree;
	long frametimeus;
	long C0[2], C1[2], C2[2], C3[2];	// corners of screen, if needed. 
	long H, W;
	double h, w;
	double p[2], q[2];
	int stepsPerSide;
	double stepSize;

	// Screen dimensions, corner coordinates
	H = vsgGetScreenHeightPixels();
	W = vsgGetScreenWidthPixels();
	C0[0] = W/2; C0[1] = -H/2;
	C1[0] = -W/2; C1[1] = -H/2;
	C2[0] = -W/2; C2[1] = H/2;
	C3[0] = W/2; C3[1] = H/2;

	// bar width, height in pixels, speed in pixels per frame
	frametimeus = vsgGetSystemAttribute(vsgFRAMETIME);
	vsgUnitToUnit(vsgDEGREEUNIT, 1, vsgPIXELUNIT, &pixperdegree);
	pixperframe = m_degreesPerSecond * pixperdegree * frametimeus / 1000000;
	vsgUnitToUnit(vsgDEGREEUNIT, m_barHeight, vsgPIXELUNIT, &h);
	vsgUnitToUnit(vsgDEGREEUNIT, m_barWidth, vsgPIXELUNIT, &w);
	cout << "pix per frame is " << pixperframe << " box dimensions " << w << "x" << h << endl;

	// Unit vectors and mid-endpoints
	uhat[0] = -sin((*m_iterator)* M_PI/180.0);
	uhat[1] = -cos((*m_iterator)* M_PI/180.0);
	vhat[0] = -uhat[1];
	vhat[1] = uhat[0];
	for (i=0; i<2; i++)
	{
		p[i] = -w/2*vhat[i];
		q[i] = w/2*vhat[i];
	}


	// normalize ori
	ori = *m_iterator;
	ori = ori - (floor(ori/360.0) * 360.0);
	cout << "Ori is " << ori << endl;

	// now find d, travel distance
	if (ori == 0)
	{
		d = H/2;
	}
	else if (ori < 90)
	{
		d = getDistance(C0, C1, C2, p, q, uhat, vhat);
	}
	else if (ori == 90)
	{
		d = W/2;
	}
	else if (ori < 180)
	{
		d = getDistance(C1, C2, C3, p, q, uhat, vhat);
	}
	else if (ori == 180)
	{
		d = H/2;
	}
	else if (ori < 270)
	{
		d = getDistance(C2, C3, C0, p, q, uhat, vhat);
	}
	else if (ori == 270)
	{
		d = W/2;
	}
	else 
	{
		d = getDistance(C3, C0, C1, p, q, uhat, vhat);
	}

	// We have the travel distance, now we must set up the cycling system.
	// The travel distance that has been computed is the distance from a line through
	// the center of the box.

	// Number of frames/steps on either side of center. 
	stepsPerSide = (int)fabs(((d+h/2)/pixperframe));
	stepSize = fabs(d+h/2) / stepsPerSide;
	cout << "Travel distance is " << d << " pix per frame " << pixperframe << " steps per side " << stepsPerSide << " step size " << stepSize << endl;

	// First half of travel...
	for (i = 0; i < stepsPerSide; i++)
	{
		cycle[i].Frames = 1;
		cycle[i].Page = m_pageStim;
		cycle[i].Xpos = (short)((stepsPerSide - i)*stepSize*uhat[0] + m_barOffsetXPixels);
		cycle[i].Ypos = (short)((stepsPerSide - i)*stepSize*uhat[1] + m_barOffsetYPixels);
		cycle[i].Stop = 0;
	}

	// Second half of travel
	for (i = 0; i < stepsPerSide; i++)
	{
		cycle[i+stepsPerSide].Frames = 1;
		cycle[i+stepsPerSide].Page = m_pageStim;
		cycle[i+stepsPerSide].Xpos = -(short)(i*stepSize*uhat[0] - m_barOffsetXPixels);
		cycle[i+stepsPerSide].Ypos = -(short)(i*stepSize*uhat[1] - m_barOffsetYPixels);
		cycle[i+stepsPerSide].Stop = 0;
	}

	cycle[stepsPerSide*2].Frames = 1;
	cycle[stepsPerSide*2].Page = m_pageBackground + vsgTRIGGERPAGE;
	cycle[stepsPerSide*2].Xpos = 0;
	cycle[stepsPerSide*2].Ypos = 0;
	cycle[stepsPerSide*2].Stop = 1;

	vsgPageCyclingSetup(stepsPerSide*2+1, &cycle[0]);
	//cerr << "Page cycling ready" << endl;

}

double CBarStimSet::getDistance(long *c0, long *c1, long *c2, double *p, double *q, double *u, double *v)
{
	double PP[2], QQ[2];
	double dPP, dQQ;
	int i;
	double dist;
	for (i=0; i<2; i++) 
	{
		PP[i] = c1[i] - p[i];
		QQ[i] = c1[i] - q[i];
	}
	dPP = PP[0] * v[0] + PP[1] * v[1];
	dQQ = QQ[0] * v[0] + QQ[1] * v[1];

	cout << "u " << u[0] << "," << u[1] << " v " << v[0] << "," << v[1] << endl;
	cout << "PP " << PP[0] << "," << PP[1] << " dPP " << dPP << endl;
	cout << "QQ " << QQ[0] << "," << QQ[1] << " dQQ " << dQQ << endl;
	if (dPP <= 0 && dQQ <= 0)
	{
		cout << "(dPP <= 0 && dQQ <= 0)" << endl;
		dist = max(getIntersectDistance(c0, c1, p, u), getIntersectDistance(c0, c1, q, u));
	}
	else if (dPP > 0 && dQQ <= 0)
	{
		cout << "Corner shot" << endl;
		dist = PP[0] * u[0] + PP[1] * u[1];
	}
	else
	{
		cout << "else" << endl;
		dist = max(getIntersectDistance(c1, c2, p, u), getIntersectDistance(c1, c2, q, u));
	}
	return dist;
}

double CBarStimSet::getIntersectDistance(long *ca, long *cb, double *p, double *u)
{
	double a = 0;
	long U[2];
	U[0] = ca[0] - cb[0];
	U[1] = ca[1] - cb[1];
	if (U[1] == 0)
	{
		a = (ca[1] - p[1])/u[1];
	}
	else
	{
		a = (ca[0] - p[0])/u[0];
	}
	return a;
}


int CounterphaseStimSet::init(ARvsg& vsg, std::vector<int> pages)
{
	int status = 0;
	m_pages[0] = pages[0];
	m_pages[1] = pages[1];
	cout << "Pages " << m_pages[0] << ", " << m_pages[1] << endl;
	m_contrast = m_grating.contrast;
	m_iterator = m_phases.begin();
	vsgSetDrawPage(vsgVIDEOPAGE, m_pages[1], vsgBACKGROUND);
	vsgSetDrawPage(vsgVIDEOPAGE, m_pages[0], vsgBACKGROUND);
	m_current_page = 0;
	m_grating.init(vsg, 40);
	m_grating.setContrast(0);
	m_grating.setTemporalFrequency(0);	// bad terminology, this calls vsgObjSetDriftVelocity()
	m_grating.phase = *m_iterator;
	m_grating.draw();
	if (!m_bStepTW)
	{
		vsgObjTableSinWave(vsgTWTABLE);
	}
	else
	{
//		vsgObjTableSquareWave(vsgTWTABLE, vsgObjGetTableSize(vsgTWTABLE)*0.25, vsgObjGetTableSize(vsgTWTABLE)*0.75);
		vsgObjTableSquareWave(vsgTWTABLE, 0, (DWORD)(vsgObjGetTableSize(vsgTWTABLE)*0.5));
	}
	vsgObjSetTemporalFrequency(0);
	vsgObjSetTemporalPhase(0);

	// Set trigger options to trigger on temporal phase
	vsgSetTriggerOptions(vsgTRIGOPT_OAS, -1, vsgTRIG_TEMPFREQX2, 0.5, 0, 0, 0);


	if (m_bHaveFixpt)
	{
		m_fixpt.init(vsg, 2);
		m_fixpt.setContrast(0);
		m_fixpt.draw();
	}
	vsgPresent();
	return status;
}

int CounterphaseStimSet::handle_trigger(std::string& s)
{
	int status = 0;
	if (s == "F")
	{
		if (m_bHaveFixpt)
		{
			m_fixpt.setContrast(100);
			status = 1;
		}
	}
	else if (s == "S")
	{
		m_grating.select();
		vsgObjSetTemporalPhase(0);
		vsgObjSetTemporalFrequency(m_tf);
		m_grating.setContrast(m_contrast);
		status = 1;
	}
	else if (s == "s")
	{
		vsgObjSetTemporalFrequency(0);
		m_grating.setContrast(0);
		status = 1;
	}
	else if (s == "a")
	{
		// increment iterator, reset to beginning if at end
		m_iterator++;
		if (m_iterator == m_phases.end())
		{
			cerr << "at end of phases, back to beginning." << endl;
			m_iterator = m_phases.begin();
		}
		cerr << "Phase " << *m_iterator << endl;

		// Change current page and clear it. Note that this is not the page currently in view.
		// Set the sf and draw the grating, then draw the fixpt. 
		m_current_page = 1 - m_current_page;
		vsgSetDrawPage(vsgVIDEOPAGE, m_pages[m_current_page], vsgBACKGROUND);
		m_grating.phase = *m_iterator;
		m_grating.draw();
		if (!m_bStepTW)
		{
			vsgObjTableSinWave(vsgTWTABLE);
		}
		else
		{
//			vsgObjTableSquareWave(vsgTWTABLE, vsgObjGetTableSize(vsgTWTABLE)*0.25, vsgObjGetTableSize(vsgTWTABLE)*0.75);
			vsgObjTableSquareWave(vsgTWTABLE, 0, (DWORD)(vsgObjGetTableSize(vsgTWTABLE)*0.5));
		}
		vsgObjSetTemporalPhase(0);
		if (m_bHaveFixpt)
		{
			//m_fixpt.setContrast(0);
			m_fixpt.draw();
		}
		status = 1;
	}
	else if (s == "X")
	{
		m_fixpt.setContrast(0);
		m_grating.setContrast(0);
		m_grating.select();
		vsgObjSetTemporalFrequency(0);
		status = 1;
	}
	return status;
}

int parse_attcues(const string& s, int nstim, vector<AttentionCue>& vecCues)
{
	COLOR_TYPE color;
	double rdiff;
	int i;
	vector<string> tokens;
	istringstream iss;
	tokenize(s, tokens, ",");
	if (tokens.size() % (nstim*2) == 0)
	{
		for (i=0; i<tokens.size()/nstim; i++)
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
