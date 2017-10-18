#include "BarUStim.h"

#define BLANK_PAGE 0
#define FIXPT_PAGE 1
#define BAR_PAGE 2
#define FIXPT_BAR_PAGE 3
#define BAR_DOT_PAGE 4
#define FIXPT_BAR_DOT_PAGE 5

const string BarUStim::m_allowedArgs("avf:t:r:g:b:d:ho:p:B:");

BarUStim::BarUStim()
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

BarUStim::~BarUStim()
{
}

bool BarUStim::parse(int argc, char **argv)
{
	bool b = false;
	int status;
	status = prargs(argc, argv, (process_args_func)NULL, m_allowedArgs.c_str(), 'F', this);
	if (!status) b = true;
	return b;
}

int BarUStim::process_arg(int c, std::string& arg)
{
	static bool have_f=false;		// have fixation spec
	static bool have_d=false;		// have screen dist
	ARContrastFixationPointSpec *pspec;
	ARGratingSpec *pgrating;
	ARContrastRectangleSpec *prect;

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
			m_targets.push_back(boost::make_tuple(TargetType::grating, (unsigned int)(m_gratings.size() - 1), pgrating->contrast));
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
			m_targets.push_back(boost::make_tuple(TargetType::fixpt, (unsigned int)(m_dots.size() - 1), 100));
		}
		else 
		{
			m_errflg++;
			cerr << "Error in target spec (" << arg << ")" << endl;
			delete(pspec);
		}
		break;
	case 'r':
		prect = new ARContrastRectangleSpec();
		if (!parse_rectangle(arg, *prect))
		{
			m_rectangles.push_back(prect);
			m_targets.push_back(boost::make_tuple(TargetType::rectangle, (unsigned int)(m_rectangles.size() - 1), 100));
		}
		else
		{
			m_errflg++;
			cerr << "Error in target spec (" << arg << ")" << endl;
			delete(prect);
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



void BarUStim::run_stim(alert::ARvsg& vsg)
{
	cout << "BarUStim::run_stim(): started" << endl;

	// clear all dig outputs
	vsgIOWriteDigitalOut(0, 0xff);
	vsgPresent();

	// initialize triggers
	TSpecificFunctor<BarUStim> functor(this, &BarUStim::callback);
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


void BarUStim::init_triggers(TSpecificFunctor<BarUStim>* pfunctor)
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

void BarUStim::update_page()
{
	TargetType t = boost::get<0>(m_targets[*m_iterator]);
	unsigned int index = boost::get<1>(m_targets[*m_iterator]);
	int contrast = boost::get<2>(m_targets[*m_iterator]);
	VSGPAGEDESCRIPTOR descr;
	vsgGetCurrentDrawPage(&descr);
	vsgSetDrawPage(vsgVIDEOPAGE, BAR_DOT_PAGE, vsgBACKGROUND);
	if (m_pBackgroundGrating)
		m_pBackgroundGrating->draw();
	if (t == TargetType::fixpt)
	{
		m_dots[index]->setContrast(contrast);
		m_dots[index]->draw();
	}
	else if (t == TargetType::grating)
	{
		m_gratings[index]->setContrast(contrast);
		m_gratings[index]->draw();
	}
	else if (t == TargetType::rectangle)
	{
		m_rectangles[index]->setContrast(contrast);
		m_rectangles[index]->draw();
	}

	vsgSetDrawPage(vsgVIDEOPAGE, FIXPT_BAR_DOT_PAGE, vsgBACKGROUND);
	if (m_pBackgroundGrating)
		m_pBackgroundGrating->draw();
	m_fixpt.draw();
	if (t == TargetType::fixpt)
	{
		m_dots[index]->setContrast(contrast);
		m_dots[index]->draw();
	}
	else if (t == TargetType::grating)
	{
		m_gratings[index]->setContrast(contrast);
		m_gratings[index]->draw();
	}
	else if (t == TargetType::rectangle)
	{
		m_rectangles[index]->setContrast(contrast);
		m_rectangles[index]->draw();
	}

	vsgSetDrawPage(descr.PageZone, descr.Page, vsgNOCLEAR);
}

void BarUStim::init_pages()
{
	m_iterator = m_vecTargetOrder.begin();

	m_fixpt.init(2);
	m_fixpt.setContrast(100);

	// background grating?
	if (m_pBackgroundGrating)
	{
		m_pBackgroundGrating->init(30);
	}

	if (m_dots.size() > 0)
	{
		cerr << "init first target" << endl;
		m_dots[0]->init(2);
		for (unsigned int i = 1; i<m_dots.size(); i++)
		{
			cerr << "init target " << i << endl;
			m_dots[i]->init(*m_dots[0]);
		}
	}
	if (m_gratings.size() > 0)
	{
		cerr << "init first grating" << endl;
		m_gratings[0]->init(30);
		for (unsigned int i = 1; i < m_gratings.size(); i++)
		{
			cerr << "init grating " << i << endl;
			m_gratings[i]->init(*m_gratings[0]);
		}
	}
	if (m_rectangles.size() > 0)
	{
		cerr << "init first rectangle" << endl;
		m_rectangles[0]->init(2);
		for (unsigned int i = 1; i < m_rectangles.size(); i++)
		{
			cerr << "init rectangle " << i << endl;
			m_rectangles[i]->init(*m_rectangles[0]);
		}
	}

	// init pages except for those with stim - those are init'd in update_pages

	vsgSetDrawPage(vsgVIDEOPAGE, FIXPT_PAGE, vsgBACKGROUND);
	m_fixpt.draw();

	vsgSetDrawPage(vsgVIDEOPAGE, BAR_PAGE, vsgBACKGROUND);
	if (m_pBackgroundGrating)
		m_pBackgroundGrating->draw();

	vsgSetDrawPage(vsgVIDEOPAGE, FIXPT_BAR_PAGE, vsgBACKGROUND);
	if (m_pBackgroundGrating)
		m_pBackgroundGrating->draw();
	m_fixpt.draw();

	// do this page past so it will be displayed initially. Note that update_page should leave
	// the current draw page the same on exiting. 
	vsgSetDrawPage(vsgVIDEOPAGE, BLANK_PAGE, vsgBACKGROUND);


	cerr << "udpate page" << endl;
	update_page();
	vsgPresent();
}



int BarUStim::callback(int &output, const FunctorCallbackTrigger* ptrig)
{
	int ival=1;
	string key = ptrig->getKey();

	TargetType t = boost::get<0>(m_targets[*m_iterator]);
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
		// page 4 or 5 have stim - subtract 2 to get same page without stim
		VSGPAGEDESCRIPTOR descr;
		vsgGetCurrentDrawPage(&descr);
		if (descr.Page == 4 || descr.Page == 5)
			vsgSetDrawPage(vsgVIDEOPAGE, descr.Page - 2, vsgNOCLEAR);
	}
	else if (key == "S")
	{
		// page 2 or 3 have fixpt+rect - add 2 to get same page with stim
		// NOTE - "S" does not work from page 0 or 1!
		VSGPAGEDESCRIPTOR descr;
		vsgGetCurrentDrawPage(&descr);
		if (descr.Page == 2 || descr.Page == 3)
			vsgSetDrawPage(vsgVIDEOPAGE, descr.Page + 2, vsgNOCLEAR);
	}
	else if (key == "F")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, 3, vsgNOCLEAR);
	}
	else if (key == "f")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgNOCLEAR);
	}
	else if (key == "X")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	}
	else if (key == "0")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	}
	else if (key == "1")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
	}
	else if (key == "2")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgNOCLEAR);
	}
	else if (key == "3")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, 3, vsgNOCLEAR);
	}
	else if (key == "4")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, 4, vsgNOCLEAR);
	}
	else if (key == "5")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, 5, vsgNOCLEAR);
	}

	return ival;
}





