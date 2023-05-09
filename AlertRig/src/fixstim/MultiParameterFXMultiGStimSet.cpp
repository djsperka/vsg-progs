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

	cerr << "init: num_stim_pages " << m_num_stim_pages << " drawGroups? " << m_bUseDrawGroups << endl;

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
	if (has_fixpt())
	{
		fixpt().init(vsg, 2);
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

	// Set initial params in grating(s)
	set_initial_parameters();
	if (m_iCyclingType != CYCLING_TYPE_NONE)
	{
		if (m_iCyclingType == CYCLING_TYPE_PURSUIT)
		{
			vsgSetCommand(vsgVIDEODRIFT);
		}
		setup_cycling();
	}

	draw_current();
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	vsgPresent();

	return status;
}

int MultiParameterFXMultiGStimSet::handle_trigger(const std::string& s, const std::string& args)
{
	int status = 0;
	if (s == "F")
	{
		for (unsigned int i = 0; i < distractor_count(); i++)
		{
			distractor(i).select();
			vsgObjResetDriftPhase();
			distractor(i).setContrast(distractor_contrast(i));
			status = 1;
		}
		vsgSetDrawPage(vsgVIDEOPAGE, m_fixpt_page, vsgNOCLEAR);

		// move screen back to origin (if pursuit and trial was ended mid-pursuit)
		vsgMoveScreen(0, 0);

		// fixpt should always be at 100 contrast. Cannot support "f" when "S" is on if dots are present. 
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
			if (m_bResetPhaseOnTrigger) vsgObjResetDriftPhase();
			grating(i).setContrast(contrast(i));
		}
		m_bResetPhaseOnTrigger = false;
		if (CYCLING_TYPE_NONE != m_iCyclingType)
		{
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
			//vsgObjResetDriftPhase();
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
		// Note that after this trigger is completed, the current draw page
		// is the NEXT stimulus page to be shown. 


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
					draw_stuff_on_page(m_pageflippages[m_pageflipindex], true, true, true, true);
					status = 3;
					// this change makes the flip happen, and will pulse the FRAME channel. Return value 0!
					//vsgSetZoneDisplayPage(vsgVIDEOPAGE, m_pageflippages[m_pageflipindex] + vsgTRIGGERPAGE);
					//std::cerr << "ind " << m_pageflipindex << " page " << m_pageflippages[m_pageflipindex] << std::endl;
					//status = 0;
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
		if (CYCLING_TYPE_NONE != m_iCyclingType)
		{
			vsgSetCommand(vsgCYCLEPAGEDISABLE);
			status = 2;
		}
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
	draw_stuff_on_page(m_blank_page, false, false, false, false);
	draw_stuff_on_page(m_fixpt_page, true, true, false, false);
	draw_stuff_on_page(m_fixpt_dot_page, true, true, false, true);
	draw_stuff_on_page(m_stim_page, true, true, true, true, 1);
	if (m_num_stim_pages > 1)
	{
		if (!m_bUseDrawGroups)
		{
			advance();
			draw_stuff_on_page(m_alt_page, true, true, true, true);
		}
		else 
		{
			draw_stuff_on_page(m_alt_page, true, true, true, true, 2);
		}
	}
	m_pageflipindex = 0;	// in case this is used, reset so flipping works right the first time

}


void MultiParameterFXMultiGStimSet::draw_stuff_on_page(int pagenumber, bool bFixpt, bool bDistractor, bool bGrating, bool bDots, int iDrawGroup)
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
	VSGCYCLEPAGEENTRY cycle[32768];	// warning! No check on usage. You have been warned. 
	int status = 0;
	int count = 0;

	if (CYCLING_TYPE_NONE == m_iCyclingType)
		return;

	memset(cycle, 0, sizeof(cycle));


	if (CYCLING_TYPE_REGULAR == m_iCyclingType)
	{
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
	}
	else if (CYCLING_TYPE_PURSUIT == m_iCyclingType)
	{
		//cerr << "PURSUIT " << m_iCyclingDelay << " " << m_iStimDuration << " " << m_dxPursuit << " " << m_dyPursuit << endl;
		// If there is a delay -- i.e. delay before starting "stim" (pursuit)
		if (m_iCyclingDelay > 0)
		{
			cycle[count].Frames = 1 + (m_iCyclingDelay > 0 ? m_iCyclingDelay : 0);
			cycle[count].Page = m_fixpt_dot_page + vsgTRIGGERPAGE;
			cycle[count].Stop = 0;
			count++;
		}
		for (int i = 0; i < m_iStimDuration; i++)
		{
			cycle[count].Frames = 1;
			cycle[count].Page = m_stim_page + (i == 0 ? vsgTRIGGERPAGE : 0);	// trigger only onset of pursuit.
			cycle[count].Xpos = (short)(-1 * (i + 1) * m_dxPursuit);
			cycle[count].Ypos = (short)(-1 * (i + 1) * m_dyPursuit);
			cycle[count].Stop = 0;
			count++;
		}
		cycle[count].Frames = 1;
		cycle[count].Page = m_blank_page + vsgTRIGGERPAGE;
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
			cerr << " Grating " << i << "multi? " << std::boolalpha << grating(i).bIsMulti << "(" << grating(i).m_multi.size() <<") drawgroup 1/2/3: " << grating(i).inDrawGroup(1) << "/" << grating(i).inDrawGroup(2) << "/" << grating(i).inDrawGroup(3) << " : " << grating(i) << endl;
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


