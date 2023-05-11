#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__

#include "vsgv8.h"
#include "Alertlib.h"
#include "AlertUtil.h"
#include "alert-triggers.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment (lib, "vsgv8.lib")

int args(int argc, char **argv);
void init_triggers();
int callback(int &output, const alert::CallbackTrigger* ptrig);

using namespace std;
using namespace alert;

ARConteSpec f_aConte, f_bConte;
ARContrastFixationPointSpec m_afp;
COLOR_TYPE m_background;
vector<ARGratingSpec*> m_gratings;
int m_screenDistanceMM=0;
bool m_verbose=false;
TriggerVector triggers;
bool m_binaryTriggers = true;
bool m_bCalibration = false;
const int f_iPage0 = 0;
const int f_iPage1 = 1;
const int f_iPageBlank = 2;

//struct CGabor {
//	double x, y, w, h;
//	double ori, sf, dev, ph;
//	bool bHorizontal;
//	double distractor_factor;	
//	DWORD cue_line_width;
//	COLOR_TYPE cue_color;
//};
//CGabor f_gabor_A;
//CGabor f_gabor_B;

//int parse_gabor(const std::string& s, CGabor& gabor);
//void usage();
int init_pages();
//void copyConteSpec(ARConteSpec& conte, const CGabor& gabor);


//std::ostream& operator<<(std::ostream& out, CGabor& gabor)
//{
//	out <<  gabor.x << "," << gabor.y << "," << gabor.w << "," << gabor.h << "," << gabor.ori << "," << gabor.sf << "," << gabor.dev << "," << gabor.ph;
//	return out;
//}


int main (int argc, char *argv[])
{

	// Check input arguments
	if (args(argc, argv))
	{
		return 1;
	}
	else
	{
		cout << "Screen distance " << m_screenDistanceMM << endl;
		cout << "Background color " << m_background << endl;
		cout << "Gabor specs A: " << f_aConte << endl;
		cout << "Gabor specs B: " << f_bConte << endl;
	}

	// INit vsg
	if (ARvsg::instance().init(m_screenDistanceMM, m_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}


	// write video pages and create triggers
	init_pages();

	// Issue "ready" triggers to spike2.
	// These commands pulse spike2 port 6. 
	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x20, 0);
	vsgPresent();

	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x00, 0);
	vsgPresent();

	triggers.reset(vsgIOReadDigitalIn());



	// All right, start monitoring triggers........
	std::string s;
	int last_output_trigger=0;
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
			vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
//			cout << "SetTriggers=" << tf.output_trigger() << endl;
			vsgPresent();
		}
	}

	ARvsg::instance().clear();

	return 0;
}



int args(int argc, char **argv)
{	
	bool have_d=false;
	string s;
	int c;
	ARGratingSpec *pspec=NULL;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "A:B:s:b:hd:va")) != -1)
	{
		switch (c) 
		{
		case 'a':
			m_binaryTriggers = false;
			break;
		case 'v':
			m_verbose = true;
			break;
		case 'b': 
			s.assign(optarg);
			if (parse_color(s, m_background)) errflg++; 
			break;
		case 'd':
			s.assign(optarg);
			if (parse_distance(s, m_screenDistanceMM)) errflg++;
			else have_d=true;
			break;
		case 'A':
			s.assign(optarg);
			if (parse_conte(s, f_aConte)) errflg++;
			break;
		case 'B':
			s.assign(optarg);
			if (parse_conte(s, f_bConte)) errflg++;
			break;
		case 'h':
			errflg++;
			break;
		case '?':
            errflg++;
			break;
		default:
			errflg++;
			break;
		}
	}
	
	//if (m_gratings.size() != 2)
	//{
	//	cerr << "Must specify exactly two gratings!" << endl;
	//	errflg++;
	//}
	if (!have_d)
	{
		cerr << "Screen distance not specified!" << endl; 
		errflg++;
	}
	if (errflg) 
	{
		cerr << "errflg=" << errflg << endl;
	}
	return errflg;
}

//void usage()
//{
//	cerr << "usage: gratings -d screen_distance_MM -b g|b|w -s x,y,w,h,contrast%,sf,tf,orientation,color_vector,s|q,r|e -s x,y,w,h,contrast%,sf,tf,orientation,color_vector,s|q,r|e" << endl;
//}

//int parse_gabor(const std::string& s, CGabor& gabor)
//{
//	int status = -1;
//	vector<string> tokens;
//	tokenize(s, tokens, ",");
//
//	if (tokens.size() != 12)
//	{
//		cerr << "expect 12 tokens: " << tokens.size() << ":" << s << endl;
//		return status;
//	}
//	else
//	{
//		istringstream iss;
//		iss.str(tokens[0]);
//		iss >> gabor.x;
//		if (!iss)
//		{
//			cerr << "bad x: " << tokens[0] << endl;
//			return(1);
//		}
//		iss.str(tokens[1]);
//		iss.clear();
//		iss >> gabor.y;
//		if (!iss)
//		{
//			cerr << "bad y: " << tokens[1] << endl;
//			return(1);
//		}
//		iss.str(tokens[2]);
//		iss.clear();
//		iss >> gabor.w;
//		if (!iss)
//		{
//			cerr << "bad width: " << tokens[2] << endl;
//			return(1);
//		}
//		iss.str(tokens[3]);
//		iss.clear();
//		iss >> gabor.h;
//		if (!iss)
//		{
//			cerr << "bad height: " << tokens[3] << endl;
//			return(1);
//		}
//		iss.str(tokens[4]);
//		iss.clear();
//		iss >> gabor.ori;
//		if (!iss)
//		{
//			cerr << "bad ori: " << tokens[4] << endl;
//			return(1);
//		}
//		iss.str(tokens[5]);
//		iss.clear();
//		iss >> gabor.sf;
//		if (!iss)
//		{
//			cerr << "bad sf: " << tokens[5] << endl;
//			return(1);
//		}
//		iss.str(tokens[6]);
//		iss.clear();
//		iss >> gabor.dev;
//		if (!iss)
//		{
//			cerr << "bad dev: " << tokens[6] << endl;
//			return(1);
//		}
//		iss.str(tokens[7]);
//		iss.clear();
//		iss >> gabor.ph;
//		if (!iss)
//		{
//			cerr << "bad ph: " << tokens[7] << endl;
//			return(1);
//		}
//		if (tokens[8] == "h" || tokens[8] == "H")
//			gabor.bHorizontal = true;
//		else
//			gabor.bHorizontal = false;
//		iss.str(tokens[9]);
//		iss.clear();
//		iss >> gabor.distractor_factor;
//		if (!iss)
//		{
//			cerr << "bad distractor_factor: " << tokens[9] << endl;
//			return(1);
//		}
//		iss.str(tokens[10]);
//		iss.clear();
//		iss >> gabor.cue_line_width;
//		if (!iss)
//		{
//			cerr << "bad cue line width: " << tokens[10] << endl;
//			return(1);
//		}
//		iss.str(tokens[11]);
//		iss.clear();
//		iss >> gabor.cue_color;
//		if (!iss)
//		{
//			cerr << "bad cue color: " << tokens[11] << endl;
//			return(1);
//		}
//
//		status = 0;
//	}
//	return status;
//}
//

//int drawConteStim(CGabor& gabor)
//{
//	double xx[2], yy[2];
//	double rect[4];
//	double distractor_sf = 0.01;
//	VSGTRIVAL b, w;
//	b.a = b.b = b.c = 0;
//	w.a = w.b = w.c = 1;
//
//	vsgSetDrawMode(vsgCENTREXY);
//	vsgSetPen1(0);
//	vsgSetPen2(250);
//	vsgObjSetColourVector(&b, &w, vsgBIPOLAR);
//	vsgDrawGabor(gabor.x, gabor.y, gabor.w, gabor.h, gabor.ori, gabor.sf, gabor.dev, gabor.ph);
//
//	if (gabor.bHorizontal)
//	{
//		xx[0] = gabor.x - gabor.w;
//		xx[1] = gabor.x + gabor.w;
//		yy[0] = yy[1] = gabor.y;
//		rect[0] = gabor.x - 1.5 * gabor.w;
//		rect[1] = gabor.x + 1.5 * gabor.w;
//		rect[2] = gabor.y - 0.5 * gabor.h;
//		rect[3] = gabor.y + 0.5 * gabor.h;
//	}
//	else
//	{
//		yy[0] = gabor.y - gabor.h;
//		yy[1] = gabor.y + gabor.h;
//		xx[0] = xx[1] = gabor.x;
//		rect[0] = gabor.x - 0.5 * gabor.w;
//		rect[1] = gabor.x + 0.5 * gabor.w;
//		rect[2] = gabor.y - 1.5 * gabor.h;
//		rect[3] = gabor.y + 1.5 * gabor.h;
//	}
//
//	vsgSetPen1(250);
//	vsgSetPen2(125);
//	vsgDrawGaussian(xx[0], yy[0], gabor.w, gabor.h, gabor.dev);
//	vsgDrawGaussian(xx[1], yy[1], gabor.w, gabor.h, gabor.dev);
//
//	// draw border cue
//	vsgSetDrawMode(vsgSOLIDPEN);
//	vsgSetPenSize(gabor.cue_line_width, gabor.cue_line_width);
//	arutil_color_to_palette(gabor.cue_color, 251);
//	vsgSetPen1(251);
//	vsgDrawLine(rect[0], rect[2], rect[1], rect[2]);	// top
//	vsgDrawLine(rect[1], rect[2], rect[1], rect[3]);	// right
//	vsgDrawLine(rect[1], rect[3], rect[0], rect[3]);	// bottom
//	vsgDrawLine(rect[0], rect[3], rect[0], rect[2]);	// left
//
//
//	return 0;
//}
//
//void copyConteSpec(ARConteSpec& conte, const CGabor& gabor)
//{
//	conte.x = gabor.x;
//	conte.y = gabor.y;
//	conte.w = gabor.w;
//	conte.h = gabor.h;
//	conte.orientation = gabor.ori;
//	conte.sf = gabor.sf;
//	conte.dev = gabor.dev;
//	conte.phase = gabor.ph;
//	conte.bHorizontal = gabor.bHorizontal;
//	conte.distractor_factor = gabor.distractor_factor;
//	conte.cue_line_width = gabor.cue_line_width;
//	conte.cue_color = gabor.cue_color;
//}
//

int init_pages()
{
	int status=0;
	int islice=50;
	CallbackTrigger *pcall = NULL;


//	copyConteSpec(f_aConte, f_gabor_A);
//	copyConteSpec(f_bConte, f_gabor_B);

	f_aConte.do_init(50);
	f_bConte.do_init(50);
	//vsgPresent();

	cout << "init_pages(): " << f_aConte << endl;
	cout << "init_pages(): " << f_bConte << endl;

	VSGLUTBUFFER buffer;
	vsgPaletteRead(&buffer);
	for (int i = 0; i < 100; i++)
	{
		cerr << i << ":" << buffer[i].a << "," << buffer[i].b << "," << buffer[i].c << endl;
	}

	vsgSetDrawPage(vsgVIDEOPAGE, f_iPage0, vsgBACKGROUND);

	f_aConte.draw();
	f_bConte.draw();
	//drawConteStim(f_gabor_A);
	//drawConteStim(f_gabor_B);
	vsgSetDrawPage(vsgVIDEOPAGE, f_iPageBlank, vsgBACKGROUND);


	// trigger to turn stim on
	triggers.addTrigger(new CallbackTrigger("M", 0x2, 0x2, 0x2, 0x2, callback));
	triggers.addTrigger(new CallbackTrigger("m", 0x2, 0x0, 0x2, 0x0, callback));

	// trigger to turn stim OFF
	triggers.addTrigger(new CallbackTrigger("X", 0x6, 0x0, 0x2, 0x0, callback));

	// quit trigger
	triggers.addTrigger(new QuitTrigger("q", 0x80, 0x80, 0xff, 0x0, 0));

	// Set vsg trigger mode
	vsgObjSetTriggers(vsgTRIG_ONPRESENT+vsgTRIG_TOGGLEMODE,0,0);


	// Dump triggers
	std::cout << "Triggers:" << std::endl;
	for (unsigned int i=0; i<triggers.size(); i++)
	{
		std::cout << "Trigger " << i << " " << *(triggers[i]) << std::endl;
	}


	return status;
}



// The return value from this trigger callback determines whether a vsgPresent() is issued. 

int callback(int &output, const CallbackTrigger* ptrig)
{
	int ival=1;
	string key = ptrig->getKey();


//	cout << "callback: key " << ptrig->getKey() << endl;

	if (key == "M")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, f_iPage0, vsgNOCLEAR);
//		vsgObjSelect(f_gabor.handle);
//		vsgObjSetContrast(100);
	}
	else if (key == "m")
	{
//		vsgObjSelect(f_gabor.handle);
//		vsgObjSetContrast(0);
	}
	else if (key == "X")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, f_iPageBlank, vsgNOCLEAR);
//djs		m_gratings[0]->setContrast(0);
//djs		m_gratings[1]->setContrast(0);
	}

	return ival;
}
