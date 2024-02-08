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
#include "UStim.h"

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

ARConteSpec f_conte;
COLOR_TYPE f_background = COLOR_TYPE(gray);
int f_screenDistanceMM=500;
bool m_verbose=false;
TriggerVector triggers;
bool m_binaryTriggers = true;
bool m_bCalibration = false;
const int f_iPage0 = 0;
const int f_iPage1 = 1;
const int f_iPageBlank = 2;

int init_pages();


void dumpPal(int ilevel)
{
	VSGLUTBUFFER palVSG;
	vsgPaletteRead(&palVSG);
	cerr << "VSG level " << ilevel << " " << palVSG[ilevel].a << ", " << palVSG[ilevel].b << ", " << palVSG[ilevel].c << endl;
}




int main (int argc, char *argv[])
{
	// fixstim does this
	if (UStim::initialize(ARvsg::instance(), f_screenDistanceMM, f_background))
	{
		cerr << "VSG init failed!!!" << endl;
		return 1;
	}



	COLOR_TYPE fixptColor(blue);
	COLOR_TYPE rectColor(.2, .8, .6);
	ARFixationPointSpec fixpt;
	ARRectangleSpec rect;
	ARContrastRectangleSpec crect;
	ARContrastFixationPointSpec cfixpt;

	rect.init(1, false);
	crect.init(1);
	cfixpt.init(1);
	fixpt.init(1, false);
	arutil_color_to_palette(fixptColor, fixpt.getFirstLevel());
	arutil_color_to_palette(rectColor, rect.getFirstLevel());







	vsgSetDrawPage(vsgVIDEOPAGE, f_iPage0, vsgBACKGROUND);

	string s;
	while (s != "q")
	{
		std::cout << "Enter test RFrfcq: ";
		std::cin >> s;
		if (s == "R")
		{
			vsgSetDrawPage(vsgVIDEOPAGE, f_iPage1, vsgBACKGROUND);

			crect.x = crect.y = 0;
			crect.w = 10;
			crect.h = 2;
			crect.color = red;
			crect.orientation = 0;
			crect.setContrast(100);
			crect.draw();
			vsgPresent();

			string stmp;
			std::cout << "Hit key to set contrast to 0." << endl;
			std::cin >> stmp;
			crect.setContrast(0);
			vsgPresent();
			std::cout << "Hit key to exit." << endl;
			std::cin >> stmp;
		}
		else if (s == "r")
		{

			vsgSetDrawPage(vsgVIDEOPAGE, f_iPage1, vsgBACKGROUND);

			rect.x = rect.y = 0;
			rect.w = rect.h = 6;
			rect.orientation = 45;
			rect.color = rectColor;

			// put color into palette
			//vsgSetPen1(fixpt.getFirstLevel());
			rect.draw();
			vsgPresent();

			std::string stmp;
			std::cout << "Hit key to exit." << endl;
			std::cin >> stmp;

		}
		else if (s == "F")
		{
			vsgSetDrawPage(vsgVIDEOPAGE, f_iPage1, vsgBACKGROUND);

			cfixpt.x = cfixpt.y = 0;
			cfixpt.d = 10;
			cfixpt.color = red;
			cfixpt.setContrast(100);
			cfixpt.draw();
			vsgPresent();

			string stmp;
			std::cout << "Hit key to set contrast to 0." << endl;
			std::cin >> stmp;
			cfixpt.setContrast(0);
			vsgPresent();
			std::cout << "Hit key to exit." << endl;
			std::cin >> stmp;

		}
		else if (s == "f")
		{
			vsgSetDrawPage(vsgVIDEOPAGE, f_iPage1, vsgBACKGROUND);

			fixpt.x = fixpt.y = 0;
			fixpt.d = 10;
			fixpt.color = fixptColor;

			// put color into palette
			//vsgSetPen1(fixpt.getFirstLevel());
			fixpt.draw();
			vsgPresent();

			std::string stmp;
			std::cout << "Hit key to exit." << endl;
			std::cin >> stmp;

		}
		else if (s == "c")
		{
			vsgSetDrawPage(vsgVIDEOPAGE, f_iPage1, vsgBACKGROUND);

			// set up conte spec
			f_conte.x = f_conte.y = 0;
			f_conte.w = f_conte.h = 5;
			f_conte.iHorizontal= 0;	
			f_conte.sf = 0.5;
			f_conte.cue_line_width = 2;
			f_conte.divisor = 5;
			f_conte.phase = 0;

			// init spec
			f_conte.init(60, false);

			f_conte.draw();
			vsgPresent();

			std::string stmp;
			std::cout << "Hit key to exit." << endl;
			std::cin >> stmp;

		}
		vsgSetDrawPage(vsgVIDEOPAGE, f_iPage0, vsgNOCLEAR);
		vsgPresent();
	}




	////dumpPal(cueLevel);
	////dumpPal(rect.getFirstLevel());
	//dumpPal(cueLevel);
	//dumpPal(cueLevel2);
	//dumpPal(vsgBACKGROUND);
	//cerr << "dist " << vsgGetViewDistMM() << endl;
	return 0;
}



//int args(int argc, char **argv)
//{	
//	bool have_d=false;
//	string s;
//	int c;
//	ARGratingSpec *pspec=NULL;
//	extern char *optarg;
//	extern int optind;
//	int errflg = 0;
//	while ((c = getopt(argc, argv, "A:B:s:b:hd:va")) != -1)
//	{
//		switch (c) 
//		{
//		case 'a':
//			m_binaryTriggers = false;
//			break;
//		case 'v':
//			m_verbose = true;
//			break;
//		case 'b': 
//			s.assign(optarg);
//			if (parse_color(s, m_background)) errflg++; 
//			break;
//		case 'd':
//			s.assign(optarg);
//			if (parse_distance(s, m_screenDistanceMM)) errflg++;
//			else have_d=true;
//			break;
//		case 'A':
//			s.assign(optarg);
//			if (parse_conte(s, f_aConte)) errflg++;
//			break;
//		case 'B':
//			s.assign(optarg);
//			if (parse_conte(s, f_bConte)) errflg++;
//			break;
//		case 'h':
//			errflg++;
//			break;
//		case '?':
//            errflg++;
//			break;
//		default:
//			errflg++;
//			break;
//		}
//	}
//	
//	if (!have_d)
//	{
//		cerr << "Screen distance not specified!" << endl; 
//		errflg++;
//	}
//	if (errflg) 
//	{
//		cerr << "errflg=" << errflg << endl;
//	}
//	return errflg;
//}
//
//
//int init_pages()
//{
//	int status=0;
//	int islice=50;
//	CallbackTrigger *pcall = NULL;
//
//	vsgSetDrawPage(vsgVIDEOPAGE, f_iPage0, vsgBACKGROUND);
//
//	f_aConte.draw();
//	f_bConte.draw();
//	//drawConteStim(f_gabor_A);
//	//drawConteStim(f_gabor_B);
//	vsgSetDrawPage(vsgVIDEOPAGE, f_iPageBlank, vsgBACKGROUND);
//
//	return status;
//}
//
//
//
