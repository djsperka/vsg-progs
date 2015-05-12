/* $Id: FixUStim.cpp,v 1.1 2015-05-12 17:27:00 devel Exp $*/

#include "FixUStim.h"
#include <iostream>
using namespace std;
using namespace boost::filesystem;

const string FixUStim::m_allowedArgs("ab:d:e:f:g:h:k:p:s:vzA:B:C:D:G:H:I:J:KL:M:NO:Q:R:S:T:U:V:W:Y:Z:");

FixUStim::FixUStim(bool bStandAlone)
: UStim()
, m_bStandAlone(bStandAlone)
, m_binaryTriggers(true)
, m_verbose(false)
, m_dumpStimSetsOnly(false)
, m_bPresentOnTrigger(false)
, m_sTriggeredTriggers()
, m_ulTriggerArmed(0)
, m_pStimSet(0)
, m_bUsingMultiParameterStimSet(false)
, m_iDistanceToScreenMM(-1)
, m_pulse(0x40)
, m_bDaemon(false)
, m_bClient(false)
, m_errflg(0)
{
	m_background.type = gray;
	m_background.color.a = m_background.color.b = m_background.color.c = 0.5;
};

bool FixUStim::parse(int argc, char **argv)
{
	bool b = false;
	int status;
	status = prargs(argc, argv, (process_args_func)NULL, m_allowedArgs.c_str(), 'F', this);
	if (!status)
	{
		b = true;
		if (m_dumpStimSetsOnly)
		{
			cout << *m_pStimSet << endl;
			b = false;
		}
	}
	return b;
}

void FixUStim::run_stim(alert::ARvsg& vsg)
{
	cout << "FixUStim::run_stim(): started" << endl;

	// If standalone, have to initialize vsg....
	// The static UStim method does it consistently for standalone/server.
	if (m_bStandAlone)
	{
		cout << "FixUStim::run_stim(): standalone instance. Initialize vsg..." << endl;
		if (UStim::initialize(vsg, m_iDistanceToScreenMM, m_background))
		{
			cerr << "FixUStim::run_stim(): VSG init failed!" << endl;
			return;
		}
	}
	else
	{
		vsg.setViewDistMM(m_iDistanceToScreenMM);
	}

	// clear all dig outputs
	vsgIOWriteDigitalOut(0, 0xff);
	vsgPresent();

	// initialize triggers
	TSpecificFunctor<FixUStim> functor(this, &FixUStim::callback);
	init_triggers(&functor);
	if (m_verbose)
	{
		for (unsigned int i=0; i<triggers().size(); i++)
		{
			std::cout << "FixUStim::run_stim(): Trigger " << i << " " << *(triggers().at(i)) << std::endl;
		}
	}

	// Tell the stim set to initialize its pages. Note that the current display
	// page is page 0, and the stim set is given different pages (starting at 1 and depending on 
	// how many pages it requires)
	if (m_verbose)
	{
		cout << "FixUStim::run_stim(): Configured stim set:" << endl;
		cout << m_pStimSet->toString() << endl;
	}

	vector<int> pages;
	for (int i=0; i<m_pStimSet->num_pages(); i++) pages.push_back(i+1);
	if (m_pStimSet->init(pages))
	{
		cerr << "FixUStim::run_stim(): StimSet initialization failed." << endl;
	}

	if (m_bPresentOnTrigger)
		cout << "FixUStim::run_stim(): Present on trigger is ON" << endl;
	else
		cout << "FixUStim::run_stim(): Present on trigger is OFF" << endl;

	// switch display back to first client page
	vsgSetZoneDisplayPage(vsgVIDEOPAGE, pages[0]);
	vsg.ready_pulse(100, m_pulse);

	// reset all triggers if using binary triggers
	if (m_binaryTriggers) triggers().reset(vsgIOReadDigitalIn());


	// All right, start monitoring triggers........
	int last_output_trigger=0;
	long input_trigger = 0;
	string s;

	while (!quit_enabled())
	{
		// If user-triggered, get a trigger entry. 
		if (!m_binaryTriggers)
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
			(m_binaryTriggers ? TriggerFunc(input_trigger, last_output_trigger, false) : TriggerFunc(s, last_output_trigger)));

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

			// Check whether we do an ordinary present(), or if we are doing dualstim rig hijinks we'll want to 
			// do a presendOnTrigger. In the presentOnTrigger case, we do a further check on whether any of the
			// triggers matched (you can have multiple triggers matched in a single check) is on the list of 
			// those to be triggered on (see commandline arg -V). 
			if (!m_bPresentOnTrigger)
			{
				vsgPresent();
			}
			else
			{
				if (m_sTriggeredTriggers.empty() || m_sTriggeredTriggers.find_first_of(tf.triggers_matched()) < string::npos)
				{
					if (m_verbose) cerr << "FixUStim::run_stim(): Present armed (" << std::hex << m_ulTriggerArmed << "), wait for trigger..." << endl;
					vsgIOWriteDigitalOut(m_ulTriggerArmed, m_ulTriggerArmed);
					vsgFrameSync();	// this blocks until the next refresh, when the IO output is written. 
					vsgPresentOnTrigger(vsgTRIG_ONRISINGEDGE + vsgDIG7);
					if (m_verbose) cerr << "FixUStim::run_stim(): got trigger..." << endl;
				}
				else
				{
					if (m_verbose) cerr << "FixUStim::run_stim(): Present unfettered for trigger " << tf.triggers_matched() << endl;
					vsgPresent();
				}
			}
		}
		Sleep(10);
	}

	vsg.clear();

	return ;
}


void FixUStim::init_triggers(TSpecificFunctor<FixUStim>* pfunctor)
{
	triggers().clear();
	triggers().addTrigger(new FunctorCallbackTrigger("F", 0x2, 0x2, 0x2, 0x2, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("S", 0x4, 0x4, 0x4, 0x4, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("s", 0x4, 0x0, 0x4, 0x0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("X", 0x6, 0x0, 0x6, 0x0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("a", 0x8, 0x8|AR_TRIGGER_TOGGLE, 0x8, 0x8|AR_TRIGGER_TOGGLE, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("u", 0x20, 0x20|AR_TRIGGER_TOGGLE, 0x10, 0x10|AR_TRIGGER_TOGGLE, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("v", 0x40, 0x40|AR_TRIGGER_TOGGLE, 0x20, 0x20|AR_TRIGGER_TOGGLE, pfunctor));

	// hack
	triggers().addTrigger(new FunctorCallbackTrigger("1", 0, 0, 0, 0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("2", 0, 0, 0, 0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("3", 0, 0, 0, 0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("4", 0, 0, 0, 0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("5", 0, 0, 0, 0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("6", 0, 0, 0, 0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("7", 0, 0, 0, 0, pfunctor));


	// quit trigger
	triggers().addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));

	return;
}

int FixUStim::callback(int &output, const FunctorCallbackTrigger* ptrig)
{
	return  m_pStimSet->handle_trigger(ptrig->getKey());
}



int FixUStim::process_arg(int c, std::string& arg)
{
	static bool have_fixpt = false;
	static bool have_stim = false;
	static bool have_xhair = false;
	static bool have_g = false;
	static bool have_d = false;
	static bool have_sequence = false;	// command-line sequence of page numbers 0,1...
	static string the_sequence;			// see above.
	static int errflg = 0;

	// This is a loooooong stanza. Be patient.
	switch(c)
	{
	case 'a':
		m_binaryTriggers = false;
		break;
	case 'z':
		m_dumpStimSetsOnly = true;
		break;
	case 'v':
		m_verbose = true;
		break;
	case 'b': 
		if (parse_color(arg, m_background)) errflg++; 
		break;
	case 'd':
		if (parse_distance(arg, m_iDistanceToScreenMM)) errflg++;
		else have_d=true;
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
			size_t pos = arg.find_first_of(",");
			if (pos)
			{
				m_sTriggeredTriggers = arg.substr(pos);
				if (parse_ulong(arg.substr(0, pos), m_ulTriggerArmed)) errflg++;
				else m_bPresentOnTrigger = true;
			}
			else
			{
				m_sTriggeredTriggers = "";
				if (parse_ulong(arg, m_ulTriggerArmed)) errflg++;
				else m_bPresentOnTrigger = true;
			}
			break;
		}
	case 'f':
		if (parse_fixation_point(arg, m_fixpt)) errflg++;
		else 
		{
			have_fixpt = true;
		}
		break;
	case 'h':
		if (parse_xhair(arg, m_xhair)) 
		{
			cerr << "Error in xhair arg: " << arg << endl;
			errflg++;
		}
		else
		{
			have_xhair = true;
		}
		break;
	case 'g':
		if (parse_grating(arg, m_grating)) errflg++;
		else 
		{
			if (!have_fixpt)
			{
				m_pStimSet = new GratingStimSet(m_grating);
			}
			else if (have_fixpt && !have_xhair)
			{
				m_pStimSet = new FixptGratingStimSet(m_fixpt, m_grating);
			}
			else
			{
				m_pStimSet = new FixptGratingStimSet(m_fixpt, m_xhair, m_grating);
			}
		}
		break;
	case 's':
		if (m_vecGratings.size() == 8)
		{
			cerr << "Maximum number of gratings(8) reached." << endl;
			errflg++;
		}
		else if (parse_grating(arg, m_grating)) 
		{
			cerr << "Error in grating input: " << arg << endl;
			errflg++;
		}
		else 
		{
			have_stim = true;
			m_vecGratings.push_back(m_grating);
		}
		break;
	case 'k':
		if (m_vecDistractors.size() == 8)
		{
			cerr << "Maximum number of distractors(8) reached." << endl;
			errflg++;
		}
		else if (parse_grating(arg, m_grating)) 
		{
			cerr << "Error in grating input: " << arg << endl;
			errflg++;
		}
		else 
		{
			m_vecDistractors.push_back(m_grating);
		}
		break;
	case 'p':
		if (parse_integer(arg, m_pulse))
			errflg++;
		break;
	case 'N':
		// Stim set without grating; fixpt (and xhair) only
		if (!have_fixpt && !have_xhair)
		{
			m_pStimSet = new FixptGratingStimSet();
		}
		else if (!have_xhair)
		{
			m_pStimSet = new FixptGratingStimSet(m_fixpt);
		}
		else
		{
			m_pStimSet = new FixptGratingStimSet(m_fixpt, m_xhair);
		}
		break;
	case 'K':
		{
			FixptMultiGratingStimSet* pss = NULL;

			// Stim set with gratings as distractors
			if (!have_fixpt && !have_xhair)
			{
				pss = new FixptMultiGratingStimSet();
			}
			else if (!have_xhair)
			{
				pss = new FixptMultiGratingStimSet(m_fixpt);
			}
			else
			{
				pss = new FixptMultiGratingStimSet(m_fixpt, m_xhair);
			}
			for (unsigned int i=0; i<m_vecGratings.size(); i++) pss->set_grating(m_vecGratings[i]);
			for (unsigned int i=0; i<m_vecDistractors.size(); i++) pss->set_distractor(m_vecDistractors[i]);
			m_pStimSet = pss;
			break;
		}
	case 'H':
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
					// must have two gratings specified, it is assumed that the first is the donut, second is the hole. 
					if (have_fixpt)
					{
						if (m_vecGratings.size() == 2)
							m_pStimSet = new DanishStimSet(m_fixpt, m_vecGratings[0], m_vecGratings[1], tuning_parameters);
						else
							m_pStimSet = new DanishStimSet(m_fixpt, m_vecGratings[0], tuning_parameters);
					}
					else
					{
						if (m_vecGratings.size() == 2)
						{
							m_pStimSet = new DanishStimSet(m_vecGratings[0], m_vecGratings[1], tuning_parameters);
						}
						else
						{
							m_pStimSet = new DanishStimSet(m_vecGratings[0], tuning_parameters);
						}
					}
				}
			}
			break;
		}
	case 'O':
	case 'T':
	case 'S':
	case 'C':
	case 'A':
	case 'Z':
	case 'M':
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
					if (!m_pStimSet)
					{
						m_pStimSet = create_multiparameter_stimset(have_fixpt, m_fixpt, have_xhair, m_xhair, m_grating);
						m_bUsingMultiParameterStimSet = true;
					}

					if (!m_bUsingMultiParameterStimSet)
					{
						cerr << "Error - Cannot mix COASTXKMP with other stim types!" << endl;
						errflg++;
					}
					else
					{
						MultiParameterFXGStimSet* pmulti = static_cast<MultiParameterFXGStimSet*>(m_pStimSet);

						// Create a stim parameter set
						FXGStimParameterList *plist = NULL;
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
						case 'Z':
							plist = new GratingXYList(tuning_parameters);
							break;
						case 'M':
							plist = new StimHoleList(tuning_parameters);
							break;
						default:
							cerr << "Unhandled varying stim parameter type (" << (char)c << ")" << endl;
							errflg++;
						}
						if (plist) pmulti->push_back(plist);
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
							m_pStimSet = new CRGStimSet(m_fixpt, m_grating, fpt, seq, list, balanced);
						}
						else
						{
							m_pStimSet = new CRGStimSet(m_fixpt, m_grating, fpt, seq, balanced);
						}
					}
					else
					{
						if (list.size() > 0)
						{
							m_pStimSet = new CRGStimSet(m_grating, fpt, seq, list, balanced);
						}
						else
						{
							m_pStimSet = new CRGStimSet(m_grating, fpt, seq, balanced);
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
						m_pStimSet = new FlashStimSet(m_fixpt, fpt, seq);
					}
					else
					{
						m_pStimSet = new FlashStimSet(fpt, seq);
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
							m_pStimSet = new FlashStimSet(m_fixpt, colors, fpt, seq);
						}
						else
						{
							m_pStimSet = new FlashStimSet(colors, fpt, seq);
						}
					}
					else
					{
						if (have_fixpt)
						{
							m_pStimSet = new FlashStimSet(m_fixpt, fpt, seq);
						}
						else
						{
							m_pStimSet = new FlashStimSet(fpt, seq);
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
	case 'Y':		// CRG alternate - input file is a list of separate sequences, and each sequence is a "stim"
		{
			int ifpt;
			vector<string> tokens;
			vector<string> sequences;
			vector<int> order;
			StimSetCRG* pSSCRG = NULL;

			tokenize(arg, tokens, ",");
			if (tokens.size() < 2)
			{
				cerr << "Bad format for CRG stim. Expecting \"-R frames_per_term,filename[,i1,i2,i3...]\", got \"" << arg << "\"." << endl;
				errflg++;
			}
			else if (parse_integer(tokens[0], ifpt))
			{
				cerr << "Bad format for CRG frames_per_term. Expecting \"-R frames_per_term,filename[,i1,i2,i3...]\", got \"" << arg << "\"." << endl;
				errflg++;
			}
			else if (arutil_load_sequences(sequences, tokens[1]))
			{
				cerr << "Error loading sequences for CRG stim. Check format of stim file \"" << tokens[1] << "\"" << endl;
				errflg++;
			}

			if (!errflg)
			{
				tokens.erase(tokens.begin());
				tokens.erase(tokens.begin());
				// anything left? 
				if (tokens.size() > 0)
				{
					if (parse_int_list(tokens, order))
					{
						cerr << "Error in sequence order list. Expecting fpt,filename,i0,i1,i2,... where iN are integers." << endl;
						errflg++;
					}
				}
				else
				{
					unsigned int i;
					for (i=0; i<sequences.size(); i++)
					{
						order.push_back(i);
					}
					cerr << "Warning: No sequence order supplied on command line. Using default ordering for sequences." << endl;
				}
			}


			if (!have_stim)
			{
				cerr << "Error - must pass template grating stimulus with \"-s\" before passing CRG sequence parameters." << endl;
				errflg++;
			}

			// Check that all sequences are the same length. 
			for (vector<string>::const_iterator it = sequences.begin(); it != sequences.end(); it++)
			{
				if (it->length() != sequences.begin()->length())
				{
					cerr << "Error - all sequences are not the same length (" << sequences.begin()->length() << "). Check sequence file." << endl;
					errflg++;
				}
			}

			if (!errflg)
			{
				if (!have_fixpt && !have_xhair)
					m_pStimSet = new StimSetCRG(m_grating, ifpt, sequences, order);
				else if (have_fixpt && !have_xhair)
					m_pStimSet = new StimSetCRG(m_fixpt, m_grating, ifpt, sequences, order);
				else 
					m_pStimSet = new StimSetCRG(m_fixpt, m_xhair, m_grating, ifpt, sequences, order);
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
				m_pStimSet = new CBarStimSet(color, ww, hh, dps, tuning_parameters);
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
					m_pStimSet = new DotStimSet(m_fixpt, dotx, doty, color, dotdiam, dotspeed, dotdensity, (int)dotsize, tuning_parameters);
				}
				else
				{
					m_pStimSet = new DotStimSet(dotx, doty, color, dotdiam, dotspeed, dotdensity, (int)dotsize, tuning_parameters);
				}

			}
			break;
		}
	case 'I':			// master stim
	case 'U':			// slave stim
		{
			FGGXStimSet* pss = (FGGXStimSet*)NULL;
			shared_ptr<SSInfo> pssinfo(new SSInfo());
			if (!SSInfo::load(arg, *pssinfo))
			{
				cerr << "Error parsing stim set info file " << arg << endl;
				errflg++;
			}

			// Make sure to assign the "core" and "donut" correctly! 
			if (pssinfo->getIsDanish())
			{
				cerr << "Danish stim" << endl;
				if (pssinfo->getCoreIsMaster())
				{
					cerr << "Core is master" << endl;
					if (c=='I') pss = new FGGXDanishStimSet(pssinfo);
					else        pss = new FGGXNullStimSet(pssinfo, m_dSlaveXOffset, m_dSlaveYOffset);
				}
				else
				{
					cerr << "Core is slave" << endl;
					if (c=='U') pss = new FGGXDanishStimSet(pssinfo, m_dSlaveXOffset, m_dSlaveYOffset);
					else        pss = new FGGXNullStimSet(pssinfo);
				}
			}
			else
			{
				if (pssinfo->getCoreIsMaster())
				{
					if (c=='I') pss = new FGGXCoreStimSet(pssinfo);
					else        pss = new FGGXDonutStimSet(pssinfo, m_dSlaveXOffset, m_dSlaveYOffset);
				}
				else
				{
					if (c=='U') pss = new FGGXCoreStimSet(pssinfo, m_dSlaveXOffset, m_dSlaveYOffset);
					else        pss = new FGGXDonutStimSet(pssinfo);
				}
			}
			if (have_fixpt) pss->set_fixpt(m_fixpt);
			if (have_xhair) pss->set_xhair(m_xhair);
			m_pStimSet = (StimSet*)pss;
			break;
		}
	case 'Q':
		{
			if (parse_attcues(arg, m_vecGratings.size(), m_vecAttentionCues))
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
			if (parse_attparams(arg, m_vecGratings.size(), vecInput, tCC))
			{
				cerr << "Error in input." << endl;
				errflg++;
			}
			else
			{
				if (have_fixpt)
				{
					if (m_vecGratings.size() > 0)
					{
						if (m_vecAttentionCues.size() == 0)
						{
							m_pStimSet = new AttentionStimSet(m_fixpt, tCC, m_vecGratings, vecInput);
						}
						else
						{
							m_pStimSet = new AttentionStimSet(m_fixpt, tCC, m_vecGratings, m_vecAttentionCues, vecInput);
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
	case 'W':
		{
			// argument is a filename. The path is taken as the root path for stimuli.
			path p(arg);
			if (!exists(p))
			{
				cerr << "Error: Cue file does not exist: " << arg;
				errflg++;
			}
			else
			{
				vector<struct EQParams> vecEQParams;
				m_pathCues = p.parent_path();

				// open file, read line-by-line and parse
				string line;
				ifstream myfile(arg.c_str());
				if (myfile.is_open())
				{
					while (getline(myfile, line))
					{
						struct EQParams e;
						if (parse_eqparams(line, m_vecGratings.size(), e))
							errflg++;	// this will stop processing, eventually.
						else
						{
							if (m_verbose) cerr << "Got cue file " << e.cueFile << endl;
							if (exists(path(e.cueFile))) 
							{
								cerr << " Found cue file at absolute path " << path(e.cueFile) << endl;
							}
							else 
							{
								cerr << " Absolute path " << path(e.cueFile) << " not found." << endl;
								if (exists(m_pathCues / e.cueFile))
								{
									cerr << " Found cue file at path relative to config file " << (m_pathCues / e.cueFile) << endl;
									e.cueFile = (m_pathCues / e.cueFile).make_preferred().string();
								}
								else 
								{
									cerr << " Relative path " << (m_pathCues / e.cueFile) << " not found." << endl;
									errflg++;
								}
							}
							vecEQParams.push_back(e);
						}
					}
					myfile.close();
					m_pStimSet = new EQStimSet(m_fixpt, m_vecGratings, vecEQParams);
				}
			}
			break;
		}
	case 0:
		{
			if (!m_pStimSet)
			{
				cerr << "Error - you must specify a fixpt or a stimset (COASTg...)" << endl;
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

MultiParameterFXGStimSet* FixUStim::create_multiparameter_stimset(bool bHaveFixpt, ARContrastFixationPointSpec& fixpt, bool bHaveXhair, ARXhairSpec& xhair, ARGratingSpec& grating)
{
	MultiParameterFXGStimSet* pstimset = NULL;
	ARContrastFixationPointSpec f(fixpt);
	ARGratingSpec g(grating);
	ARXhairSpec h(xhair);
	if (bHaveFixpt && !bHaveXhair)
	{
		pstimset = new MultiParameterFXGStimSet(g, f);
	}
	else if (bHaveFixpt && bHaveXhair)
	{
		pstimset = new MultiParameterFXGStimSet(g, f, h);
	}
	else
	{
		pstimset = new MultiParameterFXGStimSet(g);
	}
	return pstimset;
}


template <class T>
StimSet* FixUStim::create_stimset(bool bHaveFixpt, ARContrastFixationPointSpec& fixpt, bool bHaveXhair, ARXhairSpec& xhair, ARGratingSpec& grating, vector<double> params)
{
	StimSet *pstimset=(StimSet *)NULL;
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
