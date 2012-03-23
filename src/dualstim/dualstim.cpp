/* $Id: dualstim.cpp,v 1.7 2012-03-23 22:18:21 devel Exp $ */

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
#include "StimSetFGX.h"
#include "StimSetCRG.h"
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
COLOR_TYPE f_background = { gray, {0.5, 0.5, 0.5}};
StimSetBase *f_pstimset = NULL;
StimSetBase *f_pstimsetSlave = NULL;
int f_iDistanceToScreenMM = -1;
double f_spatialphase = 0;
TriggerVector f_triggers;
bool f_bSlaveSynch = false;
string f_szOffsetFile;
double f_dSlaveXOffset=0, f_dSlaveYOffset=0;
int f_pulse = 0x40;
int f_nFramesDelay = 3;
double f_dStimTime = 0;
string f_szResponseFile;
bool f_bHaveResponseFile = false;

// function prototypes

int prargs_callback(int c, string& arg);
void usage();
void init_triggers();
int callback(int &output, const CallbackTrigger* ptrig);


int main (int argc, char *argv[])
{
	int status = 0;

	// Check input arguments
	status = prargs(argc, argv, prargs_callback, "f:b:d:avg:s:C:T:S:O:A:DMVr:H:Zp:Ky:t:X:Y:h:nR:", 'F');
	if (status)
	{
		return -1;
	}

	if (f_verbose)
		cout << "Initializing master VSG..." << endl;
	if (ARvsg::master().init(f_iDistanceToScreenMM, f_background))
	{
		cerr << "VSG init for master failed!" << endl;
		return -1;
	}

	if (f_verbose)
		cout << "Initializing slave VSG..." << endl;
	if (ARvsg::slave().init(f_iDistanceToScreenMM, f_background, true, f_bSlaveSynch))
	{
		cerr << "VSG init for slave failed!" << endl;
		return -1;
	}

	if (f_verbose)
		cout << "Initializing stim..." << endl;
	f_pstimset->init(ARvsg::master());
	f_pstimsetSlave->init(ARvsg::slave());

	ARvsg::master().select();

	// Initialize triggers
	init_triggers();
	if (f_verbose)
	{
		for (unsigned int i=0; i<f_triggers.size(); i++)
		{
			std::cout << "Trigger " << i << " " << *(f_triggers[i]) << std::endl;
		}
	}

	// Pulse
	ARvsg::master().select();
	ARvsg::master().ready_pulse(250, f_pulse);
	ARvsg::slave().select();
	ARvsg::slave().ready_pulse(250, f_pulse);

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
			ARvsg::master().select();
			input_trigger = vsgIOReadDigitalIn();
		}

		TriggerFunc	tf = std::for_each(f_triggers.begin(), f_triggers.end(), 
			(f_binaryTriggers ? TriggerFunc(input_trigger, last_output_trigger) : TriggerFunc(s, last_output_trigger)));

		// Now analyze input trigger
	 	
		if (tf.quit()) break;
		else if (tf.present())
		{	
			// For the dual vsg mode we just do present on both vsgs.
			last_output_trigger = tf.output_trigger();
			ARvsg::master().select();
			vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
			vsgPresent();

			ARvsg::slave().select();
			vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
			vsgPresent();
		}

		Sleep(100);
	}
	ARvsg::master().select();
	ARvsg::master().clear();
	vsgPresent();
	ARvsg::slave().select();
	ARvsg::slave().clear();
	vsgPresent();

	return 0;
}


int callback(int &output, const CallbackTrigger* ptrig)
{
	int status=0;
	ARvsg::master().select();
	status = f_pstimset->handle_trigger(ptrig->getKey());
	ARvsg::slave().select();
	status |= f_pstimsetSlave->handle_trigger(ptrig->getKey());
	return status;

}

void init_triggers()
{
	f_triggers.addTrigger(new CallbackTrigger("F", 0x2, 0x2, 0x2, 0x2, callback));
	f_triggers.addTrigger(new CallbackTrigger("S", 0x4, 0x4, 0x4, 0x4, callback));
	f_triggers.addTrigger(new CallbackTrigger("s", 0x4, 0x0, 0x4, 0x0, callback));
	f_triggers.addTrigger(new CallbackTrigger("X", 0x6, 0x0, 0x6, 0x0, callback));
	f_triggers.addTrigger(new CallbackTrigger("a", 0x8, 0x8|AR_TRIGGER_TOGGLE, 0x8, 0x8|AR_TRIGGER_TOGGLE, callback));

	// quit trigger
	f_triggers.addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));

	return;
}


int prargs_callback(int c, string& arg)
{	
	string s;
	static ARContrastFixationPointSpec fixpt;
	static ARXhairSpec xhair;
	static ARGratingSpec grating;
	static int errflg = 0;
	static bool have_fixpt = false;
	static bool have_xhair = false;
	static bool have_grating = false;
	static bool have_g = false;
	static bool have_d = false;
	static bool have_w = false;
	static bool have_t = false;
	static bool have_offset = false;
	int delay=0, frames=0;
	string sequence_filename;
	static char activeScreen = ' ';
	static StimSetFGX *pSSFGX = NULL;
	static StimSetFGX *pSSFGXSlave = NULL;

	switch(c)
	{
	case 'a':
		f_binaryTriggers = false;
		break;
	case 'F':
		f_bHaveResponseFile = true;
		f_szResponseFile.assign(optarg);
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
		if (parse_fixation_point(s, fixpt)) errflg++;
		else 
		{
			have_fixpt = true;
		}
		break;
	case 'n':
		// Trigger creation of FGX stim set. 
		switch (activeScreen)
		{
		case 'M':
		case ' ':
			if (!pSSFGX) 
			{
				pSSFGX = new StimSetFGX();
				f_pstimset = pSSFGX;
			}
			if (have_fixpt) pSSFGX->set_fixpt(fixpt);
			if (have_xhair) pSSFGX->set_xhair(xhair);
			cout << "Setting master stimset without grating" << endl;
			break;
		case 'V':
			// For slave stimset, must add offset values to stim, fixpt, xhair. 
			if (!pSSFGXSlave) 
			{
				pSSFGXSlave = new StimSetFGX();
				f_pstimsetSlave = pSSFGXSlave;
			}
			if (have_fixpt) pSSFGXSlave->set_fixpt(fixpt, f_dSlaveXOffset, f_dSlaveYOffset);
			if (have_xhair) pSSFGXSlave->set_xhair(xhair, f_dSlaveXOffset, f_dSlaveYOffset);
			cout << "Setting slave stimset without grating" << endl;
			break;
		case 'D':
			if (activeScreen != ' ' && !have_offset)
			{
				cerr << "Error - cannot specify active screen (-[D|M|V]) without also specifying slave screen offset!" << endl;
				errflg++;
			}
			else
			{
				if (!pSSFGX) 
				{
					pSSFGX = new StimSetFGX();
					f_pstimset = pSSFGX;
				}
				if (have_fixpt) pSSFGX->set_fixpt(fixpt);
				if (have_xhair) pSSFGX->set_xhair(xhair);
				if (!pSSFGXSlave) 
				{
					pSSFGXSlave = new StimSetFGX();
					f_pstimsetSlave = pSSFGXSlave;
				}
				if (have_fixpt) pSSFGXSlave->set_fixpt(fixpt, f_dSlaveXOffset, f_dSlaveYOffset);
				if (have_xhair) pSSFGXSlave->set_xhair(xhair, f_dSlaveXOffset, f_dSlaveYOffset);
			}
			break;
		}
		break;
	case 'g':
		s.assign(optarg);
		if (parse_grating(s, grating)) 
		{
			cerr << "Error: cannot parse grating spec: " << string(optarg) << endl;
			errflg++;
		}
		else 
		{
			switch (activeScreen)
			{
			case 'M':
			case ' ':
				if (!pSSFGX) 
				{
					pSSFGX = new StimSetFGX();
					f_pstimset = pSSFGX;
				}
				if (have_fixpt) pSSFGX->set_fixpt(fixpt);
				if (have_xhair) pSSFGX->set_xhair(xhair);
				cout << "Setting master stimset grating to this:" << endl << grating << endl;
				pSSFGX->set_grating(grating);
				break;
			case 'V':
				// For slave stimset, must add offset values to stim, fixpt, xhair. 
				if (!pSSFGXSlave) 
				{
					pSSFGXSlave = new StimSetFGX();
					f_pstimsetSlave = pSSFGXSlave;
				}
				if (have_fixpt) pSSFGXSlave->set_fixpt(fixpt, f_dSlaveXOffset, f_dSlaveYOffset);
				if (have_xhair) pSSFGXSlave->set_xhair(xhair, f_dSlaveXOffset, f_dSlaveYOffset);
				cout << "Setting slave stimset grating to this:" << endl << grating << endl;
				pSSFGXSlave->set_grating(grating, f_dSlaveXOffset, f_dSlaveYOffset);
				break;
			case 'D':
				if (activeScreen != ' ' && !have_offset)
				{
					cerr << "Error - cannot specify active screen (-[D|M|V]) without also specifying slave screen offset!" << endl;
					errflg++;
				}
				else
				{
					if (!pSSFGX) 
					{
						pSSFGX = new StimSetFGX();
						f_pstimset = pSSFGX;
					}
					if (have_fixpt) pSSFGX->set_fixpt(fixpt);
					if (have_xhair) pSSFGX->set_xhair(xhair);
					pSSFGX->set_grating(grating);
					if (!pSSFGXSlave) 
					{
						pSSFGXSlave = new StimSetFGX();
						f_pstimsetSlave = pSSFGXSlave;
					}
					if (have_fixpt) pSSFGXSlave->set_fixpt(fixpt, f_dSlaveXOffset, f_dSlaveYOffset);
					if (have_xhair) pSSFGXSlave->set_xhair(xhair, f_dSlaveXOffset, f_dSlaveYOffset);
					pSSFGXSlave->set_grating(grating, f_dSlaveXOffset, f_dSlaveYOffset);
				}
				break;
			}
		}
		break;
	case 's':
		s.assign(optarg);
		if (parse_grating(s, grating)) 
			errflg++;
		else 
			have_grating = true;
		break;
	case 'p':
		s.assign(optarg);
		if (parse_integer(s, f_pulse))
			errflg++;
		break;
	case 'C':
	case 'O':
	case 'A':
	case 'S':
	case 'T':
	case 'H':
	case 'X':
	case 'Y':
	case 'Z':
		{
			vector<double> tuning_parameters;
			int nsteps;

			s.assign(optarg);
			if (parse_tuning_list(s, tuning_parameters, nsteps)) errflg++;
			else 
			{
				if (!have_grating)
				{
					cerr << "Error - must pass template grating stimulus with \"-s\" before passing tuning parameters." << endl;
					errflg++;
				}
				else if (activeScreen != ' ' && !have_offset)
				{
					cerr << "Error - cannot specify active screen (-[D|M|V]) without also specifying slave screen offset!" << endl;
					errflg++;
				}
				else
				{
					// Create a stim parameter set
					StimParameterList *plist;
					switch(c)
					{
					case 'C':
						plist = new StimContrastList(tuning_parameters);
						break;
					case 'O':
						plist = new StimOrientationList(tuning_parameters);
						break;
					case 'A':
						plist = new StimAreaList(tuning_parameters);
						break;
					case 'S':
						plist = new StimSFList(tuning_parameters);
						break;
					case 'T':
						plist = new StimTFList(tuning_parameters);
						break;
					case 'H':
						plist = new StimHoleList(tuning_parameters);
						break;
					case 'X':
						plist = new StimXList(tuning_parameters);
						break;
					case 'Y':
						plist = new StimYList(tuning_parameters);
						break;
					case 'Z':
						plist = new StimXYList(tuning_parameters);
						break;
					default:
						cerr << "Unhandled varying stim parameter type (" << (char)c << ")" << endl;
						errflg++;
					}

					// Initialize stimset(s) and add the StimParameterList. Note that in 
					// some cases we'll be initializing stimset(s) multiple times (if multiple
					// stim lists are generated for either or both screens.
					//
					// The appearance of a one of the StimParameterList args (COASTHXY) triggers
					// initialization of the stim set. The same is true when "-g" is encountered. 

					switch (activeScreen)
					{
					case 'M':
					case ' ':
						if (!pSSFGX) 
						{
							pSSFGX = new StimSetFGX();
							f_pstimset = pSSFGX;
						}
						if (have_fixpt) pSSFGX->set_fixpt(fixpt);
						if (have_xhair) pSSFGX->set_xhair(xhair);
						if (have_grating) pSSFGX->set_grating(grating);
						pSSFGX->push_back(plist);
						break;
					case 'V':
						// For slave stimset, must add offset values to stim, fixpt, xhair. 
						if (!pSSFGXSlave) 
						{
							pSSFGXSlave = new StimSetFGX();
							f_pstimsetSlave = pSSFGXSlave;
						}
						if (have_fixpt) pSSFGXSlave->set_fixpt(fixpt, f_dSlaveXOffset, f_dSlaveYOffset);
						if (have_xhair) pSSFGXSlave->set_xhair(xhair, f_dSlaveXOffset, f_dSlaveYOffset);
						if (have_grating) pSSFGXSlave->set_grating(grating, f_dSlaveXOffset, f_dSlaveYOffset);
						pSSFGXSlave->push_back(plist);
						break;
					case 'D':
						if (activeScreen != ' ' && !have_offset)
						{
							cerr << "Error - cannot specify active screen (-[D|M|V]) without also specifying slave screen offset!" << endl;
							errflg++;
						}
						else
						{
							if (!pSSFGX) 
							{
								pSSFGX = new StimSetFGX();
								f_pstimset = pSSFGX;
							}
							if (have_fixpt) pSSFGX->set_fixpt(fixpt);
							if (have_xhair) pSSFGX->set_xhair(xhair);
							if (have_grating) pSSFGX->set_grating(grating);
							pSSFGX->push_back(plist->clone());

							if (!pSSFGXSlave) 
							{
								pSSFGXSlave = new StimSetFGX();
								f_pstimsetSlave = pSSFGXSlave;
							}
							if (have_fixpt) pSSFGXSlave->set_fixpt(fixpt, f_dSlaveXOffset, f_dSlaveYOffset);
							if (have_xhair) pSSFGXSlave->set_xhair(xhair, f_dSlaveXOffset, f_dSlaveYOffset);
							if (have_grating) pSSFGXSlave->set_grating(grating, f_dSlaveXOffset, f_dSlaveYOffset);
							pSSFGXSlave->push_back(plist);
						}
						break;
					}
				}
			}
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
			s.assign(optarg);

			// First attempt to read two doubles from the string
			if (parse_xy(s, f_dSlaveXOffset, f_dSlaveYOffset))
			{
				f_szOffsetFile.assign(optarg);
				have_offset = true;
				in.open(f_szOffsetFile.c_str());
				if (in)
				{
					in >> f_dSlaveXOffset >> f_dSlaveYOffset;
					if (!in)
					{
						cerr << "Format error in offset file!" << endl;
						errflg++;
					}
					else
					{
						cout << "Got slave offset (" << f_dSlaveXOffset << ", " << f_dSlaveYOffset << ") from file (" << f_szOffsetFile << ")." << endl;
					}
				}
				else
				{
					cerr << "Cannot open offset file (" << f_szOffsetFile << ")." << endl;
					errflg++;
				}
			}
			else
			{
				have_offset = true;
				cout << "Got slave offset (" << f_dSlaveXOffset << ", " << f_dSlaveYOffset << ") from arg list." << endl;
			}
			break;
		}
	case 't':
		{
			s.assign(optarg);
			if (parse_double(s, f_dStimTime))
			{
				cerr << "Bad format for stim time (-t): should be time(in sec)" << endl;
				errflg++;
			}
			else
				have_t = true;
			break;
		}
	case 'y':
		{
			s.assign(optarg);
			if (parse_integer(s, f_nFramesDelay) || f_nFramesDelay<=0)
			{
				cerr << "Bad format for delay frames (-y): should be integer>0" << endl;
				errflg++;
			}
			break;
		}
	case 'R':
		{
			int ifpt;
			vector<string> tokens;
			vector<string> sequences;
			StimSetCRG* pSSCRG = NULL;

			s.assign(optarg);
			tokenize(s, tokens, ",");
			if (tokens.size() != 2)
			{
				cerr << "Bad format for CRG stim. Expecting \"-R frames_per_term,filename\", got \"" << s << "\"." << endl;
				errflg++;
			}
			else if (parse_integer(tokens[0], ifpt))
			{
				cerr << "Bad format for CRG frames_per_term. Expecting \"-R frames_per_term,filename\", got \"" << s << "\"." << endl;
				errflg++;
			}
			else if (arutil_load_sequences(sequences, tokens[1]))
			{
				cerr << "Error loading sequences for CRG stim. Check format of stim file \"" << tokens[1] << "\"" << endl;
				errflg++;
			}
			if (!have_grating)
			{
				cerr << "Error - must pass template grating stimulus with \"-s\" before passing sequence parameters." << endl;
				errflg++;
			}
			else if (activeScreen != ' ' && !have_offset)
			{
				cerr << "Error - cannot specify active screen (-[D|M|V]) without also specifying slave screen offset!" << endl;
				errflg++;
			}

			// Check that all sequences are the same length. 
			for (vector<string>::const_iterator it = sequences.begin(); it != sequences.end(); it++)
			{
				if (it->length() != sequences.begin()->length())
				{
					cerr << "Error - all sequences are not the same lenth (" << sequences.begin()->length() << "). Check sequence file." << endl;
					errflg++;
				}
			}

			if (!errflg)
			{
				pSSCRG = new StimSetCRG(ifpt, sequences);
				switch (activeScreen)
				{
				case 'M':
				case ' ':
					if (have_fixpt) pSSCRG->set_fixpt(fixpt);
					if (have_xhair) pSSCRG->set_xhair(xhair);
					pSSCRG->set_grating(grating);
					f_pstimset = pSSCRG;
					break;
				case 'V':
					if (have_fixpt) pSSCRG->set_fixpt(fixpt, f_dSlaveXOffset, f_dSlaveYOffset);
					if (have_xhair) pSSCRG->set_xhair(xhair, f_dSlaveXOffset, f_dSlaveYOffset);
					pSSCRG->set_grating(grating, f_dSlaveXOffset, f_dSlaveYOffset);
					f_pstimsetSlave = pSSCRG;
					break;
				case 'D':
					if (have_fixpt) pSSCRG->set_fixpt(fixpt);
					if (have_xhair) pSSCRG->set_xhair(xhair);
					pSSCRG->set_grating(grating);
					f_pstimset = pSSCRG;

					//Make a new one for slave
					pSSCRG = new StimSetCRG(ifpt, sequences);
					if (have_fixpt) pSSCRG->set_fixpt(fixpt, f_dSlaveXOffset, f_dSlaveYOffset);
					if (have_xhair) pSSCRG->set_xhair(xhair, f_dSlaveXOffset, f_dSlaveYOffset);
					pSSCRG->set_grating(grating, f_dSlaveXOffset, f_dSlaveYOffset);
					f_pstimsetSlave = pSSCRG;
					break;
				}
			}
			break;
		}
	case 'h':
		{
			s.assign(optarg);
			if (parse_xhair(s, xhair)) 
			{
				cerr << "Error parsing xhair arg (" << s << ")" << endl;
				errflg++;
			}
			else
			{
				have_xhair = true;
			}
			break;
		}
	case 0:
		{
			f_pstimset->set_stimtime(f_dStimTime);
			f_pstimset->set_frames_delay(f_nFramesDelay+1);
			f_pstimset->set_frames_fixpt_delay(f_nFramesDelay+2);
			f_pstimsetSlave->set_stimtime(f_dStimTime);
			f_pstimsetSlave->set_frames_delay(f_nFramesDelay);
			f_pstimsetSlave->set_frames_fixpt_delay(f_nFramesDelay);

			if (!have_t)
			{
				cerr << "Error - must specify stim time (-t) in sec." << endl;
				errflg++;
			}

			if (!have_offset)
			{
				cerr << "Error - must specify slave screen offset with \"-r filename\" or \"-r x,y\"." << endl;
				errflg++;
			}


			if (!f_pstimset)
			{
				cerr << "Error - no master stim set specified!" << endl;
				errflg++;
			}

			if (!f_pstimsetSlave)
			{
				cerr << "Error - no slave stim set specified!" << endl;
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

