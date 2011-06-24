/* $Id: fixstim.cpp,v 1.3 2011-06-24 01:51:02 djsperka Exp $ */

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
StimSet *f_pStimSetSlave = NULL;
int f_iDistanceToScreenMM = -1;
double f_spatialphase = 0;
TriggerVector triggers;
bool f_bDualVSG = false;
bool f_bDualVSGBothStim = false;
bool f_bDualVSGMasterStim = false;
bool f_bDualVSGSlaveStim = false;
bool f_bSlaveSynch = false;
string f_szOffsetFile;
double f_dSlaveXOffset=0, f_dSlaveYOffset=0;
int f_pulse = 0x40;

// function prototypes

int args(int argc, char **argv);
void usage();
void init_triggers();
int callback(int &output, const CallbackTrigger* ptrig);
template <class T> StimSet* create_stimset(bool bHaveFixpt, ARContrastFixationPointSpec& fixpt, ARGratingSpec& grating, vector<double> params, double offsetX, double offsetY, double spatialphase);

int main (int argc, char *argv[])
{
	std::vector<int> pages;
	int i;

	// Check input arguments
	if (args(argc, argv))
	{
		return 1;
	}

	if (f_dumpStimSetsOnly)
	{
		if (f_bDualVSG)
		{
			cout << *f_pStimSet << endl;
			cout << *f_pStimSetSlave << endl;
		}
		else
		{
			cout << *f_pStimSet << endl;
		}
		return 1;
	}

	// Initialize vsg or vsg's depending on whether this is a dual vsg run (-D)
	if (!f_bDualVSG)
	{

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

	}
	else
	{
		if (ARvsg::master().init(f_iDistanceToScreenMM, f_background))
		{
			cerr << "VSG init for master failed!" << endl;
			return -1;
		}
		ARvsg::master().select();
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);

		if (f_verbose)
			cerr << "Configuring " << f_pStimSet->num_pages() << " pages." << endl;
		for (i=0; i<f_pStimSet->num_pages(); i++) pages.push_back(i+1);
		if (f_pStimSet->init(ARvsg::master(), pages))
		{
			cerr << "Master StimSet initialization failed." << endl;
			return -1;
		}

		if (ARvsg::slave().init(f_iDistanceToScreenMM, f_background, true, f_bSlaveSynch))
		{
			cerr << "VSG init for slave failed!" << endl;
			return -1;
		}
		ARvsg::slave().select();
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);

		pages.clear();
		if (f_verbose)
			cerr << "Configuring " << f_pStimSetSlave->num_pages() << " pages." << endl;
		for (i=0; i<f_pStimSetSlave->num_pages(); i++) pages.push_back(i+1);
		if (f_pStimSetSlave->init(ARvsg::slave(), pages))
		{
			cerr << "Slave StimSet initialization failed." << endl;
			return -1;
		}

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
	if (!f_bDualVSG)
	{
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);
		ARvsg::instance().ready_pulse(100, f_pulse);
	}
	else
	{
		ARvsg::master().select();
		ARvsg::master().ready_pulse(250, f_pulse);
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);
		ARvsg::slave().select();
		ARvsg::slave().ready_pulse(250, f_pulse);
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);
	}

	// All right, start monitoring triggers........
	int last_output_trigger=0;
	long input_trigger = 0;
	string s;


	if (!f_bDualVSG)
	{
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
				vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
				vsgPresent();
			}
			Sleep(10);
		}

		ARvsg::instance().clear();
	}
	else
	{
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
				ARvsg::master().select();
				input_trigger = vsgIOReadDigitalIn();
			}

			TriggerFunc	tf = std::for_each(triggers.begin(), triggers.end(), 
				(f_binaryTriggers ? TriggerFunc(input_trigger, last_output_trigger) : TriggerFunc(s, last_output_trigger)));

			// Now analyze input trigger
		 	
			if (tf.quit()) break;
			else if (tf.present())
			{	
				// For the dual vsg mode we just do present on both vsgs.
				last_output_trigger = tf.output_trigger();
				cout << "master select...";
				ARvsg::master().select();
				vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
				cout << "master present...";
				vsgPresent();

				cout << "slave select...";
				ARvsg::slave().select();
				vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
				cout << "slave present...";
				vsgPresent();
				cout << "done." << endl;
			}

			Sleep(100);
		}
		ARvsg::master().select();
		ARvsg::master().clear();
		ARvsg::slave().select();
		ARvsg::slave().clear();
	}

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
		if (!f_bDualVSG)
		{
			ival = f_pStimSet->handle_trigger(key);
		}
		else
		{
			ival = 0;
			cout << "cb master select..." << endl;
			ARvsg::master().select();
			cout << "cb master handle trigger..." << endl;
			ival = f_pStimSet->handle_trigger(key);
			cout << "cb master done." << endl;
			cout << "cb slave select..." << endl;
			ARvsg::slave().select();
			cout << "cb slave handle trigger..." << endl;
			ival |= f_pStimSetSlave->handle_trigger(key);
			cout << "cv slave done." << endl;
		}
	}
	else
	{
		cerr << "ERROR! Unknown trigger sent to callback!" << endl;
	}

	return ival;
}

template <class T>
StimSet* create_stimset(bool bHaveFixpt, ARContrastFixationPointSpec& fixpt, ARGratingSpec& grating, vector<double> params, double offsetX, double offsetY, double spatialphase)
{
	StimSet *pstimset=(StimSet *)NULL;
	ARContrastFixationPointSpec f(fixpt);
	ARGratingSpec g(grating);
	g.x += offsetX;
	g.y += offsetY;
	if (bHaveFixpt)
	{
		f.x += offsetX;
		f.y += offsetY;
		pstimset = new T(f, g, params, spatialphase);
	}
	else
	{
		pstimset = new T(g, params, spatialphase);
	}
	return pstimset;
}

int args(int argc, char **argv)
{	
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	bool have_fixpt = false;
	bool have_stim = false;
	bool have_g = false;
	bool have_d = false;
	string sequence_filename;
	bool have_F = false;
	char activeScreen = ' ';
	while ((c = getopt(argc, argv, "f:b:d:avg:s:C:T:S:O:A:R:B:F:DMVr:H:Zp:KP:")) != -1)
	{
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
		case 'K':
			f_bSlaveSynch = true;
			break;
		case 'b': 
			s.assign(optarg);
			if (parse_color(s, f_background)) errflg++; 
			break;
		case 'd':
			s.assign(optarg);
			if (parse_distance(s, f_iDistanceToScreenMM)) errflg++;
			else have_d=true;
			break;
		case 'f':
			s.assign(optarg);
			if (parse_fixation_point(s, f_fixpt)) errflg++;
			else 
			{
				have_fixpt = true;
			}
			break;
		case 'g':
			s.assign(optarg);
			if (parse_grating(s, f_grating)) errflg++;
			else 
			{
				switch (activeScreen)
				{
				case 'M':
				case ' ':
					if (!have_fixpt)
					{
						f_pStimSet = new GratingStimSet(f_grating, 0, 0, f_spatialphase);
					}
					else
					{
						f_pStimSet = new FixptGratingStimSet(f_fixpt, f_grating, 0, 0, f_spatialphase);
					}
					break;
				case 'V':
					if (!have_fixpt)
					{
						f_pStimSetSlave = new GratingStimSet(f_grating, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
					}
					else
					{
						f_pStimSetSlave = new FixptGratingStimSet(f_fixpt, f_grating, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
					}
					break;
				case 'D':
					if (activeScreen != ' ' && !f_bDualVSG)
					{
						cerr << "Error - cannot specify active screen (-[D|M|V]) without also specifying slave screen offset!" << endl;
						errflg++;
					}
					else if (!have_fixpt)
					{
						f_pStimSet = new GratingStimSet(f_grating, 0, 0, f_spatialphase);
						f_pStimSetSlave = new GratingStimSet(f_grating, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
					}
					else
					{
						f_pStimSet = new FixptGratingStimSet(f_fixpt, f_grating, 0, 0, f_spatialphase);
						f_pStimSetSlave = new FixptGratingStimSet(f_fixpt, f_grating, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
					}
					break;
				}
			}
			break;
		case 's':
			s.assign(optarg);
			if (parse_grating(s, f_grating)) 
				errflg++;
			else 
				have_stim = true;
			break;
		case 'p':
			s.assign(optarg);
			if (parse_integer(s, f_pulse))
				errflg++;
			break;
		case 'P':
			s.assign(optarg);
			if (parse_double(s, f_spatialphase))
				errflg++;
			break;
		case 'O':
		case 'T':
		case 'S':
		case 'C':
		case 'A':
		case 'H':
			{
				vector<double> tuning_parameters;
				int nsteps;

				s.assign(optarg);
				if (parse_tuning_list(s, tuning_parameters, nsteps)) errflg++;
				else 
				{
					if (!have_stim)
					{
						cerr << "Error - must pass template grating stimulus with \"-s\" before passing tuning parameters." << endl;
						errflg++;
					}
					else if (activeScreen != ' ' && !f_bDualVSG)
					{
						cerr << "Error - cannot specify active screen (-[D|M|V]) without also specifying slave screen offset!" << endl;
						errflg++;
					}
					else
					{
						switch (c)
						{
						case 'S':
							{
								switch (activeScreen)
								{
								case 'M':
								case ' ':
									f_pStimSet = create_stimset<SFStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									break;
								case 'V':
									f_pStimSetSlave = create_stimset<SFStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
									break;
								case 'D':
									f_pStimSet = create_stimset<SFStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									f_pStimSetSlave = create_stimset<SFStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
									break;
								}
								break;
							}
						case 'C':
							{
								switch (activeScreen)
								{
								case 'M':
								case ' ':
									f_pStimSet = create_stimset<ContrastStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									break;
								case 'V':
									f_pStimSetSlave = create_stimset<ContrastStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
									break;
								case 'D':
									f_pStimSet = create_stimset<ContrastStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									f_pStimSetSlave = create_stimset<ContrastStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
									break;
								}
								break;
							}
						case 'T':
							{
								switch (activeScreen)
								{
								case 'M':
								case ' ':
									f_pStimSet = create_stimset<TFStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									break;
								case 'V':
									f_pStimSetSlave = create_stimset<TFStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
									break;
								case 'D':
									f_pStimSet = create_stimset<TFStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									f_pStimSetSlave = create_stimset<TFStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
									break;
								}
								break;
							}
						case 'O':
							{
								switch (activeScreen)
								{
								case 'M':
								case ' ':
									f_pStimSet = create_stimset<OrientationStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									break;
								case 'V':
									f_pStimSetSlave = create_stimset<OrientationStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
									break;
								case 'D':
									f_pStimSet = create_stimset<OrientationStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									f_pStimSetSlave = create_stimset<OrientationStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
									break;
								}
								break;
							}
						case 'A':
							{
								switch (activeScreen)
								{
								case 'M':
								case ' ':
									f_pStimSet = create_stimset<AreaStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									break;
								case 'V':
									f_pStimSetSlave = create_stimset<AreaStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
									break;
								case 'D':
									f_pStimSet = create_stimset<AreaStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									f_pStimSetSlave = create_stimset<AreaStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
									break;
								}
								break;
							}
						case 'H':
							{
								switch (activeScreen)
								{
								case 'M':
								case ' ':
									f_pStimSet = create_stimset<DonutStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									break;
								case 'V':
									f_pStimSetSlave = create_stimset<DonutStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
									break;
								case 'D':
									f_pStimSet = create_stimset<DonutStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									f_pStimSetSlave = create_stimset<DonutStimSet>(have_fixpt, f_fixpt, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
									break;
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
				char *sequence = NULL;
				bool balanced = (c=='B');

				s.assign(optarg);
				if (parse_int_list(s, list) || list.size() != 3)
				{
					cerr << "Bad format in repeating arg. Should be 3 ints: frames_per_term,first_term(0...),nterms" << endl;
					errflg++;
				}
				else
				{
					if (have_F)
					{
						// Open file and load sequence

						nterms = arutil_load_sequence(&sequence, sequence_filename);

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

							free(sequence);
						}
						else
						{
							cerr << "Error - cannot get sequence (start term " << list[1] << ", nterms " << list[2] << ") from sequence file " << sequence_filename << ". The sequence file has only " << nterms << " terms." << endl;
							errflg++;
						}
					}
					else
					{
						cerr << "Error - must supply sequence file (\"-F\") before CRG argument (\"-R\")" << endl;
						errflg++;
					}
				}
				break;
			}
		case 'F':
			{
				sequence_filename.assign(optarg);
				have_F = true;
				break;
			}
		case 'D':
			{
				activeScreen = 'D';
				break;
			}
		case 'M':
			{
				activeScreen = 'M';
				break;
			}
		case 'V':
			{
				activeScreen = 'V';
				break;
			}
		case 'r':
			{
				ifstream in;
				f_szOffsetFile.assign(optarg);
				f_bDualVSG = true;
				in.open(f_szOffsetFile.c_str());
				if (in)
				{
					in >> f_dSlaveXOffset >> f_dSlaveYOffset;
					if (!in)
					{
						cerr << "Format error in offset file!" << endl;
						errflg++;
					}
				}
				else
				{
					cerr << "Cannot open offset file (" << f_szOffsetFile << ")." << endl;
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
	}

	if (f_bDualVSG)
	{
		if (!f_pStimSet && !f_pStimSetSlave)
		{
			cerr << "Error - when using dual VSG mode you must specify one of D/M/V options and a stimulus set (COASTg)." << endl;
			errflg++;
		}
		else if (!f_pStimSetSlave)
		{
			f_pStimSetSlave = new FixptGratingStimSet(f_fixpt, f_dSlaveXOffset, f_dSlaveYOffset);
		}
		else if (!f_pStimSet)
		{
			f_pStimSet = new FixptGratingStimSet(f_fixpt, 0, 0);
		}
	}
	else
	{
		if (!f_pStimSet)
		{
			cerr << "Error - you must specify a stimulus set of some sort (COASTg)." << endl;
			errflg++;
		}
	}

	return errflg;
}

void usage()
{
	cerr << "usage: fixstim NI" << endl;
}

