/* $Id: dualstim.cpp,v 1.1 2011-08-30 01:59:34 djsperka Exp $ */

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
#include "DualStimSet.h"
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
ARXhairSpec f_xhair;
ARGratingSpec f_grating;
StimSet *f_pStimSet = NULL;			// This is for master in dualVSG mode
StimSet *f_pStimSetSlave = NULL;
class MyDualStimSet;	// forward declaration
MyDualStimSet *f_pDualStimSet = NULL;
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
int f_nFramesDelay = 3;
double f_dStimTime = 0;
string f_szResponseFile;
bool f_bHaveResponseFile = false;

// function prototypes

int args(int argc, char **argv);
void usage();
void init_triggers();
int callback(int &output, const FunctorCallbackTrigger* ptrig);
template <class T> StimSet* create_stimset(bool bHaveFixpt, ARContrastFixationPointSpec& fixpt, bool bHaveXhair, ARXhairSpec& xhair, ARGratingSpec& grating, vector<double> params, double offsetX, double offsetY, double spatialphase);
StimSet* create_stimset(bool bHaveFixpt, ARContrastFixationPointSpec& fixpt, bool bHaveXhair, ARXhairSpec& xhair, ARGratingSpec& grating, double offsetX, double offsetY, double spatialphase);

class MyDualStimSet : public DualStimSet
{
private: 
	StimSet *m_pMasterStimSet;
	StimSet *m_pSlaveStimSet;
	ARGratingSpec m_gratingMaster;
	ARGratingSpec m_gratingSlave;
	TSpecificFunctor<MyDualStimSet> *m_pcallbackFunctor;
	int m_nFramesDelay;
	double m_dStimTime;
public:
	MyDualStimSet(StimSet* pMaster, StimSet *pSlave, int nframes_delay, double tstim_on) : m_pMasterStimSet(pMaster), m_pSlaveStimSet(pSlave), m_nFramesDelay(nframes_delay), m_dStimTime(tstim_on)
	{
		m_pcallbackFunctor = new TSpecificFunctor<MyDualStimSet>(this, &MyDualStimSet::callback);
	};
	virtual ~MyDualStimSet() {};
	virtual int init(ARvsg& master, ARvsg& slave);
	virtual int init_triggers(TriggerVector& triggers);
	virtual int cleanup(ARvsg& master, ARvsg& slave);

	// Dump something relevant for verbosity's sake
	virtual std::string toString() const;
	int callback(int &output, const FunctorCallbackTrigger* ptrig);
};

int MyDualStimSet::init(ARvsg& master, ARvsg& slave)
{
	int status=0;
	int nframes_on = (int)(m_dStimTime * 1000000 /(double)vsgGetSystemAttribute(vsgFRAMETIME));
	status = m_pMasterStimSet->init(master, m_nFramesDelay+1, nframes_on, m_nFramesDelay+2);
	if (status) return status;

	status = m_pSlaveStimSet->init(slave, m_nFramesDelay, nframes_on, m_nFramesDelay);
	if (status) return status;

	return 0;
}

int MyDualStimSet::callback(int& output, const FunctorCallbackTrigger* ptrig)
{
	int status=0;
	ARvsg::master().select();
	status = m_pMasterStimSet->handle_trigger(ptrig->getKey());
	ARvsg::slave().select();
	status |= m_pSlaveStimSet->handle_trigger(ptrig->getKey());
	return status;
}


int MyDualStimSet::init_triggers(TriggerVector& triggers)
{
	triggers.addTrigger(new FunctorCallbackTrigger("F", 0x2, 0x2, 0x2, 0x2, m_pcallbackFunctor));
	triggers.addTrigger(new FunctorCallbackTrigger("S", 0x4, 0x4, 0x4, 0x4, m_pcallbackFunctor));
	triggers.addTrigger(new FunctorCallbackTrigger("s", 0x4, 0x0, 0x4, 0x0, m_pcallbackFunctor));
	triggers.addTrigger(new FunctorCallbackTrigger("X", 0x6, 0x0, 0x6, 0x0, m_pcallbackFunctor));
	triggers.addTrigger(new FunctorCallbackTrigger("a", 0x8, 0x8|AR_TRIGGER_TOGGLE, 0x8, 0x8|AR_TRIGGER_TOGGLE, m_pcallbackFunctor));

	// quit trigger
	triggers.addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));

	return 0;
}

int MyDualStimSet::cleanup(ARvsg& master, ARvsg& slave)
{
	cout << "MyDualStimSet::cleanup()" << endl;
	return 0;
}

std::string MyDualStimSet::toString() const
{
	std::ostringstream oss;
	oss << "MY Dual StimSet" << endl;
	oss << *m_pMasterStimSet << endl;
	oss << *m_pSlaveStimSet << endl;
	return oss.str();
}



int main (int argc, char *argv[])
{
	int status = 0;

	// Check input arguments
	status = args(argc, argv);
	if (f_bHaveResponseFile)
	{
		ifstream ifs(f_szResponseFile.c_str());
		if (!ifs)
		{
			cerr << "Error - cannot open response file " << f_szResponseFile << endl;
			status = -1;
		}
		else
		{
			int argc_r;
			char **argv_r = NULL;
			make_argv(ifs, argc_r, argv_r);
			status = args(argc_r, argv_r);
			free_argv(argc_r, argv_r);
		}
	}
	if (status)
	{
		return 1;
	}

	f_pDualStimSet = new MyDualStimSet(f_pStimSet, f_pStimSetSlave, f_nFramesDelay, f_dStimTime);
	if (f_verbose)
	{
		cout << "Dual Stim Set:" << endl << *f_pDualStimSet << endl;
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
	ARvsg::master().select();
	if (f_pDualStimSet->init(ARvsg::master(), ARvsg::slave()))
	{
		cerr << "Stim set initialization failed." << endl;
		return -1;
	}


	// Initialize triggers
	f_pDualStimSet->init_triggers(triggers);
	if (f_verbose)
	{
		for (unsigned int i=0; i<triggers.size(); i++)
		{
			std::cout << "Trigger " << i << " " << *(triggers[i]) << std::endl;
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

		TriggerFunc	tf = std::for_each(triggers.begin(), triggers.end(), 
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
	cout << "Clear" << endl;
	ARvsg::master().select();
	ARvsg::master().clear();
	vsgPresent();
	ARvsg::slave().select();
	ARvsg::slave().clear();
	vsgPresent();
	f_pDualStimSet->cleanup(ARvsg::master(), ARvsg::slave());

	return 0;
}

StimSet* create_stimset(bool bHaveFixpt, ARContrastFixationPointSpec& fixpt, bool bHaveXhair, ARXhairSpec& xhair, ARGratingSpec& grating, double offsetX, double offsetY, double spatialphase)
{
	StimSet *pstimset=(StimSet *)NULL;
	ARContrastFixationPointSpec f(fixpt);
	ARGratingSpec g(grating);
	ARXhairSpec x(xhair);
	g.x += offsetX;
	g.y += offsetY;
	if (bHaveFixpt)
	{
		if (!bHaveXhair) 
		{
			f.x += offsetX;
			f.y += offsetY;
			pstimset = new FixptGratingStimSet(f, g, spatialphase);
		}
		else
		{
			f.x += offsetX;
			f.y += offsetY;
			x.x += offsetX;
			x.y += offsetY;
			pstimset = new FixptGratingStimSet(f, x, g, spatialphase);
		}
	}
	else
	{
		pstimset = new FixptGratingStimSet(g, spatialphase);
	}
	return pstimset;
}

template <class T>
StimSet* create_stimset(bool bHaveFixpt, ARContrastFixationPointSpec& fixpt, bool bHaveXhair, ARXhairSpec& xhair, ARGratingSpec& grating, vector<double> params, double offsetX, double offsetY, double spatialphase)
{
	StimSet *pstimset=(StimSet *)NULL;
	ARContrastFixationPointSpec f(fixpt);
	ARGratingSpec g(grating);
	ARXhairSpec x(xhair);
	g.x += offsetX;
	g.y += offsetY;
	if (bHaveFixpt)
	{
		if (!bHaveXhair) 
		{
			f.x += offsetX;
			f.y += offsetY;
			pstimset = new T(f, g, params, spatialphase);
		}
		else
		{
			f.x += offsetX;
			f.y += offsetY;
			x.x += offsetX;
			x.y += offsetY;
			pstimset = new T(f, x, g, params, spatialphase);
		}

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
	bool have_xhair = false;
	bool have_stim = false;
	bool have_g = false;
	bool have_d = false;
	bool have_w = false;
	bool have_t = false;
	int delay=0, frames=0;
	string sequence_filename;
	char activeScreen = ' ';
	while ((c = getopt(argc, argv, "F:f:b:d:avg:s:C:T:S:O:A:DMVr:H:Zp:KP:w:y:t:X:")) != -1)
	{
		switch(c)
		{
		case 'a':
			f_binaryTriggers = false;
			break;
		case 'F':
			f_bHaveResponseFile = true;
			f_szResponseFile.assign(optarg);
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
					f_pStimSet = create_stimset(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, 0, 0, f_spatialphase);
					break;
				case 'V':
					f_pStimSetSlave = create_stimset(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
					break;
				case 'D':
					if (activeScreen != ' ' && !f_bDualVSG)
					{
						cerr << "Error - cannot specify active screen (-[D|M|V]) without also specifying slave screen offset!" << endl;
						errflg++;
					}
					else
					{
						f_pStimSet = create_stimset(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, 0, 0, f_spatialphase);
						f_pStimSetSlave = create_stimset(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
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
									f_pStimSet = create_stimset<SFStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									break;
								case 'V':
									f_pStimSetSlave = create_stimset<SFStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
									break;
								case 'D':
									f_pStimSet = create_stimset<SFStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									f_pStimSetSlave = create_stimset<SFStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
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
									f_pStimSet = create_stimset<ContrastStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									break;
								case 'V':
									f_pStimSetSlave = create_stimset<ContrastStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
									break;
								case 'D':
									f_pStimSet = create_stimset<ContrastStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									f_pStimSetSlave = create_stimset<ContrastStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
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
									f_pStimSet = create_stimset<TFStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									break;
								case 'V':
									f_pStimSetSlave = create_stimset<TFStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
									break;
								case 'D':
									f_pStimSet = create_stimset<TFStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									f_pStimSetSlave = create_stimset<TFStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
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
									f_pStimSet = create_stimset<OrientationStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									break;
								case 'V':
									f_pStimSetSlave = create_stimset<OrientationStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
									break;
								case 'D':
									f_pStimSet = create_stimset<OrientationStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									f_pStimSetSlave = create_stimset<OrientationStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
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
									f_pStimSet = create_stimset<AreaStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									break;
								case 'V':
									f_pStimSetSlave = create_stimset<AreaStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
									break;
								case 'D':
									f_pStimSet = create_stimset<AreaStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									f_pStimSetSlave = create_stimset<AreaStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
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
									f_pStimSet = create_stimset<DonutStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									break;
								case 'V':
									f_pStimSetSlave = create_stimset<DonutStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
									break;
								case 'D':
									f_pStimSet = create_stimset<DonutStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, 0, 0, f_spatialphase);
									f_pStimSetSlave = create_stimset<DonutStimSet>(have_fixpt, f_fixpt, have_xhair, f_xhair, f_grating, tuning_parameters, f_dSlaveXOffset, f_dSlaveYOffset, f_spatialphase);
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
					f_bDualVSG = true;
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
		case 'X':
			{
				s.assign(optarg);
				if (parse_xhair(s, f_xhair)) errflg++;
				else
				{
					have_xhair = true;
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

	// If response file found, just return. This could be bad - user should pass nothing else on command line 
	// if response file is used!!! 
	if (f_bHaveResponseFile)
	{
		return 0;
	}

	if (!have_t)
	{
		cerr << "Error - must specify stim time (-t) in sec." << endl;
		errflg++;
	}


	if (!f_pStimSet && !f_pStimSetSlave)
	{
		cerr << "Error - when using dual VSG mode you must specify one of D/M/V options and a stimulus set (COASTg)." << endl;
		errflg++;
	}
	else if (!f_pStimSetSlave)
	{
		f_fixpt.x += f_dSlaveXOffset;
		f_fixpt.y += f_dSlaveYOffset;
		f_pStimSetSlave = new FixptGratingStimSet(f_fixpt);
		f_fixpt.x -= f_dSlaveXOffset;
		f_fixpt.y -= f_dSlaveYOffset;
	}
	else if (!f_pStimSet)
	{
		f_pStimSet = new FixptGratingStimSet(f_fixpt);
	}

	return errflg;
}

void usage()
{
	cerr << "usage: fixstim NI" << endl;
}

