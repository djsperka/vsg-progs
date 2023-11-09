#include "ConteUStim.h"
#include "ConteMisc.h"
#include "alert-triggers.h"
#include "alert-argp.h"
#include "ARtypes.h"
#include <memory>
#include <boost/algorithm/string.hpp>
#include <boost/range/algorithm/random_shuffle.hpp>

WORD ConteUStim::cOvPageClear = 0;
WORD ConteUStim::cOvPageAperture = 1;
WORD ConteUStim::cPageBackground = 0;
WORD ConteUStim::cPageFixpt = 1;
WORD ConteUStim::cPageSample = 2;
WORD ConteUStim::cPageTarget = 3;
WORD ConteUStim::cPageCue = 4;

// for argp
static struct argp_option options[] = {
	{"ascii", 'a', 0, 0, "Use ascii triggers (user-entered)"},
	{"verbose",  'v', 0, 0, "Produce verbose output" },
	{"background", 'b', "COLOR", 0, "background color"},
	{"distance-to-screen", 'd', "DIST_MM", 0, "screen distance in MM"},
	{"ready-pulse", 'p', "BITPATTERN", 0, "Ready pulse issued when startup is complete"},
	{"ready-pulse-delay", 'l', "DELAY_MS", 0, "Delay ready pulse for this many ms"},
	{"fixpt", 'f', "FIXPT_SPEC", 0, "fixation point"},
	{"dot-supply", 701, "filename", 0, "File with x,y positions in [-0.5,0.5]"},
	{"trials", 702, "filename", 0, "Conte trials stim specification"},
	{"color", 'c', "COLOR", 0, "cue color; specify in order of usage (0, then 1)"},
	{ 0 }
};

static struct argp f_argp = { options, parse_conte_opt, 0, "fixstim -- all-purpose stimulus engine" };


ConteUStim::ConteUStim()
: UStim()
, m_errflg(0)
{
};

ConteUStim::~ConteUStim()
{
}

bool ConteUStim::parse(int argc, char** argv)
{
	error_t ret = alert_argp_parse(&f_argp, argc, argv, ARGP_NO_EXIT, 0, &m_arguments, 'F');
	return (!ret);
}


void ConteUStim::run_stim(alert::ARvsg& vsg)
{
	cout << "ConteUStim::run_stim(): started" << endl;

	// clear all dig outputs
	vsgIOWriteDigitalOut(0, 0xff);
	vsgPresent();

	// initialize graphics
	init();

	// draw first trial
	m_itrial = 0;
	draw_current();

	// initialize triggers
	TSpecificFunctor<ConteUStim> functor(this, &ConteUStim::callback);
	init_triggers(&functor);

	// ready pulse
	vsg.ready_pulse(100, m_arguments.iPulseBits);

	// reset all triggers if using binary triggers
	if (m_arguments.bBinaryTriggers) triggers().reset(vsgIOReadDigitalIn());

	// All right, start monitoring triggers........
	int last_output_trigger=0;
	long input_trigger = 0;
	string s;

	while (!quit_enabled())
	{
		// If user-triggered, get a trigger entry. 
		if (!m_arguments.bBinaryTriggers)
		{
			// Get a new "trigger" from user
			cout << "Enter trigger/key: ";
			cin >> s;
		}
		else
		{
			input_trigger = vsgIOReadDigitalIn();
		}

		TriggerFunc	tf = std::for_each(triggers().begin(), triggers().end(), 
			(m_arguments.bBinaryTriggers ? TriggerFunc(input_trigger, last_output_trigger, false) : TriggerFunc(s, last_output_trigger)));

		// Now analyze input trigger
	 	
		if (tf.quit()) break;
		else if (tf.present())
		{	
			last_output_trigger = tf.output_trigger();
			//cout << "out trig " << hex << tf.output_trigger() << endl;
			//vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
			vsgIOWriteDigitalOut(tf.output_trigger() << 1, 0xffff);
			vsgPresent();
		}
		Sleep(10);
	}

	cleanup();

	vsg.clear();

	return ;
}

void ConteUStim::init()
{
	// background color and screen dist
	VSGTRIVAL c = m_arguments.bkgdColor.trival();
	ARvsg::instance().setViewDistMM(m_arguments.iDistanceToScreenMM);
	ARvsg::instance().setBackgroundColor(m_arguments.bkgdColor);

	// enable overlay
	vsgSetCommand(vsgOVERLAYMASKMODE + vsgVIDEODRIFT + vsgOVERLAYDRIFT);

	// background color needed on overlay palette
	m_levelOverlayBackground = 1;
	arutil_color_to_overlay_palette(m_arguments.bkgdColor, m_levelOverlayBackground);

	// make a color for testing stuff
	ARvsg::instance().request_single(m_levelTest);
	arutil_color_to_palette(COLOR_TYPE(0, 1, 1), m_levelTest);

	// Fixpt will be drawn on video, not overlay, so create obj.
	if (m_arguments.bHaveFixpt)
		m_arguments.fixpt.init(2);

	// these are the colors specified on command line
	for (auto c : m_arguments.colors)
	{
		PIXEL_LEVEL l;
		ARvsg::instance().request_single(l);
		arutil_color_to_palette(c, l);
		m_levelCueColors.push_back(l);
	}

	// overlay clear page - all transparent
	vsgSetDrawPage(vsgOVERLAYPAGE, cOvPageClear, 0);


	// background page
	vsgSetDrawPage(vsgVIDEOPAGE, cPageBackground, vsgBACKGROUND);

	m_sample0.init(40, false);
	m_sample1.init(40, false);
	m_target0.init(40, false);
	m_target1.init(40, false);

	// only need to do this once
	setup_cycling_clear_fixpt();
}

void ConteUStim::cleanup()
{
	vsgMoveScreen(0, 0);
	vsgSetCommand(vsgOVERLAYDISABLE);
}

void ConteUStim::init_triggers(TSpecificFunctor<ConteUStim>* pfunctor)
{
	triggers().clear();
	triggers().addTrigger(new FunctorCallbackTrigger("F", 0x2, 0x2, 0x2, 0x2, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("f", 0x2, 0x0, 0x2, 0x0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("S", 0x4, 0x4, 0x4, 0x4, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("s", 0x4, 0x0, 0x4, 0x0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("a", 0x8, 0x8|AR_TRIGGER_TOGGLE, 0x8, 0x8|AR_TRIGGER_TOGGLE, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("X", 0x6, 0x0, 0x6, 0x0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("Y", 0x0, 0x0, 0x0, 0x0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("Z", 0x0, 0x0, 0x0, 0x0, pfunctor));
	triggers().addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));

	return;
}

void ConteUStim::draw_dot_patches(const ConteXYHelper& xyhelper, unsigned int npatches)
{
	const conte_trial_t& trial = m_arguments.trials.at(m_itrial);
	
	// clear all pages needed
	for (WORD p=0; p<xyhelper.getNumPages(); p++)
		vsgSetDrawPage(vsgVIDEOPAGE, cPageCue + p, vsgBACKGROUND);

	// now draw each patch
	for (unsigned int i = 0; i < npatches; i++)
	{
		DWORD page_ind;
		const ContePatch& patch = m_arguments.dot_supply.patch(i);

		double term_x, term_y;
		xyhelper.getPageIndDrawOrigin(i, page_ind, term_x, term_y);
		vsgSetDrawPage(vsgVIDEOPAGE, cPageCue + page_ind, vsgNOCLEAR);
		vsgSetDrawOrigin(term_x, term_y);

		patch.draw(m_levelCueColors[0], m_levelCueColors[1], trial.cue_w, trial.cue_h, trial.cue_d);

		if (m_arguments.bHaveFixpt)
			m_arguments.fixpt.draw();
	}

	// restore draw origin to center of screen
	vsgSetSpatialUnits(vsgPIXELUNIT);
	vsgSetDrawOrigin(vsgGetScreenWidthPixels() / 2, vsgGetScreenHeightPixels() / 2);
	vsgSetSpatialUnits(vsgDEGREEUNIT);
	return;
}

void ConteUStim::copy_params_to_spec(const struct conte_stim_params& params, ARConteSpec& spec)
{
	spec.x = params.x;
	spec.y = params.y;
	spec.w = params.w;
	spec.h = params.h;
	spec.orientation = params.ori;
	spec.sf = params.sf;
	spec.divisor = params.divisor;
	spec.phase = params.phase;
	spec.iHorizontal = params.iHorizontal;
	spec.cue_line_width = params.lwt;
	spec.cue_level = m_levelCueColors[params.icolor];	// TODO - no check on size here!
	return;
}

void ConteUStim::draw_current()
{
	const conte_trial_t& trial = m_arguments.trials.at(m_itrial);

	// overlay 0 is initialized in init()

	// overlay 1 has aperture for patch and background
	vsgSetDrawPage(vsgOVERLAYPAGE, cOvPageAperture, m_levelOverlayBackground);
	vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
	vsgSetPen1(0);	// clear on overlay page
	vsgDrawRect(trial.cue_x, trial.cue_y, trial.cue_w, trial.cue_h);

	// fixation point page
	vsgSetDrawPage(vsgVIDEOPAGE, cPageFixpt, vsgBACKGROUND);
	if (m_arguments.bHaveFixpt)
		m_arguments.fixpt.draw();

	// draw dot patch(es)
	vsgSetDrawPage(vsgVIDEOPAGE, cPageCue, vsgBACKGROUND);
	ConteXYHelper xyhelper(trial.cue_w, trial.cue_h, trial.cue_d, trial.cue_x, trial.cue_y, trial.cue_nterms);
	draw_dot_patches(xyhelper, trial.cue_nterms);

	// draw stim pages
	vsgSetDrawPage(vsgVIDEOPAGE, cPageSample, vsgBACKGROUND);
	copy_params_to_spec(trial.s0, m_sample0);
	m_sample0.draw();
	copy_params_to_spec(trial.s1, m_sample1);
	m_sample1.draw();
	if (m_arguments.bHaveFixpt)
		m_arguments.fixpt.draw();
	vsgSetDrawPage(vsgVIDEOPAGE, cPageTarget, vsgBACKGROUND);
	copy_params_to_spec(trial.t0, m_target0);
	m_target0.draw();
	copy_params_to_spec(trial.t1, m_target1);
	m_target1.draw();
	if (m_arguments.bHaveFixpt)
		m_arguments.fixpt.draw();

	// leave draw pages at reasonable place for next present()
	vsgSetDrawPage(vsgVIDEOPAGE, cPageBackground, vsgNOCLEAR);
	vsgSetDrawPage(vsgOVERLAYPAGE, cOvPageClear, vsgNOCLEAR);

	// setup cycling
	setup_cycling(xyhelper, trial.cue_nterms);
}

void ConteUStim::setup_cycling_clear_fixpt()
{
	m_cycle_clear_params[0].Frames = 2;
	m_cycle_clear_params[0].Page = cPageBackground + vsgDUALPAGE;
	m_cycle_clear_params[0].Xpos = m_cycle_clear_params[0].Ypos = 0;
	m_cycle_clear_params[0].ovPage = cOvPageClear;
	m_cycle_clear_params[0].ovXpos = m_cycle_clear_params[0].ovYpos = 0;
	m_cycle_clear_params[0].Stop = 1;

	m_cycle_fixpt_params[0].Frames = 2;
	m_cycle_fixpt_params[0].Page = cPageFixpt + vsgDUALPAGE + vsgTRIGGERPAGE;
	m_cycle_fixpt_params[0].Xpos = m_cycle_fixpt_params[0].Ypos = 0;
	m_cycle_fixpt_params[0].ovPage = cOvPageClear;
	m_cycle_fixpt_params[0].ovXpos = m_cycle_fixpt_params[0].ovYpos = 0;
	m_cycle_fixpt_params[0].Stop = 1;

}

void ConteUStim::setup_cycling(const ConteXYHelper& xyhelper, unsigned int nterms_in_cue)
{
	const conte_trial_t& trial = m_arguments.trials.at(m_itrial);
	short Xpos, Ypos;
	DWORD page_ind;
	bool bContinue = true;	// used to stop presentation when one of the time periods <= 0

	long frametime = vsgGetSystemAttribute(vsgFRAMETIME);	// in microseconds
	auto nframes = [frametime](const unsigned int ms)
	{
		return (unsigned int)(1000.0 * ms / frametime);
	};

	// cue presentation
	m_cycle_params_count = 0;
	for (unsigned int i = 0; i < trial.cue_nterms; i++)
	{
		//xyhelper.getPageXYpos(m_cycle_params_count, Xpos, Ypos);
		xyhelper.getPageIndXYpos(m_cycle_params_count, page_ind, Xpos, Ypos);

		m_cycle_params[m_cycle_params_count].Frames = trial.cue_fpt;
		m_cycle_params[m_cycle_params_count].Stop = 0;
		m_cycle_params[m_cycle_params_count].Page = cPageCue + page_ind + vsgDUALPAGE + vsgTRIGGERPAGE;
		m_cycle_params[m_cycle_params_count].ovPage = cOvPageAperture;
		m_cycle_params[m_cycle_params_count].ovXpos = m_cycle_params[m_cycle_params_count].ovYpos = 0;
		m_cycle_params[m_cycle_params_count].Xpos = Xpos;
		m_cycle_params[m_cycle_params_count].Ypos = Ypos;
		m_cycle_params_count++;
	}

	if (bContinue)
	{
		if (trial.cue_to_sample_delay_ms <= 0)
		{
			bContinue = false;
		}
		else
		{
			// cue-to-sample delay
			m_cycle_params[m_cycle_params_count].Stop = 0;
			m_cycle_params[m_cycle_params_count].Page = cPageFixpt + vsgDUALPAGE + vsgTRIGGERPAGE;
			m_cycle_params[m_cycle_params_count].Xpos = m_cycle_params[m_cycle_params_count].Ypos = 0;
			m_cycle_params[m_cycle_params_count].ovPage = cOvPageClear;
			m_cycle_params[m_cycle_params_count].ovXpos = m_cycle_params[m_cycle_params_count].ovYpos = 0;
			m_cycle_params[m_cycle_params_count].Frames = nframes(trial.cue_to_sample_delay_ms);
			m_cycle_params_count++;
		}
	}

	if (bContinue)
	{
		if (trial.sample_display_ms <= 0)
		{
			bContinue = false;
		}
		else
		{
			// sample presentation
			m_cycle_params[m_cycle_params_count].Stop = 0;
			m_cycle_params[m_cycle_params_count].Page = cPageSample + vsgDUALPAGE + vsgTRIGGERPAGE;
			m_cycle_params[m_cycle_params_count].Xpos = m_cycle_params[m_cycle_params_count].Ypos = 0;
			m_cycle_params[m_cycle_params_count].ovPage = cOvPageClear;
			m_cycle_params[m_cycle_params_count].ovXpos = m_cycle_params[m_cycle_params_count].ovYpos = 0;
			m_cycle_params[m_cycle_params_count].Frames = nframes(trial.sample_display_ms);
			m_cycle_params_count++;
		}
	}

	if (bContinue)
	{
		if (trial.sample_to_target_delay_ms <= 0)
		{
			bContinue = false;
		}
		else
		{
			// sample to target delay
			m_cycle_params[m_cycle_params_count].Stop = 0;
			m_cycle_params[m_cycle_params_count].Page = cPageFixpt + vsgDUALPAGE + vsgTRIGGERPAGE;
			m_cycle_params[m_cycle_params_count].Xpos = m_cycle_params[m_cycle_params_count].Ypos = 0;
			m_cycle_params[m_cycle_params_count].ovPage = cOvPageClear;
			m_cycle_params[m_cycle_params_count].ovXpos = m_cycle_params[m_cycle_params_count].ovYpos = 0;
			m_cycle_params[m_cycle_params_count].Frames = nframes(trial.sample_to_target_delay_ms);
			m_cycle_params_count++;
		}
	}

	if (bContinue)
	{
		if (trial.target_display_ms <= 0)
		{
			bContinue = false;
		}
		else
		{
			// target presentation
			m_cycle_params[m_cycle_params_count].Stop = 0;
			m_cycle_params[m_cycle_params_count].Page = cPageTarget + vsgDUALPAGE + vsgTRIGGERPAGE;
			m_cycle_params[m_cycle_params_count].Xpos = m_cycle_params[m_cycle_params_count].Ypos = 0;
			m_cycle_params[m_cycle_params_count].ovPage = cOvPageClear;
			m_cycle_params[m_cycle_params_count].ovXpos = m_cycle_params[m_cycle_params_count].ovYpos = 0;
			m_cycle_params[m_cycle_params_count].Frames = nframes(trial.target_display_ms);
			m_cycle_params_count++;
		}
	}

	if (bContinue)
	{
		if (trial.saccade_response_time_ms <= 0)
		{
			bContinue = false;
		}
		else
		{
			// response time
			m_cycle_params[m_cycle_params_count].Stop = 0;
			m_cycle_params[m_cycle_params_count].Page = cPageFixpt + vsgDUALPAGE + vsgTRIGGERPAGE;
			m_cycle_params[m_cycle_params_count].Xpos = m_cycle_params[m_cycle_params_count].Ypos = 0;
			m_cycle_params[m_cycle_params_count].ovPage = cOvPageClear;
			m_cycle_params[m_cycle_params_count].ovXpos = m_cycle_params[m_cycle_params_count].ovYpos = 0;
			m_cycle_params[m_cycle_params_count].Frames = nframes(trial.saccade_response_time_ms);
			m_cycle_params_count++;
		}
	}

	// all done - clear
	m_cycle_params[m_cycle_params_count].Stop = 1;
	m_cycle_params[m_cycle_params_count].Page = cPageBackground + vsgDUALPAGE + vsgTRIGGERPAGE;
	m_cycle_params[m_cycle_params_count].Xpos = m_cycle_params[m_cycle_params_count].Ypos = 0;
	m_cycle_params[m_cycle_params_count].ovPage = cOvPageClear;
	m_cycle_params[m_cycle_params_count].ovXpos = m_cycle_params[m_cycle_params_count].ovYpos = 0;
	m_cycle_params[m_cycle_params_count].Frames = 1;
	m_cycle_params_count++;

}

int ConteUStim::callback(int &output, const FunctorCallbackTrigger* ptrig, const std::string&)
{
	int ival=1;
	string key = ptrig->getKey();

	if (key == "a")
	{
		m_itrial++;
		if (m_itrial >= m_arguments.trials.size()) m_itrial = 0;
		draw_current();
		ival = 0;	// no present()
	}
	else if (key == "s")
	{
		cerr << "ConteUStim::callback(\"s\") not implemented." << endl;
	}
	else if (key == "S")
	{
		//for (unsigned int i = 0; i < m_cycle_params_count; i++)
		//	cerr << i << ": " << m_cycle_params[i].Page-(m_cycle_params[i].Page & vsgDUALPAGE) << "/" << m_cycle_params[i].ovPage << " n=" << m_cycle_params[i].Frames << " s=" << m_cycle_params[i].Stop << endl;
		vsgPageCyclingSetup(m_cycle_params_count, &m_cycle_params[0]);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
	}
	else if (key == "F")
	{
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		//vsgMoveScreen(0, 0);
		vsgPageCyclingSetup(1, &m_cycle_fixpt_params[0]);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
	}
	else if (key == "f")
	{
		cerr << "ConteUStim::callback(\"f\") not implemented." << endl;
	}
	else if (key == "X")
	{
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		vsgPageCyclingSetup(1, &m_cycle_clear_params[0]);	
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
	}
	else if (key == "Z")
	{
		int i, j;
		cout << "video " << vsgGetZoneDisplayPage(vsgVIDEOPAGE) << " ov " << vsgGetZoneDisplayPage(vsgOVERLAYPAGE) << endl;
		cout << "Enter vid ov pages e.g. 1 1: ";
		cin >> i >> j;
		cout << "got " << i << " " << j << endl;
		//vsgSetDrawPage(vsgVIDEOPAGE, i, vsgNOCLEAR);
		//vsgSetDrawPage(vsgOVERLAYPAGE, j, vsgNOCLEAR);
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, i);
		vsgSetZoneDisplayPage(vsgOVERLAYPAGE, j);
		ival = 0;
	}
	else if (key == "Y")
	{
		cout << "current video page " << vsgGetZoneDisplayPage(vsgVIDEOPAGE) << endl;
		cout << "current overlay page " << vsgGetZoneDisplayPage(vsgOVERLAYPAGE) << endl;
		cout << "page cycling state " << vsgGetSystemAttribute(vsgPAGECYCLINGSTATE) << endl;
		ival = 0;
	}

	return ival;
}

ConteXYHelper::ConteXYHelper(double w, double h, double d, double x, double y, unsigned int npatches)
	: m_wdeg(w)
	, m_hdeg(h)
	, m_ddeg(d)
	, m_xdeg(x)
	, m_ydeg(y)
{
	m_WpixScr = vsgGetScreenWidthPixels();
	m_HpixScr = vsgGetScreenHeightPixels();
	m_WpixZone = vsgGetSystemAttribute(vsgVIDEOZONEWIDTH);
	m_HpixZone = vsgGetSystemAttribute(vsgVIDEOZONEHEIGHT);

	// Figure out how many patches can be drawn on a page..........
	double WdegZone, HdegZone;
	vsgUnit2Unit(vsgPIXELUNIT, m_WpixZone, vsgDEGREEUNIT, &WdegZone);
	vsgUnit2Unit(vsgPIXELUNIT, m_HpixZone, vsgDEGREEUNIT, &HdegZone);
	m_nPatchPerRow = trunc(WdegZone / (m_ddeg + m_wdeg));
	m_nRowsPerPage = trunc(HdegZone / (m_ddeg + m_wdeg));
	m_nPatchRows = ceil((double)npatches / (double)m_nPatchPerRow);
	m_nPatchPages = ceil((double)npatches / (double)(m_nRowsPerPage * m_nPatchPerRow));
	cerr << "ConteXYHelper: nPatchPerRow " << m_nPatchPerRow << " rows per page " << m_nRowsPerPage << endl;
	cerr << "num patches for cue: " << npatches << " pages needed: " << m_nPatchPages << endl;
}

void ConteXYHelper::getPageIndDrawOrigin(unsigned int i, DWORD& page_ind, double& x_origin_deg, double& y_origin_deg) const
{
	page_ind = (DWORD)(i / (m_nRowsPerPage * m_nPatchPerRow));
	unsigned int ipage = i % (m_nRowsPerPage * m_nPatchPerRow);
	x_origin_deg = (ipage % m_nPatchPerRow + 0.5) * (m_wdeg + m_ddeg);
	y_origin_deg = (ipage / m_nPatchPerRow + 0.5) * (m_wdeg + m_ddeg);
	return;
}

void ConteXYHelper::getPageIndXYpos(unsigned int i, DWORD& page_ind, short& Xpos_pix, short& Ypos_pix) const
{
	page_ind = (DWORD)(i / (m_nRowsPerPage * m_nPatchPerRow));
	unsigned int ipage = i % (m_nRowsPerPage * m_nPatchPerRow);

	double x_origin_degrees, y_origin_degrees;
	double x_origin_pixels, y_origin_pixels;

	// draw origin relative to upper left corner of page, with +y = down. 
	// convert to pixels
	DWORD dummy;
	getPageIndDrawOrigin(i, dummy, x_origin_degrees, y_origin_degrees);
	//getDrawOrigin(i, x_origin_degrees, y_origin_degrees);
	vsgUnit2Unit(vsgDEGREEUNIT, x_origin_degrees, vsgPIXELUNIT, &x_origin_pixels);
	vsgUnit2Unit(vsgDEGREEUNIT, y_origin_degrees, vsgPIXELUNIT, &y_origin_pixels);

	Xpos_pix = (short)(x_origin_pixels - m_WpixScr / 2);
	Ypos_pix = (short)(y_origin_pixels - m_HpixScr / 2);
	return;
};

ContePatch::ContePatch(unsigned int n0, unsigned int n1, double* p)
	: m_n0(n0)
	, m_n1(n1)
{
	for (unsigned int i = 0; i < (n0 + n1); i++)
	{
		m_x.push_back(p[2 * i]);
		m_y.push_back(p[2 * i + 1]);
	}
}

void ContePatch::draw(PIXEL_LEVEL level0, PIXEL_LEVEL level1, double patch_width, double patch_height, double dot_diam) const
{
	unsigned int i;
	vector < PIXEL_LEVEL> levels;
	vector <unsigned int> ind;
	for (i = 0; i < m_n0; i++)
		levels.push_back(level0);
	for (i = 0; i < m_n1; i++)
		levels.push_back(level1);
	// randomize the order
	for (i = 0; i < (m_n0 + m_n1); i++)
		ind.push_back(i);
	boost::range::random_shuffle(ind);

	// draw
	long mode_saved = vsgGetDrawMode();
	vsgSetDrawMode(vsgCENTREXY | vsgSOLIDFILL);
	for (i = 0; i < (m_n0 + m_n1); i++)
	{
		vsgSetPen1(levels[ind[i]]);
		vsgDrawOval(m_x[ind[i]] * patch_width, m_y[ind[i]] * patch_height, dot_diam, dot_diam);
	}
	vsgSetDrawMode(mode_saved);
}

