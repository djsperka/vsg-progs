/* $Id: StarUStim.cpp,v 1.1 2016-04-01 22:26:34 devel Exp $*/

#include "StarUStim.h"

const string StarUStim::m_allowedArgs("avf:t:b:d:ho:p:");

StarUStim::StarUStim()
: UStim()
, m_binaryTriggers(true)
, m_verbose(false)
, m_iDistanceToScreenMM(-1)
, m_pulse(0x2)
, m_errflg(0)
{
	m_background.setType(gray);
};

StarUStim::~StarUStim()
{
}

bool StarUStim::parse(int argc, char **argv)
{
	bool b = false;
	int status;
	status = prargs(argc, argv, (process_args_func)NULL, m_allowedArgs.c_str(), 'F', this);
	if (!status) b = true;
	return b;
}

int StarUStim::process_arg(int c, std::string& arg)
{
	static bool have_f=false;		// have fixation spec
	static bool have_d=false;		// have screen dist
	ARContrastFixationPointSpec *pspec;
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
	case 'f': 
		if (parse_fixation_point(arg, m_fixpt)) m_errflg++;
		else have_f = true;
		break;
	case 't': 
		pspec = new ARContrastFixationPointSpec();
		if (!parse_fixation_point(arg, *pspec))
		{
			m_targets.push_back(pspec);
		}
		else 
		{
			m_errflg++;
			cerr << "Error in target spec (" << arg << ")" << endl;
			delete(pspec);
		}
		break;
	case 'b': 
		if (parse_color(arg, m_background)) m_errflg++; 
		break;
	case 'd':
		if (parse_distance(arg, m_iDistanceToScreenMM)) m_errflg++;
		else have_d=true;
		break;
	case 'o':
		if (parse_int_list(arg, m_vecTargetOrder))
		{
			cerr << "Error in target order list. Expecting comma-separated list of integers." << endl;
			m_errflg++;
		}
		break;
	case 0:
		if (!have_f) 
		{
			cerr << "Fixation point not specified!" << endl; 
			m_errflg++;
		}
		if (!have_d)
		{
			cerr << "Screen distance not specified!" << endl; 
			m_errflg++;
		}

		// Make sure there were some targets
		if (m_targets.size()==0)
		{
			cerr << "No targets specified!" << endl;
			m_errflg++;
		}

		// If no random order specified, populate the target order vector with 1,2,...,n
		if (!m_errflg && m_vecTargetOrder.size()==0)
		{
			for (unsigned int i=0; i < m_targets.size(); i++) m_vecTargetOrder.push_back(i);
		}

		break;
	default:
		m_errflg++;
		break;
	}
	return m_errflg;
}



void StarUStim::run_stim(alert::ARvsg& vsg)
{
	cout << "StarUStim::run_stim(): started" << endl;

	// clear all dig outputs
	vsgIOWriteDigitalOut(0, 0xff);
	vsgPresent();

	// initialize triggers
	TSpecificFunctor<StarUStim> functor(this, &StarUStim::callback);
	init_triggers(&functor);
	init_pages();

	// switch display back to first client page
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

			vsgPresent();
		}
		Sleep(10);
	}

	vsg.clear();

	return ;
}


void StarUStim::init_triggers(TSpecificFunctor<StarUStim>* pfunctor)
{
	triggers().clear();
	triggers().addTrigger(new FunctorCallbackTrigger("F", 0x2, 0x2, 0x2, 0x2, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("f", 0x2, 0x0, 0x2, 0x0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("S", 0x4, 0x4, 0x4, 0x4, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("s", 0x4, 0x0, 0x4, 0x0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("a", 0x8, 0x8|AR_TRIGGER_TOGGLE, 0x8, 0x8|AR_TRIGGER_TOGGLE, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("X", 0x6, 0x0, 0x6, 0x0, pfunctor));
	triggers().addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));

	return;
}

void StarUStim::update_page()
{
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	m_fixpt.setContrast(0);
	m_fixpt.draw();

	m_targets[*m_iterator]->setContrast(0);
	m_targets[*m_iterator]->draw();
}



void StarUStim::init_pages()
{
	m_iterator = m_vecTargetOrder.begin();

	m_fixpt.init(2);
	cerr << "init first target" << endl;
	m_targets[0]->init(2);
	for (int i=1; i<m_targets.size(); i++)
	{
		cerr << "init target " << i << endl;
		m_targets[i]->init(*m_targets[0]);
	}
	cerr << "udpate page" << endl;
	update_page();
	vsgPresent();
}



int StarUStim::callback(int &output, const FunctorCallbackTrigger* ptrig)
{
	int ival=1;
	string key = ptrig->getKey();
	if (key == "a")
	{
		m_iterator++;
		if (m_iterator == m_vecTargetOrder.end()) m_iterator = m_vecTargetOrder.begin();
		update_page();
	}
	else if (key == "s")
	{
		m_targets[*m_iterator]->setContrast(0);
	}
	else if (key == "S")
	{
		m_targets[*m_iterator]->setContrast(100);
	}
	else if (key == "F")
	{
		m_fixpt.setContrast(100);
	}
	else if (key == "f")
	{
		m_fixpt.setContrast(0);
	}
	else if (key == "X")
	{
		m_fixpt.setContrast(0);
		m_targets[*m_iterator]->setContrast(0);
	}

	return ival;
}




