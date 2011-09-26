#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include "Alertlib.h"
#include "Alertutil.h"
#include <boost/filesystem.hpp>

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

using namespace std;
using namespace alert;
using namespace boost::filesystem;

ARFixationPointSpec f_fixpt;
TriggerVector triggers;
int f_screenDistanceMM = 500;
int f_framesPerTerm;
int f_termsPerSequence;
// djs HACK
COLOR_TYPE f_background = {green, {}};
COLOR_TYPE f_background2 = {blue, {}};
string f_sPrefix;
string f_sDirectory;
int f_iXSizePixels;
int f_iYSizePixels;
double f_apertureDegreesX=0, f_apertureDegreesY=0;
int f_aperturePixelsX, f_aperturePixelsY;					// aperture upper left hand corner, in pixels
int f_pulse = 0x40;
bool f_binaryTriggers = true;
int f_iNumberLength=3;		// number of digits in image filenames, e.g. image000.bmp, image001,bmp has 3.
int f_sequenceIndex = 1;
int f_sequenceLength = 20;
bool bTesting = false;
VSGCYCLEPAGEENTRY f_cycle[32768];

typedef struct image_holder
{
	path p;
	int x, y;
} ImageHolder;
vector<ImageHolder> f_vecImages;

int args(int argc, char **argv);
int load_images(vector<ImageHolder>& images, int imagePixelsWide, int imagePixelsHigh);
int init_pages();
void init_triggers();
int callback(int &output, const CallbackTrigger* ptrig);
void move_window(int image);




int main(int argc, char **argv)
{
	string s;

	if (args(argc, argv))
	{
		cerr << "Error in input args." << endl;
		return 1;
	}

	// Init screen. We end up in pixel mode. 
	if (ARvsg::instance().init(-1, f_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}
	//vsgSetSpatialUnits(vsgPIXELUNIT);
	vsgSetViewDistMM(f_screenDistanceMM);

	if (init_pages())
	{
		cerr << "Error initializing pages for stimulus." <<endl;
		return 1;
	}

	init_triggers();

	vsgObjCreate();
	vsgObjSetPixelLevels(100, 1);

	ARvsg::instance().ready_pulse(100, f_pulse);

	// All right, start monitoring triggers........
	int last_output_trigger=0;
	long input_trigger = 0;

	if (!bTesting)
	{
		while (1)
		{
			// If user-triggered, get a trigger entry. 
			if (!f_binaryTriggers)
			{
				// Get a new "trigger" from user
				cout << "Enter trigger/key: ";
				cin >> s;
			}
			else
			{
				input_trigger = vsgIOReadDigitalIn();
			}

			TriggerFunc	tf = std::for_each(triggers.begin(), triggers.end(), 
				(f_binaryTriggers ? TriggerFunc(input_trigger, last_output_trigger) : TriggerFunc(s, last_output_trigger)));

			// Now analyze input trigger
		 	
			if (tf.quit()) break;
			else if (tf.present())
			{	
				last_output_trigger = tf.output_trigger();
				vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
				vsgPresent();
			}
			Sleep(10);
		}

	}
	else
	{
		cout << "Enter image#, m, b or q: ";
		cin >> s;
		while (s != "q")
		{
			if (s == "b")
			{
				vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 0);
			}
			else if (s == "m")
			{
				int x, y;
				cout << "Enter x y: ";
				cin >> x >> y;
				vsgMoveScreen(x, y);
			}
			else
			{
				int i;
				istringstream iss(s);
				iss >> i;
				if (iss)
				{
					if (i<0)
					{
						cout << "Change to ov page " << i << endl;
						vsgSetZoneDisplayPage(vsgOVERLAYPAGE, -1*i);
					}
					else if (i <= (int)f_vecImages.size())
					{
						int x, y;
						vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);
						x = f_vecImages[i].x;
						y = f_vecImages[i].y;
						cout << "Image pos is " << f_vecImages[i].x << ", " << f_vecImages[i].y << endl;
						cout << "Move window to " << x << ", " << y << endl;
						vsgMoveScreen(x, y);
						vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 1);
					}
				}
			}
			cout << "xy or q: ";
			cin >> s;
		}
	}

	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 0);

	return 0;
}


void init_triggers()
{
	triggers.addTrigger(new CallbackTrigger("F", 0x2, 0x2, 0x2, 0x2, callback));
	triggers.addTrigger(new CallbackTrigger("S", 0x4, 0x4, 0x4, 0x4, callback));
	triggers.addTrigger(new CallbackTrigger("s", 0x4, 0x0, 0x4, 0x0, callback));
	triggers.addTrigger(new CallbackTrigger("X", 0x6, 0x0, 0x6, 0x0, callback));
	triggers.addTrigger(new CallbackTrigger("a", 0x8, 0x8|AR_TRIGGER_TOGGLE, 0x8, 0x8|AR_TRIGGER_TOGGLE, callback));

	// quit trigger
	triggers.addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));

	return;
}

void move_window(int image)
{
	cout << "Calling vsgMoveScreen(" << f_vecImages[image].x << ", " << f_vecImages[image].y << endl;
	vsgMoveScreen(f_vecImages[image].x, f_vecImages[image].y);
}



void advance_sequence()
{
	// the sequence consists of the images, in order. Remember that the first image in f_vecImages is a
	// dummy - the background color. Each sequence played has f_sequenceLength images. The sequence will 
	// wrap as needed, skipping over the dummy background image each time. 
	// f_sequenceIndex = index of current sequence start.
	f_sequenceIndex += f_sequenceLength;
	if (f_sequenceIndex >= (int)f_vecImages.size())
	{
		f_sequenceIndex = f_sequenceIndex%f_vecImages.size() + 1;	// the "+1" is due to the dummy image
	}
	return;
}

void prepare_cycling()
{
	int i, index;
	memset(f_cycle, 0, (f_sequenceLength+1)*sizeof(VSGCYCLEPAGEENTRY));

	for (i=0, index=f_sequenceIndex; i<f_sequenceLength; i++, index++)
	{
		if (index == f_vecImages.size()) index = 1;	// skip over dummy image
		f_cycle[i].Page = 0+vsgDUALPAGE+vsgTRIGGERPAGE;
		f_cycle[i].Xpos = f_vecImages[index].x;
		f_cycle[i].Ypos = f_vecImages[index].y;
		f_cycle[i].Frames = f_framesPerTerm;
		f_cycle[i].Stop = 0;
		f_cycle[i].ovPage = 1;
		f_cycle[i].ovXpos = 0;
		f_cycle[i].ovYpos = 0;
		cout << "cycle[" << i << "] x,y = " << f_cycle[i].Xpos << ", " << f_cycle[i].Ypos <<  " fpt " << f_framesPerTerm << endl;
	}
	f_cycle[f_sequenceLength].Page = 0+vsgDUALPAGE+vsgTRIGGERPAGE;
	f_cycle[f_sequenceLength].Xpos = f_vecImages[0].x;
	f_cycle[f_sequenceLength].Ypos = f_vecImages[0].y;
	f_cycle[f_sequenceLength].Frames = 1;
	f_cycle[f_sequenceLength].Stop = 1;
	f_cycle[f_sequenceLength].ovPage = 1;
	f_cycle[f_sequenceLength].ovXpos = 0;
	f_cycle[f_sequenceLength].ovYpos = 0;
	
	i=vsgPageCyclingSetup(f_sequenceLength+1, &f_cycle[0]);
	cout << "cycling status " << i << endl;
}


int callback(int &output, const CallbackTrigger* ptrig)
{
	int ival=1;
	string key = ptrig->getKey();

	if (key == "F")
	{
		// does not work with vsgObj triggers! vsgSetDrawPage(vsgOVERLAYPAGE, 1, vsgNOCLEAR);
		// The trigger output will not be coincident with the actual page change
		vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 1 + vsgTRIGGERPAGE);
//		ival = 0;
//		vsgSetDrawPage(vsgOVERLAYPAGE, 1, vsgNOCLEAR);
//		ival = 1;
	}
	else if (key == "S")
	{
		prepare_cycling();
		vsgSetCommand(vsgCYCLEPAGEENABLE);
		ival = 0;
		//vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
	}
	else if (key == "s")
	{
		//move_window(0);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);

	}
	else if (key == "X")
	{
		cout << "page cycling state " << vsgGetSystemAttribute(vsgPAGECYCLINGSTATE);
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		//move_window(0);
		//vsgSetDrawPage(vsgOVERLAYPAGE, 0, vsgNOCLEAR);
		vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 0);
	}
	else if (key == "a")
	{
		advance_sequence();
	}
	else 
	{
		cerr << "ERROR - Unknown trigger/key " << key << endl;
	}
	return ival;
}


int init_pages()
{
	// vsg commands to prepare for drawing. 
	// Turn off CENTERXY draw mode - all drawing and image loading done relative to upper-left hand 
	// corner of the aperture and/or image. The draw origin is also set to the upper left hand corner of the 
	// screen, so all drawing commnands must use pixels. 
	vsgSetDrawMode(0);
	vsgSetDrawOrigin(0, 0);
	vsgSetDrawPage(vsgVIDEOPAGE, 0, 0);
	vsgSetCommand(vsgOVERLAYMASKMODE);
	vsgSetCommand(vsgVIDEODRIFT);
	vsgSetVideoMode(vsgPANSCROLLMODE);					// Using vsgMoveScreen doesn't work right without this -- 
														// esp when moving screen to positions < 0. 
	cout << "Background level is " << ARvsg::instance().background_level() << endl;
	vsgSetPen2(ARvsg::instance().background_level());
	vsgSetCommand(vsgVIDEOCLEAR);


	// set up overlay colors. 
	// djs HACK
	arutil_color_to_overlay_palette(f_background2, 1);
	arutil_color_to_overlay_palette(f_fixpt, 2);

	// Fixpt diameter and position are in degrees. Change to pixels.
	double pixels;
	vsgUnitToUnit(vsgDEGREEUNIT, f_fixpt.d, vsgPIXELUNIT, &pixels);
	f_fixpt.d = pixels;
	vsgUnitToUnit(vsgDEGREEUNIT, f_fixpt.x, vsgPIXELUNIT, &pixels);
	f_fixpt.x = pixels;
	vsgUnitToUnit(vsgDEGREEUNIT, f_fixpt.y, vsgPIXELUNIT, &pixels);
	f_fixpt.y = pixels;

	// Now compute the position of the upper left hand corner of the aperture - in top-down pixel coords. 
	double pX, pY;
	vsgUnitToUnit(vsgDEGREEUNIT, f_apertureDegreesX, vsgPIXELUNIT, &pX);
	vsgUnitToUnit(vsgDEGREEUNIT, -f_apertureDegreesY, vsgPIXELUNIT, &pY);
	f_aperturePixelsX = (int)(vsgGetSystemAttribute(vsgSCREENWIDTH)/2 + pX - f_iXSizePixels/2);
	f_aperturePixelsY = (int)(vsgGetSystemAttribute(vsgSCREENHEIGHT)/2 + pY - f_iYSizePixels/2);
	if (f_aperturePixelsX/4 * 4 != f_aperturePixelsX)
	{
		cout << "Adjusting apertureX position to be multiple of 4... was " << f_aperturePixelsX << " now ";
		f_aperturePixelsX = f_aperturePixelsX/4 * 4;
		cout << f_aperturePixelsX << endl;
	}
	else
	{
		cout << "ApertureX position is a multiple of 4, no adjustments needed." << endl;
	}

	// fixpt and aperture on overlay page 1
	vsgSetDrawPage(vsgOVERLAYPAGE, 1, 1);
	vsgSetPen1(0);	// level 0 on overlay page is clear
	vsgDrawRect(f_aperturePixelsX, f_aperturePixelsY, f_iXSizePixels, f_iYSizePixels);
	cout << "Center pos is " << pX << ", " << pY << endl;
	cout << "Aperture pos is " << f_aperturePixelsX << ", " << f_aperturePixelsY << endl;
	cout << "Aperture size is " << f_iXSizePixels << "x" << f_iYSizePixels << " pixels" << endl;
	vsgSetDrawOrigin(vsgGetSystemAttribute(vsgSCREENWIDTH)/2, vsgGetSystemAttribute(vsgSCREENHEIGHT)/2);
	arutil_draw_overlay(f_fixpt, 2, 1);

	// arutil_draw_overlay uses a draw mode and different draw origin. I don't think we need to 
	// change it back to that used above. Leaving these lines here as a reminder.
	vsgSetDrawOrigin(0, 0);
	vsgSetDrawMode(0);

	// page 0 is just background
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1);

	// load images onto video page 0
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	if (load_images(f_vecImages, f_iXSizePixels, f_iYSizePixels))
	{
		cerr << "Image loading failed." << endl;
		return 1;
	}

	vsgPresent();
	return 0;
}

int args(int argc, char **argv)
{	
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	bool have_X = false;
	bool have_Y = false;
	bool have_P = false;
	bool have_d = false;
	bool have_t = false;
	bool have_n = false;
	bool have_f = false;
	bool have_D = false;

	while ((c = getopt(argc, argv, "af:t:n:P:p:d:X:Y:l:d:x:y:D:T")) != -1)
	{
		switch (c) 
		{
		case 'T': 
			bTesting = true;
			break;
		case 'a':
			f_binaryTriggers = false;
			break;
		case 'f':
			s.assign(optarg);
			if (parse_fixation_point(s, f_fixpt))
			{
				cerr << "Error parsing fixation point." << endl;
				errflg++;
			}
			else
			{
				have_f = true;
			}
			break;
		case 't':
			s.assign(optarg);
			if (parse_integer(s, f_framesPerTerm))
			{
				cerr << "Error parsing frames per term (-t). Must be an integer>0." << endl;
				errflg++;
			}
			else
			{
				have_t = true;
			}
			break;
		case 'n':
			s.assign(optarg);
			if (parse_integer(s, f_termsPerSequence))
			{
				cerr << "Error parsing terms per sequence (-n). Must be an integer>0." << endl;
				errflg++;
			}
			else
			{
				have_n = true;
			}
			break;
		case 'x':
			s.assign(optarg);
			if (parse_double(s, f_apertureDegreesX))
			{
				cerr << "Error parsing aperture x position. Must be a double." << endl;
				errflg++;
			}
			break;
		case 'y':
			s.assign(optarg);
			if (parse_double(s, f_apertureDegreesY))
			{
				cerr << "Error parsing aperture y position. Must be a double." << endl;
				errflg++;
			}
			break;
		case 'd':
			s.assign(optarg);
			if (parse_integer(s, f_screenDistanceMM))
			{
				cerr << "Error parsing screen distance. Must be integer." << endl;
				errflg++;
			}
			else
			{
				have_d = true;
			}
			break;
		case 'p':
			s.assign(optarg);
			if (parse_integer(s, f_pulse))
			{
				cerr << "Error parsing pulse (-p) bit. Must be integer." << endl;
				errflg++;
			}
			break;
		case 'P':
			f_sPrefix.assign(optarg);
			have_P = true;
			break;
		case 'D':
			f_sDirectory.assign(optarg);
			have_D = true;
			break;
		case 'X':
			s.assign(optarg);
			if (parse_integer(s, f_iXSizePixels))
			{
				cerr << "Error parsing x pixels. Must be integer." << endl;
				errflg++;
			}
			else
			{
				have_X = true;
			}
			break;
		case 'Y':
			s.assign(optarg);
			if (parse_integer(s, f_iYSizePixels))
			{
				cerr << "Error parsing y pixels. Must be integer." << endl;
				errflg++;
			}
			else
			{
				have_Y = true;
			}
			break;
		case 'l':
			s.assign(optarg);
			if (parse_integer(s, f_iNumberLength))
			{
				cerr << "Error parsing number length for image files. Must be integer." << endl;
				errflg++;
			}
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
		cerr << "Error - screen distance not specified on command line." << endl;
		errflg++;
	}

	if (!have_f)
	{
		cerr << "Error - fixation point not specified on command line." << endl;
		errflg++;
	}

	if (!have_n)
	{
		cerr << "Error - terms per sequence (-n) not specified on command line." << endl;
		errflg++;
	}

	if (!have_t)
	{
		cerr << "Error - frames per term (-t) not specified on command line." << endl;
		errflg++;
	}

	if (!have_P || !have_D || !have_X || !have_Y)
	{
		cerr << "Error - must specify directory (-D), prefix (-P), x pixels (-X) and y pixels (-Y)" << endl;
		errflg++;
	}

	return errflg;
}

// Load images.

int load_images(vector<ImageHolder>& images, int imagePixelsWide, int imagePixelsHigh)
{
	int iStatus = 0;
	int pageWidth, pageHeight, nPages;
	int screenWidth, screenHeight;
	int nWidePerPage, nRowsPerPage;
	int nPagesRequired;
	int useImagePixelsWide;		// this will be a multiple of 4, in case imagePixelsWide is not
	path p(f_sDirectory);
	ostringstream oss;
	ImageHolder ih;

	// Generate dummy image as a placeholder for background.
	ih.p = p / "no_image";
	f_vecImages.push_back(ih);
	if (exists(p) && is_directory(p))
	{
		int i=0;
		string s;
		while (true)
		{
			oss.str("");
			oss << f_sPrefix << setw(f_iNumberLength) << setfill('0') << i << ".bmp";
			ih.p = p / oss.str();
			if (exists(ih.p) && is_regular_file(ih.p))
			{
				//cout << "File " << ih.p << endl;
				f_vecImages.push_back(ih);
				i++;
			}
			else
			{
				break;
			}
		}
		cout << "Found " << f_vecImages.size()-1 << " images in directory " << p << endl;
		cout << "One additional image will be allocated for background." << endl;
		for (vector<ImageHolder>::const_iterator it = f_vecImages.begin(); it<f_vecImages.end(); it++)
		{
			cout << it->p << endl;
		}
	}
	else
	{
		cerr << "Error - directory (" << p << ") not found." << endl;
		return 1;
	}

	// Get page width and height, and number of pages.
	pageHeight = vsgGetSystemAttribute(vsgPAGEHEIGHT);
	pageWidth = vsgGetSystemAttribute(vsgPAGEWIDTH);
	nPages = vsgGetSystemAttribute(vsgNUMVIDEOPAGES);
	screenHeight = vsgGetSystemAttribute(vsgSCREENHEIGHT);
	screenWidth = vsgGetSystemAttribute(vsgSCREENWIDTH);

	// Set useImagePixelsWide
	if (imagePixelsWide/4 * 4 != imagePixelsWide) useImagePixelsWide = (imagePixelsWide/4 + 1)*4;
	else useImagePixelsWide = imagePixelsWide;

	// How many of the images will fit in the width of a page?
	nWidePerPage = pageWidth/useImagePixelsWide;

	// How many rows will fit on a page?
	nRowsPerPage = pageHeight/imagePixelsHigh;

	// How many pages will be needed? We add an extra image to account for background image.
	nPagesRequired = (images.size() + 1 + (nWidePerPage*nRowsPerPage-1)) / (nWidePerPage*nRowsPerPage);

	cout << "Images are " << imagePixelsWide << "x" << imagePixelsHigh << endl;
	cout << "Using size " << useImagePixelsWide << "x" << imagePixelsHigh << endl;
	cout << "Pages  are " << pageWidth << "x" << pageHeight << endl;
	cout << "There are " << nPages << " pages available." << endl;
	cout << "We can fit " << nWidePerPage << " images across a page, and " << nRowsPerPage << " such rows will fit on a page." << endl;
	cout << "We require " << nPagesRequired << " pages to hold all images." << endl;

	if (nPagesRequired > nPages)
	{
		cerr << "ERROR - Cannot fit all images into memory!" << endl;
		return 1;
	}

	// Now iterate and load images.
	// We leave the first image position empty - that will be background color.
	int iPage, iColumn, iRow, i;
	int xpos, ypos;			// origin for image loading
	for (i=0; i<(int)images.size(); i++)
	{
		iPage = i/(nRowsPerPage*nWidePerPage);
		iRow = (i-iPage*(nRowsPerPage*nWidePerPage)) / nWidePerPage;
		iColumn = (i-iPage*(nRowsPerPage*nWidePerPage)) % nWidePerPage;
		cout << "Image " << i << " Page/Row/Column " << iPage << "/" << iRow << "/" << iColumn << endl;
		xpos = iColumn * useImagePixelsWide;
		ypos = iRow * imagePixelsHigh;
		images[i].x = xpos - f_aperturePixelsX;
		images[i].y = ypos - f_aperturePixelsY;
		cout << "image " << i << " is at " << xpos << ", " << ypos << " mw " << images[i].x << ", " << images[i].y << endl;
		// Skip image 0 - that's a dummy image for background.
		if (i>0)
		{
			iStatus = vsgDrawImage(0, xpos, ypos, (char *)images[i].p.string().c_str());
		}
		if (iStatus)
		{
			cerr << "Error (" << iStatus << ") loading image " << i << " filename " << images[i].p << endl;
			return 1;
		}
	}


	return 0;
}