#include "ConteUStim.h"
#include "ConteMisc.h"
#include "alert-triggers.h"
#include "alert-argp.h"
#include "ARtypes.h"
#include <memory>
#include <boost/algorithm/string.hpp>

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
	{"show-cue-rects", 703, 0, 0, "Show cue rects starting with cue period."},
	{"generate-dots", 704, "NPts/Patch", 0, "If no dot supply file provided, will generate a set of dots with this many dots per patch. Trials are 0,25,50,75,100% of first color, from trial to trial."}, 
	{"show-aperture", 705, 0, 0, "Show cue aperture outline - for testing only."},
	{"border", 706, "COLOR,outer_pixels,line_pixels", 0, "Display a border on all screens for F,S."},
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

	// draw first trial's cues, sample&target stim. 
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
	sState = "IDLE";	// DO NOT TOUCH OUTSIDE OF callback()!!!
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

		if (tf.output_trigger() != last_output_trigger)
		{
			vsgIOWriteDigitalOut(tf.output_trigger() << 1, 0xfffe);
			last_output_trigger = tf.output_trigger();
		}
		
		if (tf.present())
		{	
			vsgPresent();
		}

		//if (tf.fired())
		//{
		//	cout << "trig " << tf.triggers_matched() << " fired; VIDEO/OV pages " << vsgGetZoneDisplayPage(vsgVIDEOPAGE) << "/" << vsgGetZoneDisplayPage(vsgOVERLAYPAGE) << endl;
		//}

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

	// overlay level for fixation point and cue rectangles, if used. The draw commands will assign 
	// the color to the palette. 
	m_levelOverlayFixpt = 2;
	m_levelOverlayCueRect0 = 3;
	m_levelOverlayCueRect1 = 4;
	m_levelOverlayAperture = 5;
	m_levelOverlayBorder = 6;

	// make a color for testing stuff
	ARvsg::instance().request_single(m_levelTest);	// waste a level - testing this DJS. 
	ARvsg::instance().request_single(m_levelTest);
	cerr << "m_level test " << m_levelTest << endl;
	arutil_color_to_palette(COLOR_TYPE(1, 1, 0), m_levelTest);

	// these are the colors specified on command line
	for (auto c : m_arguments.colors)
	{
		PIXEL_LEVEL l;
		ARvsg::instance().request_single(l);
		arutil_color_to_palette(c, l);
		m_levelCueColors.push_back(l);
	}
	// background page
	vsgSetDrawPage(vsgVIDEOPAGE, cPageFixpt, vsgBACKGROUND);
	vsgSetDrawPage(vsgVIDEOPAGE, cPageBackground, vsgBACKGROUND);

	m_sample0.init(60, false);
	m_sample1.init(60, false);
	m_target0.init(60, false);
	m_target1.init(60, false);

	if (m_arguments.bShowBorder)
	{
		m_rectBorder.init(1, false);
		//ARvsg::instance().request_single(m_levelBorderLine);
		arutil_color_to_palette(m_arguments.borderColor, m_rectBorder.getFirstLevel());
		m_rectBorder.drawmode = vsgSOLIDPEN + vsgCENTREXY;
		m_rectBorder.color = m_arguments.borderColor;
		m_rectBorder.linewidth = m_arguments.iBorderLineWidth;
		m_rectBorder.x = m_arguments.iBorderOuterWidth / 2;
		m_rectBorder.y = 0;
		m_rectBorder.w = vsgGetScreenWidthPixels() - m_arguments.iBorderOuterWidth;
		m_rectBorder.h = vsgGetScreenHeightPixels();
	}

	// only need to do this once
	//setup_cycling_clear_fixpt();
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
	triggers().addTrigger(new FunctorCallbackTrigger("S", 0x4, 0x4, 0x4, 0x4, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("U", 0x20, 0x20, 0x10, 0x10, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("X", 0x6, 0x0, 0x6, 0x0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("a", 0x8, 0x8|AR_TRIGGER_TOGGLE, 0x8, 0x8|AR_TRIGGER_TOGGLE, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("Y", 0x0, 0x0, 0x0, 0x0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("Z", 0x0, 0x0, 0x0, 0x0, pfunctor));
	triggers().addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));

	return;
}

void ConteUStim::draw_dot_patches(const ConteXYHelper& xyhelper, const conte_trial_t& trial)
{
	// clear all pages needed
	for (WORD p=0; p<xyhelper.getNumPages(); p++)
		vsgSetDrawPage(vsgVIDEOPAGE, cPageCue + p, vsgBACKGROUND);

	// now draw each patch
	for (unsigned int i = 0; i < trial.cue_nterms; i++)
	{
		DWORD page_ind;
		const ContePatch& patch = m_arguments.dot_supply.next_patch();

		double term_x, term_y;
		xyhelper.getPageIndDrawOrigin(i, page_ind, term_x, term_y);
		vsgSetDrawPage(vsgVIDEOPAGE, cPageCue + page_ind, vsgNOCLEAR);
		vsgSetDrawOrigin(term_x, term_y);
		//cout << "patch " << i << " " << term_x << ", " << term_y << endl;

		patch.draw(m_levelCueColors[0], m_levelCueColors[1], trial.cue_w, trial.cue_h, trial.cue_d);
		if (m_arguments.bShowAperture)
		{
			long mode_saved = vsgGetDrawMode();
			vsgSetDrawMode(vsgCENTREXY | vsgPIXELPEN);
			vsgSetPen1(m_levelTest);
			vsgDrawRect(0, 0, trial.cue_w, trial.cue_h);
			vsgSetDrawMode(mode_saved);
		}
	}

	// restore draw origin to center of screen
	vsgSetSpatialUnits(vsgPIXELUNIT);
	vsgSetDrawOrigin(vsgGetScreenWidthPixels() / 2, vsgGetScreenHeightPixels() / 2);
	vsgSetSpatialUnits(vsgDEGREEUNIT);
	return;
}

void ConteUStim::copy_stim_params_to_spec(const struct conte_stim_params& params, ARConteSpec& spec)
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
	spec.cueLineWidth = params.lwt;
	spec.cueColor = m_arguments.colors[params.icolor];
	spec.gaborContrast = params.dGaborContrast;
	spec.flankerContrast = params.dFlankerContrast;
	spec.cueContrast = params.dCueContrast;
	return;
}

void ConteUStim::draw_current()
{
	const conte_trial_t& trial = m_arguments.trials.at(m_itrial);

	cout << "draw_current " << m_itrial << ": " << trial << endl;

	// Update vsg objs for stimuli
	copy_stim_params_to_spec(trial.s0, m_sample0);
	copy_stim_params_to_spec(trial.s1, m_sample1);
	copy_stim_params_to_spec(trial.t0, m_target0);
	copy_stim_params_to_spec(trial.t1, m_target1);

	// On initialization, overlay page 'cOvPageAperture' 
	// overlay 1 is all background, but has an aperture (level 0 in overlay palette)
	// for the cue patch. If border is to be shown, also draw it on this overlay page. 
	vsgSetCommand(vsgOVERLAYMASKMODE);
	vsgSetDrawPage(vsgOVERLAYPAGE, cOvPageAperture, m_levelOverlayBackground);
	vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
	vsgSetPen1(0);
	vsgDrawRect(trial.cue_x, -trial.cue_y, trial.cue_w, trial.cue_h);
	if (m_arguments.bShowAperture)
	{
		ARRectangleSpec r;
		r.x = trial.cue_x;
		r.y = trial.cue_y;
		r.w = trial.cue_w;
		r.h = trial.cue_h;
		r.color = COLOR_TYPE(.75, .15, .9);
		r.drawmode = vsgSOLIDPEN + vsgCENTREXY;
		r.linewidth = 2.0;
		r.drawOverlay(m_levelOverlayAperture);
	}

	// Draw cue rectangles
	if (m_arguments.bShowCueRects)
	{
		m_sample0.drawOverlay(m_levelOverlayCueRect0);
		m_sample1.drawOverlay(m_levelOverlayCueRect1);
	}

	// Draw border rectangle
	//if (m_arguments.bShowBorder)
	//{
	//	vsgSetSpatialUnits(vsgPIXELUNIT);
	//	cout << "Draw border rect " << m_rectBorder << endl;
	//	m_rectBorder.drawOverlay(m_levelOverlayBorder);
	//	vsgSetSpatialUnits(vsgDEGREEUNIT);
	//}

	// Just clear the clear page and draw border. It will eventually get a fixpt. 
	vsgSetDrawPage(vsgOVERLAYPAGE, cOvPageClear, 0);
	//if (m_arguments.bShowBorder)
	//{
	//	vsgSetSpatialUnits(vsgPIXELUNIT);
	//	cout << "Draw border rect " << m_rectBorder << endl;
	//	m_rectBorder.drawOverlay(m_levelOverlayBorder);
	//	vsgSetSpatialUnits(vsgDEGREEUNIT);
	//}

	// draw dot patch(es)
	vsgSetDrawPage(vsgVIDEOPAGE, cPageCue, vsgBACKGROUND);
	ConteXYHelper xyhelper(trial.cue_w, trial.cue_h, trial.cue_d, trial.cue_x, trial.cue_y, trial.cue_nterms);
	draw_dot_patches(xyhelper, trial);

	// draw stim pages
	vsgSetDrawPage(vsgVIDEOPAGE, cPageSample, vsgBACKGROUND);
	m_sample0.draw();
	m_sample1.draw();
	//if (m_arguments.bShowBorder)
	//{
	//	vsgSetSpatialUnits(vsgPIXELUNIT);
	//	m_rectBorder.draw();
	//	vsgSetSpatialUnits(vsgDEGREEUNIT);
	//}
	vsgSetDrawPage(vsgVIDEOPAGE, cPageTarget, vsgBACKGROUND);
	m_target0.draw();
	m_target1.draw();
	//if (m_arguments.bShowBorder)
	//{
	//	vsgSetSpatialUnits(vsgPIXELUNIT);
	//	m_rectBorder.draw();
	//	vsgSetSpatialUnits(vsgDEGREEUNIT);
	//}

	// setup cycling
	setup_cycling(xyhelper, trial);
}


void ConteUStim::setup_cycling(const ConteXYHelper& xyhelper, const conte_trial_t& trial)
{
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
		if (trial.cue_to_sample_delay_ms < 0)
		{
			bContinue = false;
		}
		else if (trial.cue_to_sample_delay_ms > 0)
		{
			// cue-to-sample delay
			m_cycle_params[m_cycle_params_count].Stop = 0;
			m_cycle_params[m_cycle_params_count].Page = cPageFixpt + vsgDUALPAGE + vsgTRIGGERPAGE;
			m_cycle_params[m_cycle_params_count].Xpos = m_cycle_params[m_cycle_params_count].Ypos = 0;
			m_cycle_params[m_cycle_params_count].ovPage = cOvPageAperture; //cOvPageClear;
			m_cycle_params[m_cycle_params_count].ovXpos = m_cycle_params[m_cycle_params_count].ovYpos = 0;
			m_cycle_params[m_cycle_params_count].Frames = nframes(trial.cue_to_sample_delay_ms);
			m_cycle_params_count++;
		}
	}

	if (bContinue)
	{
		if (trial.sample_display_ms < 0)
		{
			bContinue = false;
		}
		else if (trial.sample_display_ms > 0)
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
		if (trial.sample_to_target_delay_ms < 0)
		{
			bContinue = false;
		}
		else if (trial.sample_to_target_delay_ms > 0)
		{
			// sample to target delay
			m_cycle_params[m_cycle_params_count].Stop = 0;
			m_cycle_params[m_cycle_params_count].Page = cPageFixpt + vsgDUALPAGE + vsgTRIGGERPAGE;
			m_cycle_params[m_cycle_params_count].Xpos = m_cycle_params[m_cycle_params_count].Ypos = 0;
			m_cycle_params[m_cycle_params_count].ovPage = cOvPageClear; // cOvPageClear;
			m_cycle_params[m_cycle_params_count].ovXpos = m_cycle_params[m_cycle_params_count].ovYpos = 0;
			m_cycle_params[m_cycle_params_count].Frames = nframes(trial.sample_to_target_delay_ms);
			m_cycle_params_count++;
		}
	}

	if (bContinue)
	{
		if (trial.target_display_ms < 0)
		{
			bContinue = false;
		}
		else if (trial.target_display_ms > 0)
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
		if (trial.saccade_response_time_ms < 0)
		{
			bContinue = false;
		}
		else if (trial.saccade_response_time_ms > 0)
		{
			// response time
			m_cycle_params[m_cycle_params_count].Stop = 0;
			m_cycle_params[m_cycle_params_count].Page = cPageFixpt + vsgDUALPAGE + vsgTRIGGERPAGE;
			m_cycle_params[m_cycle_params_count].Xpos = m_cycle_params[m_cycle_params_count].Ypos = 0;
			m_cycle_params[m_cycle_params_count].ovPage = cOvPageClear; // cOvPageClear;
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


void print_draw_pages()
{
	VSGPAGEDESCRIPTOR pageDescr;
	pageDescr._StructSize = sizeof(VSGPAGEDESCRIPTOR);

}

void ConteUStim::drawBorderOnPages()
{
	if (m_arguments.bShowBorder)
	{
		vsgSetSpatialUnits(vsgPIXELUNIT);
		vsgSetDrawPage(vsgOVERLAYPAGE, cOvPageClear, vsgNOCLEAR);
		m_rectBorder.drawOverlay(m_levelOverlayBorder);
		vsgSetDrawPage(vsgOVERLAYPAGE, cOvPageAperture, vsgNOCLEAR);
		m_rectBorder.drawOverlay(m_levelOverlayBorder);
		vsgSetSpatialUnits(vsgDEGREEUNIT);
	}
}


void ConteUStim::pagesToIdleState()
{
	const conte_trial_t& trial = m_arguments.trials.at(m_itrial);
	vsgSetDrawPage(vsgOVERLAYPAGE, cOvPageClear, 0);
	vsgSetDrawPage(vsgOVERLAYPAGE, cOvPageAperture, m_levelOverlayBackground);
	vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
	vsgSetPen1(0);	// clear on overlay page
	cout << "Draw aperture at " << trial.cue_x << ", " << -trial.cue_y << endl;
	vsgDrawRect(trial.cue_x, -trial.cue_y, trial.cue_w, trial.cue_h);
	if (m_arguments.bShowCueRects)
	{
		m_sample0.drawOverlay(m_levelOverlayCueRect0);
		m_sample1.drawOverlay(m_levelOverlayCueRect1);
	}
}

// State transitions happen with the callbacks, but each callback letter has its own logic, because 
// not all states can have all letter/transitions.
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
	else if (key == "U")
	{
		if (sState == "IDLE")
		{
			drawBorderOnPages();
		}
		else
		{
			cerr << "ConteUStim::callback - U can only come when screen is blank" << endl;
		}

		ival = 0;
	}
	else if (key == "S")
	{
		if (sState == "IDLE" || sState == "FIXPT")
		{
			//vsgSetCommand(vsgOVERLAYMASKMODE + vsgOVERLAYDRIFT + vsgVIDEODRIFT);
			vsgPageCyclingSetup(m_cycle_params_count, &m_cycle_params[0]);
			vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
			sState = "STIM";
		}
		else
		{
			cerr << "ERROR: State \"" << sState << "\" does not allow S trigger" << endl;
		}
	}
	else if (key == "F")
	{
		if (sState == "IDLE")
		{
			vsgSetDrawPage(vsgOVERLAYPAGE, cOvPageClear, vsgNOCLEAR);
			if (m_arguments.bHaveFixpt)
				m_arguments.fixpt.drawOverlay(m_levelOverlayFixpt);
			vsgSetDrawPage(vsgOVERLAYPAGE, cOvPageAperture, vsgNOCLEAR);
			if (m_arguments.bHaveFixpt)
				m_arguments.fixpt.drawOverlay(m_levelOverlayFixpt);
			vsgSetZoneDisplayPage(vsgOVERLAYPAGE, cOvPageClear);
			sState = "FIXPT";
			ival = 0;
		}
		else
		{
			cerr << "ERROR: State \"" << sState << "\" does not allow F trigger" << endl;
		}
	}
	else if (key == "f")
	{
		cerr << "ConteUStim::callback(\"f\") not implemented." << endl;
	}
	else if (key == "X")
	{
		if (sState == "FIXPT")
		{
			pagesToIdleState();
			vsgSetZoneDisplayPage(vsgVIDEOPAGE, cPageBackground);
			sState = "IDLE";
		}
		else if (sState == "STIM")
		{
			vsgSetCommand(vsgCYCLEPAGEDISABLE);
			pagesToIdleState();
			vsgSetZoneDisplayPage(vsgVIDEOPAGE, cPageBackground);
			vsgSetZoneDisplayPage(vsgOVERLAYPAGE, cOvPageClear);
			sState = "IDLE";
		}
		ival = 0;
	}
	else if (key == "Z")
	{
		int i, j;
		cout << "video " << vsgGetZoneDisplayPage(vsgVIDEOPAGE) << " ov " << vsgGetZoneDisplayPage(vsgOVERLAYPAGE) << endl;
		cout << "Enter vid ov pages e.g. 1 1: ";
		cin >> i >> j;
		cout << "got " << i << " " << j << endl;
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, i);
		vsgSetZoneDisplayPage(vsgOVERLAYPAGE, j);
		ival = 0;
	}
	else if (key == "Y")
	{
		cout << "current video page " << vsgGetZoneDisplayPage(vsgVIDEOPAGE) << endl;
		cout << "current overlay page " << vsgGetZoneDisplayPage(vsgOVERLAYPAGE) << endl;
		cout << "page cycling state " << vsgGetSystemAttribute(vsgPAGECYCLINGSTATE) << endl;
		//drawBorderOnPages();
		ival = 0;
	}

	return ival;
}

