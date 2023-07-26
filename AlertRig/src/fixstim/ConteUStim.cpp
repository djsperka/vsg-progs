#include "ConteUStim.h"

#include "alert-argp.h"
#include "alert-triggers.h"
#include "ARtypes.h"
#include <memory>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>



DWORD ConteUStim::cOvPageBkgd = 0;
DWORD ConteUStim::cOvPageAperture = 1;
DWORD ConteUStim::cOvPageClear = 2;
DWORD ConteUStim::cPageProbe = 1;
DWORD ConteUStim::cPageTest = 2;
DWORD ConteUStim::cPageCue = 3;


// use this trial for testing
struct conte_trial_spec f_trial 
{
	0, 0, 5, 5, .5, COLOR_TYPE(red), COLOR_TYPE(green),
	{ -5, 5, 3, 3, 45, 1, 0, .6, 2, COLOR_TYPE(red) },
	{ -5, 5, 3, 3, 45, 1, 0, .6, 2, COLOR_TYPE(green) },
	{ -5, 5, 3, 3, 90, 1, 0, .6, 2, COLOR_TYPE(red) },
	{ -5, 5, 3, 3, 45, 1, 0, .6, 2, COLOR_TYPE(green) },
	4, 15
};




// parse xy file
bool parse_dot_supply_file(const std::string& filename, ConteCueDotSupply& dotsupply);

// for argp
error_t parse_conte_opt(int key, char* carg, struct argp_state* state);
static struct argp_option options[] = {
	{"ascii", 'a', 0, 0, "Use ascii triggers (user-entered)"},
	{"verbose",  'v', 0, 0, "Produce verbose output" },
	{"background", 'b', "COLOR", 0, "background color"},
	{"distance-to-screen", 'd', "DIST_MM", 0, "screen distance in MM"},
	{"ready-pulse", 'p', "BITPATTERN", 0, "Ready pulse issued when startup is complete"},
	{"ready-pulse-delay", 'l', "DELAY_MS", 0, "Delay ready pulse for this many ms"},
	{"dot-supply", 701, "filename", 0, "File with x,y positions in [-0.5,0.5]"},
	{"trials", 702, "filename", 0, "Conte trials stim specification"},
	{ 0 }
};
static struct argp f_argp = { options, parse_conte_opt, 0, "fixstim -- all-purpose stimulus engine" };


error_t parse_conte_opt(int key, char* carg, struct argp_state* state)
{
	error_t ret = 0;
	struct conte_arguments* arguments = (struct conte_arguments*)state->input;
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
	case 701:
		if (!boost::filesystem::exists(sarg))
		{
			cerr << "Cannot find dot supply file" << endl;
			ret = EINVAL;
		}
		else
		{
			if (!parse_dot_supply_file(sarg, arguments->dot_supply))
			{
				cerr << "Error parsing dot supply file" << endl;
				ret = EINVAL;
			}
			else
			{
				cerr << "Found " << arguments->dot_supply.npatches() << " patches " << endl;
			}
		}
		break;
	case 702:
		// parse conte stim spec
		if (!boost::filesystem::exists(sarg))
		{
			cerr << "Cannot find trials spec file" << endl;
			ret = EINVAL;
		}
		else
		{
			//if (parse_trials_file(sarg, arguments->trials_spec))
			//{
			//	cerr << "Error parsing dot supply file" << endl;
			//	ret = EINVAL;
			//}
			//else
			//{
			//	cerr << "Blocksize: " << arguments->dot_supply.blocksize() << " nblocks " << arguments->dot_supply.nblocks() << endl;
			//}
		}
		break;
	case ARGP_KEY_END:
		// check that everything needed has been received
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return ret;
}

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
			cout << "out trig " << hex << tf.output_trigger() << endl;
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
	vsgSetCommand(vsgOVERLAYMASKMODE + vsgVIDEODRIFT);
	m_levelOverlayBackground = 1;
	ARvsg::instance().request_single(m_levelColorA);
	arutil_color_to_palette(f_trial.cue_color_a, m_levelColorA);
	ARvsg::instance().request_single(m_levelColorB);
	arutil_color_to_palette(f_trial.cue_color_b, m_levelColorB);
	ARvsg::instance().request_single(m_levelTest);
	arutil_color_to_palette(COLOR_TYPE(0, 1, 1), m_levelTest);
	arutil_color_to_overlay_palette(m_arguments.bkgdColor, m_levelOverlayBackground);

	// overlay background page and clear page.
	//temp vsgSetDrawPage(vsgOVERLAYPAGE, cOvPageBkgd, m_levelOverlayBackground);
	vsgSetDrawPage(vsgOVERLAYPAGE, cOvPageBkgd, 0);
	//vsgSetDrawPage(vsgOVERLAYPAGE, cOvPageClear, 0);

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
	triggers().addTrigger(new FunctorCallbackTrigger("Z", 0x0, 0x0, 0x0, 0x0, pfunctor));
	triggers().addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));

	return;
}

int ConteUStim::draw_dot_patches()
{
	cerr << "Draw dot patches" << endl;
	cerr << "Patch x, y, w, h: " << f_trial.cue_x << ", " << f_trial.cue_y << ", " << f_trial.cue_w << ", " << f_trial.cue_h << endl;
	vsgSetDrawPage(vsgVIDEOPAGE, cPageCue, vsgBACKGROUND);

	double dot_diam = 0.25; 

	// Figure out how many patches can be drawn on a page..........
	long Wpix = vsgGetSystemAttribute(vsgVIDEOZONEWIDTH);
	long Hpix = vsgGetSystemAttribute(vsgVIDEOZONEHEIGHT);
	double Wdeg, Hdeg;
	vsgUnit2Unit(vsgPIXELUNIT, Wpix, vsgDEGREEUNIT, &Wdeg);
	vsgUnit2Unit(vsgPIXELUNIT, Hpix, vsgDEGREEUNIT, &Hdeg);
	double Wpatch, Hpatch;
	Wpatch = dot_diam + f_trial.cue_w;
	unsigned int Npatchperrow = trunc(Wdeg / Wpatch);
	unsigned int Nrows = ceil((double)f_trial.cue_nterms / (double)Npatchperrow);
	cerr << "Patch requires " << Wpatch << " deg, can fit " << Npatchperrow << " in one row, will need " << Nrows << " rows." << endl;
	//for (unsigned int i = 0; i < f_trial.cue_nterms; i++)

	for (unsigned int i = 0; i < f_trial.cue_nterms; i++)
	{
		PatchNNXY patch = m_arguments.dot_supply.patch(i);
		double term_x, term_y;
		term_x = (i%Npatchperrow + 0.5) * (f_trial.cue_w + dot_diam);
		term_y = (i/Npatchperrow + 0.5) * (f_trial.cue_h + dot_diam);
		vsgSetDrawOrigin(term_x, term_y);

		vsgSetPen1(m_levelTest);
		vsgSetDrawMode(vsgCENTREXY + vsgPIXELPEN);
		vsgDrawRect(0, 0, f_trial.cue_w, f_trial.cue_h);
		cerr << i << ": " << term_x << " " << term_y << endl;
		//if (i == 0)
		//{
		//	for (unsigned int j = 0; j < 3; j++)
		//		cerr << "pt " << patch.x(j) << ", " << patch.y(j) << " : " << f_trial.cue_w * patch.x(j) << ", " << f_trial.cue_h * patch.y(j) << endl;
		//}

		// dot coords are for patches with unit diameter. 
		// TODO - randomize drawing order
		vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
		vsgSetPen1(m_levelColorA);
		for (unsigned int idot = 0; idot < patch.n0; idot++)
		{
			vsgDrawOval(f_trial.cue_w * patch.x(idot), f_trial.cue_h * patch.y(idot), 0.25, 0.25);
			//if (i == 0) cerr << "patch 0: " << f_trial.cue_w * patch.x(idot) << ", " << f_trial.cue_h * patch.y(idot) << endl;
		}
		vsgSetPen1(m_levelColorB);
		for (unsigned int idot = patch.n0; idot < patch.n0+patch.n1; idot++)
		{
			vsgDrawOval(f_trial.cue_w * patch.x(idot), f_trial.cue_h * patch.y(idot), 0.25, 0.25);
		}
	}
	vsgSetSpatialUnits(vsgPIXELUNIT);
	vsgSetDrawOrigin(vsgGetScreenWidthPixels() / 2, vsgGetScreenHeightPixels() / 2);
	vsgSetSpatialUnits(vsgDEGREEUNIT);
	return Npatchperrow;
}


void ConteUStim::draw_conte_stim(const struct conte_stim_spec& stim)
{
	cerr << "Draw conte stim" << endl;
	return;
}

void ConteUStim::draw_current()
{
	// overlay 1 has aperture for patch
	vsgSetDrawPage(vsgOVERLAYPAGE, cOvPageAperture, m_levelOverlayBackground);
	vsgSetPen1(0);	// clear on overlay page
	vsgDrawRect(f_trial.cue_x, f_trial.cue_y, f_trial.cue_w, f_trial.cue_h);

	// draw dot patch(es)
	vsgSetDrawPage(vsgVIDEOPAGE, cPageCue, vsgBACKGROUND);
	int nperrow = draw_dot_patches();	// should pass trial spec, but use global for now

	// draw stim pages
	vsgSetDrawPage(vsgVIDEOPAGE, cPageProbe, 0);
	draw_conte_stim(f_trial.s0);
	draw_conte_stim(f_trial.s1);
	vsgSetDrawPage(vsgVIDEOPAGE, cPageTest, 0);
	draw_conte_stim(f_trial.t0);
	draw_conte_stim(f_trial.t1);

	// setup cycling
	setup_cycling(nperrow);
}

void ConteUStim::setup_cycling(int nperrow)
{
	int status = 0;
	int count = 0;
	//static const int m_max_cycle_count = 32768;
	VSGCYCLEPAGEENTRY cycle_params[100];	// warning! No check on usage. You have been warned. 

	double dot_diam = 0.25;
	double term_x, term_y;
	double term_x_pixels, term_y_pixels;

	for (unsigned int i = 0; i < f_trial.cue_nterms; i++)
	{
		cycle_params[i].Frames = f_trial.cue_fpt;
		cycle_params[i].Stop = 0;
		cycle_params[i].Page = cPageCue + vsgDUALPAGE;
		cycle_params[i].ovPage = cOvPageAperture;
		cycle_params[i].ovXpos = cycle_params[i].ovYpos = 0;

		term_x = 0.5 * dot_diam + (i % nperrow) * (f_trial.cue_w + dot_diam);
		term_y = 0.5 * dot_diam + (i / nperrow) * (f_trial.cue_h + dot_diam);
		vsgUnit2Unit(vsgDEGREEUNIT, term_x, vsgPIXELUNIT, &term_x_pixels);
		vsgUnit2Unit(vsgDEGREEUNIT, term_y, vsgPIXELUNIT, &term_y_pixels);
		cycle_params[i].Xpos = term_x_pixels;
		cycle_params[i].Ypos = term_y_pixels;
	}
	cycle_params[f_trial.cue_nterms].Frames = 1;
	cycle_params[f_trial.cue_nterms].Stop = 1;
	cycle_params[f_trial.cue_nterms].Page = cPageCue + vsgDUALPAGE;
	cycle_params[f_trial.cue_nterms].ovPage = 0;
	cycle_params[f_trial.cue_nterms].ovXpos = cycle_params[f_trial.cue_nterms].ovYpos = 0;

	vsgPageCyclingSetup(f_trial.cue_nterms+1, &cycle_params[0]);

}

int ConteUStim::callback(int &output, const FunctorCallbackTrigger* ptrig, const std::string&)
{
	int ival=1;
	string key = ptrig->getKey();


	if (key == "a")
	{
		cerr << "ConteUStim::callback(\"a\")" << endl;
	}
	else if (key == "s")
	{
		cerr << "ConteUStim::callback(\"s\")" << endl;
	}
	else if (key == "S")
	{
		cerr << "ConteUStim::callback(\"S\")" << endl;
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
	}
	else if (key == "F")
	{
		cerr << "ConteUStim::callback(\"F\")" << endl;
	}
	else if (key == "f")
	{
		cerr << "ConteUStim::callback(\"f\")" << endl;
	}
	else if (key == "X")
	{
		cerr << "ConteUStim::callback(\"X\")" << endl;
	}
	else if (key == "Z")
	{
		int i, j;
		cout << "Enter vid ov pages e.g. 1 1: ";
		cin >> i >> j;
		cout << "got " << i << " " << j << endl;
		//vsgSetDrawPage(vsgVIDEOPAGE, i, vsgNOCLEAR);
		//vsgSetDrawPage(vsgOVERLAYPAGE, j, vsgNOCLEAR);
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, i);
		vsgSetZoneDisplayPage(vsgOVERLAYPAGE, j);
		ival = 0;
	}

	return ival;
}


bool parse_dot_supply_file(const std::string& filename, ConteCueDotSupply& dotsupply)
{
	bool bReturn = false;

	boost::filesystem::path p(filename);
	if (!exists(p))
	{
		std::cerr << "Error: dot supply file does not exist: " << filename << endl;
		return false;
	}
	else
	{
		bReturn = true;
		boost::filesystem::path folder = p.parent_path();		// if file has relative pathnames to images, they are relative to dir file lives in
		std::cerr << "Found dot supply file " << p << " at path " << folder << std::endl;

		ifstream ifs(filename, ios::in | ios::binary);

		unsigned int N[2];
		double d[1000];				// hard limit to 500 points per patch

		while (ifs.read((char *)N, 2*sizeof(int)) && ifs.read((char *)d, 2*(N[0] + N[1])*sizeof(double)))
		{
			dotsupply.add_patch(N[0], N[1], d);
			//cerr << "patch " << dotsupply.npatches() << " with " << N[0] << " " << N[1] << endl;
			//if (dotsupply.npatches() == 1)
			//{
			//	cerr << "First patch: " << endl;
			//	for (unsigned int ii = 0; ii < 5; ii++)
			//		cerr << d[ii * 2] << ", " << d[ii * 2 + 1] << endl;
			//	cerr << endl;
			//}
		}
		ifs.close();
	}
	return bReturn;
}

