#include "attention.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__

#include "vsgv8.h"
#include "Alertlib.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")


int args(int argc, char **argv);

using namespace std;
using namespace alert;

ARContrastFixationPointSpec m_spec_fixpt;
ARContrastFixationPointSpec m_spec_anspt_up, m_spec_anspt_down;
ARContrastCircleSpec m_spec_stim_circle, m_spec_dist_circle;
ARContrastLineSpec m_spec_stim_line;
ARContrastLineSpec m_spec_dist_line;
double m_circle_diameter_differential = 0.1;
double m_cue_line_fraction = 0.75;
double m_cue_line_gap = .5;
int m_iLollipopContrast = 100;
int m_iLollipopStickContrast = 100;
bool m_bLollipops = false;
double m_anspt_offset_degrees = 5;
double m_anspt_diameter_degrees = 0.5;
COLOR_TYPE m_background;
ARGratingSpec m_spec_stimulus;
bool m_bstimulus=false;
int m_iStimulusOriginalContrast;
ARGratingSpec m_spec_distractor;
bool m_bdistractor=false;
int m_iDistractorOriginalContrast;
int m_screenDistanceMM=0;
bool m_verbose=false;
TriggerVector triggers;
bool m_binaryTriggers = true;
bool m_bTrainingContrast = false;
vector<double> m_contrasts;
vector<double>::const_iterator m_iterator;
int m_iContrastDown=0;
int m_iContrastBase=50;
int m_iContrastUp=100;
bool m_bFStimulus;
bool m_bNoAnswerPoints = false;
int m_imageCount = 0;
bool m_bCueCircles = false;
bool m_bSingleStim = false;
bool m_bStimulusOn = false;
bool m_bDistractorOn = false;

static void usage();
static int init_pages();
static int init_answer_points();
static int prargs_callback(int c, string& arg);


int main (int argc, char *argv[])
{
	int status;
	std::string s;
	int last_output_trigger=0;


	// Check input arguments
	status = prargs(argc, argv, prargs_callback, "f:b:g:hd:vas:t:A:D:T:c:l:p:P:LG:NQS", 'F');
	if (status)
	{
		return -1;
	}
	else
	{
		if (m_verbose)
		{
			cout << "Screen distance " << m_screenDistanceMM << endl;
			cout << "Fixation point" << m_spec_fixpt << endl;
			cout << "Background color " << m_background << endl;
			cout << "Stimulus : " << m_spec_stimulus << endl;
			cout << "Distractor : " << m_spec_distractor << endl;
		}
	}

	// Init answer points - geometry only
	if (!m_bNoAnswerPoints)
	{
		init_answer_points();
	}

	// INit vsg
	if (ARvsg::instance().init(m_screenDistanceMM, m_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}

	// Set pulse trigger bit high so vsgPresent pulses are downward-going. 
	//Sleep(100);
	//vsgIOWriteDigitalOut(vsgDIG0, vsgDIG0); 
	// reset all output triggers
	vsgIOWriteDigitalOut(0, 0xff);


	// write video pages
	init_pages();


	// Dump triggers
	{
		for (unsigned int i=0; i<triggers.size(); i++)
		{
			std::cout << "Trigger " << i << " " << *(triggers[i]) << std::endl;
		}
	}


	ARvsg::instance().ready_pulse(500);

	// All right, start monitoring triggers........
	while (1)
	{
		// If user-triggered, get a trigger entry. 
		if (!m_binaryTriggers)
		{
			// Get a new "trigger" from user
			cout << "Enter trigger/key: ";
			cin >> s;
		}

		TriggerFunc	tf = std::for_each(triggers.begin(), triggers.end(), 
			(m_binaryTriggers ? TriggerFunc(vsgIOReadDigitalIn(), last_output_trigger) : TriggerFunc(s, last_output_trigger)));

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

	ARvsg::instance().clear();


	return 0;
}

//int args(int argc, char **argv)

int prargs_callback(int c, string& arg)
{	
	static bool have_f=false;
	static bool have_d=false;
	static bool have_b=false;
	static bool have_t=false;
	static bool have_s=false;
	static bool have_g=false;
	int status = 0;
	int iunused;
	ARGratingSpec agtemp;
	switch (c) 
	{
	case 'a':
		m_binaryTriggers = false;
		break;
	case 'v':
		m_verbose = true;
		break;
	case 'f': 
		if (parse_fixation_point(arg, m_spec_fixpt)) status++;
		else have_f = true;
		break;
	case 'b': 
		if (parse_color(arg, m_background)) status++; 
		else have_b = true;
		break;
	case 'd':
		if (parse_distance(arg, m_screenDistanceMM)) status++;
		else have_d=true;
		break;
	case 's':
		if (!parse_grating(arg, m_spec_stimulus))
		{
			have_s = true;
			m_iStimulusOriginalContrast = m_spec_stimulus.contrast;
		}
		else status++;
		break;
	case 'g':
		if (!parse_grating(arg, m_spec_distractor))
		{
			have_g = true;
			m_iDistractorOriginalContrast = m_spec_distractor.contrast;
		}
		else status++;
		break;
	case 't':
		if (parse_tuning_list(arg, m_contrasts, iunused)) status++;
		else 
		{
			// check that the number is divisible by three....
			if (m_contrasts.size() % 3)
			{
				cerr << "Contrast list must have #entries be a multiple of 3." << endl;
				status++;
			}
			else
			{
				have_t = true;
				m_iterator = m_contrasts.begin();
				m_iContrastDown = (int)*m_iterator;
				m_iContrastBase = (int)*(m_iterator+1);
				m_iContrastUp = (int)*(m_iterator+2);
			}
		}
		break;
	case 'A':
		if (parse_double(arg, m_anspt_offset_degrees)) status++;
		break;
	case 'D':
		if (parse_double(arg, m_anspt_diameter_degrees)) status++;
		break;
	case 'c':
		if (parse_double(arg, m_circle_diameter_differential)) 
		{
			cerr << "Bad circle diameter differential: " << arg << endl;
			status++;
		}
		break;
	case 'l':
		if (parse_double(arg, m_cue_line_fraction)) 
		{
			cerr << "Bad cue line fraction: " << arg << endl;
			status++;
		}
		break;
	case 'G':
		if (parse_double(arg, m_cue_line_gap)) 
		{
			cerr << "Bad cue line gap: " << arg << endl;
			status++;
		}
		break;
	case 'p':
		if (parse_integer(arg, m_iLollipopContrast)) 
		{
			cerr << "Bad lollipop contrast value: " << arg << endl;
			status++;
		}
		break;
	case 'P':
		if (parse_integer(arg, m_iLollipopStickContrast)) 
		{
			cerr << "Bad lollipop stick contrast value: " << arg << endl;
			status++;
		}
		break;
	case 'L':
		m_bLollipops = true;
		break;
	case 'N':
		m_bNoAnswerPoints = true;
		break;
	case 'Q':
		m_bCueCircles = true;
		break;
	case 'S':
		m_bSingleStim = true;
		break;
	case 'h':
		status++;
		break;
	case '?':
        status++;
		break;
	case 0:
		if (!have_s)
		{
			cerr << "Stimulus not specified!" << endl;
			status++;
		}
		if (!have_g)
		{
			cerr << "Distractor not specified!" << endl;
			status++;
		}
		if (!have_f) 
		{
			cerr << "Fixation point not specified!" << endl; 
			status++;
		}
		if (!have_d)
		{
			cerr << "Screen distance not specified!" << endl; 
			status++;
		}
		if (!have_b)
		{
			cerr << "Background color not specified!" << endl;
			status++;
		}
		if (!have_t)
		{
			cerr << "Contrast triplet not specified!" << endl;
			status++;
		}
		break;
	default:
		status++;
		break;
	}

	return status;
}

void usage()
{
	cerr << "usage: attention -f x,y,d[,color] -d screen_distance_MM -b g|b|w [-s x,y,w,h,contrast%,sf,tf,orientation,color_vector,s|q,r|e] -g distractor_settings" << endl;
}


int init_answer_points()
{
	m_spec_anspt_up = m_spec_fixpt;
	m_spec_anspt_down = m_spec_fixpt;

	m_spec_anspt_up.x = m_spec_fixpt.x;
	m_spec_anspt_up.y = m_spec_fixpt.y + m_anspt_offset_degrees;
	m_spec_anspt_up.d = m_anspt_diameter_degrees;
	m_spec_anspt_down.x = m_spec_fixpt.x;
	m_spec_anspt_down.y = m_spec_fixpt.y -  + m_anspt_offset_degrees;
	m_spec_anspt_down.d = m_anspt_diameter_degrees;

	return 0;
}

// The return value from this trigger callback determines whether a vsgPresent() is issued. 

int callback(int &output, const CallbackTrigger* ptrig)
{
	int ival=1;
	VSGTRIVAL from, to;
	string key = ptrig->getKey();

	if (key == "S")
	{
		if (!m_bNoAnswerPoints)
		{
			m_spec_anspt_up.setContrast(100); 
			m_spec_anspt_down.setContrast(100);
		}

		// ignoring training contrast for now. 

		if ((m_bFStimulus && m_bSingleStim) || !m_bSingleStim)
		{
			// Rather than redraw stim, just reset drift phase and set contrast. Will become visible on present.
			m_spec_stimulus.setContrast(m_iContrastBase);
			m_spec_stimulus.select();
			vsgObjResetDriftPhase();
			m_bStimulusOn = true;
		}

		if ((!m_bFStimulus && m_bSingleStim) || !m_bSingleStim)
		{
			// Rather than redraw distractor, just reset drift phase and set contrast. Will become visible on present.
			m_spec_distractor.setContrast(m_iContrastBase);
			m_spec_distractor.select();
			vsgObjResetDriftPhase();
			m_bDistractorOn = true;
		}
	}
	else if (key == "s")
	{

		m_spec_stimulus.setContrast(0);
		m_spec_distractor.setContrast(0);
		m_bStimulusOn = false;
		m_bDistractorOn = false;
		if (m_bLollipops)
		{
			m_spec_stim_circle.setContrast(0);
			m_spec_dist_circle.setContrast(0);
			m_spec_stim_line.setContrast(0);
			m_spec_dist_line.setContrast(0);
		}
	}
	else if (key == "X")
	{
		m_spec_fixpt.setContrast(0);
		if (!m_bNoAnswerPoints)
		{
			m_spec_anspt_up.setContrast(0); 
			m_spec_anspt_down.setContrast(0);
		}

		m_spec_stimulus.setContrast(0);
		m_spec_distractor.setContrast(0);
		m_bStimulusOn = false;
		m_bDistractorOn = false;

		if (m_bLollipops)
		{
			m_spec_stim_circle.setContrast(0);
			m_spec_dist_circle.setContrast(0);
			m_spec_stim_line.setContrast(0);
			m_spec_dist_line.setContrast(0);
		}

		if (m_bCueCircles)
		{
			// If the cue circle pointer is non-null, then turn it off 
			if (m_bFStimulus) m_spec_stim_circle.setContrast(0);
			else m_spec_dist_circle.setContrast(0);
		}

	}
	else if (key == "F")
	{
		get_color(m_background, from);
		to.a = 1; to.b = to.c = 0;
		m_spec_fixpt.select();
		vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);
		m_spec_fixpt.setContrast(100);
		m_bFStimulus = true;

		if (m_bLollipops)
		{
			m_spec_stim_circle.setContrast(m_iLollipopContrast);
			m_spec_dist_circle.setContrast(m_iLollipopContrast);
			m_spec_stim_line.setContrast(m_iLollipopStickContrast);
		}
	}
	else if (key == "f")
	{
		m_spec_fixpt.setContrast(0);
		if (m_bLollipops)
		{
			m_spec_stim_circle.setContrast(0);
			m_spec_dist_circle.setContrast(0);
			m_spec_stim_line.setContrast(0);
			m_spec_dist_line.setContrast(0);
		}
	}
	else if (key == "G")
	{
		get_color(m_background, from);
		to.b = 1; to.a = to.c = 0;
		m_spec_fixpt.select();
		vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);
		m_spec_fixpt.setContrast(100);
		if (m_bLollipops)
		{
			m_spec_stim_circle.setContrast(m_iLollipopContrast);
			m_spec_dist_circle.setContrast(m_iLollipopContrast);
			m_spec_dist_line.setContrast(m_iLollipopStickContrast);
		}
		m_bFStimulus = false;
	}
	else if (key == "g")
	{
		m_spec_fixpt.setContrast(0);
		if (m_bLollipops)
		{
			m_spec_stim_circle.setContrast(0);
			m_spec_dist_circle.setContrast(0);
			m_spec_stim_line.setContrast(0);
			m_spec_dist_line.setContrast(0);
		}
	}
	else if (key == "W")
	{
		if (m_bCueCircles)
		{
			// Which cue circle we turn on depends on which fixation point was used. 
			if (m_bFStimulus) m_spec_stim_circle.setContrast(100);
			else m_spec_dist_circle.setContrast(100);
		}
	}
	else if (key == "w")
	{
		if (m_bCueCircles)
		{
			if (m_bFStimulus) m_spec_stim_circle.setContrast(0);
			else m_spec_dist_circle.setContrast(0);
		}
	}
	else if (key == "C")
	{
		if (!m_bSingleStim || (m_bSingleStim && m_bFStimulus))
		{
			if (m_bStimulusOn)
			{
				m_spec_stimulus.setContrast(m_iContrastUp);
			}
			else
			{
				// djs 8-16-10
				// If the stimulus is NOT on and this command is received then we display the 
				// stimulus at its original contrast (the contrast used in the command line
				// arg for the stim). 
				m_spec_stimulus.setContrast(m_iStimulusOriginalContrast);
				m_bStimulusOn = true;
			}
		}

		if (m_bSingleStim && !m_bFStimulus)
		{
			cerr << endl << "Warning! In single stim mode (-S), cannot change contrast of non-cued stimulus!" << endl << endl;
		}
	}
	else if (key == "c")
	{
		if (!m_bSingleStim || (m_bSingleStim && m_bFStimulus))
		{
			m_spec_stimulus.setContrast(m_iContrastDown);
		}

		if (m_bSingleStim && !m_bFStimulus)
		{
			cerr << endl << "Warning! In single stim mode (-S), cannot change contrast of non-cued stimulus!" << endl << endl;
		}
	}
	else if (key == "D")
	{
		if (!m_bSingleStim || (m_bSingleStim && !m_bFStimulus))
		{
			if (m_bDistractorOn)
			{
				m_spec_distractor.setContrast(m_iContrastUp);
			}
			else
			{
				m_spec_distractor.setContrast(m_iDistractorOriginalContrast);
				m_bDistractorOn = true;
			}
		}

		if (m_bSingleStim && m_bFStimulus)
		{
			cerr << endl << "Warning! In single stim mode (-S), cannot change contrast of non-cued stimulus!" << endl << endl;
		}
	}
	else if (key == "d")
	{
		if (!m_bSingleStim || (m_bSingleStim && !m_bFStimulus))
		{
			m_spec_distractor.setContrast(m_iContrastDown);
		}

		if (m_bSingleStim && m_bFStimulus)
		{
			cerr << endl << "Warning! In single stim mode (-S), cannot change contrast of non-cued stimulus!" << endl << endl;
		}
	}
	else if (key == "a")
	{
		cout << "Advance trigger received." << endl;
		m_iterator += 3;
		if (m_iterator == m_contrasts.end())
		{
			cout << "Reached end of contrast triplets. Start over at beginning...." << endl;
			m_iterator = m_contrasts.begin();
		}
		m_iContrastDown = (int)*m_iterator;
		m_iContrastBase = (int)*(m_iterator+1);
		m_iContrastUp = (int)*(m_iterator+2);
	}
	else if (key == "i")
	{
		char filename[256];
		int ii;
		int w, h;
		w = vsgGetScreenWidthPixels();
		h = vsgGetScreenHeightPixels();
		vsgSetSpatialUnits(vsgPIXELUNIT);
		sprintf_s(filename, "attention-%d.bmp", m_imageCount);
		ii = vsgImageExport(vsgBMPPICTURE, 0, 0, w, h, filename);
		vsgSetSpatialUnits(vsgDEGREEUNIT);
		cout << "Image trigger: wrote " << w << "x" << h << " image " << m_imageCount << " status " << ii << endl;
		m_imageCount++;
	}

	return ival;
}



// There are just two pages. Page 0 is blank background. 
// Page 1 has all the objects (fixation point, answer points, stimulus grating, distractor). 
// All triggers set (see init_triggers) are ContrastTriggers, meaning their action (see ContrastTrigger::execute)
// is to change the contrast of one (or more) of the objects. For convenience we wrap the objects in an ARObj
// and call ARObj::draw once (supplying pixel levels we have allocated, the background color). 
// 
// Create ARObjs here and set up triggers as well. Remember that the ContrastTrigger does cleanup on the ARObj!


int init_pages()
{
	int status=0;
	int islice=50;
	COLOR_TYPE r = { red, {0,0,0} };
	COLOR_TYPE g = { green, {0,0,0} };
	std::vector<std::pair<std::string, int> >vecInputs;

	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	
	m_spec_stimulus.init(islice);
	m_spec_stimulus.draw();
	m_spec_stimulus.setContrast(0);

	m_spec_distractor.init(islice);
	m_spec_distractor.draw();
	m_spec_distractor.setContrast(0);

	// Now fixation point
	m_spec_fixpt.init(2);
	m_spec_fixpt.draw();
	m_spec_fixpt.setContrast(0);

	// Now answer point - upper

	if (!m_bNoAnswerPoints)
	{
		m_spec_anspt_up.init(2);
		m_spec_anspt_up.draw();
		m_spec_anspt_up.setContrast(0);
		m_spec_anspt_down.init(2);
		m_spec_anspt_down.draw();
		m_spec_anspt_down.setContrast(0);
	}

	// Lollipops if necessary
	if (m_bLollipops)
	{
		m_spec_stim_circle.x = m_spec_stimulus.x;
		m_spec_stim_circle.y = m_spec_stimulus.y;
		m_spec_stim_circle.d = m_spec_stimulus.w + m_circle_diameter_differential;	// assuming circular stim....
		m_spec_stim_circle.color = r;
		m_spec_stim_circle.init(2);
		m_spec_stim_circle.draw();
		m_spec_stim_circle.setContrast(0);

		m_spec_dist_circle.x = m_spec_distractor.x;
		m_spec_dist_circle.y = m_spec_distractor.y;
		m_spec_dist_circle.d = m_spec_distractor.w + m_circle_diameter_differential;	// assuming circular stim....
		m_spec_dist_circle.color = g;
		m_spec_dist_circle.init(2);
		m_spec_dist_circle.draw();
		m_spec_dist_circle.setContrast(0);

		// Stimulus line
		double v[2];
		double dtotal;		// center-to-center
		double dmax;		// edge of stim circle to gap distance

		m_spec_stim_line.x0 = 0;
		m_spec_stim_line.y0 = 0;
		m_spec_stim_line.x1 = 0;
		m_spec_stim_line.y1 = 0;
		m_spec_stim_line.color = r;

		v[0] = m_spec_fixpt.x - m_spec_stimulus.x;
		v[1] = -1*(m_spec_fixpt.y - m_spec_stimulus.y);
		dtotal = sqrt(v[0]*v[0] + v[1]*v[1]);
		
		if (dtotal > 0)
		{
			v[0] = v[0]/dtotal;
			v[1] = v[1]/dtotal;
			dmax = dtotal - m_spec_stimulus.w/2 - m_spec_fixpt.d/2 - m_circle_diameter_differential/2 - m_cue_line_gap;
			m_spec_stim_line.x0 = m_spec_stimulus.x + 
				v[0] * ((m_spec_stimulus.w + m_circle_diameter_differential)/2 + dmax * (1 - m_cue_line_fraction));
			m_spec_stim_line.y0 = -m_spec_stimulus.y + 
				v[1] * ((m_spec_stimulus.w + m_circle_diameter_differential)/2 + dmax * (1 - m_cue_line_fraction));
			m_spec_stim_line.x1 = m_spec_stim_line.x0 + v[0] * dmax * m_cue_line_fraction;
			m_spec_stim_line.y1 = m_spec_stim_line.y0 + v[1] * dmax * m_cue_line_fraction;
		}
		m_spec_stim_line.init(2);
		m_spec_stim_line.draw();
		m_spec_stim_line.setContrast(0);

		m_spec_dist_line.x0 = 0;
		m_spec_dist_line.y0 = 0;
		m_spec_dist_line.x1 = 0;
		m_spec_dist_line.y1 = 0;
		m_spec_dist_line.color = g;

		v[0] = m_spec_fixpt.x - m_spec_distractor.x;
		v[1] = -1*(m_spec_fixpt.y - m_spec_distractor.y);
		dtotal = sqrt(v[0]*v[0] + v[1]*v[1]);
		if (dtotal > 0)
		{
			v[0] = v[0]/dtotal;
			v[1] = v[1]/dtotal;
			dmax = dtotal - m_spec_distractor.w/2 - m_spec_fixpt.d/2 - m_circle_diameter_differential/2 - m_cue_line_gap;
			m_spec_dist_line.x0 = m_spec_distractor.x + 
				v[0] * ((m_spec_distractor.w + m_circle_diameter_differential)/2 + dmax * (1 - m_cue_line_fraction));
			m_spec_dist_line.y0 = -m_spec_distractor.y + 
				v[1] * ((m_spec_distractor.w + m_circle_diameter_differential)/2 + dmax * (1 - m_cue_line_fraction));
			m_spec_dist_line.x1 = m_spec_dist_line.x0 + v[0] * dmax * m_cue_line_fraction;
			m_spec_dist_line.y1 = m_spec_dist_line.y0 + v[1] * dmax * m_cue_line_fraction;
		}
		m_spec_dist_line.init(2);
		m_spec_dist_line.draw();
		m_spec_dist_line.setContrast(0);

	}
	else if (m_bCueCircles)
	{
		m_spec_stim_circle.x = m_spec_stimulus.x;
		m_spec_stim_circle.y = m_spec_stimulus.y;
		m_spec_stim_circle.d = m_spec_stimulus.w * sqrt(2.0f) + m_circle_diameter_differential;
		m_spec_stim_circle.color = r;
		m_spec_stim_circle.init(2);
		m_spec_stim_circle.draw();
		m_spec_stim_circle.setContrast(0);

		m_spec_dist_circle.x = m_spec_distractor.x;
		m_spec_dist_circle.y = m_spec_distractor.y;
		m_spec_dist_circle.d = m_spec_distractor.w * sqrt(2.0f) + m_circle_diameter_differential;
		m_spec_dist_circle.color = g;
		m_spec_dist_circle.init(2);
		m_spec_dist_circle.draw();
		m_spec_dist_circle.setContrast(0);
	}



	// Triggers are slightly different if using the cue circles - that's because we need a separate 
	// trigger (and response bit) for the cue circles. I'm using the W trigger for cue circles - that
	// means that when using cue circles you don't get to use the white spot. 

	vecInputs.push_back(std::pair< string, int>("F", 0x2));
	vecInputs.push_back(std::pair< string, int>("G", 0x4));
	triggers.push_back(new MultiInputSingleOutputCallbackTrigger(vecInputs, 0x66, 0x1, 0x1, callback));
	triggers.addTrigger(new CallbackTrigger("X", 0x6E, 0x0, 0xf, 0x0, callback));

	if (m_bCueCircles)
	{
		triggers.addTrigger(new CallbackTrigger("W", 0x10, 0x10, 0x4, 0x4, callback));
		triggers.addTrigger(new CallbackTrigger("w", 0x10, 0x0, 0x4, 0x0, callback));
	}	


	// trigger to turn stim, distractor and answer points ON
	triggers.addTrigger(new CallbackTrigger("S", 0x8, 0x8, 0x2, 0x2, callback));

	// trigger to turn stimand distractor OFF (answer points remain on)
	triggers.addTrigger(new CallbackTrigger("s", 0x8, 0x0, 0x2, 0x0, callback));

	// Triggers for contrast change. These were ContrastTriggers, but changed them to callback triggers when 
	// adding "SingleStim" option. When using SingleStim I want to be able to guard against the WRONG contrast
	// change trigger being issued - that would cause the non-displayed stim to suddenly appear when its 
	// contrast is changed. 
	triggers.addTrigger(new CallbackTrigger("C", 0x6E, 0x2A, 0x8, 0x8, callback));
	triggers.addTrigger(new CallbackTrigger("c", 0x6E, 0x4A, 0x8, 0x8, callback));
	triggers.addTrigger(new CallbackTrigger("D", 0x6E, 0x2C, 0x8, 0x8, callback));
	triggers.addTrigger(new CallbackTrigger("d", 0x6E, 0x4C, 0x8, 0x8, callback));

	// Image trigger. Will only work with ascii triggers (i.e. keyboard). 
	triggers.addTrigger(new CallbackTrigger("i", 0x0, 0x0, 0x0, 0x0, callback));

	// advance trigger
	triggers.addTrigger(new CallbackTrigger("a", 0x80, 0x80|AR_TRIGGER_TOGGLE, 0x10, 0x10|AR_TRIGGER_TOGGLE, callback));
	//triggers.addTrigger(new CallbackTrigger("a", 0x8, 0x8|AR_TRIGGER_TOGGLE, 0x8, 0x8|AR_TRIGGER_TOGGLE, callback));

	// quit trigger
	triggers.addTrigger(new QuitTrigger("q", 0x7E, 0x7E, 0xff, 0x0, 0));

	return status;
}

