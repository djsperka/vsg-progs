#include "StimSet.h"
#include <iostream>
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

std::string DonutStimSet::toString() const
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
	oss <<     "  donut         : " << m_donut << endl;
	oss << "  (outer, inner) diameters: ";
	for (i=0; i<m_diameters.size(); i++)
	{
		if (i>0) oss << ", ";
		oss << "(" << m_diameters[i].first << ", " << m_diameters[i].second << ")";
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
//	m_grating.draw(true);
	m_grating.draw((long)vsgTRANSONLOWER);
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
		m_grating.draw((long)vsgTRANSONLOWER);
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
	m_grating.draw((long)vsgTRANSONLOWER);
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
	m_grating.draw((long)vsgTRANSONLOWER);
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
	m_grating.draw((long)vsgTRANSONLOWER);
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
		//m_grating.setContrast(0);
		m_grating.sf = *m_iterator;
		m_grating.draw((long)vsgTRANSONLOWER);
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
	m_grating.draw((long)vsgTRANSONLOWER);
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
		//m_grating.setContrast(0);
		m_grating.orientation = *m_iterator;
		m_grating.draw((long)vsgTRANSONLOWER);
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
	m_grating.draw((long)vsgTRANSONLOWER);
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
		//m_grating.setContrast(0);
		m_grating.w = m_grating.h = *m_iterator;
		m_grating.draw((long)vsgTRANSONLOWER);
		if (m_bHaveFixpt)
		{
			m_fixpt.draw();
		}
		// HACK
		VSGPAGEDESCRIPTOR page;
		vsgGetCurrentDrawPage(&page);
		cout << "handle a: zdp=" << vsgGetZoneDisplayPage(vsgVIDEOPAGE) << " dp=" << page.Page << endl;
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, m_pages[m_current_page]);
		//end hack
		status = 0;

//		status = 1;
	}
	else if (s == "X")
	{
		m_fixpt.setContrast(0);
		m_grating.setContrast(0);
		status = 1;
	}
	return status;
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


int DonutStimSet::init(ARvsg& vsg, std::vector<int> pages)
{
	int status = 0;
	std::pair<double, double> p;
	m_pages[0] = pages[0];
	m_pages[1] = pages[1];
	m_contrast = m_donut.contrast;
	m_iterator = m_diameters.begin();
	vsgSetDrawPage(vsgVIDEOPAGE, m_pages[1], vsgBACKGROUND);
	vsgSetDrawPage(vsgVIDEOPAGE, m_pages[0], vsgBACKGROUND);
	m_current_page = 0;
	m_donut.init(vsg, 40);
	m_donut.setContrast(0);
	m_donut.w = m_donut.h = m_iterator->first;
	m_donut.wd = m_donut.hd = m_iterator->second;
	m_donut.draw((long)vsgTRANSONLOWER);
	if (m_bHaveFixpt)
	{
		m_fixpt.init(vsg, 2);
		m_fixpt.setContrast(0);
		m_fixpt.draw();
	}
	vsgPresent();
	return status;
}

int DonutStimSet::handle_trigger(std::string& s)
{
	int status = 0;
	VSGPAGEDESCRIPTOR page;
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
		m_donut.select();
		vsgObjResetDriftPhase();
		m_donut.setContrast(m_contrast);
		vsgGetCurrentDrawPage(&page);
		cout << "handle S: zdp=" << vsgGetZoneDisplayPage(vsgVIDEOPAGE) << " dp=" << page.Page << endl;
		status = 1;
	}
	else if (s == "s")
	{
		vsgGetCurrentDrawPage(&page);
		cout << "handle s: zdp=" << vsgGetZoneDisplayPage(vsgVIDEOPAGE) << " dp=" << page.Page << endl;
		m_donut.setContrast(0);
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
		cerr << "Diameter " << m_iterator->first <<  " Inner " << m_iterator->second << endl;

		// Change current page and clear it. Note that this is not the page currently in view.
		// Set the sf and draw the grating, then draw the fixpt. 
		m_current_page = 1 - m_current_page;
		vsgSetDrawPage(vsgVIDEOPAGE, m_pages[m_current_page], vsgBACKGROUND);
		//m_grating.setContrast(0);
		m_donut.w = m_donut.h = m_iterator->first;
		m_donut.wd = m_donut.hd = m_iterator->second;
		m_donut.draw((long)vsgTRANSONLOWER);
		if (m_bHaveFixpt)
		{
			//m_fixpt.setContrast(0);
			m_fixpt.draw();
		}
		// HACK
		vsgGetCurrentDrawPage(&page);
		cout << "handle a: zdp=" << vsgGetZoneDisplayPage(vsgVIDEOPAGE) << " dp=" << page.Page << endl;
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, m_pages[m_current_page]);
		//end hack
		status = 0;
	}
	else if (s == "X")
	{
		m_fixpt.setContrast(0);
		m_donut.setContrast(0);
		status = 1;
	}
	return status;
}



CRGStimSet::CRGStimSet(alert::ARContrastFixationPointSpec& f, alert::ARGratingSpec& g, int frames_per_term, const std::string& sequence, bool balanced) : m_grating1(g), m_grating0(g), m_fixpt(f), m_bHaveFixpt(true), m_fpt(frames_per_term), m_balanced(balanced)
{ 
	m_seq.assign(sequence); 
	m_contrast = m_grating1.contrast;
}

CRGStimSet::CRGStimSet(alert::ARGratingSpec& g, int frames_per_term, const std::string& sequence, bool balanced) : m_grating1(g), m_grating0(g), m_bHaveFixpt(false), m_fpt(frames_per_term), m_balanced(balanced)
{ 
	m_seq.assign(sequence); 
	m_contrast = m_grating1.contrast;
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
	m_grating0.draw((long)vsgTRANSONLOWER);
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
	m_grating1.draw((long)vsgTRANSONLOWER);
	if (m_bHaveFixpt)
	{
		// do not init - the fixpt was init'd above
		// m_fixpt.init(2);
		// m_fixpt.setContrast(0);
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
	cerr << "Page cycling ready, " << index << " terms" << endl;

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
		m_grating0.setContrast(-1*m_contrast);
		m_grating1.setContrast(m_contrast);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "a")
	{
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
