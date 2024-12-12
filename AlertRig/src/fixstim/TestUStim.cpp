#include "TestUStim.h"

const string TestUStim::m_allowedArgs("avf:t:g:b:d:ho:p:B:");
double f_tf = .1;

TestUStim::TestUStim()
: UStim()
, m_binaryTriggers(true)
, m_verbose(false)
, m_iDistanceToScreenMM(-1)
, m_pulse(0x2)
, m_errflg(0)
, m_fixptPage(1)
, m_fixptStimPage(2)
{
};

TestUStim::~TestUStim()
{
}

bool TestUStim::parse(int argc, char **argv)
{
	bool b = false;
	int status;
	status = prargs(argc, argv, (process_args_func)NULL, m_allowedArgs.c_str(), 'F', this);
	if (!status) b = true;
	return b;
}

int TestUStim::process_arg(int c, std::string& arg)
{
	static bool have_f=false;		// have fixation spec
	static bool have_d=false;		// have screen dist
	ARGratingSpec *pgrating;

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
	case 'g':
		pgrating = new ARGratingSpec();
		if (!parse_grating(arg, *pgrating))
		{
			m_gratings.push_back(pgrating);
		}
		else
		{
			m_errflg++;
			cerr << "Error in grating spec (" << arg << ")" << endl;
			delete pgrating;
		}
		break;
	case 'b': 
		if (parse_color(arg, m_background)) m_errflg++; 
		break;
	case 'd':
		if (parse_distance(arg, m_iDistanceToScreenMM)) m_errflg++;
		else have_d=true;
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

		break;
	default:
		m_errflg++;
		break;
	}
	return m_errflg;
}



void TestUStim::run_stim(alert::ARvsg& vsg)
{
	cout << "TestUStim::run_stim(): started" << endl;

	// screen distance and bkgd color
	vsg.setViewDistMM(m_iDistanceToScreenMM);
	vsg.setBackgroundColor(m_background);

	// clear all dig outputs
	vsgIOWriteDigitalOut(0, 0xffff);
	vsgPresent();

	// initialize triggers
	TSpecificFunctor<TestUStim> functor(this, &TestUStim::callback);
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

		if (tf.quit()) break;
		
		// see if vsgPresent() should be called....
		if (tf.present())
		{
			cerr << "Got tf.present()" << endl;
			vsgObjSetTriggers(vsgTRIG_ONPRESENT, 0, 0);
			//vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0, 0);
			vsgPresent();
		}

		// now see if any output bits need be written
		// vsgIOWriteDigitalOut needs vsgPresent()!!!
		if (tf.output_trigger() != last_output_trigger)
		{	
			cerr << "trig " << hex << tf.output_trigger() << " old " << hex << last_output_trigger << endl;
			last_output_trigger = tf.output_trigger();
#if 1
			vsgFrameSync();
			vsgIOWriteDigitalOut(tf.output_trigger() << 1, 0xfffe);
#else
			cerr << "not calling vsgIOWriteDigitalOut" << endl;
#endif
			//vsgPresent();
		}
		Sleep(10);
	}

	vsg.clear();

	return ;
}


void TestUStim::init_triggers(TSpecificFunctor<TestUStim>* pfunctor)
{
	triggers().clear();
	triggers().addTrigger(new FunctorCallbackTrigger("F", 0x2, 0x2, 0x2, 0x2, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("S", 0x4, 0x4, 0x4, 0x4, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("a", 0x8, 0x8|AR_TRIGGER_TOGGLE, 0x8, 0x8|AR_TRIGGER_TOGGLE, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("X", 0x6, 0x0, 0x6, 0x0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("f", 0x2, 0x0, 0x2, 0x0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("s", 0x4, 0x0, 0x4, 0x0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("0", 0x0, 0x0, 0x0, 0x0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("1", 0x0, 0x0, 0x0, 0x0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("2", 0x0, 0x0, 0x0, 0x0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("3", 0x0, 0x0, 0x0, 0x0, pfunctor));
	triggers().addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));

	return;
}

void TestUStim::init_pages()
{
	m_fixpt.init(2);
	//m_fixpt.setContrast(0);

	if (m_gratings.size() > 0)
	{
		cerr << "init first grating target" << endl;
		m_gratings[0]->init(30);
		if (m_gratings.size() > 1)
		{
			cerr << "Ignoring additional gratings" << endl;
			//cerr << "fake-init " << m_gratings.size() - 1 << " additional grating targets" << endl;
			//for (unsigned int i = 1; i < m_gratings.size(); i++)
			//{
			//	cerr << "fake-init grating target " << i << endl;
			//	m_gratings[i]->init(*m_gratings[0]);
			//}
		}
	}
	draw_current();
	vsgPresent();
	cerr << "init done." << endl;
}

void TestUStim::draw_current()
{
	vsgSetDrawPage(vsgVIDEOPAGE, m_fixptStimPage, vsgBACKGROUND);
	m_gratings[0]->draw();
	m_gratings[0]->setContrast(0);
	m_fixpt.draw();
	m_fixpt.setContrast(0);
}



int TestUStim::callback(int &output, const FunctorCallbackTrigger* ptrig, const std::string&)
{
	int ival=1;
	string key = ptrig->getKey();

	if (key == "a")
	{
		draw_current();
	}
	else if (key == "s")
	{
		m_gratings[0]->setContrast(0);
	}
	else if (key == "S")
	{
		m_gratings[0]->setContrast(100);
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
		m_gratings[0]->setContrast(0);
	}
	else if (key == "0")
	{
		//vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0 + vsgTRIGGERPAGE);
		//vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		m_gratings[0]->setTemporalFrequency(f_tf);
		f_tf += 0.2;
	}
	else if (key == "1")
	{
		//vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1 + vsgTRIGGERPAGE);
		//vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		m_gratings[0]->setTemporalFrequency(f_tf);
	}
	else if (key == "2")
	{
		//vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1 + vsgTRIGGERPAGE);
		//vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		m_gratings[0]->setContrast(50);
	}
	else if (key == "3")
	{
		//vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1 + vsgTRIGGERPAGE);
		//vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		m_gratings[0]->setTemporalFrequency(f_tf);
		m_gratings[0]->setContrast(50);
	}

	return ival;
}





