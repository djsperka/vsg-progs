/* $Id: StarUStim.cpp,v 1.1 2016-04-01 22:26:34 devel Exp $*/

#include "StarUStim.h"

const string StarUStim::m_allowedArgs("avf:t:g:b:d:ho:p:B:");

StarUStim::StarUStim()
: UStim()
, m_binaryTriggers(true)
, m_verbose(false)
, m_iDistanceToScreenMM(-1)
, m_pulse(0x2)
, m_errflg(0)
, m_currentPage(1)
, m_pBackgroundGrating(NULL)
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
	case 'B':
		pgrating = new ARGratingSpec();
		if (!parse_grating(arg, *pgrating))
		{
			m_pBackgroundGrating = pgrating;
		}
		else
		{
			m_errflg++;
			cerr << "Error in background grating spec (" << arg << ")" << endl;
			delete pgrating;
		}
		break;
	case 'g':
		pgrating = new ARGratingSpec();
		if (!parse_grating(arg, *pgrating))
		{
			m_gratings.push_back(pgrating);
			// m_targets.push_back(make_pair(false, (unsigned int)(m_gratings.size() - 1)));
			m_targets.push_back(boost::make_tuple(false, (unsigned int)(m_gratings.size() - 1), pgrating->contrast));
		}
		else
		{
			m_errflg++;
			cerr << "Error in grating spec (" << arg << ")" << endl;
			delete pgrating;
		}
		break;
	case 't': 
		pspec = new ARContrastFixationPointSpec();
		if (!parse_fixation_point(arg, *pspec))
		{
			m_dots.push_back(pspec);
			//m_targets.push_back(make_pair(true, (unsigned int)(m_dots.size() - 1)));
			m_targets.push_back(boost::make_tuple(true, (unsigned int)(m_dots.size() - 1), 100));
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

	// screen distance and bkgd color
	vsg.setViewDistMM(m_iDistanceToScreenMM);
	vsg.setBackgroundColor(m_background);

	// clear all dig outputs
	vsgIOWriteDigitalOut(0, 0xffff);
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

		// Now analyze input triggers. 	 	
		// 11/13/2024 djs. Change the logic here so that the vsgPresent() happens (initiating any FRAME signals) _before_ the 
		// call to write digital I/O bits. This ensures that the FRAME signal marks the stim change, and the I/O bits
		// that follow explain what happened. 
		// 12/12/2024 djs. Further change here to put a vsgFrameSync() before the call to vsgIOWrite... Without this, the write
		// seemed to squash _both_ the IO bits and the preceding FRAME. 
		if (tf.quit()) break;
		
		// see if vsgPresent() should be called....
		if (tf.present())
		{
			vsgObjSetTriggers(vsgTRIG_ONPRESENT, 0, 0);
			vsgPresent();
		}

		// now see if any output bits need be written
		if (tf.output_trigger() != last_output_trigger)
		{	
			last_output_trigger = tf.output_trigger();
			vsgFrameSync();
			vsgIOWriteDigitalOut(tf.output_trigger() << 1, 0xfffe);
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
	triggers().addTrigger(new FunctorCallbackTrigger("S", 0x4, 0x4, 0x4, 0x4, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("a", 0x8, 0x8|AR_TRIGGER_TOGGLE, 0x8, 0x8|AR_TRIGGER_TOGGLE, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("X", 0x6, 0x0, 0x6, 0x0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("f", 0x2, 0x0, 0x2, 0x0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("s", 0x4, 0x0, 0x4, 0x0, pfunctor));
	triggers().addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));

	return;
}

void StarUStim::update_page()
{
	vsgSetDrawPage(vsgVIDEOPAGE, 1-m_currentPage, vsgBACKGROUND);
	m_currentPage = 1 - m_currentPage;

	// draw background grating if needed
	if (m_pBackgroundGrating)
	{
		m_pBackgroundGrating->draw();
	}

	// fixpt is drawn at current contrast. 
	// for "fixpt stays on" as target location changes, make sure the fixpt contrast remains at 100
	// when this is called. 

	m_fixpt.draw();

	bool isFixpt = boost::get<0>(m_targets[*m_iterator]);
	unsigned int index = boost::get<1>(m_targets[*m_iterator]);
	int contrast = boost::get<2>(m_targets[*m_iterator]);

	if (isFixpt)
	{
		m_dots[index]->setContrast(0);
		m_dots[index]->draw();
	}
	else
	{
		m_gratings[index]->setContrast(0);
		m_gratings[index]->draw();
	}
}



void StarUStim::init_pages()
{
	m_iterator = m_vecTargetOrder.begin();

	m_fixpt.init(2);
	m_fixpt.setContrast(0);

	// background grating?
	if (m_pBackgroundGrating)
	{
		// adjust settings a bit
		m_pBackgroundGrating->aperture = rectangle;
		m_pBackgroundGrating->x = m_fixpt.x;
		m_pBackgroundGrating->y = m_fixpt.y;

		// get width and height correct using x,y and screen size
		// make sure its large enough that the entire screen is covered
		double d;
		vsgUnit2Unit(vsgPIXELUNIT, vsgGetScreenWidthPixels(), vsgDEGREEUNIT, &d);
		m_pBackgroundGrating->w = m_pBackgroundGrating->h = d;
		m_pBackgroundGrating->wd = m_pBackgroundGrating->hd = 0;

		m_pBackgroundGrating->init(30);

	}
	if (m_dots.size() > 0)
	{
		cerr << "init first dot target" << endl;
		m_dots[0]->init(2);
		if (m_dots.size() > 1)
		{
			cerr << "fake-init " << m_dots.size() - 1 << " additional dot targets" << endl;
			for (unsigned int i = 1; i < m_dots.size(); i++)
			{
				cerr << "fake init dot target " << i << endl;
				m_dots[i]->init(*m_dots[0]);
			}
		}
	}
	if (m_gratings.size() > 0)
	{
		cerr << "init first grating target" << endl;
		m_gratings[0]->init(30);
		if (m_gratings.size() > 1)
		{
			cerr << "fake-init " << m_gratings.size() - 1 << " additional grating targets" << endl;
			for (unsigned int i = 1; i < m_gratings.size(); i++)
			{
				cerr << "fake-init grating target " << i << endl;
				m_gratings[i]->init(*m_gratings[0]);
			}
		}
	}
	update_page();
	vsgPresent();
	cerr << "init done." << endl;
}



int StarUStim::callback(int &output, const FunctorCallbackTrigger* ptrig, const std::string&)
{
	int ival=1;
	string key = ptrig->getKey();

	bool isFixpt = boost::get<0>(m_targets[*m_iterator]);
	unsigned int index = boost::get<1>(m_targets[*m_iterator]);
	int contrast = boost::get<2>(m_targets[*m_iterator]);

	if (key == "a")
	{
		m_iterator++;
		if (m_iterator == m_vecTargetOrder.end()) m_iterator = m_vecTargetOrder.begin();
		update_page();
	}
	else if (key == "s")
	{
		if (isFixpt)
		{
			m_dots[index]->setContrast(0);
		}
		else
		{
			m_gratings[index]->setContrast(0);
		}
	}
	else if (key == "S")
	{
		if (isFixpt)
		{
			m_dots[index]->setContrast(contrast);
		}
		else
		{
			m_gratings[index]->setContrast(contrast);
		}
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
		if (isFixpt)
		{
			m_dots[index]->setContrast(0);
		}
		else
		{
			m_gratings[index]->setContrast(0);
		}
	}

	return ival;
}





