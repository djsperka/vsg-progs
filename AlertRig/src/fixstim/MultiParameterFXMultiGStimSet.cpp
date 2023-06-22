#include "MultiParameterFXMultiGStimSet.h"
#include "FXGStimParameterList.h"
#define _USE_MATH_DEFINES
#include <math.h>

using namespace boost;
using namespace alert;
using namespace std;

#define CYCLING_TYPE_NONE 0
#define CYCLING_TYPE_REGULAR 1
#define CYCLING_TYPE_PURSUIT 2

std::string f_pursuit_types[3] = { "None", "Regular", "Pursuit" };


MultiParameterFXMultiGStimSet::MultiParameterFXMultiGStimSet() 
	: FXMultiGStimSet() 
	, m_num_stim_pages(1)
	, m_bUseDrawGroups(false)
	, m_bSweepNotPursuit(false)
	, m_iCyclingType(CYCLING_TYPE_NONE)
{};

MultiParameterFXMultiGStimSet::MultiParameterFXMultiGStimSet(ARGratingSpec& grating) 
	: FXMultiGStimSet() 
	, m_num_stim_pages(1)
	, m_bUseDrawGroups(false)
	, m_bSweepNotPursuit(false)
	, m_iCyclingType(CYCLING_TYPE_NONE)
{ 
	add_grating(grating); 
};

MultiParameterFXMultiGStimSet::MultiParameterFXMultiGStimSet(ARGratingSpec& grating, ARContrastFixationPointSpec& fixpt)
	: FXMultiGStimSet(fixpt) 
	, m_num_stim_pages(1)
	, m_bUseDrawGroups(false)
	, m_bSweepNotPursuit(false)
	, m_iCyclingType(CYCLING_TYPE_NONE)
{
	add_grating(grating); 
};

MultiParameterFXMultiGStimSet::MultiParameterFXMultiGStimSet(ARContrastFixationPointSpec& fixpt)
	: FXMultiGStimSet(fixpt) 
	, m_num_stim_pages(1)
	, m_bUseDrawGroups(false)
	, m_bSweepNotPursuit(false)
	, m_iCyclingType(CYCLING_TYPE_NONE)
{};

MultiParameterFXMultiGStimSet::MultiParameterFXMultiGStimSet(ARContrastFixationPointSpec& fixpt, ARXhairSpec& xhair)
	: FXMultiGStimSet(fixpt, xhair) 
	, m_num_stim_pages(1)
	, m_bUseDrawGroups(false)
	, m_bSweepNotPursuit(false)
	, m_iCyclingType(CYCLING_TYPE_NONE)
{};

MultiParameterFXMultiGStimSet::MultiParameterFXMultiGStimSet(ARGratingSpec& grating, ARXhairSpec& xhair)
	: FXMultiGStimSet(xhair) 
	, m_num_stim_pages(1)
	, m_bUseDrawGroups(false)
	, m_bSweepNotPursuit(false)
	, m_iCyclingType(CYCLING_TYPE_NONE)
{
	add_grating(grating); 
};

MultiParameterFXMultiGStimSet::MultiParameterFXMultiGStimSet(ARGratingSpec& grating, ARContrastFixationPointSpec& fixpt, ARXhairSpec& xhair)
	: FXMultiGStimSet(fixpt, xhair) 
	, m_num_stim_pages(1)
	, m_bUseDrawGroups(false)
	, m_bSweepNotPursuit(false)
	, m_iCyclingType(CYCLING_TYPE_NONE)
{
	add_grating(grating); 
};

int MultiParameterFXMultiGStimSet::init(ARvsg& vsg, std::vector<int> pages, int num_stim_pages)
{
	int status = 0;
	int levels = 0;
	const int max_levels = 40;
	m_blank_page = pages[0];
	m_fixpt_page = pages[1];
	m_fixpt_dot_page = pages[2];
	m_stim_page = pages[3];
	m_alt_page = pages[4];
	m_iCyclingType = CYCLING_TYPE_NONE;
	//m_bUseCycling = false;
	m_iCyclingDelay = 0;
	m_iStimDuration = 0;
	m_num_stim_pages = abs(num_stim_pages);
	m_bUseDrawGroups = (num_stim_pages<0);
	m_pageflipindex = 0;
	m_pageflippages[0] = m_stim_page;
	m_pageflippages[1] = m_alt_page;

	cerr << "init: num_stim_pages " << m_num_stim_pages << " drawGroups? " << m_bUseDrawGroups << " Sweep not pursuit? " << m_bSweepNotPursuit <<  endl;

	if (m_bSweepNotPursuit)
	{
		// enable overlay. Fixpt will be drawn on this page.
		vsgSetCommand(vsgOVERLAYMASKMODE);
	}

	if (has_xhair())
	{
		xhair().init(vsg, 16);
	}
	if (has_grating() || has_distractor())
	{
		levels = min(240/(int)(count() + distractor_count()), max_levels);
		for (unsigned int i = 0; i < count(); i++)
		{
			grating(i).init(vsg, levels);
		}
		for (unsigned int i = 0; i < distractor_count(); i++)
		{
			distractor(i).init(vsg, levels);
		}
	}

#if 0
	if (has_rectangle())
	{
		for (unsigned int i = 0; i < rectangle_count(); i++)
		{
			rectangle(i).init(vsg, 2);
		}
	}
#endif

	if (has_fixpt())
	{
		fixpt().init(vsg, 2, !m_bSweepNotPursuit);
	}

	// saved dot colors is a thing for Gregg's Grid. 
	//m_dotColorsSaved.clear();
	if (has_dot())
	{
		for (unsigned int i = 0; i < dot_count(); i++)
		{
			// How many levels will this dot need? 
			// Maybe not the best method, but assume that the number needed 
			// for the first trial is the number needed for all trials. 
			if (dot(i).isMulti())
				dot(i).init(vsg, (int)dot(i).getMulti().size(), false);
			else
				dot(i).init(vsg, 1, false);
//			m_dotColorsSaved.push_back(dot(i).color);
		}
	}

	if (has_rectangle())
	{
		for (unsigned int i = 0; i < rectangle_count(); i++)
		{
			// How many levels will this dot need? 
			// Maybe not the best method, but assume that the number needed 
			// for the first trial is the number needed for all trials. 
			if (rectangle(i).isMulti())
				rectangle(i).init(vsg, (int)rectangle(i).getMulti().size(), false);
			else
				rectangle(i).init(vsg, 1, false);
		}
	}


	// Set initial params in grating(s)
	set_initial_parameters();
	if (m_iCyclingType != CYCLING_TYPE_NONE)
	{
		if (m_iCyclingType == CYCLING_TYPE_PURSUIT)
		{
			vsgSetCommand(vsgVIDEODRIFT + vsgOVERLAYDRIFT);
		}
		setup_cycling();
	}

	draw_current();
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, vsgNOCLEAR);
	vsgSetDisplayPage(0);
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	return status;
}

void MultiParameterFXMultiGStimSet::cleanup(std::vector<int> pages)
{
	if (m_bSweepNotPursuit)
		vsgSetCommand(vsgOVERLAYDISABLE);
}

int MultiParameterFXMultiGStimSet::handle_trigger(const std::string& s, const std::string& args)
{
	int status = 0;
	if (s == "F")
	{
		// In all cases, do these things. 

		// move screen back to origin (if pursuit and trial was ended mid-pursuit)
		vsgMoveScreen(0, 0);

		// turn on distractors if any
		for (unsigned int i = 0; i < distractor_count(); i++)
		{
			distractor(i).select();
			vsgObjResetDriftPhase();
			distractor(i).setContrast(distractor_contrast(i));
			status = 1;
		}

		// when doing a sweepNotPursuit, the fixpt is on the overlay page and things
		// must be done differently. In order to get the two pages (overlay and video) to change
		// simultaneously at onset, set up page cycling to do a single-page flip. 

		if (!m_bSweepNotPursuit)
		{
			// fixpt should always be at 100 contrast. Cannot support "f" when "S" is on if dots are present. 
			if (has_fixpt())
			{
				if (!m_bSweepNotPursuit) fixpt().setContrast(100);
				status = 1;
			}

			vsgSetDrawPage(vsgVIDEOPAGE, m_fixpt_page, vsgNOCLEAR);
		}
		else
		{
			vsgSetCommand(vsgOVERLAYMASKMODE);
			vsgPageCyclingSetup(m_fixpt_cycle_count, m_cycle_params + m_fixpt_cycle_start);
			vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
			status = 1;
		}
		
	}
	else if (s == "f")
	{
		if (has_fixpt())
		{
			fixpt().setContrast(0);
			status = 1;
		}
	}
	else if (s == "S")
	{
		for (unsigned int i = 0; i < count(); i++)
		{
			grating(i).select();
			if (m_bResetPhaseOnTrigger) vsgObjResetDriftPhase();
			grating(i).setContrast(contrast(i));
		}
		m_bResetPhaseOnTrigger = false;
		if (CYCLING_TYPE_NONE != m_iCyclingType)
		{
			vsgPageCyclingSetup(m_stim_cycle_count, m_cycle_params + m_stim_cycle_start);
			vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		}
		else
		{
			vsgSetDrawPage(vsgVIDEOPAGE, m_stim_page, vsgNOCLEAR);
		}
		status = 1;
	}
	else if (s == "s" || s == "u")
	{
		status = 1;
		if (CYCLING_TYPE_NONE != m_iCyclingType)
		{
			vsgSetCommand(vsgCYCLEPAGEDISABLE);
			status = 2;
		}
		vsgSetDrawPage(vsgVIDEOPAGE, m_fixpt_page, vsgNOCLEAR);
	}
	else if (s == "U")
	{
		for (unsigned int i = 0; i < count(); i++)
		{
			grating(i).select();
			grating(i).setContrast(contrast(i));
		}
		if (CYCLING_TYPE_NONE != m_iCyclingType)
		{
			vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		}
		else
		{
			vsgSetDrawPage(vsgVIDEOPAGE, m_alt_page, vsgNOCLEAR);
		}
		status = 1;
	}
	else if (s == "a")
	{
		// unset parameters that trigger cycling, if any. One of the parameter lists called in advance() must enable it. 
		setCyclingDelay(-1);
		setStimDuration(-1);
		setPursuitParameters(-1, 0, 0);
		advance();
		draw_current();
		if (CYCLING_TYPE_NONE != m_iCyclingType)
		{
			setup_cycling();
		}
		// return 0 so vsgPresent() will not be called. 
		status = 0;
	}
	else if (s == "D")
	{
		status = 0;
		if (has_dot())
		{
			// expecting N,color
			stringstream ss(args);
			int index = -1;
			COLOR_TYPE color;
			ss >> index;
			if (ss)
			{
				ss.ignore(1);
				ss >> color;
			}
			else
			{
				cerr << "Error parsing D arg: expecting N,COLOR, got " << args << endl;
			}

			if (ss)
			{
				if (index > -1 && index < dot().getMulti().size())
				{
					dot(0).getMulti().at(index).color = color;
					m_pageflipindex = 1 - m_pageflipindex;
					draw_stuff_on_page(m_pageflippages[m_pageflipindex], true, true, true, true, true);
					status = 3;
				}
				else
				{
					cerr << "Error - dot index (" << index << ") out of range, must be less than " << dot_count() << endl;
				}
			}
			else
			{
				cerr << "Error parsing D arg: expecting N,COLOR, got " << args << endl;
			}
		}
	}
	else if (s == "X")
	{
		status = 1;
		if (m_bSweepNotPursuit)
		{
			vsgSetCommand(vsgCYCLEPAGEDISABLE);
			vsgPageCyclingSetup(m_clear_cycle_count, m_cycle_params + m_clear_cycle_start);
			vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
			status = 2;
		}
		else
		{
			vsgSetDrawPage(vsgVIDEOPAGE, m_blank_page, vsgNOCLEAR);
		}
	}
	else if (s == "A")
	{
		int itmp=-1;
		cout << "Enter page: ";
		cin >> itmp;
		if (itmp > -1 && itmp < 10)
		{
			vsgSetDrawPage(vsgVIDEOPAGE, itmp, vsgNOCLEAR);
			status = 1;
		}
	}
	else if (s == "B")
	{
		int itmp = -1;
		cout << "Enter ov page: ";
		cin >> itmp;
		if (itmp > -1 && itmp < 2)
		{
			vsgSetDrawPage(vsgOVERLAYPAGE, itmp, vsgNOCLEAR);
			status = 1;
		}
	}
	else if (s == "Z")
	{
		DWORD vtmp, otmp;
		vtmp = vsgGetZoneDisplayPage(vsgVIDEOPAGE);
		otmp = vsgGetZoneDisplayPage(vsgOVERLAYPAGE);
		cerr << "Video page " << vtmp << " overlay page " << otmp << endl;
	}

	return status;
}


void MultiParameterFXMultiGStimSet::set_current(size_t index)
{
	// TODO
	std::cerr << "set_current NOT IMPL" << std::endl;
}

// draw pages for this stimulus/trial. 
// Page indices used are:
// m_blank_page = background only
// m_fixpt_page = fixpt only, distractors and xhair if present.
// m_fixpt_dot_page = fixpt and distractors, xhair, and dots (if present)
void MultiParameterFXMultiGStimSet::draw_current()
{
	// When Pursuit is used (or anything that drifts the video screen), make sure to move the 
	// screen back to its home before drawing. If cycling was active and page left mid-drift, 
	// this would be needed to fix. Should have no effect otherwise.
	vsgMoveScreen(0, 0);

	// When cycling is used, we'll need this page with fixpt, xhair (if present), distractors(if present) 
	m_bResetPhaseOnTrigger = true;
	bool bFixptOnVideoPage = (m_bSweepNotPursuit ? false : true);
	draw_stuff_on_page(m_blank_page, false, false, false, false, false);
	draw_stuff_on_page(m_fixpt_page, bFixptOnVideoPage, true, false, false, false);
	draw_stuff_on_page(m_fixpt_dot_page, bFixptOnVideoPage, true, false, true, false);
	draw_stuff_on_page(m_stim_page, bFixptOnVideoPage, true, true, true, true, 1);
	if (m_num_stim_pages > 1)
	{
		if (!m_bUseDrawGroups)
		{
			advance();
			draw_stuff_on_page(m_alt_page, bFixptOnVideoPage, true, true, true, true);
		}
		else 
		{
			draw_stuff_on_page(m_alt_page, bFixptOnVideoPage, true, true, true, true, 2);
		}
	}
	m_pageflipindex = 0;	// in case this is used, reset so flipping works right the first time


	// When using sweep not pursuit, prepare the overlay pages.
	// overlay page 0 - clear (transparent)
	// overlay page 1 - fixation point
	vsgSetDrawPage(vsgOVERLAYPAGE, 1, 0);
	if (has_fixpt())
	{
		fixpt().drawOverlay();
	}
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 0);
	vsgSetDisplayPage(0);	// displays overlay page 0, which should be empty.
}


void MultiParameterFXMultiGStimSet::draw_stuff_on_page(int pagenumber, bool bFixpt, bool bDistractor, bool bGrating, bool bDots, bool bRectangle, int iDrawGroup)
{
	// xhair, fixpt, dots and stim. 
	vsgSetDrawPage(vsgVIDEOPAGE, pagenumber, vsgBACKGROUND);
	if (bFixpt && has_xhair())
	{
		xhair().draw();
	}
	if (bDistractor && has_distractor())
	{
		for (unsigned int i = 0; i < distractor_count(); i++)
		{
			if (distractor(i).inDrawGroup(iDrawGroup))
				distractor(i).draw();
		}
	}
	if (bGrating && has_grating())
	{
		for (unsigned int i = 0; i < count(); i++)
		{
			if (grating(i).inDrawGroup(iDrawGroup))
				grating(i).draw();
		}
	}

	if (bRectangle && has_rectangle())
	{
		for (unsigned int i = 0; i < rectangle_count(); i++)
		{
			if (rectangle(i).inDrawGroup(iDrawGroup))
				rectangle(i).draw();
		}
	}

	if (bFixpt && has_fixpt())
	{
		fixpt().draw();
	}
	if (bDots && has_dot())
	{
		for (unsigned int i = 0; i < dot_count(); i++)
		{
			dot(i).draw();
		}
	}
}

void MultiParameterFXMultiGStimSet::setCyclingDelay(int ndelay)
{
	if (ndelay < 0)
	{
		m_iCyclingType = CYCLING_TYPE_NONE;
		m_iCyclingDelay = 0;
	}
	else
	{
		m_iCyclingType = CYCLING_TYPE_REGULAR;
		m_iCyclingDelay = ndelay;
	}
	return;
}

void MultiParameterFXMultiGStimSet::setStimDuration(double seconds)
{
	if (seconds < 0)
	{
		m_iCyclingType = CYCLING_TYPE_NONE;
		m_iStimDuration = 0;
	}
	else
	{
		// Convert stim duration to frames. Note frame time returned is in us.
		m_iCyclingType = CYCLING_TYPE_REGULAR;
		m_iStimDuration = (int)floor(seconds * 1000000.0 / vsgGetSystemAttribute(vsgFRAMETIME));
	}
	return;
}

void MultiParameterFXMultiGStimSet::setPursuitParameters(double durSeconds, double dirDegrees, double degPerSecond)
{
	if (durSeconds < 0)
	{
		m_iCyclingType = CYCLING_TYPE_NONE;
		m_iStimDuration = 0;
	}
	else
	{
		// Convert stim duration to frames. Note frame time returned is in us.
		m_iCyclingType = CYCLING_TYPE_PURSUIT;
		m_iStimDuration = (int)floor(durSeconds * 1000000.0 / vsgGetSystemAttribute(vsgFRAMETIME));

		// Convert to x- and y- displacement, in pixels, of the fixation point, per frame.
		// When page cycling is set up to simulate this effect, the page origin is what is moved, hence there is a 
		// multiplication by -1 that takes place when this value is used. 
		// Note that m_dyPursuit value has "-1*" here: in pixel coords the vsg is positive-down, so 
		// an UPWARDS displacement on the screen is a NEGATIVE displacement in vsg pixel coords.

		double pixels;
		vsgUnit2Unit(vsgDEGREEUNIT, durSeconds * degPerSecond, vsgPIXELUNIT, &pixels);
		m_dxPursuit = pixels * cos(M_PI * dirDegrees / 180.0) / m_iStimDuration;
		m_dyPursuit = -1 * pixels * sin(M_PI * dirDegrees / 180.0) / m_iStimDuration;
	}
	return;
}

// As originally written, page cycling is only used when there is a cycling delay, 
// or when there is a stim duration. If neither of those was set, then there's no need for cycling, as
// the "S" trigger transitions to the fixpt+stim page, without need for precise timing or delay. 


void MultiParameterFXMultiGStimSet::setup_cycling()
{
	int status = 0;
	int count = 0;

	if (CYCLING_TYPE_NONE == m_iCyclingType)
		return;

	//memset(m_cycle_params, 0, sizeof(m_cycle_params));
	m_fixpt_cycle_start = m_fixpt_cycle_count = 0;
	m_stim_cycle_start = m_stim_cycle_count = 0;
	m_clear_cycle_start = m_clear_cycle_count = 0;


	if (CYCLING_TYPE_REGULAR == m_iCyclingType)
	{
		m_stim_cycle_start = 0;
		VSGCYCLEPAGEENTRY* cycle = m_cycle_params + m_stim_cycle_start;
		count = 0;

		if (m_iCyclingDelay > 0)
		{
			cycle[count].Frames = 1 + (m_iCyclingDelay > 0 ? m_iCyclingDelay : 0);
			cycle[count].Page = m_fixpt_dot_page;
			cycle[count].Stop = 0;
			count++;
		}

		cycle[count].Page = m_stim_page + vsgTRIGGERPAGE;
		if (m_iStimDuration <= 0)
		{
			cycle[count].Frames = 0;
			cycle[count].Stop = 1;
			count++;
		}
		else
		{
			cycle[count].Frames = m_iStimDuration;
			cycle[count].Stop = 0;
			count++;

			cycle[count].Frames = 0;
			cycle[count].Page = m_blank_page + +vsgTRIGGERPAGE;
			cycle[count].Stop = 1;
			count++;
		}

		m_stim_cycle_count = count;
	}
	else if (CYCLING_TYPE_PURSUIT == m_iCyclingType)
	{
		m_stim_cycle_start = 0;
		VSGCYCLEPAGEENTRY* cycle = m_cycle_params + m_stim_cycle_start;
		count = 0;

		// If there is a delay -- i.e. delay before starting "stim" (pursuit)
		if (m_iCyclingDelay > 0)
		{
			cycle[count].Frames = 1 + (m_iCyclingDelay > 0 ? m_iCyclingDelay : 0);
			cycle[count].Page = m_fixpt_dot_page + vsgTRIGGERPAGE + (m_bSweepNotPursuit ? vsgDUALPAGE : 0);
			cycle[count].ovPage = 1;
			cycle[count].Stop = 0;
			count++;
		}
		for (int i = 0; i < m_iStimDuration; i++)
		{
			cycle[count].Frames = 1;
			cycle[count].Page = m_stim_page + (i == 0 ? vsgTRIGGERPAGE : 0) + (m_bSweepNotPursuit ? vsgDUALPAGE : 0);	// trigger only onset of pursuit.
			cycle[count].ovPage = 1;
			cycle[count].Xpos = (short)(-1 * (i + 1) * m_dxPursuit);
			cycle[count].Ypos = (short)(-1 * (i + 1) * m_dyPursuit);
			cycle[count].Stop = 0;
			count++;
		}
		cycle[count].Frames = 1;
		cycle[count].Page = m_blank_page + vsgTRIGGERPAGE + vsgDUALPAGE;
		cycle[count].ovPage = 0;
		cycle[count].Stop = 1;
		count++;

		m_stim_cycle_count = count;

		// if sweep-not-pursuit, then must create trivial page cycle to get simultaneous page change
		if (m_bSweepNotPursuit)
		{
			m_fixpt_cycle_start = m_stim_cycle_count;
			cycle = m_cycle_params + m_fixpt_cycle_start;
			cycle[0].Frames = 1;
			cycle[0].Page = m_fixpt_page + vsgDUALPAGE;
			cycle[0].ovPage = 1;
			cycle[0].Stop = 1;
			m_fixpt_cycle_count = 1;

			m_clear_cycle_start = m_fixpt_cycle_start + m_fixpt_cycle_count;
			cycle = m_cycle_params + m_clear_cycle_start;
			cycle[0].Frames = 1;
			cycle[0].Page = m_blank_page + vsgDUALPAGE;
			cycle[0].ovPage = 0;
			cycle[0].ovXpos = cycle[0].ovYpos = 0;
			cycle[0].Stop = 1;
			m_clear_cycle_count = 1;
		}
	}

	if (m_fixpt_cycle_count + m_stim_cycle_count + m_clear_cycle_count > m_max_cycle_count)
	{
		cerr << "ERROR: cycling array too long. Undefined behavior to follow...." << endl;
	}
	//status = vsgPageCyclingSetup(count, &cycle[0]);
}

string MultiParameterFXMultiGStimSet::toString() const
{
	string s;
	cerr << "MultiParameterFXMultiGStimSet: " << endl;
	cerr << "cycle type " << m_iCyclingType << " sweep? " << std::boolalpha << m_bSweepNotPursuit << endl;
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
			cerr << " Grating " << i << " multi? " << std::boolalpha << grating(i).bIsMulti << "(" << grating(i).m_multi.size() << " drawgroup 1/2/3: " << grating(i).inDrawGroup(1) << " / " << grating(i).inDrawGroup(2) << " / " << grating(i).inDrawGroup(3) << " : " << grating(i) << endl;
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
	if (has_rectangle())
	{
		for (unsigned int i = 0; i < rectangle_count(); i++)
			cerr << " Rect " << i << ": " << rectangle(i) << endl;
	}
	else
		cerr << " Rect: NONE" << endl;
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


