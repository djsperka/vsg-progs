/* $Id: FixUStim.cpp,v 1.11 2016-05-05 18:25:15 devel Exp $*/

#include "FixUStim.h"
#include "FXGStimParameterList.h"
#include "FXImageStimSet.h"
#include "SequencedAttentionStimSet.h"
#include "SequencedImagesAttentionStimSet.h"
#include "MelStimSet.h"
#include "EQStimSet.h"
#include "BorderStimSet.h"
#include "CycleTestStimSet.h"
#include <conio.h>
#include <iostream>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/exception/exception.hpp>
#include <boost/exception/get_error_info.hpp>
#include "alert-argp.h"
#include "alert-triggers.h"
#include "ARtypes.h"

using namespace std;
using namespace boost::algorithm;

const string FixUStim::m_allowedArgs("ab:c:d:e:f:g:h:i:j:k:l:m:n:o:q:p:r:t:s:vy:A:B:C:D:E:G:H:I:J:KL:M:NO:P:Q:R:S:T:U:V:W:X:Y:Z:");

// for argp
error_t parse_fixstim_opt(int key, char* carg, struct argp_state* state);
static struct argp_option options[] = {
	{"ascii", 'a', 0, 0, "Use ascii triggers (user-entered)"},
	{"verbose",  'v', 0, 0, "Produce verbose output" },
	{"background", 'b', "COLOR", 0, "background color"},
	{"distance-to-screen", 'd', "DIST_MM", 0, "screen distance in MM"},
	{"fixpt", 'f', "FIXPT_SPEC", 0, "fixation point"},
	{"grating", 'g', "GRATING", 0, "grating spec" },
	{"xhair", 'h', "XHAIR_SPEC", 0, "xhair spec"},
	{"images", 'i', "IMAGE_SPEC", 0, "image file list"},
	{"stimulus", 's', "STIMULUS_SPEC", 0, "stimulus spec"},
	{"distractor", 'k', "STIMULUS_SPEC", 0, "distractor spec, for those stimsets that use them"},
	{"thats-enough", 'K', 0, 0, "create stimulus using current fixpt, grating, distractors"},
	{"ready-pulse", 'p', "BITPATTERN", 0, "Ready pulse issued when startup is complete"},
	{"ready-pulse-delay", 'l', "DELAY_MS", 0, "Delay ready pulse for this many ms"},
	{"border-ownership", 'c', "BORDER_SPEC", 0, "Border ownership stimulus"},
	{"orientation", 'O', "ORIENTATION_LIST", 0, "Grating orientation, applies to most recent stim on command line"},
	{"drift-velocity", 'T', "DRIFT_VELOCITY_LIST", 0, "Drift velocity (deg/sec), applies to most recent stim on command line"},
	{"spatial-frequency", 'S', "SPATIAL_FREQUENCY_LIST", 0, "Spatial frequency (cycles/deg), applies to most recent stim on command line"},
	{"contrast", 'C', "CONTRAST_LIST", 0, "Contrast (0-100), applies to most recent stim on command line"},
	{"area", 'A', "DIAMETER_LIST", 0, "Grating diameter (deg), applies to most recent stim on command line"},
	{"position", 'Z', "XY_LIST", 0, "Grating position, params are x,y pairs, applies to most recent stim on command line"},
	{"x-position", 'X', "X_LIST", 0, "Grating x position, applies to most recent stim on command line"},
	{"inner-diameter", 'M', "DIAM_LIST", 0, "Donut inner diameter, applies to most recent stim on command line"},
	{"fixpt-position", 'y', "XY_LIST", 0, "Fixation point position, parames are x,y pairs, applies to most recent stim on command line"},
	{"stim-delay", 'E', "DELAY_MS_LIST", 0, "delay presentation of stimulus this long (ms) after \"S\" trigger"},
	{"stim-duration", 't', "DURATION_MS_LIST", 0, "stim presentation for this long (ms) before turning off"},
	{"initial-phase", 'P', "PHASE_LIST", 0, "Grating initial phase (deg), applies to most recent stim on command line"},
	{"counterphase-frequency", 'I', "FREQUENCY_LIST", 0, "Counterphase frequency, applies to most recent stim on command line"},
	{"fixpt-color", 'U', "COLOR_LIST", 0, "per - trial Color of fixation point"},
	{"contrast-reverse", 'R', "CRG_PARAMS", 0, "contrast-reversing grating, according to current sequence (-e) or msequence (default)"},
	{"contrast-reverse-balanced", 'B', "CRG_PARAMS", 0, "contrast-reversing grating, balanced, according to current sequence (-e) or msequence (default)"},
	{"sequence", 'e', "SEQUENCE_0101", 0, "sequence, as series of 0's and 1's, e.g. 0101010"},
	{"flash", 'L', "fpt,first,nterms[[color0,color1[,color2...]]]", 0, "full-field flash, by sequence. Sequence 0=color0, 1=color1, ..."},
	{"contrast-reverse-file", 'Y', "fpt,FILENAME[,n0,n1,...]", 0, "contrast reversing grating, sequences in file, one per line, all same length"},
	{"drifting-bar", 'G', "color,width,height,deg_per_sec,ori1,ori2,...", 0, "drifting bar with trigger pulses when box appears on (disappears from) screen"},
	{"drifting-dots", 777, "color,x,y,diam,speed,density,dotsize,angle1,angle2,...", 0, "drifting dots"},
	{"flashies", 'j', "nk,x,y,w,h,t_on,t_off[,nk,x,y,w,h,t_on,t_off...]", 0, "flashies - briefly flash distractors during attention trial"},
	{"cue-dots", 'q', "CUES_PARAMS", 0, "For Attention-style stimuli, use cue dots at stimulus locations."},
	{"cue-rects", 'r', "CUES_PARAMS", 0, "For Attention-style stimuli, use cue rects at stimulus locations."},
	{"original-attention", 'J', "ATTENTION_PARAMS", 0, "original attention stim set (ask for details)"},
	{"interleaved-attention", 'm', "INTERLEAVED_PARAMS", 0, "precursor to sequenced attention (-n)"},
	{"sequenced-attention", 'n', "SEQUENCED_PARAMS", 0, "sequenced attention stim set; allows precise timing of cues, onset/offset/contrast-change of gratings"},
	{"mel", 'o', "MEL_PARAMS", 0, "mel stim set, lost to time"},
	{"stacey-evan", 'W', "filename[,ip:port]", 0, "Stacey-Evan stimulus"},
	{"dot", 'D', "FIXPT_SPEC[/FIXPT_SPEC[...]]", 0, "dot(s) to (dis)appear with stim trigger"},
	{"pursuit", 778, "PURSUIT_SPEC", 0, "smooth pursuit"},
	{"sweep", 768, "PURSUIT_SPEC", 0, "sweep - fixpt stationary, stim moves"},
	{"serial", 779, "PORT", 0, "serial port to listen on for triggers"},
	{"no-gamma", 776, 0, 0, "Disable gamma correction for this stim. Will be re-enabled when this stim is complete."},
	{"cycle-test", 774, 0, 0, "Cycling test, trigger every frame"},
	{"wh", 773, "w0,h0[[,w1,h1],...]", 0, "Width,height tuning curve"},
	{"multi-ori", 772, "x0,y0,ori0[,x1,y1,ori1[...]]", 0, "Multi-grating pos with ori"},
	{"num-stim-pages", 771, "1(default)|2|3", 1, "Number of stimulus pages prepared for each trial."},
	{"colorvector", 770, "cv1,cv2,...", 0, "grating/distractor colorvector list"},
	{"draw-group", 769, "0|1|2", 0, "the next object specified should be in this drawing group. Group 1(2) drawn on first(second) stim page."},
	{"rect", 767, "RECTANGLE_SPEC[;RECTANGLE_SPEC[...]]", 0, "List of rect for a single trial. Separate trials with !."},
	{"grating-bars", 766, "x1,y1,w1,h1,ori1[,x2,w2,w2,h2,ori2[...]]", 0, "List of grating bar params, 5 for each trial. Comma-separated."},
	{"bmp-image-list", 765, "filename,x,y,dur,dly,nlevels", 0, "BMP images, compressed color index"},
	{"bmp-image-order", 764, ",,,0,,1,,,,,2....", 0, "order to display images from bmp-image-list file, 0-based index. * stops all grating etc display."},
	{ 0 }
};
static struct argp f_argp = { options, parse_fixstim_opt, 0, "fixstim -- all-purpose stimulus engine" };


// my parsers
bool parseImageArg(const std::string& arg, std::string& filename, double& x, double& y, double& duration, double& delay, int& nlevels);
int parse_multigrating(const std::string& sarg, vector<vector<std::tuple<double, double, double> > >& params);
int parse_dot_list(const std::string& sarg, vector<vector<alert::ARFixationPointSpec> >& dot_list);
int parse_rectangle_arg(const std::string& sarg, vector<vector<alert::ARRectangleSpec> >& rect_list);





FixUStim::FixUStim()
	: UStim()
	, m_arguments()
{
};

FixUStim::~FixUStim()
{
	if (m_arguments.pStimSet)
		delete m_arguments.pStimSet;
}

bool FixUStim::parse(int argc, char **argv)
{
	error_t ret = alert_argp_parse(&f_argp, argc, argv, ARGP_NO_EXIT, 0, &m_arguments, 'F');
	return (!ret);
}

void FixUStim::run_stim(alert::ARvsg& vsg)
{
	cout << "FixUStim::run_stim(): started" << endl;

	if (m_arguments.bDisableGammaCorrection)
	{
		cout << "WARNING: DISABLING GAMMA CORRECTION." << endl;
		vsgSetVideoMode(vsgNOGAMMACORRECT);
	}

	VSGTRIVAL c = m_arguments.bkgdColor.trival();
	vsg.setViewDistMM(m_arguments.iDistanceToScreenMM);
	vsg.setBackgroundColor(m_arguments.bkgdColor);

	// clear all dig outputs
	vsgIOWriteDigitalOut(0, 0xffff);
	vsgPresent();

	// initialize triggers
	TSpecificFunctor<FixUStim> functor(this, &FixUStim::callback);
	init_triggers(&functor, m_arguments.nStimPages);
	for (unsigned int i = 0; i < triggers().size(); i++)
	{
		std::cout << "FixUStim::run_stim(): Trigger " << i << " " << *(triggers().at(i)) << std::endl;
	}

	// If using serial, now is the time to open the serial port
	if (m_arguments.bUsingSerial)
	{
		cout << "Using serial port \"" << m_arguments.serial_port << "\"" << endl;
		try 
		{
			m_serial_port.open(m_arguments.serial_port, 115200);
		}
		catch (const std::runtime_error& e)
		{
			cout << "Error opening serial port: " << e.what() << endl;
			return;
		}
	}

	// Tell the stim set to initialize its pages. Note that the current display
	// page is page 0, and the stim set is given different pages (starting at 1 and depending on 
	// how many pages it requires)
	cout << "FixUStim::run_stim(): Configured stim set:" << endl;
	cout << m_arguments.pStimSet->toString() << endl;

	vector<int> pages;
	for (int i = 0; i < m_arguments.pStimSet->num_pages(); i++) pages.push_back(i + 1);
	int iHack = 1;
	if (m_arguments.bUseDrawGroups) iHack = -1;
	if (m_arguments.pStimSet->init(pages, iHack * m_arguments.nStimPages))
	{
		cerr << "FixUStim::run_stim(): StimSet initialization failed." << endl;
	}

	if (m_arguments.bPresentOnTrigger)
		cout << "FixUStim::run_stim(): Present on trigger is ON" << endl;
	else
		cout << "FixUStim::run_stim(): Present on trigger is OFF" << endl;

	// switch display back to first client page
	vsgSetZoneDisplayPage(vsgVIDEOPAGE, pages[0]);

	// delay ready pulse - might be useful for dual-vsg situations to make sure
	// ready pulse doesn't come too early.
	if (m_arguments.iReadyPulseDelay > 0)
		Sleep(m_arguments.iReadyPulseDelay);
	vsg.ready_pulse(100, m_arguments.iPulseBits);

	// All right, start monitoring triggers........
	int last_output_trigger = 0;
	long input_trigger = 0;
	long saved_input_trigger = 0;
	bool bHaveAsciiTrigger;
	bool bHaveBinaryTrigger;
	int trigger_count = 0;

	// reset all triggers if using binary triggers
	if (m_arguments.bBinaryTriggers)
	{
		long digin = vsgIOReadDigitalIn();
		cout << "Reset input triggers with current value of " << std::hex << digin << endl;
		triggers().reset(digin);
		saved_input_trigger = digin;
	}


	// prompt for input if ascii inputs
	if (!m_arguments.bBinaryTriggers)
	{
		cout << "Enter trigger/key: ";
	}

	string s;
	while (!quit_enabled())
	{
		// Triggers can come as binary (uses vsg digital I/O to read), or as ascii (for testing). 
		// Now that serial connection is allowed, each of the original types (binary or ascii) can have 
		s.clear();
		bHaveAsciiTrigger = false;
		bHaveBinaryTrigger = false;
		if (!m_arguments.bBinaryTriggers)
		{
			if (_kbhit())
			{
				// Get a new "trigger" from user. Will block here, so if user keys something, this will block until CR entered.
				cin >> s;
				cout << "Enter trigger/key: ";
				bHaveAsciiTrigger = true;
			}
			else if (m_arguments.bUsingSerial)
			{
				s = m_serial_port.readStringUntil(";");
				if (s.size() > 0)
				{
					//cout << "Got serial string: " << s << endl;
					bHaveAsciiTrigger = true;
					m_serial_port.writeString(s);
				}
			}
		}
		else
		{
			if (m_arguments.bUsingSerial)
			{
				s = m_serial_port.readStringUntil(";");
				if (s.size() > 0)
				{
					//cout << "Got serial string: " << s << endl;
					bHaveAsciiTrigger = true;
					m_serial_port.writeString(s);
				}
			}
			// If there was an actual serial trigger read, it trumps the digital trigger. If no serial read, 
			// then we still get the digital i/o. 
			if (!bHaveAsciiTrigger)
			{
				input_trigger = vsgIOReadDigitalIn();
				if (input_trigger != saved_input_trigger)
				{
					saved_input_trigger = input_trigger;
					bHaveBinaryTrigger = true;
					//std::cerr << "Changed binary input trig: " << std::hex << input_trigger << std::endl;
				}
			}
		}

		//TriggerFunc	tf = std::for_each(triggers().begin(), triggers().end(),
		//	(m_arguments.bBinaryTriggers ? TriggerFunc(input_trigger, last_output_trigger, false) : TriggerFunc(s, last_output_trigger)));

		// only exexute the trigger checks if there was a trigger of some sort
		if (bHaveAsciiTrigger || bHaveBinaryTrigger)
		{
			TriggerFunc tf;
			if (bHaveAsciiTrigger)
			{
				//std::cerr << "check string trigs" << std::endl;
				tf = TriggerFunc(s, last_output_trigger);
				for (auto ptrig : triggers())
				{
					tf(ptrig);
				}
			}
			else if (bHaveBinaryTrigger)
			{
				//std::cerr << std::endl << std::endl << "check binary trigs" << std::endl;
				//for (unsigned int i = 0; i < triggers().size(); i++)
				//{
				//	std::cout << "Trigger " << i << " " << *(triggers().at(i)) << std::endl;
				//}
				//std::cerr << std::endl << std::endl;

				tf = TriggerFunc(input_trigger, last_output_trigger);

				for (auto ptrig : triggers())
				{
					tf(ptrig);
				}
			}

			// if quit requested, get out now
			if (tf.quit()) break;

			// write digout if current output is different than last write. 
			if (tf.output_trigger() != last_output_trigger)
			{
				// If the execute() function of the trigger that fired tells us that a page cycling operation
				// is finishing - we will wait here for a little bit until the vsg tells us that the cycling is 
				// done (via vsgLUTCYCLINGSTATE). We make sure that there is at least one  vsgFrameSync() call. 
				// This will HOPEFULLY) prevent trigger pulses delivered by the page cycling facility from 
				// interfering with those from our own triggers.
				// There is nothing here to actually disable the cycling -- that should have been done in the 
				// handler. 
				if (tf.pending_cycling_disable())
				{
					int n = 0;
					while (n < 10 && vsgGetSystemAttribute(vsgLUTCYCLINGSTATE) > -1)
					{
						vsgFrameSync();
						n++;
					}
					if (n==10)
						cerr << "ERROR - pending_cycling_disable n=10" << endl;
					vsgFrameSync();
				}
				vsgIOWriteDigitalOut(tf.output_trigger() << 1, 0xfffe);
				last_output_trigger = tf.output_trigger();
			}

			// if vsgPresent() is called for....
			if (tf.present())
			{
				//cout << "Got present(): old " << hex << last_output_trigger << " new " << hex << tf.output_trigger() << endl;
				// Check whether we do an ordinary present(), or if we are doing dualstim rig hijinks we'll want to 
				// do a presendOnTrigger. In the presentOnTrigger case, we do a further check on whether any of the
				// triggers matched (you can have multiple triggers matched in a single check) is on the list of 
				// those to be triggered on (see commandline arg -V). 
				if (!m_arguments.bPresentOnTrigger)
				{
					if (tf.present_with_trigger())
					{
						vsgObjSetTriggers(vsgTRIG_ONPRESENT, 0, 0);	// will this persist?
					}
					vsgPresent();
				}
				else
				{
					if (m_arguments.sTriggeredTriggers.empty() || m_arguments.sTriggeredTriggers.find_first_of(tf.triggers_matched()) < string::npos)
					{
						cerr << "FixUStim::run_stim(): Present armed (" << std::hex << m_arguments.ulTriggerArmed << "), wait for trigger..." << endl;
						vsgIOWriteDigitalOut(m_arguments.ulTriggerArmed, m_arguments.ulTriggerArmed);
						vsgFrameSync();	// this blocks until the next refresh, when the IO output is written. 
						vsgPresentOnTrigger(vsgTRIG_ONRISINGEDGE + vsgDIG7);
						cerr << "FixUStim::run_stim(): got trigger..." << endl;
					}
					else
					{
						vsgPresent();
					}
				}
			}
		}
		// short sleep
		Sleep(10);
	}

	if (m_arguments.pStimSet)
		m_arguments.pStimSet->cleanup(pages);
	vsg.clear();

	if (m_arguments.bDisableGammaCorrection)
	{
		cout << "WARNING: RE-ENABLING GAMMA CORRECTION." << endl;
		vsgSetVideoMode(vsgGAMMACORRECT);
	}

	return;
}


void FixUStim::init_triggers(TSpecificFunctor<FixUStim>* pfunctor, int npages)
{
	triggers().clear();
	triggers().addTrigger(new FunctorCallbackTrigger("F", 0x2, 0x2, 0x2, 0x2, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("S", 0x4, 0x4, 0x14, 0x4, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("X", 0x26, 0x0, 0x16, 0x0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("f", 0x2, 0x0, 0x2, 0x0, pfunctor));

	// MODIFIED - npages can be negative. Negative values mean we are using "draw groups". 
	// 1 page is the way things have always been. 
	// 2 pages presumes that a second trigger line is available and wired and ready
	// 3 pages presumes a third. StimSets must be able to react to these triggers if they are to be used. 

	if (abs(npages) == 1)
	{
		triggers().addTrigger(new FunctorCallbackTrigger("u", 0x20, 0x20 | AR_TRIGGER_TOGGLE, 0x10, 0x10 | AR_TRIGGER_TOGGLE, pfunctor));
		triggers().addTrigger(new FunctorCallbackTrigger("v", 0x40, 0x40 | AR_TRIGGER_TOGGLE, 0x20, 0x20 | AR_TRIGGER_TOGGLE, pfunctor));
		triggers().addTrigger(new FunctorCallbackTrigger("D", 0, AR_TRIGGER_ASCII_ONLY, 0, 0, pfunctor));
	}
	else if (abs(npages) == 2)
	{
		triggers().addTrigger(new FunctorCallbackTrigger("U", 0x20, 0x20, 0x14, 0x10, pfunctor));
	}
	else if (abs(npages) == 3)
	{
		triggers().addTrigger(new FunctorCallbackTrigger("U", 0x20, 0x20, 0x10, 0x10, pfunctor));
		triggers().addTrigger(new FunctorCallbackTrigger("V", 0x40, 0x40, 0x20, 0x20, pfunctor));
	}


	triggers().addTrigger(new FunctorCallbackTrigger("s", 0x4, 0x0, 0x4, 0x0, pfunctor));	// this must come after X so it will not fire with an X.

	std::vector< std::pair<std::string, int> > vec;
	vec.push_back(make_pair("a", 0x8 | AR_TRIGGER_TOGGLE));
	vec.push_back(make_pair("g", AR_TRIGGER_ASCII_ONLY));
	triggers().addTrigger(new MISOFunctorCallbackTrigger(vec, 0x8, 0x8, 0x8 | AR_TRIGGER_TOGGLE, pfunctor));


	// For UStim-specific testing. The UStim should handle this trigger and do whatever. Ascii only trigger.
	triggers().addTrigger(new FunctorCallbackTrigger("A", 0, 0, 0, 0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("B", 0, 0, 0, 0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("Z", 0, 0, 0, 0, pfunctor));

	// quit trigger
	triggers().addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));

	return;
}

int FixUStim::callback(int &output, const FunctorCallbackTrigger* ptrig, const std::string&)
{
	// TODO - must change this call to include args. That will trigger big changes in StimSet-derived classes:(

	return  m_arguments.pStimSet->handle_trigger(ptrig->getMatchedKey(), ptrig->getArgs());
}


error_t parse_fixstim_opt(int key, char* carg, struct argp_state* state)
{
	error_t ret = 0;
	struct fixstim_arguments* arguments = (struct fixstim_arguments*)state->input;
	std::string sarg;
	if (carg) sarg = carg;
	switch (key)
	{
	case 'a':
		arguments->bBinaryTriggers = false;
		break;
	case 'v':
		arguments->bVerbose = true;
		break;
	case 'b':
		if (parse_color(sarg, arguments->bkgdColor))
			ret = EINVAL;
		break;
	case 'd':
		if (parse_distance(sarg, arguments->iDistanceToScreenMM))
			ret = EINVAL;
		else
			arguments->bHaveDistance = true;
		break;
	case 'l':
		if (parse_integer(sarg, arguments->iReadyPulseDelay))
			ret = EINVAL;
		break;
	case 'p':
		if (parse_integer(sarg, arguments->iPulseBits))
			ret = EINVAL;
		break;
	case 771:
		if (parse_integer(sarg, arguments->nStimPages) || arguments->nStimPages < 1 || arguments->nStimPages > 3)
			ret = EINVAL;
		break;
	case 776:
		arguments->bDisableGammaCorrection = true;
		break;
	case 779:
		arguments->bUsingSerial = true;
		arguments->serial_port = sarg;
		break;
	case 774:
		arguments->pStimSet = new CycleTestStimSet();
		break;
	case 769:
		if (parse_integer(sarg, arguments->iPendingDrawGroup))
			ret = EINVAL;
		else
		{
			arguments->bPendingDrawGroup = true;
			arguments->bUseDrawGroups = true;
			arguments->nStimPages = max(arguments->iPendingDrawGroup, arguments->nStimPages);
		}
		break;
	case 'V':
	{
		// This arg should have a number representing the bit to be used as the trigger
		// (e.g. 128 corresponds to vsgDIG??? - its placed directly on the digout).
		// Additionally it can have the specific triggers that should be triggered. 
		// Only triggers that will require a present() matter here (so "q" will never 
		// be triggered, e.g.). If omitted, then all triggers that require a present()
		// will be triggered. 
		// -V 128    trigger all using bit7
		// -V 128,FS trigger only F,S triggers using bit7. X will not require trigger.
		//
		// This arg must be coordinated with the PLS file in use by the spike2 script! 
		size_t pos = sarg.find_first_of(",");
		if (pos)
		{
			arguments->sTriggeredTriggers = sarg.substr(pos);
			if (parse_ulong(sarg.substr(0, pos), arguments->ulTriggerArmed))
				ret = EINVAL;
			else arguments->bPresentOnTrigger = true;
		}
		else
		{
			arguments->sTriggeredTriggers = "";
			if (parse_ulong(sarg, arguments->ulTriggerArmed))
				ret = EINVAL;
			else arguments->bPresentOnTrigger = true;
		}
		break;
	}
	case 'f':
		if (parse_fixation_point(sarg, arguments->fixpt))
			ret = EINVAL;
		else
		{
			arguments->bHaveFixpt = true;
			arguments->bLastWasFixpt = true;
			arguments->bLastWasGrating = arguments->bLastWasDistractor = false;
			arguments->vecFixpts.push_back(arguments->fixpt);					// this vector might be ignored by some (most) stim sets.
		}
		break;
	case 'g':
		if (parse_grating(sarg, arguments->grating)) 
			ret = EINVAL;
		else
		{
			if (!arguments->bHaveFixpt)
			{
				arguments->pStimSet = new GratingStimSet(arguments->grating);
			}
			else if (arguments->bHaveFixpt && !arguments->bHaveXhair)
			{
				arguments->pStimSet = new FixptGratingStimSet(arguments->fixpt, arguments->grating);
			}
			else
			{
				arguments->pStimSet = new FixptGratingStimSet(arguments->fixpt, arguments->xhair, arguments->grating);
			}
		}
		break;
	case 'h':
		if (parse_xhair(sarg, arguments->xhair))
			ret = EINVAL;
		else
			arguments->bHaveXhair = true;
		break;
	case 'i':
	{
		// parse image arg
		// -i filename								load on demand, display each at 0,0
		// -i filename,x,y							load on demand, display each at x,y

		double x = 0, y = 0;
		double duration = 0;
		double delay = 0;
		int nlevels = 0;
		string filename;

		if (!parseImageArg(sarg, filename, x, y, duration, delay, nlevels))
		{
			cerr << "Error parsing image argument: " << sarg << endl;
			ret = EINVAL;
			break;
		}

		if (arguments->bHaveFixpt)
		{
			arguments->pStimSet = createImageStimSet(arguments->fixpt, filename, x, y, duration, delay, nlevels);
		}
		else
		{
			arguments->pStimSet = createImageStimSet(filename, x, y, duration, delay, nlevels);
		}
		if (!arguments->pStimSet)
		{
			cerr << "Error - bad input file for image stim set." << endl;
			ret = EINVAL;
		}

		break;
	}
	case 's':
		if (arguments->vecGratings.size() == 8)
		{
			cerr << "Maximum number of gratings(8) reached." << endl;
			ret = EINVAL;
		}
		else if (parse_grating(sarg, arguments->grating))
		{
			cerr << "Error in grating input: " << sarg << endl;
			ret = EINVAL;
		}
		else
		{
			arguments->bHaveStim = true;
			arguments->bLastWasGrating = true;
			arguments->bLastWasFixpt = arguments->bLastWasDistractor = false;

			if (arguments->bPendingDrawGroup)
			{
				arguments->grating.setDrawGroups(arguments->iPendingDrawGroup);
				arguments->bPendingDrawGroup = false;
				arguments->iPendingDrawGroup = 0;
			}
			arguments->vecGratings.push_back(arguments->grating);

			if (arguments->bUsingMultiParameterStimSet)
			{
				MultiParameterFXMultiGStimSet* pmulti = static_cast<MultiParameterFXMultiGStimSet*>(arguments->pStimSet);
				pmulti->add_grating(arguments->grating);
			}
		}
		break;
	case 'k':
		if (arguments->vecDistractors.size() == 8)
		{
			cerr << "Maximum number of distractors(8) reached." << endl;
			ret = EINVAL;
		}
		else if (parse_grating(sarg, arguments->grating))
		{
			cerr << "Error in grating input: " << sarg << endl;
			ret = EINVAL;
		}
		else
		{
			arguments->bLastWasDistractor = true;
			arguments->bLastWasGrating = arguments->bLastWasFixpt = false;
			arguments->vecDistractors.push_back(arguments->grating);

			if (arguments->bUsingMultiParameterStimSet)
			{
				MultiParameterFXMultiGStimSet* pmulti = static_cast<MultiParameterFXMultiGStimSet*>(arguments->pStimSet);
				pmulti->add_distractor(arguments->grating);
			}
		}
		break;
	case 'K':
	{
		/* 
		* This arg forces a stim set be constructed using the fixpt, gratings, distractors currently specified. 
		*/

		if (!arguments->bUsingMultiParameterStimSet)
		{
			MultiParameterFXMultiGStimSet* pss = NULL;

			// Stim set with gratings&distractors
			if (!arguments->bHaveFixpt && !arguments->bHaveXhair)
			{
				pss = new MultiParameterFXMultiGStimSet();
			}
			else if (!arguments->bHaveXhair)
			{
				pss = new MultiParameterFXMultiGStimSet(arguments->fixpt);
			}
			else
			{
				pss = new MultiParameterFXMultiGStimSet(arguments->fixpt, arguments->xhair);
			}
			for (unsigned int i = 0; i < arguments->vecGratings.size(); i++) pss->add_grating(arguments->vecGratings[i]);
			for (unsigned int i = 0; i < arguments->vecDistractors.size(); i++) pss->add_distractor(arguments->vecDistractors[i]);
			for (unsigned int i = 0; i < arguments->vecDots.size(); i++) pss->add_dot(arguments->vecDots.at(i));
			//for (unsigned int i = 0; i < arguments->vecRectangles.size(); i++) pss->add_rectangle(arguments->vecRectangles.at(i));

			arguments->pStimSet = pss;
			arguments->bUsingMultiParameterStimSet = true;
		}
		break;
	}
	case 'H':
	{
		vector<double> tuning_parameters;
		int nsteps;

		if (parse_tuning_list(sarg, tuning_parameters, nsteps))
			ret = EINVAL;
		else
		{
			if (!arguments->bHaveStim)
			{
				cerr << "Error - must pass template grating stimulus with \"-s\" before passing tuning parameters." << endl;
				ret = EINVAL;
			}
			else
			{
				// must have two gratings specified, it is assumed that the first is the donut, second is the hole. 
				if (arguments->bHaveFixpt)
				{
					if (arguments->vecGratings.size() == 2)
						arguments->pStimSet = new DanishStimSet(arguments->fixpt, arguments->vecGratings[0], arguments->vecGratings[1], tuning_parameters);
					else
						arguments->pStimSet = new DanishStimSet(arguments->fixpt, arguments->vecGratings[0], tuning_parameters);
				}
				else
				{
					if (arguments->vecGratings.size() == 2)
					{
						arguments->pStimSet = new DanishStimSet(arguments->vecGratings[0], arguments->vecGratings[1], tuning_parameters);
					}
					else
					{
						arguments->pStimSet = new DanishStimSet(arguments->vecGratings[0], tuning_parameters);
					}
				}
			}
		}
		break;
	}
	case 'c':
	{
		// border ownership
		// args -c sz0,sz1,c0,c1,n0,n1,n2,...
		if (!(arguments->pStimSet = parseBorderStimSet(sarg)))
			ret = EINVAL;
		break;
	}
	case 765:
	{
		if (parse_bmp_image_list(sarg, arguments->vecBmpImageSpec, arguments->uiNBmpLevels))
			ret = EINVAL;
		else
			arguments->bHaveBmpImageList = true;
		break;
	}
	case 'O':
	case 'T':
	case 'S':
	case 'C':
	case 'A':
	case 'Z':
	case 'X':
	case 'M':
	case 'y':
	case 'E':
	case 't':
	case 'P':
	case 'I':
	case 'U':
	case 'D':
	case 778:
	case 768:
	case 773:
	case 772:
	case 770:
	case 767:
	case 766:
	case 764:
	{
		vector<double> tuning_parameters;
		vector<COLOR_TYPE> color_parameters;
		vector<COLOR_VECTOR_TYPE> colorvector_parameters;
		vector<vector<alert::ARFixationPointSpec> > dot_parameters;
		vector<vector<alert::ARRectangleSpec> > rect_parameters;
		vector<vector<std::tuple<double, double, double> > >multigrating_parameter_groups;
		vector<int> bmp_image_indices;

		int nsteps;

		// the first time one of these stim parameter lists is found we must create
		// the stim set. On first creating it we have to account for any gratings and/or
		// distractors that have been specified up to this point.

		if ((key == 'D' && parse_dot_list(sarg, dot_parameters)) ||
			(key == 767 && parse_rectangle_arg(sarg, rect_parameters)) ||
			(key == 'U' && parse_color_list(sarg, color_parameters)) ||
			(key == 770 && parse_colorvector_list(sarg, colorvector_parameters)) ||
			(key == 772 && parse_multigrating(sarg, multigrating_parameter_groups)) ||
			(key == 764 && parse_int_list(sarg, bmp_image_indices)) ||
			((key != 'D' && key != 767 && key != 'U' && key != 770 && key != 764) && parse_tuning_list(sarg, tuning_parameters, nsteps)))
		{
			ret = EINVAL;
		}
		else
		{
			// If no stim configured, create stim set with fixpt and/or xhair.
			if (!arguments->pStimSet)
			{
				arguments->pStimSet = create_multiparameter_stimset(arguments->bHaveFixpt, arguments->fixpt, arguments->bHaveXhair, arguments->xhair);
				arguments->bUsingMultiParameterStimSet = true;

				MultiParameterFXMultiGStimSet* pmulti = static_cast<MultiParameterFXMultiGStimSet*>(arguments->pStimSet);

				for (unsigned int i = 0; i < arguments->vecGratings.size(); i++)
				{
					pmulti->add_grating(arguments->vecGratings.at(i));
				}
				for (unsigned int i = 0; i < arguments->vecDistractors.size(); i++)
				{
					pmulti->add_distractor(arguments->vecDistractors.at(i));
				}
			}

			if (!arguments->bUsingMultiParameterStimSet)
			{
				cerr << "Error - Cannot mix COASTXKMP with other stim types!" << endl;
				ret = EINVAL;
			}
			else
			{
				MultiParameterFXMultiGStimSet* pmulti = static_cast<MultiParameterFXMultiGStimSet*>(arguments->pStimSet);
				size_t stimIndex;
				if (key == 'D')
				{
					pmulti->add_dot(dot_parameters[0]);
					stimIndex = pmulti->dot_count() - 1;
				}
				else if (key == 767)
				{
					pmulti->add_rectangle(rect_parameters[0]);
					stimIndex = pmulti->rectangle_count() - 1;
				}
				else if (arguments->bLastWasFixpt) stimIndex = -1;
				else if (arguments->bLastWasGrating) stimIndex = pmulti->count() - 1;
				else if (arguments->bLastWasDistractor) stimIndex = pmulti->distractor_count() - 1;
				else stimIndex = -1;

				// Create a stim parameter set
				FXGStimParameterList* plist = NULL;
				switch (key)
				{
				case 'C':
					plist = new StimContrastList(tuning_parameters, (unsigned int)stimIndex, arguments->bLastWasDistractor);
					break;
				case 'O':
					plist = new StimOrientationList(tuning_parameters, (unsigned int)stimIndex, arguments->bLastWasDistractor);
					break;
				case 'A':
					plist = new StimAreaList(tuning_parameters, (unsigned int)stimIndex, arguments->bLastWasDistractor);
					break;
				case 'S':
					plist = new StimSFList(tuning_parameters, (unsigned int)stimIndex, arguments->bLastWasDistractor);
					break;
				case 'T':
					plist = new StimTFList(tuning_parameters, (unsigned int)stimIndex, arguments->bLastWasDistractor);
					break;
				case 'Z':
					plist = new GratingXYList(tuning_parameters, (unsigned int)stimIndex, arguments->bLastWasDistractor);
					break;
				case 'X':
					plist = new StimXList(tuning_parameters, (unsigned int)stimIndex, arguments->bLastWasDistractor);
					break;
				case 'M':
					plist = new StimHoleList(tuning_parameters, (unsigned int)stimIndex, arguments->bLastWasDistractor);
					break;
				case 'y':
					plist = new FixptXYList(tuning_parameters);
					break;
				case 'E':
					plist = new StimDelayList(tuning_parameters);
					break;
				case 't':
					plist = new StimDurationList(tuning_parameters);
					break;
				case 'P':
					plist = new StimPhaseList(tuning_parameters, (unsigned int)stimIndex, arguments->bLastWasDistractor);
					break;
				case 'I':
					plist = new StimTTFList(tuning_parameters, (unsigned int)stimIndex, arguments->bLastWasDistractor);
					break;
				case 'U':
					plist = new FixptColorList(color_parameters);
					break;
				case 'D':
					plist = new DotList(dot_parameters, (unsigned int)stimIndex);
					break;
				case 767:
					plist = new RectangleList(rect_parameters, (unsigned int)stimIndex);
					break;
				case 770:
					plist = new ColorVectorList(colorvector_parameters, (unsigned int)stimIndex, arguments->bLastWasDistractor);
					break;
				case 768:
					plist = new PursuitList(tuning_parameters);
					if (arguments->bUsingMultiParameterStimSet)
					{
						MultiParameterFXMultiGStimSet* pmulti = static_cast<MultiParameterFXMultiGStimSet*>(arguments->pStimSet);
						pmulti->setSweepNotPursuit(true);
					}
					else
					{
						arguments->bSweepNotPursuit = true;
						cerr << "Sweep but no stim set yet!!!" << endl;
					}
					break;
				case 778:
					plist = new PursuitList(tuning_parameters);
					break;
				case 773:
					plist = new GratingWHList(tuning_parameters, (unsigned int)stimIndex, arguments->bLastWasDistractor);
					break;
				case 772:
					plist = new MultiGratingOriList(multigrating_parameter_groups, (unsigned int)stimIndex, arguments->bLastWasDistractor);
					break;
				case 766:
					plist = new GratingBarList(tuning_parameters, (unsigned int)stimIndex, arguments->bLastWasDistractor);
					break;
				case 764:
				{
					if (arguments->bHaveBmpImageList)
					{
						plist = new BmpImageOrderList(bmp_image_indices, 0, false);
						pmulti->setBmpImageSpec(arguments->vecBmpImageSpec, arguments->uiNBmpLevels);
					}
					else
					{
						cerr << "Need --bmp_image-list arg before --bmp-image-order" << endl;
						ret = EINVAL;
					}
					break;
				}
				default:
					cerr << "Unhandled varying stim parameter type (" << (char)key << ")" << endl;
					ret = EINVAL;
				}
				if (plist) pmulti->push_back(plist);
			}
		}
		break;
	}
	case 'R':
	case 'B':
	{
		vector<double> list;
		int nterms;
		int iunused;
		const char* sequence = NULL;
		bool balanced = (key == 'B');

		if (!arguments->bHaveStim)
		{
			cerr << "Error - must specify a grating stim (-s) before specifying a CRG stimulus." << endl;
			ret = EINVAL;
		}
		else if (parse_tuning_list(sarg, list, iunused) || list.size() < 3)
		{
			cerr << "Bad format in repeating arg. Should be at least 3 ints: frames_per_term,first_term(0...),nterms[,contrast0,contrast1,...]" << endl;
			ret = EINVAL;
		}
		else
		{
			int first, length, fpt;
			fpt = (int)list[0];
			first = (int)list[1];
			length = (int)list[2];
			list.erase(list.begin());	// erase first three elements
			list.erase(list.begin());
			list.erase(list.begin());

			// If command line has a sequence "-e" argument, the use it; otherwise use the default msequence.
			if (!arguments->bHaveSequence)
			{
				sequence = get_msequence();
				nterms = (int)strlen(sequence);
			}
			else
			{
				sequence = arguments->sequence.c_str();
				nterms = (int)arguments->sequence.length();
			}

			// Check that sequence args work with this sequence file
			if (nterms > 0 && first > -1 && (first + length < nterms))
			{
				string seq;
				seq.assign(&sequence[first], length);

				// Create StimSet
				if (arguments->bHaveFixpt)
				{
					if (list.size() > 0)
					{
						arguments->pStimSet = new CRGStimSet(arguments->fixpt, arguments->grating, fpt, seq, list, balanced);
					}
					else
					{
						arguments->pStimSet = new CRGStimSet(arguments->fixpt, arguments->grating, fpt, seq, balanced);
					}
				}
				else
				{
					if (list.size() > 0)
					{
						arguments->pStimSet = new CRGStimSet(arguments->grating, fpt, seq, list, balanced);
					}
					else
					{
						arguments->pStimSet = new CRGStimSet(arguments->grating, fpt, seq, balanced);
					}
				}
			}
		}
		break;
	}
	case 'L':
	{
		// frames_per_term - assumes 0/1 and B/W; use entire sequence
		// frames_per_term,color0,color1,... - use entire sequence with these colors
		// frames_per_term,first_term(0...),nterms - assumes 0/1 and B/W
		// frames_per_term,first_term(0...),nterms,color0,color1,color2... - can have up to 9 colors
		// If fixpt is configured, then it is used. 
		// If sequence was not specified on command line by now, msequence is used! 
		// In other words, must specify "-e" before "-L", otherwise you get the mesequence

		int first, length, fpt, nterms;
		const char* sequence = NULL;
		string seq;
		vector<string> tokens;

		if (!arguments->bHaveSequence)
		{
			sequence = get_msequence();
			nterms = (int)strlen(sequence);
		}
		else
		{
			sequence = arguments->sequence.c_str();
			nterms = (int)strlen(sequence);
		}

		tokenize(sarg, tokens, ",");
		if (tokens.size() == 1)
		{
			if (parse_integer(tokens[0], fpt))
			{
				cerr << "Error in flash spec (-L): first arg must be an integer (fpt)." << endl;
				ret = EINVAL;
				break;
			}
			else
			{
				seq.assign(sequence);	// use the whole sequence
				if (arguments->bHaveFixpt)
				{
					arguments->pStimSet = new FlashStimSet(arguments->fixpt, fpt, seq);
				}
				else
				{
					arguments->pStimSet = new FlashStimSet(fpt, seq);
				}
			}
		}
		else if (tokens.size() < 3)
		{
			cerr << "Error in Flash arg (-L): must specify fpt,first,nterms[[color0,color1[,color2...]]]" << endl;
			ret = EINVAL;
		}
		else
		{
			// first token must be an integer.
			// If second token is an integer, then the third must also be an integer. Any remaining tokens must be colors.
			// If the second token is NOT an integer, then all remaining tokens must be colors (and entire sequence is used).
			if (parse_integer(tokens[0], fpt))
			{
				cerr << "Error in Flash arg (-L): must specify fpt,first,nterms[[color0,color1[,color2...]]]" << endl;
				ret = EINVAL;
			}
			else
			{
				unsigned int first_color_index;

				// If command line has a sequence "-e" argument, the use it; otherwise use the default msequence.
				if (!arguments->bHaveSequence)
				{
					sequence = get_msequence();
					nterms = (int)strlen(sequence);
				}
				else
				{
					sequence = arguments->sequence.c_str();
					nterms = (int)arguments->sequence.length();
				}

				// decide where the color parameters start
				if (parse_integer(tokens[1], first) || parse_integer(tokens[2], length))
				{
					first = 0;
					nterms = length = (int)strlen(sequence);
					first_color_index = 1;
				}
				else
				{
					first_color_index = 3;
				}

				// Check that sequence args work with this sequence
				if (nterms > 0 && first > -1 && (first + length <= nterms))
				{
					seq.assign(&sequence[first], length);
				}
				else
				{
					cerr << "Error in flash spec: check sequence length and first,nterms args." << endl;
					ret = EINVAL;
					break;
				}

				// now parse colors if necessary
				if (first_color_index < tokens.size())
				{
					vector<COLOR_TYPE> colors;
					tokens.erase(tokens.begin(), tokens.begin() + first_color_index);
					if (parse_color_list(tokens, colors))
					{
						cerr << "Error parsing colors for flash stimulus: " << sarg << endl;
						ret = EINVAL;
					}
					else
					{
						if (arguments->bHaveFixpt)
						{
							arguments->pStimSet = new FlashStimSet(arguments->fixpt, colors, fpt, seq);
						}
						else
						{
							arguments->pStimSet = new FlashStimSet(colors, fpt, seq);
						}
					}
				}
				else
				{
					if (arguments->bHaveFixpt)
					{
						arguments->pStimSet = new FlashStimSet(arguments->fixpt, fpt, seq);
					}
					else
					{
						arguments->pStimSet = new FlashStimSet(fpt, seq);
					}
				}
			}
		}
		break;
	}
	case 'e':
	{
		string seq;

		// Either a sequence of integers 0,1,... (might be more numbers if -L arg given and there are more than two colors)
		// or same thing in an ascii text file
		if (parse_sequence(sarg, arguments->sequence))
		{
			cerr << "Error - bad sequence format (-e)." << endl;
			ret = EINVAL;
		}
		else
		{
			arguments->bHaveSequence = true;
			cerr << "Got sequence of " << arguments->sequence.length() << " terms." << endl;
		}
		break;
	}
	case 'Y':		// CRG alternate - input file is a list of separate sequences, and each sequence is a "stim"
	{
		int ifpt;
		vector<string> tokens;
		vector<string> sequences;
		vector<int> order;
		StimSetCRG* pSSCRG = NULL;

		tokenize(sarg, tokens, ",");
		if (!arguments->bHaveStim)
		{
			cerr << "Error - must pass template grating stimulus with \"-s\" before passing CRG sequence parameters." << endl;
			ret = EINVAL;
		}
		else if (tokens.size() < 2)
		{
			cerr << "Bad format for CRG stim. Expecting \"-Y frames_per_term,filename[,i1,i2,i3...]\", got \"" << sarg << "\"." << endl;
			ret = EINVAL;
		}
		else if (parse_integer(tokens[0], ifpt))
		{
			cerr << "Bad format for CRG frames_per_term. Expecting \"-Y frames_per_term,filename[,i1,i2,i3...]\", got \"" << sarg << "\"." << endl;
			ret = EINVAL;
		}
		else if (arutil_load_sequences(sequences, tokens[1]))
		{
			cerr << "Error loading sequences for CRG stim. Check format of stim file \"" << tokens[1] << "\"" << endl;
			ret = EINVAL;
		}
		else 
		{
			tokens.erase(tokens.begin());
			tokens.erase(tokens.begin());
			// anything left? 
			if (tokens.size() > 0)
			{
				if (parse_int_list(tokens, order))
				{
					cerr << "Error in sequence order list. Expecting fpt,filename,i0,i1,i2,... where iN are integers." << endl;
					ret = EINVAL;
				}
			}
			else
			{
				unsigned int i;
				for (i = 0; i < sequences.size(); i++)
				{
					order.push_back(i);
				}
				cerr << "Warning: No sequence order supplied on command line. Using default ordering for sequences." << endl;
			}
		}


		// Check that all sequences are the same length. 
		for (vector<string>::const_iterator it = sequences.begin(); it != sequences.end(); it++)
		{
			if (it->length() != sequences.begin()->length())
			{
				cerr << "Error - all sequences are not the same length (" << sequences.begin()->length() << "). Check sequence file." << endl;
				ret = EINVAL;
			}
		}

		if (!ret)
		{
			if (!arguments->bHaveFixpt && !arguments->bHaveXhair)
				arguments->pStimSet = new StimSetCRG(arguments->grating, ifpt, sequences, order);
			else if (arguments->bHaveFixpt && !arguments->bHaveXhair)
				arguments->pStimSet = new StimSetCRG(arguments->fixpt, arguments->grating, ifpt, sequences, order);
			else
				arguments->pStimSet = new StimSetCRG(arguments->fixpt, arguments->xhair, arguments->grating, ifpt, sequences, order);
		}
		break;
	}
	case 'G':
	{
		vector<double> tuning_parameters;
		vector<string> tokens;
		double ww, hh, dps;
		COLOR_TYPE color;
		tokenize(sarg, tokens, ",");
		if (parse_color(tokens[0], color))
		{
			cerr << "Error - first parameter in bar list spec must be a color (" << tokens[0] << ")." << endl;
			ret = EINVAL;
		}
		else
		{
			tokens.erase(tokens.begin());
		}

		if (parse_number_list(tokens, tuning_parameters) || tuning_parameters.size() <= 3)
		{
			cerr << "Error - cannot parse drifting bar parameters color,width,height,deg_per_sec,ori1,ori2,..." << endl;
			ret = EINVAL;
		}
		else
		{
			ww = tuning_parameters[0];
			hh = tuning_parameters[1];
			dps = tuning_parameters[2];

			tuning_parameters.erase(tuning_parameters.begin());
			tuning_parameters.erase(tuning_parameters.begin());
			tuning_parameters.erase(tuning_parameters.begin());
			arguments->pStimSet = new CBarStimSet(color, ww, hh, dps, tuning_parameters);
		}
		break;
	}
	case 777:
	{
		// Dots arg: -D color,x,y,diam,speed,density,dotsize,angle1,angle2,...
		vector<double> tuning_parameters;
		vector<string> tokens;
		COLOR_TYPE color;
		double dotx, doty, dotdiam, dotspeed, dotdensity, dotsize;
		tokenize(sarg, tokens, ",");
		if (parse_color(tokens[0], color))
		{
			cerr << "Error - first parameter in bar list spec must be a color (" << tokens[0] << ")." << endl;
			ret = EINVAL;
		}
		else
		{
			tokens.erase(tokens.begin());
		}

		if (parse_number_list(tokens, tuning_parameters) || tuning_parameters.size() <= 6)
		{
			cerr << "Error - cannot parse dots parameters: color,x,y,diam,speed,density,dotsize,angle1,angle2,..." << endl;
			ret = EINVAL;
		}
		else
		{
			dotx = tuning_parameters[0];
			tuning_parameters.erase(tuning_parameters.begin());
			doty = tuning_parameters[0];
			tuning_parameters.erase(tuning_parameters.begin());
			dotdiam = tuning_parameters[0];
			tuning_parameters.erase(tuning_parameters.begin());
			dotspeed = tuning_parameters[0];
			tuning_parameters.erase(tuning_parameters.begin());
			dotdensity = tuning_parameters[0];
			tuning_parameters.erase(tuning_parameters.begin());
			dotsize = tuning_parameters[0];
			tuning_parameters.erase(tuning_parameters.begin());

			// whatever's left in tuning_parameters are the angles. 
			if (arguments->bHaveFixpt)
			{
				arguments->pStimSet = new DotStimSet(arguments->fixpt, dotx, doty, color, dotdiam, dotspeed, dotdensity, (int)dotsize, tuning_parameters);
			}
			else
			{
				arguments->pStimSet = new DotStimSet(dotx, doty, color, dotdiam, dotspeed, dotdensity, (int)dotsize, tuning_parameters);
			}

		}
		break;
	}
	case 'r':
	case 'q':
	{
		arguments->bUseCueCircles = false;
		arguments->bUseCuePoints = true;
		if (key == 'r')
			arguments->bCuePointIsDot = false;
		else if (key == 'q')
			arguments->bCuePointIsDot = true;
		if (parse_attcues(sarg, (int)arguments->vecGratings.size(), arguments->vecAttentionCues))
		{
			cerr << "Error in attention cues input (-r or -q)." << endl;
			ret = EINVAL;
		}
		else
		{
			cerr << "Read " << arguments->vecAttentionCues.size() << " attention cues." << endl;
			//dump_attcues(m_vecAttentionCues);
		}
		break;
	}
	case 'j':
	{
		cerr << "parse flashies" << endl;
		if (parse_flashyparams(sarg, arguments->vecFlashies))
		{
			cerr << "Error in flashy params input: " << sarg << endl;
			ret = EINVAL;
		}
		else
		{
			cerr << "Read " << arguments->vecFlashies.size() << " flashies. Should be same as # of trials." << endl;
		}
		break;
	}
	case 'm':
	{
		vector<InterleavedParams> vecInput;
		if (parse_interleaved_params(sarg, (int)arguments->vecGratings.size(), vecInput))
		{
			cerr << "Error in input for interleaved params." << endl;
		}
		else
		{
			my_print_interleaved_trials(vecInput);
			arguments->pStimSet = new AttentionStimSet(arguments->fixpt, arguments->vecGratings, arguments->vecAttentionCues, arguments->bUseCueCircles, arguments->bUseCuePoints, vecInput);
		}
		break;
	}
	case 'n':
	{
		vector<AttentionSequenceTrialSpec> trialSpecs;
		ImageFilesPositions ifp;
		if (parse_sequenced_params(sarg, (int)arguments->vecGratings.size(), trialSpecs, ifp))
		{
			cerr << "Error in sequenced param input" << endl;
		}
		else
		{
			if (arguments->vecGratings.empty())
				arguments->pStimSet = new SequencedImagesAttentionStimSet(arguments->fixpt, ifp, arguments->vecAttentionCues, arguments->bUseCueCircles, arguments->bUseCuePoints, arguments->bCuePointIsDot, trialSpecs);
			else
				arguments->pStimSet = new SequencedAttentionStimSet(arguments->fixpt, arguments->vecGratings, arguments->vecAttentionCues, arguments->bUseCueCircles, arguments->bUseCuePoints, arguments->bCuePointIsDot, trialSpecs);
		}
		break;
	}
	case 'o':
	{
		vector<MelTrialSpec> trialSpecs;
		if (parse_mel_params(sarg, trialSpecs))
		{
			cerr << "Error in mel param input" << endl;
			ret = EINVAL;
		}
		else
		{
			cerr << "Parse OK, got " << trialSpecs.size() << " trials" << endl;
			arguments->pStimSet = new MelStimSet(arguments->vecFixpts, trialSpecs);
		}
		break;
	}
	case 'J':
	{
		// Henry's Attention expt. 
		// Arg should be a sequence of comma-separated numbers. 
		// The first number is the time-after-contrast change number. This is the same for all trials. 
		// After that, there should be 6 numbers per trial,
		// - fixpt color
		// - Base contrast
		// - Up contrast
		// - integer indicating which stim changes contrast. 0, 1, ... in order that "-s" was specified
		// - initial phase of grating
		// - time to contrast change from grating onset (sec)
		// - stim OFF bitflag, int. If bit N set, then the corresponding stim is NOT on for this trial.
		//   0 = all stim on; 1 = first stim NOT on; 2 = second stim NOT on; etc. 
		// 
		// 
		double tMax;
		vector<AttParams> vecInput;
		if (parse_attparams(sarg, (int)arguments->vecGratings.size(), vecInput, tMax))
		{
			cerr << "Error in input." << endl;
			ret= EINVAL;
		}
		else
		{
			cerr << "Read " << vecInput.size() << " trials for Attention" << endl;
			if (arguments->bHaveFixpt)
			{
				if (arguments->vecGratings.size() > 0)
				{
					if (arguments->vecAttentionCues.size() == 0)
					{
						if (arguments->vecFlashies.size() > 0)
						{
							if (arguments->vecFlashies.size() == vecInput.size())
							{
								if (checkFlashyTimes(vecInput, arguments->vecFlashies, tMax) == 0)
								{
									arguments->pStimSet = new AttentionStimSet(arguments->fixpt, tMax, arguments->vecGratings, vecInput, arguments->vecDistractors, arguments->vecFlashies);
								}
								else
								{
									cerr << "Error in input for Attention stim: Flashies must start/end before end of trial." << endl;
									ret = EINVAL;
								}
							}
							else
							{
								cerr << "Error in input for Attention stim: There are flashies configured for " << arguments->vecFlashies.size() << " trials, but " << vecInput.size() << " trials are configured. These should be the same." << endl;
								ret = EINVAL;
							}
						}
						else
						{
							arguments->pStimSet = new AttentionStimSet(arguments->fixpt, tMax, arguments->vecGratings, vecInput);
						}
					}
					else
					{
						if (arguments->vecFlashies.size() > 0)
						{
							if (arguments->vecFlashies.size() == vecInput.size())
							{
								if (checkFlashyTimes(vecInput, arguments->vecFlashies, tMax) == 0)
								{
									arguments->pStimSet = new AttentionStimSet(arguments->fixpt, tMax, arguments->vecGratings, arguments->vecAttentionCues, arguments->bUseCueCircles, arguments->bUseCuePoints, vecInput, arguments->vecDistractors, arguments->vecFlashies);
								}
								else
								{
									cerr << "Error in input for Attention stim: Flashies must start/end before end of trial." << endl;
									ret = EINVAL;
								}
							}
							else
							{
								cerr << "Error in input for Attention stim: There are flashies configured for " << arguments->vecFlashies.size() << " trials, but " << vecInput.size() << " trials are configured. These should be the same." << endl;
								ret = EINVAL;
							}
						}
						else
						{
							arguments->pStimSet = new AttentionStimSet(arguments->fixpt, tMax, arguments->vecGratings, arguments->vecAttentionCues, arguments->bUseCueCircles, arguments->bUseCuePoints, vecInput);
						}
					}
				}
				else
				{
					cerr << "Error in input for Attention stim - no gratings specified!" << endl;
					ret = EINVAL;
				}
			}
			else
			{
				cerr << "Error input for Attention stim - no fixpt specified!" << endl;
				ret = EINVAL;
			}
		}
		break;
	}
	case 'W':
	{
		// argument is a filename. The path is taken as the root path for stimuli.
		// 1/21/16 djs
		// Argument expanded to allow an ip:port specification for the StarStim EEG program 'Nic'
		// That prog has a tcp server (accepts connections on port 1234) that we will put timing
		// markers into at stim onset/offset. 
		// The arg is now this:
		//
		// -W /path/to/file[,ip:port]
		// 

		vector<string> argStrings;
		vector<struct EQParams> vecEQParams;
		const char* pIPPort = (char*)NULL;	// non-NULL when ip:port arg given

		boost::split(argStrings, sarg, boost::is_any_of(","));
		if (argStrings.size() > 1)
		{
			pIPPort = argStrings[1].c_str();
		}

		if (!parse_eqparams_file(argStrings[0], (int)arguments->vecGratings.size(), vecEQParams))
		{
			cerr << "Error reading input parameter file: " << argStrings[0] << endl;
			ret = EINVAL;
		}
		else
		{
			arguments->pStimSet = new EQStimSet(arguments->fixpt, arguments->vecGratings, vecEQParams, pIPPort);
		}
		break;
	}
	default:
		ret = ARGP_ERR_UNKNOWN;
		break;
	}
	return ret;
}

MultiParameterFXMultiGStimSet* create_multiparameter_stimset(bool bHaveFixpt, ARContrastFixationPointSpec& fixpt, bool bHaveXhair, ARXhairSpec& xhair)
{
	MultiParameterFXMultiGStimSet* pstimset = NULL;
	ARContrastFixationPointSpec f(fixpt);
	ARXhairSpec h(xhair);
	if (bHaveFixpt && !bHaveXhair)
	{
		pstimset = new MultiParameterFXMultiGStimSet(f);
	}
	else if (bHaveFixpt && bHaveXhair)
	{
		pstimset = new MultiParameterFXMultiGStimSet(f, h);
	}
	else
	{
		pstimset = new MultiParameterFXMultiGStimSet();
	}
	return pstimset;
}

MultiParameterFXMultiGStimSet* create_multiparameter_stimset(bool bHaveFixpt, ARContrastFixationPointSpec& fixpt, bool bHaveXhair, ARXhairSpec& xhair, ARGratingSpec& grating)
{
	MultiParameterFXMultiGStimSet* pstimset = NULL;
	ARContrastFixationPointSpec f(fixpt);
	ARGratingSpec g(grating);
	ARXhairSpec h(xhair);
	if (bHaveFixpt && !bHaveXhair)
	{
		pstimset = new MultiParameterFXMultiGStimSet(g, f);
	}
	else if (bHaveFixpt && bHaveXhair)
	{
		pstimset = new MultiParameterFXMultiGStimSet(g, f, h);
	}
	else
	{
		pstimset = new MultiParameterFXMultiGStimSet(g);
	}
	return pstimset;
}


template <class T>
StimSet* create_stimset(bool bHaveFixpt, ARContrastFixationPointSpec& fixpt, bool bHaveXhair, ARXhairSpec& xhair, ARGratingSpec& grating, vector<double> params)
{
	StimSet *pstimset = (StimSet *)NULL;
	ARContrastFixationPointSpec f(fixpt);
	ARGratingSpec g(grating);
	ARXhairSpec h(xhair);
	if (bHaveFixpt && !bHaveXhair)
	{
		pstimset = new T(f, g, params);
	}
	else if (bHaveFixpt && bHaveXhair)
	{
		pstimset = new T(f, h, g, params);
	}
	else
	{
		pstimset = new T(g, params);
	}
	return pstimset;
}

bool parseImageArg(const std::string& arg, std::string& filename, double& x, double& y, double& duration, double& delay, int& nlevels)
{
	std::vector<std::string> vec;
	tokenize(arg, vec, ",");
	cerr << "parseImageArg: arg has " << vec.size() << " tokens." << endl;

	// can have args like this:
	// "filename"		just list file (screen pos 0,0)
	// "filename,x,y"   with position on screen
	// "filename,x,y,seconds"	with stim duration in sec. If seconds>0, this triggers usage of cycling, 
	//							which means stim duration is precisely controlled. 
	// "filename,x,y,seconds,low,hi"	with low,high water marks for caching stim. Not implemented, but if 
	//									hi>0 we load ALL images (no test for available memory!)

	// initialize 
	nlevels = 230;
	x = y = 0;
	duration = 0;
	delay = 0;

	// check for correct number of tokens
	if (vec.size() == 1 || vec.size() == 3 || vec.size() == 4 ||
		vec.size() == 5 || vec.size() == 6)
	{
		filename = vec[0];
		if (vec.size() > 1)
		{
			// get x,y
			if (parse_double(vec[1], x))
			{
				cerr << "Error parsing image X location: " << vec[1] << endl;
				return false;
			}
			if (parse_double(vec[2], y))
			{
				cerr << "Error parsing image Y location: " << vec[2] << endl;
				return false;
			}
		}
		if (vec.size() > 3)
		{
			// get duration
			if (parse_double(vec[3], duration))
			{
				cerr << "Error parsing stim duration: " << vec[3] << endl;
				return false;
			}
		}
		if (vec.size() > 4)
		{
			// get delay
			if (parse_double(vec[4], delay))
			{
				cerr << "Error parsing numlevels: " << vec[4] << endl;
				return false;
			}
		}
		if (vec.size() > 5)
		{
			// get nlevels
			if (parse_integer(vec[5], nlevels))
			{
				cerr << "Error parsing numlevels: " << vec[5] << endl;
				return false;
			}
		}
	}
	else
	{
		cerr << "bad arg for images: must have 1,3, 4, 5, or 6 args: -i filename[,x,y[,duration[,delay[,nlevels]]]]" << endl;
		return false;
	}
	return true;
}

int parse_dot_list(const std::string& sarg, vector<vector<alert::ARFixationPointSpec> >& dot_list)
{
	int status = 0;
	vector<string> tokens;
	vector<alert::ARFixationPointSpec> fixpts;

	// groups are separated by '!' - these are trials, swapped out on "a" trigger. 
	boost::split(tokens, sarg, boost::is_any_of("!"));
	for (string s : tokens)
	{
		fixpts.clear();
		status = parse_fixation_point_list(s, fixpts);
		if (!status)
		{
			dot_list.push_back(fixpts);
		}
		else
		{
			cerr << "Error parsing multidot group" << endl;
		}
	}
	return status;
}

int parse_rectangle_arg(const std::string& sarg, vector<vector<alert::ARRectangleSpec> >& rect_list)
{
	int status = 0;
	vector<string> tokens;
	vector<alert::ARRectangleSpec> rects;

	// groups are separated by '!' - these are trials, swapped out on "a" trigger. 
	boost::split(tokens, sarg, boost::is_any_of("!"));
	for (string s : tokens)
	{
		rects.clear();
		status = parse_rectangle_list(s, rects);
		if (!status)
		{
			rect_list.push_back(rects);
		}
		else
		{
			cerr << "Error parsing rectangle list: " << s << endl;
		}
	}
	return status;
}


int parse_multigrating(const std::string& sarg,vector<vector<std::tuple<double, double, double> > >& params)
{
	int status = 0;
	vector<string> tokens;
	vector<double> xyori;
	vector < std::tuple<double, double, double> > group;
	int nsteps;

	// groups are separated by '!'
	boost::split(tokens, sarg, boost::is_any_of("!"));
	for (string s : tokens)
	{
		xyori.clear();
		status = parse_tuning_list(s, xyori, nsteps);
		group.clear();
		if (!status)
		{
			for (int i = 0; i < xyori.size() - 2; i += 3)
				group.push_back(make_tuple(xyori[i], xyori[i + 1], xyori[i + 2]));
			params.push_back(group);
		}
		else
		{
			cerr << "Error parsing multigrating group" << endl;
		}
	}
	return status;
}