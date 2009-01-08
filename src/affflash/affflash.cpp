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
#include "AlertUtil.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")


using namespace std;
using namespace alert;

int args(int argc, char **argv);
void usage();
void getOnOffColors(COLOR_VECTOR_TYPE& cv, int contrast, COLOR_TYPE& on, COLOR_TYPE& off);

int f_screenDistanceMM = 1000;   // value unimportant for this prog, must be nonzero for init
int f_nRepeats = 1;
bool f_verbose = false;
double f_blankSeconds = 0;
double f_lengthSeconds = 0;
int f_iFramesPerTerm = 0;
int f_contrast = 100;
ARGratingSpec f_grating0;
ARGratingSpec f_grating1;
bool f_bUseGrating = false;
int f_nTermsOffset = 0;
bool f_bNoSplit = false;
COLOR_TYPE f_colorBackground = { gray, {0.5, 0.5, 0.5}};
COLOR_VECTOR_TYPE f_colorVector = { b_w, {0,0,0}, {1,1,1}};
string f_sFilename;
char *f_mseq = NULL;

#define OVERLAY_BLANK_PAGE 1
#define OVERLAY_APERTURE_PAGE 0
#define MSEQ_ON_PAGE 1
#define MSEQ_OFF_PAGE 0

int main (int argc, char *argv[])
{
	int nTerms = 0;
	int nFramesBlank = 0;
	double split_factor = 0.5;
	VSGCYCLEPAGEENTRY cyclepage[32768];
	int timeUS;
	int ipage=0;

	// Check input arguments
	if (args(argc, argv))
	{
		return 1;
	}

	if (f_verbose)
	{
		cout << "affflash input parameters:" << endl;
		cout << "Sequence length (s): " << f_lengthSeconds << endl;
		cout << "Split/repeat sequence for balance: " << (f_bNoSplit ? "NO" : "YES") << endl;
		cout << "Blank length (s): " << f_blankSeconds << endl;
		cout << "Offset terms: " << f_nTermsOffset <<endl;
		cout << "Number of repeats: " << f_nRepeats << endl;
		cout << "Contrast: " << f_contrast << " Color vector: " << f_colorVector << endl;
		cout << "Background color: " << f_colorBackground << endl;
		cout << "Using Grating? " << (f_bUseGrating ? "YES" : "NO") << endl;
		if (f_bUseGrating) 
		{
			cout << "Grating specs: " << f_grating1 << endl;
			cout << "Screen distance (mm): " << f_screenDistanceMM << endl;
		}
	}


	// INit vsg
	if (ARvsg::instance().init(f_screenDistanceMM, f_colorBackground))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}

	// Basic parameters for the stimulus. 
	split_factor = 0.5;
	if (f_bNoSplit) split_factor = 1.0;
	nTerms = split_factor * f_lengthSeconds * 1.0e6 / vsgGetSystemAttribute(vsgFRAMETIME) / f_iFramesPerTerm;
	nFramesBlank = f_blankSeconds * 1.0e6 / vsgGetSystemAttribute(vsgFRAMETIME);

	if (f_verbose)
	{
		cout << "seq length(s): " << f_lengthSeconds << " frames_per_term: " << f_iFramesPerTerm << " nTerms: " << nTerms << endl;
		cout << "blank length(s): " << f_blankSeconds << " blank terms : " << nFramesBlank << endl;
	}


	// Prepare pages. Page 0 and 1 will be "off" and "on" terms from the msequence. 
	// Page 2 will be the background (for the "blank" period). 
	if (!f_bUseGrating)
	{
		COLOR_TYPE cton, ctoff;
		PIXEL_LEVEL plon, ploff, plbck;
		int ipage=0;
		getOnOffColors(f_colorVector, f_contrast, cton, ctoff);

		if (f_verbose)
		{
			cout << "ON: " << cton << " OFF: " << ctoff << endl; 
		}

		// Two pages are used. Page 0 is "off", page 1 is "on".
		LevelManager::instance().request_single(ploff);
		LevelManager::instance().request_single(plon);
		arutil_color_to_palette(ctoff, ploff);
		arutil_color_to_palette(cton, plon);
		vsgSetDrawPage(vsgVIDEOPAGE, MSEQ_OFF_PAGE, ploff);
		vsgSetDrawPage(vsgVIDEOPAGE, MSEQ_ON_PAGE, plon);

		// overlay pages
		arutil_color_to_overlay_palette(f_colorBackground, 1);
		vsgSetCommand(vsgOVERLAYMASKMODE);		// makes overlay pages visible
		vsgSetDrawPage(vsgOVERLAYPAGE, OVERLAY_APERTURE_PAGE, 0);	// clear
		vsgSetDrawPage(vsgOVERLAYPAGE, OVERLAY_BLANK_PAGE, 1);		// background
		vsgSetZoneDisplayPage(vsgOVERLAYPAGE, OVERLAY_BLANK_PAGE);
	}
	else
	{
		f_grating0.init(75);
		f_grating1.init(75);
		
		arutil_color_to_overlay_palette(f_colorBackground, 1);
		vsgSetCommand(vsgOVERLAYMASKMODE);		// makes overlay pages visible
		vsgSetDrawPage(vsgOVERLAYPAGE, OVERLAY_APERTURE_PAGE, 1);
		arutil_draw_aperture(f_grating0, 0);
		vsgSetDrawPage(vsgOVERLAYPAGE, OVERLAY_BLANK_PAGE, 1);

		arutil_draw_grating(f_grating0, MSEQ_OFF_PAGE);
		vsgPresent();
		arutil_draw_grating(f_grating1, MSEQ_ON_PAGE);
		vsgPresent();
		vsgSetZoneDisplayPage(vsgOVERLAYPAGE, OVERLAY_BLANK_PAGE);
//		vsgSetCommand(vsgDISABLELUTANIM);
//		vsgSetCommand(vsgPALETTERAMP);

	
#if 0	
	
		{
			int iquit=0;

			cout << "Page: ";
			cin >> ipage;
			while (ipage>=0 && ipage<=2)
			{
				switch(ipage)
				{
				case 0:
					vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 0);
					vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);
					break;
				case 1:
					vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 0);
					vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);
					break;
				case 2:
					vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 1);
//					vsgSetZoneDisplayPage(vsgVIDEOPAGE, 2);
					break;
				}
				cout << "Page: ";
				cin >> ipage;
			}
		}

#endif
	
	}

	// Set up page cycling
	for (int i=0; i<nTerms; i++)
	{
		ipage = (f_mseq[i]=='1' ? MSEQ_ON_PAGE : MSEQ_OFF_PAGE);
		cyclepage[i].Xpos = 0;
		cyclepage[i].Ypos = 0;
		cyclepage[i].Frames = f_iFramesPerTerm;
		cyclepage[i].Stop=0;
		cyclepage[i].Page = ipage + vsgTRIGGERPAGE;
		cyclepage[i].ovPage = OVERLAY_APERTURE_PAGE;
	}
	if (!f_bNoSplit)
	{
		for (int i=0; i<nTerms; i++)
		{
			ipage = (f_mseq[i]=='1' ? MSEQ_OFF_PAGE : MSEQ_ON_PAGE);
			cyclepage[i+nTerms].Xpos = 0;
			cyclepage[i+nTerms].Ypos = 0;
			cyclepage[i+nTerms].Frames = f_iFramesPerTerm;
			cyclepage[i+nTerms].Stop=0;
			cyclepage[i+nTerms].Page = ipage + vsgTRIGGERPAGE;
			cyclepage[i+nTerms].ovPage = OVERLAY_APERTURE_PAGE;
		}
		cyclepage[2*nTerms].Xpos = 0;
		cyclepage[2*nTerms].Ypos = 0;
		cyclepage[2*nTerms].Frames = nFramesBlank;
		cyclepage[2*nTerms].Stop=0;
		cyclepage[2*nTerms].Page = vsgTRIGGERPAGE;	// WHICH PAGE DOESN'T MATTER HERE. Overlay covers it. 
		cyclepage[2*nTerms].ovPage = OVERLAY_BLANK_PAGE;

		vsgPageCyclingSetup(2*nTerms+1, &cyclepage[0]);
	}
	else
	{
		cyclepage[nTerms].Xpos = 0;
		cyclepage[nTerms].Ypos = 0;
		cyclepage[nTerms].Frames = nFramesBlank;
		cyclepage[nTerms].Stop=0;
		cyclepage[nTerms].Page = vsgTRIGGERPAGE;;
		cyclepage[nTerms].ovPage = OVERLAY_BLANK_PAGE;
		vsgPageCyclingSetup(nTerms+1, &cyclepage[0]);
	}


	// sleep a couple of seconds
	Sleep(2000);

	// reset timer and start cycling
	timeUS = (f_lengthSeconds + f_blankSeconds) * f_nRepeats * 1.0e6;
	if (f_verbose)
	{
		cout << "Total stimulus length (us): " << timeUS << endl;
	}
	vsgResetTimer();
	vsgSetCommand(vsgOVERLAYMASKMODE);		// makes overlay pages visible
	vsgSetCommand(vsgCYCLEPAGEENABLE);
	while (vsgGetTimer() < timeUS)
	{
		Sleep(1000);
	}
	vsgSetCommand(vsgCYCLEPAGEDISABLE);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, OVERLAY_BLANK_PAGE);

	if (f_mseq) free(&f_mseq);

	return 0;
}




void getOnOffColors(COLOR_VECTOR_TYPE& cv, int contrast, COLOR_TYPE& cton, COLOR_TYPE& ctoff)
{
	VSGTRIVAL from, to, mid, v, on, off;
	double norm;
	double full;

	get_colorvector(cv, from, to);
	mid.a = (from.a + to.a)/2;
	mid.b = (from.b + to.b)/2;
	mid.c = (from.c + to.c)/2;

	// Form unit vector
	v.a = to.a - from.a;
	v.b = to.b - from.b;
	v.c = to.c - from.c;
	norm = sqrt(v.a*v.a + v.b*v.b + v.c*v.c);
	full = norm/2;
	v.a = v.a/norm;
	v.b = v.b/norm;
	v.c = v.c/norm;

	on.a = mid.a + v.a * full * f_contrast/100.0;
	on.b = mid.a + v.b * full * f_contrast/100.0;
	on.c = mid.a + v.c * full * f_contrast/100.0;
	off.a = mid.a - v.a * full * f_contrast/100.0;
	off.b = mid.a - v.b * full * f_contrast/100.0;
	off.c = mid.a - v.c * full * f_contrast/100.0;

	cton.type = custom;
	cton.color = on;
	ctoff.type = custom;
	ctoff.color = off;
	return;
}


int args(int argc, char **argv)
{	
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	bool have_B = false;
	bool have_l = false;
	bool have_d = false;
	bool have_b = false;
	bool have_c = false;
	bool have_t = false;
	bool have_m = false;

	while ((c = getopt(argc, argv, "r:B:l:o:ng:d:b:V:c:t:m:v")) != -1)
	{
		switch (c) 
		{
		case 'r':
			s.assign(optarg);
			if (parse_integer(s, f_nRepeats)) errflg++;
			break;
		case 'B': 
			s.assign(optarg);
			if (parse_double(s, f_blankSeconds)) errflg++; 
			else have_B = true;
			break;
		case 'l':
			s.assign(optarg);
			if (parse_double(s, f_lengthSeconds)) errflg++;
			else have_l = true;
			break;
		case 'o':
			s.assign(optarg);
			if (parse_integer(s, f_nTermsOffset)) errflg++;
			break;
		case 'n':
			f_bNoSplit = true;
			break;
		case 'g':
			s.assign(optarg);
			if (!parse_grating(s, f_grating1))
			{
				f_bUseGrating = true;
				f_grating0 = f_grating1;
				f_grating0.contrast *= -1;
			}
			else 
			{
				cerr << "Bad grating spec: " << s << endl;
				errflg++;
			}
			break;
		case 'd':
			s.assign(optarg);
			if (parse_distance(s, f_screenDistanceMM)) errflg++;
			else have_d=true;
			break;
		case 'b': 
			s.assign(optarg);
			if (parse_color(s, f_colorBackground)) errflg++; 
			else have_b = true;
			break;
		case 'c':
			s.assign(optarg);
			if (parse_integer(s, f_contrast)) errflg++;
			else have_c = true;
			break;
		case 't':
			s.assign(optarg);
			if (parse_integer(s, f_iFramesPerTerm))
			{
				cerr << "Cannot parse frames per term (" << s << "): must be an integer." << endl;
				errflg++;
			}
			else
			{
				have_t = true;
			}
			break;
		case 'V':
			cerr << "Color vector input not implemented." << endl;
			errflg++;
		case 'm':
			f_sFilename.assign(optarg);
			have_m = true;
			break;
		case 'v':
			f_verbose = true;
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
	if (!have_t)
	{
		cerr << "Frames per term not specified!" << endl; 
		errflg++;
	}
	if (!have_B)
	{
		cerr << "Blank time not specified!" << endl;
		errflg++;
	}
	if (!have_l)
	{
		cerr << "Sequence length not specified!" << endl;
		errflg++;
	}
	if (!have_b)
	{
		cerr << "Background color not specified!" << endl; 
		errflg++;
	}
	if (!have_c)
	{
		cerr << "Contrast not specified!" << endl; 
		errflg++;
	}
	if (!have_m)
	{
		cerr << "M-sequence filename not specified!" << endl; 
		errflg++;
	}
	else
	{
		// Load m sequence
		if (arutil_load_mseq(&f_mseq, f_sFilename))
		{
			errflg++;
		}
	}

	if (errflg) 
	{
		usage();
	}
	return errflg;
}

void usage()
{
	cerr << "usage: affflash -r #repeat -l mseq_seconds [-n] -B blank_seconds -t frames_per_term -m mseq_filename -c contrast -d screen_distance_MM [-V color_vector] [-g x,y,w,h,contrast%,sf,tf,orientation,color_vector,s|q,r|e]" << endl;
}

