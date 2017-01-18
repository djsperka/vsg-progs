#include "BeatUStim.h"

const string BeatUStim::m_allowedArgs("avb:i:g:p:P:");

BeatUStim::BeatUStim()
	: UStim()
	, m_binaryTriggers(true)
	, m_verbose(false)
	, m_pulse(0x2)
	, m_errflg(0)
	, m_initial(1)
	, m_gap(100)
	, m_period(0)
{
	m_background.setType(gray);
};

BeatUStim::~BeatUStim()
{
}

bool BeatUStim::parse(int argc, char **argv)
{
	bool b = false;
	int status;
	status = prargs(argc, argv, (process_args_func)NULL, m_allowedArgs.c_str(), 'F', this);
	if (!status) b = true;
	return b;
}

int BeatUStim::process_arg(int c, std::string& arg)
{
	switch (c)
	{
	case 'a':
		m_binaryTriggers = false;
		break;
	case 'p':
		if (parse_integer(arg, m_pulse))
		{
			cerr << "Error in pulse line spec (" << arg << ")" << endl;
			m_errflg++;
		}
		break;
	case 'v':
		m_verbose = true;
		break;
	case 'b':
		if (parse_color(arg, m_background)) m_errflg++;
		break;
	case 'i':
		if (parse_ulong(arg, m_initial)) m_errflg++;
		break;
	case 'g':
		if (parse_ulong(arg, m_gap)) m_errflg++;
		break;
	case 'P':
		if (parse_ulong(arg, m_period)) m_errflg++;
		break;
	case 0:
		break;
	default:
		m_errflg++;
		break;
	}
	return m_errflg;
}



void BeatUStim::run_stim(alert::ARvsg& vsg)
{
	cout << "BeatUStim::run_stim(): started" << endl;
	cout << "Initial pulses: " << m_initial << endl;
	cout << "Gap: " << m_gap << endl;
	cout << "Period: " << m_period << endl;

	// clear all dig outputs
	vsgIOWriteDigitalOut(0, 0xff);
	vsgPresent();

	// initialize triggers
	TSpecificFunctor<BeatUStim> functor(this, &BeatUStim::callback);
	init_triggers(&functor);

	// init page
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);

	// switch display back to first client page
	vsg.ready_pulse(100, m_pulse);

	// reset all triggers if using binary triggers
	if (m_binaryTriggers) triggers().reset(vsgIOReadDigitalIn());

	// All right, start monitoring triggers........
	int last_output_trigger = 0;
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

			vsgPresent();
		}
		Sleep(10);
	}

	vsg.clear();

	return;
}


void BeatUStim::init_triggers(TSpecificFunctor<BeatUStim>* pfunctor)
{
	triggers().clear();
//	triggers().addTrigger(new FunctorCallbackTrigger("F", 0x2, 0x2, 0x2, 0x2, pfunctor));
//	triggers().addTrigger(new FunctorCallbackTrigger("f", 0x2, 0x0, 0x2, 0x0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("S", 0x4, 0x4, 0x4, 0x4, pfunctor));
//	triggers().addTrigger(new FunctorCallbackTrigger("s", 0x4, 0x0, 0x4, 0x0, pfunctor));
//	triggers().addTrigger(new FunctorCallbackTrigger("a", 0x8, 0x8 | AR_TRIGGER_TOGGLE, 0x8, 0x8 | AR_TRIGGER_TOGGLE, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("X", 0x6, 0x0, 0x6, 0x0, pfunctor));
	triggers().addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));

	return;
}

int BeatUStim::callback(int &output, const FunctorCallbackTrigger* ptrig)
{
	int ival = 1;
	string key = ptrig->getKey();
	if (key == "S")
	{
		// Setup page cycling
		VSGCYCLEPAGEENTRY cycle[32767];
		for (int i = 0; i < m_initial; i++)
		{
			cycle[i].Frames = i+1;
			cycle[i].Page = 0 + vsgTRIGGERPAGE;
			cycle[i].Stop = 0;
		}

		for (int i = m_initial; i < (m_initial + m_period); i++)
		{
			cycle[i].Frames = m_gap;
			cycle[i].Page = 0 + vsgTRIGGERPAGE;
			cycle[i].Stop = 0;
		}

		vsgPageCyclingSetup(m_initial + m_period, cycle);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);

	}
	else if (key == "X")
	{
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
	}

	return ival;
}





