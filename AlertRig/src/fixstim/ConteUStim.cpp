#include "ConteUStim.h"

#include "alert-argp.h"
#include "alert-triggers.h"
#include "ARtypes.h"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>



// for argp
error_t parse_conte_opt(int key, char* carg, struct argp_state* state);
static struct argp_option options[] = {
	{"ascii", 'a', 0, 0, "Use ascii triggers (user-entered)"},
	{"verbose",  'v', 0, 0, "Produce verbose output" },
	{"background", 'b', "COLOR", 0, "background color"},
	{"distance-to-screen", 'd', "DIST_MM", 0, "screen distance in MM"},
	{"ready-pulse", 'p', "BITPATTERN", 0, "Ready pulse issued when startup is complete"},
	{"ready-pulse-delay", 'l', "DELAY_MS", 0, "Delay ready pulse for this many ms"},
	{"dot-supply", 701, "filename", 0, "File with x,y positions in [-0.5,0.5]"},
	{"conte", 702, "CONTE_SPEC", 0, "Conte stim specification"},
	{ 0 }
};
static struct argp f_argp = { options, parse_conte_opt, 0, "fixstim -- all-purpose stimulus engine" };


error_t parse_conte_opt(int key, char* carg, struct argp_state* state)
{
	error_t ret = 0;
	struct conte_arguments* arguments = (struct conte_arguments*)state->input;
	std::string sarg;
	if (carg) sarg = carg;
	switch (key)
	{
	case 'a':
		arguments->bBinaryTriggers = false;
		break;
	case 'v':
		arguments->bVerbose = true;
		break;
	case 'b':
		if (parse_color(sarg, arguments->bkgdColor))
			ret = EINVAL;
		break;
	case 'd':
		if (parse_distance(sarg, arguments->iDistanceToScreenMM))
			ret = EINVAL;
		else
			arguments->bHaveDistance = true;
		break;
	case 'l':
		if (parse_integer(sarg, arguments->iReadyPulseDelay))
			ret = EINVAL;
		break;
	case 'p':
		if (parse_integer(sarg, arguments->iPulseBits))
			ret = EINVAL;
		break;
	case 701:
		if (!boost::filesystem::exists(sarg))
		{
			cerr << "Cannot find dot supply file" << endl;
			ret = EINVAL;
		}
		else
		{
			if (parse_dot_supply_file(sarg))
			{
				cerr << "Error parsing dot supply file" << endl;
				ret = EINVAL;
			}
		}
		break;
	case 702:
		// parse conte stim spec
		break;
	case ARGP_KEY_END:
		// check that everything needed has been received
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return ret;
}






ConteUStim::ConteUStim()
: UStim()
, m_binaryTriggers(true)
, m_verbose(false)
, m_iDistanceToScreenMM(-1)
, m_pulse(0x2)
, m_errflg(0)
, m_currentPage(1)
{
	m_background.setType(gray);
};

ConteUStim::~ConteUStim()
{
}

bool ConteUStim::parse(int argc, char** argv)
{
	error_t ret = alert_argp_parse(&f_argp, argc, argv, ARGP_NO_EXIT, 0, &m_arguments, 'F');
	return (!ret);
}

int ConteUStim::process_arg(int c, std::string& arg)
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



void ConteUStim::run_stim(alert::ARvsg& vsg)
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
			cout << "out trig " << hex << tf.output_trigger() << endl;
			//vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
			vsgIOWriteDigitalOut(tf.output_trigger() << 1, 0xffff);
			vsgPresent();
		}
		Sleep(10);
	}

	vsg.clear();

	return ;
}


void ConteUStim::init_triggers(TSpecificFunctor<StarUStim>* pfunctor)
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

void ConteUStim::update_page()
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



void ConteUStim::init_pages()
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
	cerr << "udpate page" << endl;
	update_page();
	vsgPresent();
}



int ConteUStim::callback(int &output, const FunctorCallbackTrigger* ptrig, const std::string&)
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


bool parse_dot_supply_file(const std::string& filename)
{
	bool bReturn = false;

	boost::filesystem::path p(filename);
	if (!exists(p))
	{
		std::cerr << "Error: dot supply file does not exist: " << filename << endl;
		return false;
	}
	else
	{
		bReturn = true;
		boost::filesystem::path folder = p.parent_path();		// if file has relative pathnames to images, they are relative to dir file lives in
		std::cerr << "Found dot supply file " << p << " at path " << folder << std::endl;

		// open file, read line-by-line and parse
		string line;
		int linenumber = 0;		// count lines from 0
		int imagecount = 0;
		int groupcount = 0;
		bool doingImages = true;
		bool haveGroups = false;				// only set to true if Groups line found
		double dFixptSec = 0, dImageSec = 0;	// these are only valid if haveGroups == true

		std::ifstream myfile(filename.c_str());
		if (myfile.is_open())
		{
			while (getline(myfile, line))
			{
				linenumber++;

				// line may look like these things
				// 1) c:\path\file.bmp                    filename only. Position will be 0,0 duration=delay=0
				// 2) c:\path\file.bmp,x,y                filename, position (in current units), duration=delay=0
				// 3) c:\path\file.bmp,x,y,durSec,dlySec  all 5 params. 
				// 4) <blank>                             skip this line
				// 5) # anything                          comment, ignored
				// 6) Groups i,f                          start of image groups, i=image sec, f=fixpt sec
				// 7) i0,i1,i2[...]                       list of image indices and order for a single group, only valid after Groups line

				// skip comment line
				if (line[0] == '#')
					continue;

				// skip empty lines
				boost::algorithm::trim(line);
				if (line.size() == 0)
					continue;

				if (doingImages)
				{
					// If "Groups" found on line, break out and read groups.
					if (line.find("Groups") == 0)
					{
						doingImages = false;
						haveGroups = true;
					}
					else
					{
						// tokenize/parse this line. It should be one of the filename variants.
						string sUseThisFilename;
						vector<string> tokens;
						tokenize(line, tokens, ",");
