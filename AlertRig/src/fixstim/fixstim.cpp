/* $Id: fixstim.cpp,v 1.22 2014-05-29 21:01:00 devel Exp $ */

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
vector<ARGratingSpec> f_vecGratings;
vector<AttentionCue> f_vecAttentionCues;
//ARGratingSpec f_grating2;
//bool have_second = false;
//ARGratingSpec f_grating3;
//bool have_third = false;
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
	status = prargs(argc, argv, prargs_callback, "f:b:d:avg:s:C:T:S:O:A:P:R:B:L:e:H:zp:G:D:wJ:Z:Q:", 'F');
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

	// reset all triggers if using binary triggers
	if (f_binaryTriggers) triggers.reset(vsgIOReadDigitalIn());


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
	triggers.addTrigger(new CallbackTrigger("u", 0x20, 0x20|AR_TRIGGER_TOGGLE, 0x10, 0x10|AR_TRIGGER_TOGGLE, callback));
	triggers.addTrigger(new CallbackTrigger("v", 0x40, 0x40|AR_TRIGGER_TOGGLE, 0x20, 0x20|AR_TRIGGER_TOGGLE, callback));

	// quit trigger
	triggers.addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));

	return;
}

int callback(int &output, const CallbackTrigger* ptrig)
{
	int ival=1;
	string key = ptrig->getKey();

	if (key == "S" || key == "F" || key=="X" || key=="a" || key=="s" || key=="u" || key=="v")
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
	static alert::ARGratingSpec hole;
	static bool have_hole = false;
	static bool have_sequence = false;	// command-line sequence of page numbers 0,1...
	static string the_sequence;			// see above.

	switch(c)
	{
	case 'a':
		f_binaryTriggers = false;
		break;
	case 'z':
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
		}
		break;
	case 's':
		if (f_vecGratings.size() == 8)
		{
			cerr << "Maximum number of gratings(8) reached." << endl;
			errflg++;
		}
		else if (parse_grating(arg, f_grating)) 
		{
			cerr << "Error in grating input: " << arg << endl;
			errflg++;
		}
		else 
		{
			have_stim = true;
			f_vecGratings.push_back(f_grating);
		}
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
	case 'P':
	case 'H':
	case 'Z':
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
					case 'H':
						{
							// must have two gratings specified, it is assumed that the first is the donut, second is the hole. 
							if (have_fixpt)
							{
								if (f_vecGratings.size() == 2)
									f_pStimSet = new DanishStimSet(f_fixpt, f_vecGratings[0], f_vecGratings[1], tuning_parameters);
								else
									f_pStimSet = new DanishStimSet(f_fixpt, f_vecGratings[0], tuning_parameters);
							}
							else
							{
								if (f_vecGratings.size() == 2)
								{
									f_pStimSet = new DanishStimSet(f_vecGratings[0], f_vecGratings[1], tuning_parameters);
								}
								else
								{
									f_pStimSet = new DanishStimSet(f_vecGratings[0], tuning_parameters);
								}
							}
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
					case 'Z':
						{
							f_pStimSet = create_stimset<PositionStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters);
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
			vector<double> list;
			int nterms;
			int iunused;
			const char *sequence = NULL;
			bool balanced = (c=='B');

			if (!have_stim)
			{
				cerr << "Error - must specify a grating stim (-s) before specifying a CRG stimulus." << endl;
				errflg++;
			}
			else if (parse_tuning_list(arg, list, iunused) || list.size() < 3)
			{
				cerr << "Bad format in repeating arg. Should be at least 3 ints: frames_per_term,first_term(0...),nterms[,contrast0,contrast1,...]" << endl;
				errflg++;
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
				if (!have_sequence)
				{
					sequence = get_msequence();
					nterms = strlen(sequence);
				}
				else
				{
					sequence = the_sequence.c_str();
					nterms = the_sequence.length();
				}

				// Check that sequence args work with this sequence file
				if (nterms > 0 && first > -1 && (first+length < nterms))
				{
					string seq;
					seq.assign(&sequence[first], length);

					// Create StimSet
					if (have_fixpt)
					{
						if (list.size() > 0)
						{
							f_pStimSet = new CRGStimSet(f_fixpt, f_grating, fpt, seq, list, balanced);
						}
						else
						{
							f_pStimSet = new CRGStimSet(f_fixpt, f_grating, fpt, seq, balanced);
						}
					}
					else
					{
						if (list.size() > 0)
						{
							f_pStimSet = new CRGStimSet(f_grating, fpt, seq, list, balanced);
						}
						else
						{
							f_pStimSet = new CRGStimSet(f_grating, fpt, seq, balanced);
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
			const char *sequence = NULL;
			string seq;
			vector<string> tokens;

			if (!have_sequence)
			{
				sequence = get_msequence();
				nterms = strlen(sequence);
			}
			else
			{
				sequence = the_sequence.c_str();
				nterms = the_sequence.length();
			}

			tokenize(arg, tokens, ",");
			if (tokens.size() == 1)
			{
				if (parse_integer(tokens[0], fpt))
				{
					cerr << "Error in flash spec (-L): first arg must be an integer (fpt)." << endl;
					errflg++;
					break;
				}
				else
				{
					seq.assign(sequence);	// use the whole sequence
					if (have_fixpt)
					{
						f_pStimSet = new FlashStimSet(f_fixpt, fpt, seq);
					}
					else
					{
						f_pStimSet = new FlashStimSet(fpt, seq);
					}
				}
			}
			else if (tokens.size() < 3)
			{
				cerr << "Error in Flash arg (-L): must specify fpt,first,nterms[[color0,color1[,color2...]]]" << endl;
				errflg++;
			}
			else
			{
				// first token must be an integer.
				// If second token is an integer, then the third must also be an integer. Any remaining tokens must be colors.
				// If the second token is NOT an integer, then all remaining tokens must be colors (and entire sequence is used).
				if (parse_integer(tokens[0], fpt))
				{
					cerr << "Error in Flash arg (-L): must specify fpt,first,nterms[[color0,color1[,color2...]]]" << endl;
					errflg++;
				}
				else
				{
					unsigned int first_color_index;

					// If command line has a sequence "-e" argument, the use it; otherwise use the default msequence.
					if (!have_sequence)
					{
						sequence = get_msequence();
						nterms = strlen(sequence);
					}
					else
					{
						sequence = the_sequence.c_str();
						nterms = the_sequence.length();
					}
					if (parse_integer(tokens[1], first) || parse_integer(tokens[2], length))
					{
						first = 0;
						nterms = length = strlen(sequence);
						first_color_index = 1;
					}
					else
					{
						first_color_index = 3;
					}

					// Check that sequence args work with this sequence
					if (nterms > 0 && first > -1 && (first+length <= nterms))
					{
						seq.assign(&sequence[first], length);
					}
					else
					{
						cerr << "Error in flash spec: check sequence length and first,nterms args." << endl;
						errflg++;
						break;
					}

					// now parse colors if necessary
					if (first_color_index < tokens.size())
					{
						vector<COLOR_TYPE> colors;
						COLOR_TYPE color;
						for (unsigned int i = first_color_index; i<tokens.size(); i++)
						{
							if (parse_color(tokens[i], color))
							{
								cerr << "Error in flash spec: bad color format at token " << i << ":" << tokens[i] << endl;
								errflg++;
							}
							else
							{
								colors.push_back(color);
							}
						}
						if (have_fixpt)
						{
							f_pStimSet = new FlashStimSet(f_fixpt, colors, fpt, seq);
						}
						else
						{
							f_pStimSet = new FlashStimSet(colors, fpt, seq);
						}
					}
					else
					{
						if (have_fixpt)
						{
							f_pStimSet = new FlashStimSet(f_fixpt, fpt, seq);
						}
						else
						{
							f_pStimSet = new FlashStimSet(fpt, seq);
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
			if (parse_sequence(arg, the_sequence))
			{
				cerr << "Error - bad sequence format (-e)." << endl;
				errflg++;
			}
			else
			{
				have_sequence = true;
				cerr << "Got sequence of " << the_sequence.length() << " terms." << endl;
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
	case 'Q':
		{
			if (parse_attcues(arg, f_vecGratings.size(), f_vecAttentionCues))
			{
				cerr << "Error in input." << endl;
				errflg++;
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
			double tCC;
			vector<AttParams> vecInput;
			if (parse_attparams(arg, f_vecGratings.size(), vecInput, tCC))
			{
				cerr << "Error in input." << endl;
				errflg++;
			}
			else
			{
				if (have_fixpt)
				{
					if (f_vecGratings.size() > 0)
					{
						if (f_vecAttentionCues.size() == 0)
						{
							f_pStimSet = new AttentionStimSet(f_fixpt, tCC, f_vecGratings, vecInput);
						}
						else
						{
							f_pStimSet = new AttentionStimSet(f_fixpt, tCC, f_vecGratings, f_vecAttentionCues, vecInput);
						}
					}
					else
					{
						cerr << "Error in input for Attention stim - no gratings specified!" << endl;
						errflg++;
					}
				}
				else
				{
					cerr << "Error input for Attention stim - no fixpt specified!" << endl;
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

