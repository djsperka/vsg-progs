#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
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

vector<ARImageSpec> f_vecImageSpec;
unsigned int f_icurrent = 0;
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
int f_pages[2] = { 0, 1 };
unsigned int f_ipage = 0;
ARImageSpec f_image;
unsigned int f_nlevels = 32;
int init_pages();

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
		cout << "image " << f_image << endl;
		//return 1;
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

//int parse_bmp_image_list(const string& s, vector<ARImageSpec>& vec)
//{
//	// tokenize
//	// filename,x,y,dur,dly
//	// 1, 3, or 5 args, just like ARImageSpec, parse it like one. 
//	ARImageSpec argSpec;
//	if (parse_image(s, argSpec))
//	{
//		cerr << "Canot parse image list arg: " << s << endl;
//		return 1;
//	}
//	else
//	{
//		cerr << "image arg read as: " << argSpec << endl;
//	}
//
//	// filename is read as a text file, not an image. 
//	// The params from this spec will be applied to each spec in 
//	// the set after the list is parsed.
//
//	int status = 0;
//	boost::filesystem::path p(argSpec.filename);
//	if (!exists(p))
//	{
//		std::cerr << "Error: image list file does not exist: " << argSpec.filename << endl;
//		status = 1;
//	}
//	else
//	{
//		boost::filesystem::path folder = p.parent_path();		// if file has relative pathnames to images, they are relative to dir file lives in
//		std::cerr << "Found image list file " << p << " at path " << folder << std::endl;
//
//		string line;
//		int linenumber = 0;
//		std::ifstream myfile(p.c_str());
//		if (myfile.is_open())
//		{
//			ARImageSpec imgFromFile;
//			while (getline(myfile, line) && !status)
//			{
//				boost::trim(line);
//				linenumber++;
//				cerr << "Got line " << linenumber << ": " << line << endl;
//				if (line.length() == 0 || line[0] == '#')
//				{
//					// skip empty lines and those that start with '#'
//				}
//				else if (parse_image(line, imgFromFile))
//				{
//					cerr << "parse failed on line " << linenumber << ": " << line << endl;
//					status = 1;	// this will stop processing, eventually.
//				}
//				else
//				{
//					// fix filename with prefix if relative
//					boost::filesystem::path pathBmpFile(imgFromFile.filename);
//					cerr << "filename found: " << pathBmpFile.string() << endl;
//					if (pathBmpFile.is_relative())
//					{
//						pathBmpFile = folder / pathBmpFile;
//					}
//
//					if (!exists(pathBmpFile))
//					{
//						std::cerr << "bmp image file not found,line " << linenumber << " : " << pathBmpFile << endl;
//						status = 1;
//					}
//					else
//					{
//						// The bmp file exists! 
//						// Update the filename
//
//						cerr << "bmp exists! " << imgFromFile.filename << endl;
//						string stmp = pathBmpFile.string();
//						strcpy(imgFromFile.filename, stmp.c_str());
//
//						// Also update x,y
//						imgFromFile.x = argSpec.x;
//						imgFromFile.y = argSpec.y;
//
//						// add to vec
//						cerr << " add to vec" << endl;
//
//						vec.push_back(imgFromFile);
//						cerr << "done" << endl;
//					}
//				}
//			}
//			myfile.close();
//		}
//		else
//		{
//			cerr << "Cannot open file: " << p.c_str() << endl;
//			status = 1;
//		}
//	}
//	return status;
//}


int args(int argc, char** argv)
{
	bool have_d = false;
	string s;
	int c;
	ARGratingSpec* pspec = NULL;
	extern char* optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "A:B:s:b:hd:van:")) != -1)
	{
		switch (c) 
		{
		case 'A':
			s.assign(optarg);
			if (parse_image(s, f_image)) errflg++;
			break;
		case 'B':
			s.assign(optarg);
			if (parse_bmp_image_list(s, f_vecImageSpec, f_nlevels))
			{
				errflg++;
				cerr << "Cannot parse bmp image list arg: " << s << endl;
			}
			else
			{
				cerr << "images from input:" << endl;
				for (auto img : f_vecImageSpec)
				{
					cerr << img << endl;
				}
			}
			break;
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

int init_pages()
{
	int status=0;
	int islice=50;
	CallbackTrigger *pcall = NULL;

	f_image.init(f_nlevels, false);
	for (size_t i = 0; i < f_vecImageSpec.size(); i++)
		f_vecImageSpec[i].init(f_image);

	vsgSetDrawPage(vsgVIDEOPAGE, f_pages[1], vsgBACKGROUND);
	vsgSetDrawPage(vsgVIDEOPAGE, f_pages[0], vsgBACKGROUND);
	f_ipage = 0;

	// trigger to turn stim on
	triggers.addTrigger(new CallbackTrigger("M", 0x2, 0x2, 0x2, 0x2, callback));
	//triggers.addTrigger(new CallbackTrigger("m", 0x2, 0x0, 0x2, 0x0, callback));

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
		f_icurrent++;
		if (f_icurrent >= f_vecImageSpec.size())
			f_icurrent = 0;
		f_ipage = 1 - f_ipage;
		vsgSetDrawPage(vsgVIDEOPAGE, f_ipage, vsgBACKGROUND);
		f_vecImageSpec[f_icurrent].draw();
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
