#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <conio.h>
#include "Alertlib.h"
#include "AlertUtil.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")

using namespace std;
using namespace alert;


typedef enum tuning_type { tt_orientation, tt_contrast, tt_spatial, tt_temporal, tt_area, tt_xy, tt_none_specified } tuning_type_t;

typedef struct tuning_curve_spec {
	ARGratingSpec grating;					// grating spec for this curve
	vector<double> tuned_parameter_vec;		// value(s) for the parameter which is being varied.
	tuning_type ttype;
	double temporal_contrast_freq;			// when ttype is tt_xy, this is the freq for the contrast change
	double seconds_per_stim;				// seconds for each stim on screen. Same time (approx) for blank between. 
} tuning_curve_spec_t;

// this vector contains all the tuning curves to be run
vector<tuning_curve_spec_t> f_tuning_curve_vec;

// index of current tuning curve
int f_iTuningCurveIndex = 0;

// these help keep track during the presentation of the curves
double f_tuned_param_current = 0;
int f_nsteps = 0;
int f_istep_current = 0;
int f_iSavedContrast = 0;
VSGCYCLEPAGEENTRY f_cycle[6];
PageCyclingTrigger *f_ptrigCycling = NULL;
ARGratingSpec f_stim;
bool f_binaryTriggers = true;
TriggerVector triggers;

// these parameters apply to all curves
int f_screenDistanceMM = 0;
COLOR_TYPE f_background = { gray, {0.5, 0.5, 0.5}};
double f_seconds_between_curves = 0;
bool f_automatic = false;
bool f_verbose = false;					// words, words, words,....



// init vsg
int init_vsg();

// init video and overlay pages
void init_pages();

// create obj and draw first stim
void init_stim(int stimindex);

// update tuned parameter and redraw stim
void update_stim(int stimindex, int step);

// prepare cycling array
void prepare_cycling(int stimindex, bool bLastOne);

// run through the stimuli using the cycling array. 
void run_cycling();

// ask for input - testing stuff
void testing_loop();

// parse args
int args(int argc, char **argv);

// errors? dump usage and exit. 
void usage();

int parse_tuning_specfile(string filename);
int args(int argc, char **argv);
void destroy_stim(int stimindex);

void dump_all_tuning_curve_specs();
void dump_tuning_curve_specs(tuning_curve_spec_t& tcurve);

// initialize triggers for automatic run
void init_triggers();

// callback for triggers
int callback(int &output, const CallbackTrigger* ptrig);

// callback for page cycling trigger
int page_cycling_callback(int icycle);

int main(int argc, char **argv)
{
	// parse command line args
	if (args(argc, argv)) return 1;

	// words, words, words
	if (f_verbose) dump_all_tuning_curve_specs();

	// initialize vsg, get lock, etc.
	if (ARvsg::instance().init(f_screenDistanceMM, f_background))
	{
		cerr << "Error: Cannot init vsg card." << endl;
		return -1;
	}

	// Init pages
	init_pages();

	// make sure initial screen is blank
	//vsgPresent();
	//vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);


	if (f_automatic)
	{
		// Initialize triggers
		init_triggers();

		// Issue ready pulse to spike2
		ARvsg::instance().ready_pulse();

		// Initialize stim. 
		// NOTE: this is one place where modifications would be necessary 
		// should you want to handle multiple curves in automatic mode. 
		init_stim(0);

		// Now process triggers
		std::string s;
		int last_output_trigger=0;
		int input_trigger=0;
		while (1)
		{
			// If user-triggered, get a trigger entry. 
			if (!f_binaryTriggers)
			{
				// Get a new "trigger" from user
				cout << "Enter trigger/key: ";
				cin >> s;
			}
			else
			{
				input_trigger = vsgIOReadDigitalIn();
			}

			TriggerFunc	tf = std::for_each(triggers.begin(), triggers.end(), 
				(f_binaryTriggers ? TriggerFunc(input_trigger, last_output_trigger) : TriggerFunc(s, last_output_trigger)));

			// Now analyze input trigger
		 	
			if (tf.quit()) 
			{
				// quitting in the middle of page cycling requires special handling
				vsgSetCommand(vsgCYCLEPAGEDISABLE);
				vsgMoveScreen(0, 0);
				break;
			}
			else if (tf.present())
			{	
				last_output_trigger = tf.output_trigger();
				vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
				vsgPresent();
			}

			// Throttle cpu usage a little. No need to be in hyperspeed checking for triggers here. 
			Sleep(100);
		}


	}
	else
	{

		cout << "Hit S to start stimuli ...." << endl;
		while (true)
		{
			if (_kbhit() &&	('S' == _getch())) break;
			Sleep(100);
		}
		run_cycling();
		cout << "Done." << endl;
	}

	return 0;
}


void init_triggers()
{
	triggers.addTrigger(new CallbackTrigger("s", 0x2, 0x2, 0x2, 0x2, callback));
	triggers.addTrigger(new CallbackTrigger("X", 0x2, 0x0, 0x2, 0x0, callback));

	// Initialize page cycling trigger. The number of repeats is the number of parameters in the tuning
	// curve. NOTE: Assuming that there is just one tuning curve!!!!!
	f_ptrigCycling = new PageCyclingTrigger("C", (int)f_tuning_curve_vec[0].tuned_parameter_vec.size(), page_cycling_callback);
	triggers.addTrigger(f_ptrigCycling);
	triggers.addTrigger(new QuitTrigger("q", 0x80, 0x80, 0xff, 0x0, 0));

		// Dump triggers
	std::cout << "Triggers:" << std::endl;
	for (unsigned int i=0; i<triggers.size(); i++)
	{
		std::cout << "Trigger " << i << " " << *(triggers[i]) << std::endl;
	}

	triggers.reset(vsgIOReadDigitalIn());
	
}



int callback(int &output, const CallbackTrigger* ptrig)
{
	int ival=0;
	string key = ptrig->getKey();
	if (key == "s")
	{
		update_stim(0, 0);
		vsgPresent();
		prepare_cycling(0, false);
//		vsgSetCommand(vsgVIDEODRIFT);			// allows us to move the offset of video memory
		vsgSetCommand(vsgCYCLEPAGEENABLE);
		f_ptrigCycling->started();
	}
	else if (key == "X")
	{
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		f_ptrigCycling->stopped();
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);
	}

	return 0;		// this ensures that no vsgPresent is called (which would insert an extra trigger)
}

int page_cycling_callback(int icycle)
{
	// icycle indicates which parameter we're on in the curve. 
	std::cout << "Page_cycling_callback icycle=" << icycle << std::endl;
	update_stim(0, icycle);
	vsgPresent();
	prepare_cycling(0, icycle == f_tuning_curve_vec[0].tuned_parameter_vec.size()-1);
	return 0;
}



void prepare_cycling(int stimindex, bool bLastOne)
{
	int iframes;
	tuning_curve_spec_t& tcurve = f_tuning_curve_vec[stimindex];
	iframes = (int)(tcurve.seconds_per_stim * 1000000 /(double)vsgGetSystemAttribute(vsgFRAMETIME));
	f_cycle[0].Page = 0 + vsgTRIGGERPAGE;
	f_cycle[0].Xpos = 0;
	f_cycle[0].Ypos = 0;
	f_cycle[0].ovPage = 0;
	f_cycle[0].ovXpos = 0;
	f_cycle[0].ovYpos = 0;
	f_cycle[0].Stop = 0;
	f_cycle[0].Frames = iframes;
	f_cycle[1].Page = 1 + vsgTRIGGERPAGE;
	f_cycle[1].Xpos = 0;
	f_cycle[1].Ypos = 0;
	f_cycle[1].ovPage = 1;
	f_cycle[1].ovXpos = 0;
	f_cycle[1].ovYpos = 0;
	f_cycle[1].Stop = 0;
	f_cycle[1].Frames = iframes;
	if (!bLastOne)
	{
		f_cycle[2].Page = 1 + vsgTRIGGERPAGE;
		f_cycle[2].Xpos = 0;
		f_cycle[2].Ypos = 0;
		f_cycle[2].ovPage = 1;
		f_cycle[2].ovXpos = 0;
		f_cycle[2].ovYpos = 0;
		f_cycle[2].Stop = 1;

		vsgPageCyclingSetup(3, f_cycle);
	}
	else
	{
		iframes = (int)(f_seconds_between_curves * 1000000 /(double)vsgGetSystemAttribute(vsgFRAMETIME));

		f_cycle[2].Page = 1 + vsgTRIGGERPAGE;
		f_cycle[2].Xpos = 0;
		f_cycle[2].Ypos = 0;
		f_cycle[2].ovPage = 0;
		f_cycle[2].ovXpos = 0;
		f_cycle[2].ovYpos = 0;
		f_cycle[2].Frames = 1;
		f_cycle[2].Stop = 0;
		f_cycle[3].Page = 1 + vsgTRIGGERPAGE;
		f_cycle[3].Xpos = 0;
		f_cycle[3].Ypos = 0;
		f_cycle[3].ovPage = 0;
		f_cycle[3].ovXpos = 0;
		f_cycle[3].ovYpos = 0;
		f_cycle[3].Frames = 1;
		f_cycle[3].Stop = 0;
		f_cycle[4].Page = 1 + vsgTRIGGERPAGE;
		f_cycle[4].Xpos = 0;
		f_cycle[4].Ypos = 0;
		f_cycle[4].ovPage = 0;
		f_cycle[4].ovXpos = 0;
		f_cycle[4].ovYpos = 0;
		f_cycle[4].Frames = iframes;
		f_cycle[4].Stop = 0;
		f_cycle[5].Page = 1;
		f_cycle[5].Xpos = 0;
		f_cycle[5].Ypos = 0;
		f_cycle[5].ovPage = 0;
		f_cycle[5].ovXpos = 0;
		f_cycle[5].ovYpos = 0;
		f_cycle[5].Frames = 1;
		f_cycle[5].Stop = 1;

		vsgPageCyclingSetup(6, f_cycle);
	}
}

void run_cycling()
{
	unsigned int stimindex;
	unsigned int istep;
	bool bQuit = false;
	cout << "Hit q to quit." << endl;
	for (stimindex = 0; stimindex<f_tuning_curve_vec.size() && !bQuit; stimindex++)
	{
		tuning_curve_spec_t& tcurve = f_tuning_curve_vec[stimindex];

		cout << "Tuning curve " << (stimindex+1) << "/" << f_tuning_curve_vec.size() << endl;
		dump_tuning_curve_specs(tcurve);
		cout << "Hit q to quit." << endl;

		init_stim(stimindex);
		for (istep=0; istep < tcurve.tuned_parameter_vec.size(); istep++)
		{
			update_stim(stimindex, istep);
			vsgPresent();
			prepare_cycling(stimindex, istep==(tcurve.tuned_parameter_vec.size()-1));

			vsgSetCommand(vsgCYCLEPAGEENABLE);
			Sleep(500);
			while (vsgGetSystemAttribute(vsgPAGECYCLINGSTATE) >= 0)
			{
				if (_kbhit() && ('q'==_getch()))
				{
					bQuit = true;
					break;
				}
				Sleep(10);
			}
			vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);
			if (bQuit)
			{
				cout << "Aborted by user." << endl;
				break;
			}

		}
		destroy_stim(stimindex);
	}
	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);

}


// clear page 0 and 1. Display page 1. 
// Page 0 is for stim, page 1 is background. 
// Calls vsgSetZoneDisplayPage for OVERLAY page 1 and VIDEO page 1
// NOTE: Overlay page is the background color. With this overlay active we can draw
// stimuli and call vsgPresent() without the flashes where the stim appears. 
void init_pages()
{
	VSGTRIVAL background;

	// The background level was obtained in the init() call.
	if (get_color(f_background, background))
	{
		cerr << "Cannot get trival for background color " << f_background << endl;
		return;
	}
	vsgPresent();						// must call this once so next function works
	vsgSetBackgroundColour(&background);
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);
	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);

	// Overlay page 0 will be used for all stimuli. Set overlay palette color 1 to have bg color and clear page 1 to it. 
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 0);		// clear
	vsgSetDrawPage(vsgOVERLAYPAGE, 1, 1);		// background color
	arutil_color_to_overlay_palette(f_background, 1);
	vsgSetCommand(vsgOVERLAYMASKMODE);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 0);

}

// updates tuned parameter in the stim and redraws it on page 0. 
void update_stim(int stimindex, int step)
{
	tuning_curve_spec_t& tcurve = f_tuning_curve_vec[stimindex];
	switch(tcurve.ttype)
	{
	case tt_contrast:
		tcurve.grating.setContrast((int)tcurve.tuned_parameter_vec[step]);
		break;
	case tt_spatial:
		tcurve.grating.sf = tcurve.tuned_parameter_vec[step];
		break;
	case tt_temporal:
		tcurve.grating.tf = tcurve.tuned_parameter_vec[step];
		break;
	case tt_orientation:
		tcurve.grating.orientation = tcurve.tuned_parameter_vec[step];
		break;
	case tt_area:
		tcurve.grating.w = tcurve.grating.h = tcurve.tuned_parameter_vec[step];
		break;
	case tt_xy:
		f_tuned_param_current = tcurve.tuned_parameter_vec[step];
		// do temporal waveform
		tcurve.grating.select();
		vsgObjSetTemporalFrequency(tcurve.temporal_contrast_freq);
		vsgObjTableSinWave(vsgTWTABLE);
		vsgObjSetSpatialPhase(tcurve.tuned_parameter_vec[step]);
		break;
	default:
		cerr << "Error in update_stim: unknown tuning type for tcurve=" << stimindex << " step=" << step << "!" << endl;
	}

	arutil_draw_grating(tcurve.grating, 0);

	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1);
	arutil_draw_aperture(tcurve.grating, 0);
}


void init_stim(int stimindex)
{
	// init the stim. This call creates a vsg object. Have to set draw page to a video page, otherwise vsg tells us
	// that there's only pixel levels 0-3 available. 

	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	f_tuning_curve_vec[stimindex].grating.init(1, 50);
}

void destroy_stim(int stimindex)
{
	f_tuning_curve_vec[stimindex].grating.destroy();
}

int args(int argc, char **argv)
{	
	bool have_g=false;		// have stim (grating) spec
	bool have_tt = false;	// have tuning type?
	bool have_x = false;	// when f_tuning_type==tt_xy, then temporal frequency must be set
	bool have_d = false;	// have screen distance
	bool have_f = false;
	string filename;
	string s;
	int c;
	tuning_curve_spec_t tcurve;
	int dummy;
	extern char *optarg;
	extern int optind;
	int errflg = 0;

	tcurve.seconds_per_stim = 0.0;
	tcurve.temporal_contrast_freq = 0.0;
	tcurve.ttype = tt_none_specified;

	while ((c = getopt(argc, argv, "avb:g:hC:T:S:O:A:X:x:t:cd:f:u:")) != -1)
	{
		switch (c) 
		{
		case 'f':
			have_f = true;
			filename.assign(optarg);
			break;
		case 'c':
			f_automatic = true;
			break;
		case 'v':
			f_verbose = true;
			break;
		case 't':
			s.assign(optarg);
			if (parse_double(s, tcurve.seconds_per_stim)) errflg++; 
			break;
		case 'b': 
			s.assign(optarg);
			if (parse_color(s, f_background)) errflg++; 
			break;
		case 'u':
			s.assign(optarg);
			if (parse_double(s, f_seconds_between_curves)) errflg++;
			break;
		case 'd':
			s.assign(optarg);
			if (parse_distance(s, f_screenDistanceMM)) errflg++;
			else have_d=true;
			break;
		case 'g':
			s.assign(optarg);
			if (parse_grating(s, tcurve.grating)) errflg++;
			else have_g = true;;
			break;
		case 'h':
			errflg++;
			break;
		case 'C':
			s.assign(optarg);
			if (parse_tuning_list(s, tcurve.tuned_parameter_vec, dummy)) errflg++;
			else 
			{
				have_tt = true;
				tcurve.ttype = tt_contrast;
			}
			break;
		case 'O':
			s.assign(optarg);
			if (parse_tuning_list(s, tcurve.tuned_parameter_vec, dummy)) errflg++;
			else 
			{
				have_tt = true;
				tcurve.ttype = tt_orientation;
			}
			break;
		case 'S':
			s.assign(optarg);
			if (parse_tuning_list(s, tcurve.tuned_parameter_vec, dummy)) errflg++;
			else 
			{
				have_tt = true;
				tcurve.ttype = tt_spatial;
			}
			break;
		case 'T':
			s.assign(optarg);
			if (parse_tuning_list(s, tcurve.tuned_parameter_vec, dummy)) errflg++;
			else 
			{
				have_tt = true;
				tcurve.ttype = tt_temporal;
			}
			break;
		case 'A':
			s.assign(optarg);
			if (parse_tuning_list(s, tcurve.tuned_parameter_vec, dummy)) errflg++;
			else 
			{
				have_tt = true;
				tcurve.ttype = tt_area;
			}
			break;
		case 'X':
			s.assign(optarg);
			if (parse_tuning_list(s, tcurve.tuned_parameter_vec, dummy)) errflg++;
			else 
			{
				have_tt = true;
				tcurve.ttype = tt_xy;
			}
			break;
		case 'x':
			s.assign(optarg);
			if (parse_double(s, tcurve.temporal_contrast_freq)) errflg++; 
			else have_x = true;
			break;
		case 'a':
			f_binaryTriggers = false;
			break;
		case '?':
            errflg++;
			break;
		default:
			errflg++;
			break;
		}
	}

	if (!have_d)
	{
		cerr << "Screen distance not specified!" << endl; 
		errflg++;
	}


	// You can either specify a grating, in which case you have to specify a type et al, OR you can specify an input file. 
	if (have_g)
	{
		if (!have_tt) 
		{
			cerr << "Tuning variable not specified!" << endl; 
			errflg++;
		}
		else if ((tcurve.ttype == tt_area) && !have_x)
		{
			cerr << "When specifying xy tuning (-X) you must also specify temporal contrast frequency (-x)" << endl;
			errflg++;
		}
		else
		{
			f_tuning_curve_vec.push_back(tcurve);
		}
	}
	else
	{
		if (!have_f)
		{
			cerr << "You must specify a grating (-g) and type (-C|O|A|S|T|X), or an input file (-f)!" << endl;
			errflg++;
		}
		else
		{
			parse_tuning_specfile(filename);
		}
	}
	if (errflg) 
	{
		usage();
	}
	return errflg;
}


int parse_tuning_curve_spec(int argc, char **argv, tuning_curve_spec_t& tcurve)
{	
	bool have_g=false;		// have stim (grating) spec
	bool have_tt = false;	// have tuning type?
	bool have_x = false;	// when f_tuning_type==tt_xy, then temporal frequency must be set
	bool have_d = false;	// have screen distance
	bool have_f = false;
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	int dummy;

	tcurve.seconds_per_stim = 0.0;
	tcurve.temporal_contrast_freq = 0.0;
	tcurve.ttype = tt_none_specified;
	tcurve.tuned_parameter_vec.clear();

	// force getopt to reinitialize
	optind = 0;

	while ((c = getopt(argc, argv, "g:C:T:S:O:A:X:x:t:")) != -1)
	{
		switch (c) 
		{
		case 't':
			s.assign(optarg);
			if (parse_double(s, tcurve.seconds_per_stim)) errflg++; 
			break;
		case 'g':
			s.assign(optarg);
			if (parse_grating(s, tcurve.grating)) errflg++;
			else have_g = true;;
			break;
		case 'C':
			s.assign(optarg);
			if (parse_tuning_list(s, tcurve.tuned_parameter_vec, dummy)) errflg++;
			else 
			{
				have_tt = true;
				tcurve.ttype = tt_contrast;
			}
			break;
		case 'O':
			s.assign(optarg);
			if (parse_tuning_list(s, tcurve.tuned_parameter_vec, dummy)) errflg++;
			else 
			{
				have_tt = true;
				tcurve.ttype = tt_orientation;
			}
			break;
		case 'S':
			s.assign(optarg);
			if (parse_tuning_list(s, tcurve.tuned_parameter_vec, dummy)) errflg++;
			else 
			{
				have_tt = true;
				tcurve.ttype = tt_spatial;
			}
			break;
		case 'T':
			s.assign(optarg);
			if (parse_tuning_list(s, tcurve.tuned_parameter_vec, dummy)) errflg++;
			else 
			{
				have_tt = true;
				tcurve.ttype = tt_temporal;
			}
			break;
		case 'A':
			s.assign(optarg);
			if (parse_tuning_list(s, tcurve.tuned_parameter_vec, dummy)) errflg++;
			else 
			{
				have_tt = true;
				tcurve.ttype = tt_area;
			}
			break;
		case 'X':
			s.assign(optarg);
			if (parse_tuning_list(s, tcurve.tuned_parameter_vec, dummy)) errflg++;
			else 
			{
				have_tt = true;
				tcurve.ttype = tt_xy;
			}
			break;
		case 'x':
			s.assign(optarg);
			if (parse_double(s, tcurve.temporal_contrast_freq)) errflg++; 
			else have_x = true;
			break;
		case '?':
            errflg++;
			break;
		default:
			errflg++;
			break;
		}
	}

	// You can either specify a grating, in which case you have to specify a type et al, OR you can specify an input file. 
	if (!have_g)
	{
		cerr << "You must specify a grating (-g) and type (-C|O|A|S|T|X), or an input file (-f)!" << endl;
		errflg++;
	}
	if (!have_tt) 
	{
		cerr << "Tuning variable not specified!" << endl; 
		errflg++;
	}
	else if ((tcurve.ttype == tt_area) && !have_x)
	{
		cerr << "When specifying xy tuning (-X) you must also specify temporal contrast frequency (-x)" << endl;
		errflg++;
	}

	// If using automatic tuning (i.e. for acute rig), we cannot handle multiple curves. 
	// There's not a technical reason for this - the prog could be adapted to deal with that - I'm 
	// just being lazy...and the rig software doesn't require multiple curves. 
	if (f_automatic && f_tuning_curve_vec.size() > 1)
	{
		cerr << "When using automatic (-A) tuning, multiple curves via an input file (-f) is not implemented!" << endl;
		errflg++;
	}

	return errflg;
}



void usage()
{
	cerr << "usage: meatuning -d screen_distance_MM -b g|b|w -s x,y,w,h,contrast%,sf,tf,orientation,color_vector,s|q,r|e" << endl;
}


int parse_tuning_specfile(string filename)
{
	int status = 0;
	tuning_curve_spec_t tcurve;
	char *argv[20];
	int argc;
	vector<string> tokens;
	string s;
	ifstream ifs(filename.c_str());
	if (!ifs.is_open())
	{
		cerr << "ERROR: Cannot open input file " << filename << endl;
		status = -1;
	}
	else
	{
		getline(ifs, s);
		while (ifs)
		{
			if (s[0] != '#')
			{
				tokens.clear();
				tokens.push_back(string("PROGNAME"));
				tokenize(s, tokens, " ");
				make_argv(tokens, argc, argv);
				if (parse_tuning_curve_spec(argc, argv, tcurve))
				{
					cerr << "Error in tuning curve spec file, line " + (int)(f_tuning_curve_vec.size()+1) << endl;
					status = -1;
					break;
				}
				else 
				{
					f_tuning_curve_vec.push_back(tcurve);
				}
				free_argv(argc, argv);
			}
			getline(ifs, s);
		}
		ifs.close();
	}
	return status;
}

void dump_all_tuning_curve_specs()
{
	cout << "Have " << f_tuning_curve_vec.size() << " curves" <<endl;
	for (unsigned int i=0; i<f_tuning_curve_vec.size(); i++)
		dump_tuning_curve_specs(f_tuning_curve_vec[i]);
}

void dump_tuning_curve_specs(tuning_curve_spec_t& tcurve)
{
	cout << "t=" << tcurve.seconds_per_stim << " x=" << tcurve.temporal_contrast_freq << " g=" << tcurve.grating << " ";
	switch(tcurve.ttype)
	{
	case tt_orientation:
		cout << "ORI=";
		break;
	case tt_contrast:
		cout << "CON=";
		break;
	case tt_spatial:
		cout << "SPA=";
		break;
	case tt_temporal:
		cout << "TMP=";
		break;
	case tt_area:
		cout << "AREA=";
		break;
	case tt_xy:
		cout << "XY=";
		break;
	case tt_none_specified:
	default:
		cout << "NONE SPECIFIED!!!!!";
		break;
	}
	for (unsigned int j=0; j<tcurve.tuned_parameter_vec.size(); j++)
	{
		cout << tcurve.tuned_parameter_vec[j];
		if (j<tcurve.tuned_parameter_vec.size()-1) cout << ", ";
	}
	cout << endl;
}