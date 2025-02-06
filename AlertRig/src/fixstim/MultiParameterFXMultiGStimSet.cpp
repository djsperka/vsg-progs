#include "MultiParameterFXMultiGStimSet.h"
#include "FXGStimParameterList.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <iomanip>

using namespace boost;
using namespace alert;
using namespace std;

#define CYCLING_TYPE_NONE 0
#define CYCLING_TYPE_REGULAR 1
#define CYCLING_TYPE_PURSUIT 2

std::string f_pursuit_types[3] = { "None", "Regular", "Pursuit" };

void getPanningBBox(double W, double H, double distance, double ori, double(&ullr)[4]);


MultiParameterFXMultiGStimSet::MultiParameterFXMultiGStimSet() 
	: FXMultiGStimSet() 
	, m_num_stim_pages(1)
	, m_bUseDrawGroups(false)
	, m_bUseCycling(false)
	, m_bUseSweepCycling(false)
	, m_iCyclingType(CYCLING_TYPE_NONE)
	, m_bHaveBmpImageSpec(false)
{};

MultiParameterFXMultiGStimSet::MultiParameterFXMultiGStimSet(ARGratingSpec& grating) 
	: FXMultiGStimSet() 
	, m_num_stim_pages(1)
	, m_bUseDrawGroups(false)
	, m_bUseCycling(false)
	, m_bUseSweepCycling(false)
	, m_iCyclingType(CYCLING_TYPE_NONE)
	, m_bHaveBmpImageSpec(false)
{
	add_grating(grating); 
};

MultiParameterFXMultiGStimSet::MultiParameterFXMultiGStimSet(ARGratingSpec& grating, ARContrastFixationPointSpec& fixpt)
	: FXMultiGStimSet(fixpt) 
	, m_num_stim_pages(1)
	, m_bUseDrawGroups(false)
	, m_bUseCycling(false)
	, m_bUseSweepCycling(false)
	, m_iCyclingType(CYCLING_TYPE_NONE)
	, m_bHaveBmpImageSpec(false)
{
	add_grating(grating); 
};

MultiParameterFXMultiGStimSet::MultiParameterFXMultiGStimSet(ARContrastFixationPointSpec& fixpt)
	: FXMultiGStimSet(fixpt) 
	, m_num_stim_pages(1)
	, m_bUseDrawGroups(false)
	, m_bUseCycling(false)
	, m_bUseSweepCycling(false)
	, m_iCyclingType(CYCLING_TYPE_NONE)
	, m_bHaveBmpImageSpec(false)
{};

MultiParameterFXMultiGStimSet::MultiParameterFXMultiGStimSet(ARContrastFixationPointSpec& fixpt, ARXhairSpec& xhair)
	: FXMultiGStimSet(fixpt, xhair) 
	, m_num_stim_pages(1)
	, m_bUseDrawGroups(false)
	, m_bUseCycling(false)
	, m_bUseSweepCycling(false)
	, m_iCyclingType(CYCLING_TYPE_NONE)
	, m_bHaveBmpImageSpec(false)
{};

MultiParameterFXMultiGStimSet::MultiParameterFXMultiGStimSet(ARGratingSpec& grating, ARXhairSpec& xhair)
	: FXMultiGStimSet(xhair) 
	, m_num_stim_pages(1)
	, m_bUseDrawGroups(false)
	, m_bUseCycling(false)
	, m_bUseSweepCycling(false)
	, m_iCyclingType(CYCLING_TYPE_NONE)
	, m_bHaveBmpImageSpec(false)
{
	add_grating(grating); 
};

MultiParameterFXMultiGStimSet::MultiParameterFXMultiGStimSet(ARGratingSpec& grating, ARContrastFixationPointSpec& fixpt, ARXhairSpec& xhair)
	: FXMultiGStimSet(fixpt, xhair) 
	, m_num_stim_pages(1)
	, m_bUseDrawGroups(false)
	, m_bUseCycling(false)
	, m_bUseSweepCycling(false)
	, m_iCyclingType(CYCLING_TYPE_NONE)
	, m_bHaveBmpImageSpec(false)
{
	add_grating(grating); 
};

int MultiParameterFXMultiGStimSet::init(std::vector<int> pages, int num_stim_pages)
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
	m_iCyclingDelay = 0;
	m_iStimDuration = 0;
	m_num_stim_pages = abs(num_stim_pages);
	m_bUseDrawGroups = (num_stim_pages<0);
	m_pageflipindex = 0;
	m_pageflippages[0] = m_stim_page;
	m_pageflippages[1] = m_alt_page;


	cerr << "init: num_stim_pages " << m_num_stim_pages << " drawGroups? " << m_bUseDrawGroups << " Cycling? " << m_bUseCycling << " Sweep cycling? " << m_bUseSweepCycling << endl;

	if (m_bUseCycling && m_bUseSweepCycling)
	{
		// enable overlay for use when cycling, initialize both overlay pages to clear color 0.
		vsgSetCommand(vsgOVERLAYMASKMODE);
		vsgSetCommand(vsgVIDEODRIFT + vsgOVERLAYDRIFT);	// neeed this??? 
		vsgSetDrawPage(vsgOVERLAYPAGE, 1, 0);
		vsgSetDrawPage(vsgOVERLAYPAGE, 0, 0);
	}


	// If there is a bmp image set, allocate the lowest levels for it. 
	// If not, then begin allocations at level 1 (allowing for m_cLowLevel)

	if (m_bHaveBmpImageSpec && m_vecBmpImageSpec.size() > 0)
	{
		m_vecBmpImageSpec[0].init(m_uiNBmpImageLevels, false);
		for (size_t i = 1; i < m_vecBmpImageSpec.size(); i++)
			m_vecBmpImageSpec[i].init(m_vecBmpImageSpec[0]);
	}

	if (has_xhair())
	{
		xhair().init(16);
	}
	if (has_grating() || has_distractor())
	{
		int nlevelsForGratings = ARvsg::instance().remaining() - 2;
		levels = min(nlevelsForGratings/(int)(count() + distractor_count()), max_levels);
		for (unsigned int i = 0; i < count(); i++)
		{
			grating(i).init(levels);
		}
		for (unsigned int i = 0; i < distractor_count(); i++)
		{
			distractor(i).init(levels);
		}
	}

	if (has_fixpt())
	{
		//fixpt().init(2, !m_bSweepNotPursuit);
		fixpt().init(2);
	}

	if (has_dot())
	{
		for (unsigned int i = 0; i < dot_count(); i++)
		{
			// How many levels will this dot need? 
			// Maybe not the best method, but assume that the number needed 
			// for the first trial is the number needed for all trials. 
			if (dot(i).isMulti())
				dot(i).init((int)dot(i).getMulti().size(), false);
			else
				dot(i).init(1, false);
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
				rectangle(i).init((int)rectangle(i).getMulti().size(), false);
			else
				rectangle(i).init(1, false);
		}
	}


	// Set initial params in grating(s)
	set_initial_parameters();
	draw_current();	// setup_cycling is called from draw_current if its needed
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, vsgNOCLEAR);
	vsgSetDisplayPage(0);
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	return status;
}

void MultiParameterFXMultiGStimSet::cleanup(std::vector<int> pages)
{
	cout << "MultiParameterFXMultiGStimSet::cleanup()" << endl;
	if (m_bUseCycling)
	{
		cout << "vsgOVERLAYDISABLE" << endl;
		vsgSetCommand(vsgOVERLAYDISABLE);
	}
}

void print_cycle(const std::string& smsg, DWORD n, VSGCYCLEPAGEENTRY *pages)
{
	cerr << smsg << endl;
	for (DWORD i = 0; i < n; i++)
	{
		VSGCYCLEPAGEENTRY* singlePage = pages + i;
		string sTrig, sDual;
		sTrig = (singlePage->Page & vsgTRIGGERPAGE) ? "TRIG" : "----";
		sDual = (singlePage->Page & vsgDUALPAGE) ? "DUAL" : "----";
		DWORD pageNumber = singlePage->Page & !(vsgTRIGGERPAGE | vsgDUALPAGE);
		cerr << setw(2) << pageNumber << "(" << setw(1) << singlePage->Frames << "/" << setw(1) << singlePage->Stop << ") " << setw(4) << singlePage->Xpos << " " << setw(4) << singlePage->Ypos << " " << sTrig << sDual << ": " << setw(2) << singlePage->ovPage << " " << setw(4) << singlePage->ovXpos << " " << setw(4) << singlePage->ovYpos << endl;
	}
	cerr << "=====================" << endl;

}


int MultiParameterFXMultiGStimSet::handle_trigger(const std::string& s, const std::string& args)
{
	int status = 0;
	bool bCyclingThisTrial = (CYCLING_TYPE_NONE != m_iCyclingType);
	bool bSweepCyclingThisTrial = (CYCLING_TYPE_PURSUIT == m_iCyclingType && m_bUseSweepCycling);
	if (s == "F")
	{
		// In all cases, do these things. 

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

		if (!bSweepCyclingThisTrial)
		{
			cerr << "F: no sweep cycling this trial" << endl;

			// fixpt should always be at 100 contrast. Cannot support "f" when "S" is on if dots are present. 
			if (has_fixpt())
			{
				fixpt().setContrast(100);
				status = 1;
			}

			vsgSetDrawPage(vsgVIDEOPAGE, m_fixpt_page, vsgNOCLEAR);
		}
		else
		{
			//print_cycle("F: sweep cycling TRUE", m_fixpt_cycle_count, m_cycle_params + m_fixpt_cycle_start);
			vsgPageCyclingSetup(m_fixpt_cycle_count, m_cycle_params + m_fixpt_cycle_start);
			vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
			status = 1;
		}
		
	}
	else if (s == "f")
	{
		// this will not work when using sweep cycling?
		if (bSweepCyclingThisTrial)
			cerr << "WARNING! Cannot use small \"f\" trigger on sweep trials." << endl;
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
			//print_cycle("S: CYCLING_TYPE_NONE != m_iCyclingType", m_stim_cycle_count, m_cycle_params + m_stim_cycle_start);
			vsgPageCyclingSetup(m_stim_cycle_count, m_cycle_params + m_stim_cycle_start);
			vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		}
		else
		{
			//cerr << "S: : No cycling this trial" << endl;
			vsgSetDrawPage(vsgVIDEOPAGE, m_stim_page, vsgNOCLEAR);
		}
		status = 1;
	}
	else if (s == "s" || s == "u")
	{
		status = 1;
		if (CYCLING_TYPE_NONE != m_iCyclingType)
		{
			//cerr << "s: disable cycling" << endl;
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
		//cerr << "a: START" << endl;
		setCyclingDelay(-1);
		setStimDuration(-1);
		setPursuitParameters(-1, 0, 0);
		advance();
		draw_current();
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
		if (bSweepCyclingThisTrial)
		{
			vsgSetCommand(vsgCYCLEPAGEDISABLE);
			vsgPageCyclingSetup(m_clear_cycle_count, m_cycle_params + m_clear_cycle_start);
			vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
			status = 2;
		}
		else if (bCyclingThisTrial)
		{
			vsgSetCommand(vsgCYCLEPAGEDISABLE);
			vsgSetDrawPage(vsgVIDEOPAGE, m_blank_page, vsgNOCLEAR);
			status = 2;
		}
		else
			vsgSetDrawPage(vsgVIDEOPAGE, m_blank_page, vsgNOCLEAR);
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
// will leave draw page same when done, though that page might be altered
void MultiParameterFXMultiGStimSet::draw_current()
{
	bool bSweepCyclingThisTrial = (CYCLING_TYPE_PURSUIT == m_iCyclingType && m_bUseSweepCycling);

	// get current display page - we make sure to set the draw page to that when leaving
	
	long dpVideo, dpOverlay;
	dpVideo = vsgGetZoneDisplayPage(vsgVIDEOPAGE);
	dpOverlay = vsgGetZoneDisplayPage(vsgOVERLAYPAGE);

	// When Pursuit is used (or anything that drifts the video screen), make sure to move the 
	// screen back to its home before drawing. If cycling was active and page left mid-drift, 
	// this would be needed to fix. Should have no effect otherwise.
	vsgMoveScreen(0, 0);

	// Draw pages. When doing sweep cycling (only on those trials), we will put the fixpt on an overlay page.
	m_bResetPhaseOnTrigger = true;
	draw_stuff_on_page(m_blank_page, false, false, false, false, false);
	draw_stuff_on_page(m_fixpt_page, !bSweepCyclingThisTrial, true, false, false, false);
	draw_stuff_on_page(m_fixpt_dot_page, !bSweepCyclingThisTrial, true, false, true, false);
	draw_stuff_on_page(m_stim_page, !bSweepCyclingThisTrial, true, true, true, true, 1);
	if (m_num_stim_pages > 1)
	{
		if (!m_bUseDrawGroups)
		{
			advance();
			draw_stuff_on_page(m_alt_page, !bSweepCyclingThisTrial, true, true, true, true);
		}
		else 
		{
			draw_stuff_on_page(m_alt_page, !bSweepCyclingThisTrial, true, true, true, true, 2);
		}
	}
	m_pageflipindex = 0;	// in case this is used, reset so flipping works right the first time

	if (m_iCyclingType != CYCLING_TYPE_NONE)
		setup_cycling();


	// We will use the overlay pages ONLY when we are doing a sweep trial. By doing this we can
	// keep the fixation point stationary while the stimulus moves. 
	// When doing pursuit, we will not need overlay, but we still need cycling. 
	// overlay page 0 - clear (transparent)
	// overlay page 1 - fixation point
	if (bSweepCyclingThisTrial)
	{
		vsgSetDrawPage(vsgOVERLAYPAGE, 1, 0);
		if (has_fixpt())
		{
			// Note, arbitrarily taking level 2 on overlay. 
			fixpt().drawOverlay(2);
		}
		vsgSetDrawPage(vsgOVERLAYPAGE, 0, 0);
		// not what we want? BevsgSetDisplayPage(0);	// displays overlay page 0, which should be empty.
	}
	// restore original draw page
	vsgSetDrawPage(vsgVIDEOPAGE, dpVideo, vsgNOCLEAR);
}


void MultiParameterFXMultiGStimSet::draw_stuff_on_page(int pagenumber, bool bFixpt, bool bDistractor, bool bGrating, bool bDots, bool bRectangle, int iDrawGroup)
{
	// xhair, fixpt, dots and stim. 
	vsgSetDrawPage(vsgVIDEOPAGE, pagenumber, vsgBACKGROUND);
	if (bFixpt && has_xhair())
	{
		xhair().draw();
	}

	if (!hasImageOverride())
	{
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
	}
	else
	{
		if (bGrating && has_grating())
		{
			getImageOverride().draw();
		}
	}

	if (bFixpt && has_fixpt())
	{
		fixpt().draw();
	}

	if (!hasImageOverride())
	{
		if (bDots && has_dot())
		{
			for (unsigned int i = 0; i < dot_count(); i++)
			{
				dot(i).draw();
			}
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

// Get bounding box for panning a window (W,H), in 'direction' for a 'distance' 
// (same units as W, H). Use positive-Y-down. Initial window assumed to be at (0,0).

void getPanningBBox(double W, double H, double distance, double ori, double (&ullr)[4])
{
	// use screen-origin in upper left, positive down. 
	// Get result upper-left corner, which is _minimum_, and 
	// lower-right corner, which is _maximum
	double ulx = 0;
	double uly = 0;
	double lrx = W;
	double lry = H;
	double xp, yp;
	double xy[4][2] = { {0,0},
						{W,0},
						{W,H},
						{0,H} };

	// get delta x, delta y
	MyRot mr(ori);
	mr.rotatePoint(distance, 0, xp, yp);

	// go around the rectangle, start at upper-right corner, go clockwise. 
	// at each point, add xp,yp, and check against the extremes

	for (int i = 0; i < 4; i++)
	{
		double xx = xy[i][0] + xp;
		double yy = xy[i][1] + yp;
		if (xx < ulx) ulx = xx;
		if (yy < uly) uly = yy;
		if (xx > lrx) lrx = xx;
		if (yy > lry) lry = yy;
	}

	// TODO: fix so we don't need this step
	ullr[0] = ulx;
	ullr[1] = uly;
	ullr[2] = lrx;
	ullr[3] = lry;

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

		// try out our new function!
		double ullr[4];
		double dorgx, dorgy;
		getPanningBBox(vsgGetScreenWidthPixels(), vsgGetScreenHeightPixels(), pixels, dirDegrees, ullr);
		dorgx = vsgGetScreenWidthPixels() / 2 - ullr[0];
		dorgy = vsgGetScreenHeightPixels() / 2 - ullr[1];
		//cout << "panning bbox " << ullr[0] << ", " << ullr[1] << " - " << ullr[2] << ", " << ullr[3] << endl;
		//cout << "draw origin at " << dorgx << ", " << dorgy << endl;
	}
	return;
}

bool MultiParameterFXMultiGStimSet::setUseCycling(bool bUseCycling, bool bSweepCycling)
{
	bool bvalue = false;
	if (!bUseCycling)
	{
		m_bUseCycling = false;
		m_bUseSweepCycling = false;	// input ignored here
		bvalue = true;
	}
	else
	{
		if (m_bUseCycling)
		{
			// error, already called once
			cerr << "ERROR: Can call setUseCycling(true) only once!" << endl;
		}
		else
		{
			cerr << "setUseCycling(" << boolalpha << bUseCycling << "," << boolalpha << bSweepCycling << ")" << endl;
			m_bUseCycling = bUseCycling;
			m_bUseSweepCycling = bSweepCycling;
			bvalue = true;
		}
	}
	return bvalue;
}







// As originally written, page cycling is only used when there is a cycling delay, 
// or when there is a stim duration. If neither of those was set, then there's no need for cycling, as
// the "S" trigger transitions to the fixpt+stim page, without need for precise timing or delay. 


void MultiParameterFXMultiGStimSet::setup_cycling()
{
	int status = 0;
	int count = 0;
	bool bSweepCyclingThisTrial = (CYCLING_TYPE_PURSUIT == m_iCyclingType && m_bUseSweepCycling);

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
			cycle[count].Xpos = 0;
			cycle[count].Ypos = 0;
			cycle[count].Stop = 0;
			count++;
		}

		cycle[count].Page = m_stim_page + vsgTRIGGERPAGE;
		cycle[count].Xpos = 0;
		cycle[count].Ypos = 0;
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
			cycle[count].Xpos = 0;
			cycle[count].Ypos = 0;
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
			cycle[count].Page = m_fixpt_dot_page + vsgTRIGGERPAGE + (bSweepCyclingThisTrial ? vsgDUALPAGE : 0);
			cycle[count].ovPage = 1;
			cycle[count].Stop = 0;
			cycle[count].Xpos = 0;
			cycle[count].Ypos = 0;
			cycle[count].ovXpos = 0;
			cycle[count].ovYpos = 0;
			count++;
		}
		for (int i = 0; i < m_iStimDuration; i++)
		{
			cycle[count].Frames = 1;
			cycle[count].Page = m_stim_page + (i == 0 ? vsgTRIGGERPAGE : 0) + (bSweepCyclingThisTrial ? vsgDUALPAGE : 0);	// trigger only onset of pursuit.
			cycle[count].ovPage = 1;
			cycle[count].Xpos = (short)(-1 * (i + 1) * m_dxPursuit);
			cycle[count].Ypos = (short)(-1 * (i + 1) * m_dyPursuit);
			cycle[count].ovXpos = 0;
			cycle[count].ovYpos = 0;
			cycle[count].Stop = 0;
			count++;
		}
		cycle[count].Frames = 1;
		cycle[count].Page = m_blank_page + vsgTRIGGERPAGE + vsgDUALPAGE;
		cycle[count].ovPage = 0;
		cycle[count].Xpos = 0;
		cycle[count].Ypos = 0;
		cycle[count].ovXpos = 0;
		cycle[count].ovYpos = 0;
		cycle[count].Stop = 1;
		count++;

		m_stim_cycle_count = count;

		// if sweep-not-pursuit, then must create trivial page cycle to get simultaneous page change
		if (bSweepCyclingThisTrial)
		{
			m_fixpt_cycle_start = m_stim_cycle_count;
			cycle = m_cycle_params + m_fixpt_cycle_start;
			cycle[0].Frames = 1;
			cycle[0].Page = m_fixpt_page + vsgDUALPAGE;
			cycle[0].ovPage = 1;
			cycle[0].Stop = 1;
			cycle[0].Xpos = 0;
			cycle[0].Ypos = 0;
			cycle[0].ovXpos = 0;
			cycle[0].ovYpos = 0;
			m_fixpt_cycle_count = 1;

			m_clear_cycle_start = m_fixpt_cycle_start + m_fixpt_cycle_count;
			cycle = m_cycle_params + m_clear_cycle_start;
			cycle[0].Frames = 1;
			cycle[0].Page = m_blank_page + vsgDUALPAGE;
			cycle[0].ovPage = 0;
			cycle[0].ovXpos = cycle[0].ovYpos = 0;
			cycle[0].Xpos = cycle[0].Ypos = 0;
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
	cerr << "cycling? " << std::boolalpha << m_bUseCycling << " sweep? " << std::boolalpha << m_bUseSweepCycling << endl;
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


