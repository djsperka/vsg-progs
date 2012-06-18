/* $Id: fixstim.cpp,v 1.11 2012-06-18 22:50:51 devel Exp $ */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__

#include "vsgv8.h"
#include "Alertlib.h"
#include "AlertUtil.h"
#include "StimSet.h"
using namespace std;
using namespace alert;


// libs

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif
#pragma comment (lib, "vsgv8.lib")


// globals
bool f_binaryTriggers = true;
bool f_verbose = false;
bool f_dumpStimSetsOnly = false;
COLOR_TYPE f_background = { gray, {0.5, 0.5, 0.5}};
ARContrastFixationPointSpec f_fixpt;
ARGratingSpec f_grating;
StimSet *f_pStimSet = NULL;			// This is for master in dualVSG mode
int f_iDistanceToScreenMM = -1;
TriggerVector triggers;
int f_pulse = 0x40;

// function prototypes

int prargs_callback(int c, string& arg);
void usage();
void init_triggers();
int callback(int &output, const CallbackTrigger* ptrig);
template <class T> StimSet* create_stimset(bool bHaveFixpt, ARContrastFixationPointSpec& fixpt, ARGratingSpec& grating, vector<double> params);

int main (int argc, char *argv[])
{
	std::vector<int> pages;
	int i;
	int status;

	// Check input arguments
	status = prargs(argc, argv, prargs_callback, "f:b:d:avg:s:C:T:S:O:A:P:R:B:H:Zp:G:D:w", 'F');
	if (status)
	{
		return -1;
	}

	if (f_dumpStimSetsOnly)
	{
		cout << *f_pStimSet << endl;
		return 1;
	}

	// Initialize vsg

	if (ARvsg::instance().init(f_iDistanceToScreenMM, f_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}

	// Initialize page 0 and leave display there. The stim set may safely assume that the current 
	// page is a blank background page and it may return to it (no drawing there!). 
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);

	// Tell the stim set to initialize its pages. Note that the current display
	// page is page 0, and the stim set is given different pages (starting at 1 and depending on 
	// how many pages it requires)
	if (f_verbose)
		cerr << "Configuring " << f_pStimSet->num_pages() << " pages." << endl;
	for (i=0; i<f_pStimSet->num_pages(); i++) pages.push_back(i+1);
	if (f_pStimSet->init(pages))
	{
		cerr << "StimSet initialization failed." << endl;
		return -1;
	}

	// Initialize triggers
	init_triggers();
	if (f_verbose)
	{
		for (unsigned int i=0; i<triggers.size(); i++)
		{
			std::cout << "Trigger " << i << " " << *(triggers[i]) << std::endl;
		}
	}

	// switch display back to first client page
	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);
	ARvsg::instance().ready_pulse(100, f_pulse);

	// All right, start monitoring triggers........
	int last_output_trigger=0;
	long input_trigger = 0;
	string s;

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
	 	
		if (tf.quit()) break;
		else if (tf.present())
		{	
			last_output_trigger = tf.output_trigger();
			if (IS_VISAGE)
			{
				vsgSetTriggerOptions(vsgTRIGOPT_PRESENT, 0, vsgTRIG_OUTPUTMARKER, 0.5, 0, tf.output_trigger() << 1, 0x1FE);
			}
			else
			{
				vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
			}
			vsgPresent();
		}
		Sleep(10);
	}

	ARvsg::instance().clear();

	return 0;
}

void init_triggers()
{
	triggers.addTrigger(new CallbackTrigger("F", 0x2, 0x2, 0x2, 0x2, callback));
	triggers.addTrigger(new CallbackTrigger("S", 0x4, 0x4, 0x4, 0x4, callback));
	triggers.addTrigger(new CallbackTrigger("s", 0x4, 0x0, 0x4, 0x0, callback));
	triggers.addTrigger(new CallbackTrigger("X", 0x6, 0x0, 0x6, 0x0, callback));
	triggers.addTrigger(new CallbackTrigger("a", 0x8, 0x8|AR_TRIGGER_TOGGLE, 0x8, 0x8|AR_TRIGGER_TOGGLE, callback));

	// quit trigger
	triggers.addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));

	return;
}

int callback(int &output, const CallbackTrigger* ptrig)
{
	int ival=1;
	string key = ptrig->getKey();

	if (key == "S" || key == "F" || key=="X" || key=="a" || key=="s")
	{
		ival = f_pStimSet->handle_trigger(key);
	}
	else
	{
		cerr << "ERROR! Unknown trigger sent to callback!" << endl;
	}

	return ival;
}

template <class T>
StimSet* create_stimset(bool bHaveFixpt, ARContrastFixationPointSpec& fixpt, ARGratingSpec& grating, vector<double> params)
{
	StimSet *pstimset=(StimSet *)NULL;
	ARContrastFixationPointSpec f(fixpt);
	ARGratingSpec g(grating);
	if (bHaveFixpt)
	{
		pstimset = new T(f, g, params);
	}
	else
	{
		pstimset = new T(g, params);
	}
	return pstimset;
}

int prargs_callback(int c, string& arg)
{	
	static int errflg = 0;
	static bool have_fixpt = false;
	static bool have_stim = false;
	static bool have_g = false;
	static bool have_d = false;
	static bool bStepTW = false;

	switch(c)
	{
	case 'a':
		f_binaryTriggers = false;
		break;
	case 'Z':
		f_dumpStimSetsOnly = true;
		break;
	case 'v':
		f_verbose = true;
		break;
	case 'b': 
		if (parse_color(arg, f_background)) errflg++; 
		break;
	case 'd':
		if (parse_distance(arg, f_iDistanceToScreenMM)) errflg++;
		else have_d=true;
		break;
	case 'w':
		bStepTW = true;
		break;
	case 'f':
		if (parse_fixation_point(arg, f_fixpt)) errflg++;
		else 
		{
			have_fixpt = true;
		}
		break;
	case 'g':
		if (parse_grating(arg, f_grating)) errflg++;
		else 
		{
			if (!have_fixpt)
			{
				f_pStimSet = new GratingStimSet(f_grating);
			}
			else
			{
				f_pStimSet = new FixptGratingStimSet(f_fixpt, f_grating);
			}
			break;
		}
		break;
	case 's':
		if (parse_grating(arg, f_grating)) 
			errflg++;
		else 
			have_stim = true;
		break;
	case 'p':
		if (parse_integer(arg, f_pulse))
			errflg++;
		break;
	case 'O':
	case 'T':
	case 'S':
	case 'C':
	case 'A':
	case 'H':
	case 'P':
		{
			vector<double> tuning_parameters;
			int nsteps;

			if (parse_tuning_list(arg, tuning_parameters, nsteps)) errflg++;
			else 
			{
				if (!have_stim)
				{
					cerr << "Error - must pass template grating stimulus with \"-s\" before passing tuning parameters." << endl;
					errflg++;
				}
				else
				{
					switch (c)
					{
					case 'S':
						{
							f_pStimSet = create_stimset<SFStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters);
							break;
						}
					case 'C':
						{
							f_pStimSet = create_stimset<ContrastStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters);
							break;
						}
					case 'T':
						{
							f_pStimSet = create_stimset<TFStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters);
							break;
						}
					case 'O':
						{
							f_pStimSet = create_stimset<OrientationStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters);
							break;
						}
					case 'A':
						{
							f_pStimSet = create_stimset<AreaStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters);
							break;
						}
					case 'P':
						{
							double tf = tuning_parameters[0];
							tuning_parameters.erase(tuning_parameters.begin());
							if (have_fixpt)
							{
								f_pStimSet = new CounterphaseStimSet(f_fixpt, f_grating, tuning_parameters, tf, bStepTW);
							}
							else
							{
								f_pStimSet = new CounterphaseStimSet(f_grating, tuning_parameters, tf, bStepTW);
							}
							break;
						}
					default:
						{
							cerr << "ERROR - unhandled tuning curve type (" << (char)c << ")" << endl;
							errflg++;
							break;
						}
					}
				}
			}
			break;
		}
	case 'R':
	case 'B':
		{
			vector<int> list;
			int nterms;
			const char *sequence = NULL;
			bool balanced = (c=='B');

			if (!have_stim)
			{
				cerr << "Error - must specify a grating stim (-s) before specifying a CRG stimulus." << endl;
				errflg++;
			}
			else if (parse_int_list(arg, list) || list.size() != 3)
			{
				cerr << "Bad format in repeating arg. Should be 3 ints: frames_per_term,first_term(0...),nterms" << endl;
				errflg++;
			}
			else
			{
				sequence = get_msequence();
				nterms = strlen(sequence);

				// Check that sequence args work with this sequence file
				if (nterms > 0 && list[1] > -1 && (list[1]+list[2] < nterms))
				{
					string seq;
					seq.assign(&sequence[list[1]], list[2]);

					// Create StimSet
					if (have_fixpt)
					{
						f_pStimSet = new CRGStimSet(f_fixpt, f_grating, list[0], seq, balanced);
					}
					else
					{
						f_pStimSet = new CRGStimSet(f_grating, list[0], seq, balanced);
					}
				}
			}
			break;
		}
	case 'G':
		{
			vector<double> tuning_parameters;
			vector<string> tokens;
			double ww, hh, dps;
			COLOR_TYPE color;
			tokenize(arg, tokens, ",");
			if (parse_color(tokens[0], color))
			{
				cerr << "Error - first parameter in bar list spec must be a color (" << tokens[0] << ")." << endl;
				errflg++;
			}
			else
			{
				tokens.erase(tokens.begin());
			}

			if (parse_number_list(tokens, tuning_parameters) || tuning_parameters.size() <= 3)
			{
				cerr << "Error - cannot parse drifting bar parameters color,width,height,deg_per_sec,ori1,ori2,..." << endl;
				errflg++;
			}
			else
			{
				ww = tuning_parameters[0];
				hh = tuning_parameters[1];
				dps = tuning_parameters[2];
				
				tuning_parameters.erase(tuning_parameters.begin());
				tuning_parameters.erase(tuning_parameters.begin());
				tuning_parameters.erase(tuning_parameters.begin());
				f_pStimSet = new CBarStimSet(color, ww, hh, dps, tuning_parameters);
			}
			break;
		}
	case 'D':
		{
			// Dots arg: -D color,x,y,diam,speed,density,dotsize,angle1,angle2,...
			vector<double> tuning_parameters;
			vector<string> tokens;
			COLOR_TYPE color;
			double dotx, doty, dotdiam, dotspeed, dotdensity, dotsize;
			tokenize(arg, tokens, ",");
			if (parse_color(tokens[0], color))
			{
				cerr << "Error - first parameter in bar list spec must be a color (" << tokens[0] << ")." << endl;
				errflg++;
			}
			else
			{
				tokens.erase(tokens.begin());
			}

			if (parse_number_list(tokens, tuning_parameters) || tuning_parameters.size() <= 6)
			{
				cerr << "Error - cannot parse dots parameters: color,x,y,diam,speed,density,dotsize,angle1,angle2,..." << endl;
				errflg++;
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
				// TODO: should probably do something with those. 


				if (have_fixpt)
				{
					f_pStimSet = new DotStimSet(f_fixpt, dotx, doty, color, dotdiam, dotspeed, dotdensity, (int)dotsize, tuning_parameters);
				}
				else
				{
					f_pStimSet = new DotStimSet(dotx, doty, color, dotdiam, dotspeed, dotdensity, (int)dotsize, tuning_parameters);
				}

			}
			break;
		}
	case 0:
		{
			if (!f_pStimSet)
			{
				cerr << "Error - you must specify a stimulus set of some sort (COASTg)." << endl;
				errflg++;
			}
			break;
		}
	default:
		{
			cerr << "Unknown option - " << (char)c << endl;
			errflg++;
			break;
		}
	}


	return errflg;
}

void usage()
{
	cerr << "usage: fixstim NI" << endl;
}

