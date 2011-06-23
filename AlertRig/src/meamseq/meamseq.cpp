#include "VSGV8.H"
#include "meamseq.h"
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__
#include <iostream>
#include <cstdio>
#include <conio.h>
#include <cmath>
#include "alertlib.h"

using namespace std;
using namespace alert;

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")



VSGCYCLEPAGEENTRY mpos[32768];

int f_iRows=16;
int f_iCols=16;
bool f_verbose = false;
int f_iOrder=15;		// order of the m-sequence. Expect 2**order-1 terms in the sequence.
int f_nterms = 0;
string f_sFilename;	// sequence filename
int f_iFramesPerTerm=1;
int f_iDot = 4;
int f_iapX=0;
int f_iapY=0;
int f_iapXCorner=0;
int f_iapYCorner=0;
char *f_sequence=NULL;
bool f_testing = false;

bool blankPage();
int draw_mseq(char *seq, int order, int r, int c, int d);
void prepare_cycling();
void testing_loop();

#define NO_APERTURE_PAGE 1
#define APERTURE_PAGE 0


// draw mseq. See Reid, Victor, Shapley m-sequence paper, eq 14. 
// We assume here that n=16, r=c=16. This makes p=128. 
// Also, eq (14) assumes that terms are indexed starting at term 0 (not 1) 
// and that the first row is row "0", first column is column "0". 
//
// The technique iterates across the video memory area to be used,
// (p*r + (r-1)) rows and (c + (c-1)) columns. Each (cindex, rindex) pair
// corresponds to the msequence term 'term', which is computed using the 
// spatial offset term in eq 14. If a term is '1' in our msequence, we draw
// a white rectangle at that grid location in memory. 
//
// parameters:
//	int r, c;	// rows, columns
//	int d;		// dot size; width and height, in pixels, of each square in the grid. 
//	int M;			// number of terms in msequence
//  int order;		// order of msequence. size of sequence is 2**n - 1 (this had better be the length of seq!)
//  char *seq		// character array of '0' and '1' values. Note these are char not int!


int draw_mseq(char *seq, int order, int r, int c, int d)
{
	int status = 0;
	int M;
	int j;
	int pg, offset;
	int x, y;
	int ind;
	int xrect, yrect;
	int p;
	int w_s, h_s;	// width and height of screen;
	int w_v, h_v;	// width and height of video page
	int N;			// number of video pages

	// some initialization
	M = (int)pow(2.0f, (float)order) - 1;
	p =  (int)pow(2.0f, (float)order)/(r*c);
	N = vsgGetSystemAttribute(vsgNUMVIDEOPAGES);
	w_v = vsgGetSystemAttribute(vsgPAGEWIDTH);
	h_v = vsgGetSystemAttribute(vsgPAGEHEIGHT);
	w_s = vsgGetScreenWidthPixels();
	h_s = vsgGetScreenHeightPixels();

	if (f_verbose)
	{
		cout << "drawing msequence with these parameters:" << endl;
		cout << "r,c = " << r << ", " << c << endl;
		cout << "order = " << order << endl;
		cout << "#terms = " << M << endl;
		cout << "p = " << p << endl;
		cout << "dotsize = " << d << endl;
		cout << "screen width, height = " << w_s << ", " << h_s << endl;
		cout << "vid pg width, height = " << w_v << ", " << h_v << endl;
		cout << "num vid pages = " << N << endl;
	}

	// verify input parameters are kosher. Too tall?
	if ((p * r * d) > (N * h_v))
	{
		cout << "ERROR: The mseq will not fit in video memory." << endl;
		cout << "       Total height required is p * r * d = " << (p*r*d) << endl;
		cout << "       System has " << N << " video pages of height " << h_v << " pixels, for a total height of " << (N*h_v) << " pixels." << endl;
		cout << "       You can try reducing dot size (d) or the number of rows (r), or using a shorter msequence." << endl;
		status = 1;
	}

	// verify input parameters are kosher. Too tall?
	if ((2*c-1)*d > w_v)
	{
		cout << "ERROR: The mseq will not fit in video memory." << endl;
		cout << "       Total page width required is (2c-1)*d = " << (2*c-1)*d << endl;
		cout << "       System video pages have width " << w_v << " pixels" << endl;
		cout << "       You can try reducing dot size (d) or the number of columns (c)." << endl;
		cout << "       Its also possible that by increasing the screen resolution the video page size will be increased, though that may make the video memory height too small." << endl;
		status = 1;
	}

	// will it fit on screen?
	if ((r * d) > h_s)
	{
		cout << "ERROR: The mseq will not fit on the screen. " << endl;
		cout << "       Screen height is " << h_s << " pixels, but the mseq requires at least r*d = " << (r*d) << " pixels." << endl;
		cout << "       You can try reducing the dot size (d) or the number of rows (r)." << endl;
		status = 1;
	}

	if ((c * d) > w_s)
	{
		cout << "ERROR: The mseq will not fit on the screen. " << endl;
		cout << "       Screen width is " << w_s << " pixels, but the mseq requires at least c*d = " << (c*d) << " pixels." << endl;
		cout << "       You can try reducing the dot size (d) or the number of columns (c)." << endl;
		status = 1;
	}

	if (status) return status;


	// prepare vsg for drawing
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	vsgSetCommand(vsgPALETTERAMP);
	vsgSetPen1(255);	// "ON" is white
	vsgSetPen2(0);		// "OFF" is black
	vsgSetCommand(vsgVIDEOCLEAR);	// clear all video pages to pen2 (black)

	// start drawing. 
	for (j=0; j<(p+1); j++)
	{
		// what page does this term start on?
		pg = (j*r*d)/h_v;
		vsgSetDrawPage(vsgVIDEOPAGE, pg, vsgNOCLEAR);

		// offset from top of page?
		offset = (j * r * d)%h_v;

		// draw dots
		for (x=0; x<(2*c-1); x++)
		{
			for (y=0; y<r; y++)
			{
				ind = (j + p*x + p*c*y) % M;
				if (seq[ind] == '1')
				{
					xrect = x*d;
					yrect = offset + y*d;
					vsgDrawRect(xrect, yrect, d, d);
				}
			}
		}
	}

	return 0;
}


int load_mseq(string& filename)
{
	FILE *fp=NULL;
	int istatus=0;
	f_nterms = (int)pow(2.0f, f_iOrder) -1;

	// Open mseq file
	if (fopen_s(&fp, filename.c_str(), "r"))
	{
		istatus=1;
		cerr << "Cannot open sequence file " << filename << endl;
	}
	else
	{
		f_sequence = (char *)malloc(f_nterms+1);
		memset(f_sequence, 0, f_nterms+1);
		if (!fread(f_sequence, sizeof(char), f_nterms, fp))
		{
			istatus=2;
			cerr << "Expected " << f_nterms << " terms in seq. Check mseq file " << filename << endl;
		}
		else if ((int)strlen(f_sequence) != f_nterms)
		{
			istatus=3;
			cerr << "Expected " << f_nterms << " terms in seq. Found " << strlen(f_sequence) << ". Check mseq file." << endl;
		}
		fclose(fp);
	}

	return istatus;
}

void prepareOverlay()
{
	// prepare overlay
	VSGTRIVAL ovcolor;
	ovcolor.a = ovcolor.b = ovcolor.c = 0.5;
	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&ovcolor, 1, 1);
	vsgSetCommand(vsgOVERLAYMASKMODE);
	vsgSetDrawPage(vsgOVERLAYPAGE, NO_APERTURE_PAGE, 1);
	vsgSetDrawPage(vsgOVERLAYPAGE, APERTURE_PAGE, 1);
	vsgSetPen1(0);	// that's clear on the overlay page!
	vsgDrawRect(f_iapXCorner, f_iapYCorner, f_iDot*f_iCols, f_iDot*f_iRows);
}


int main(int argc, char **argv)
{
	int istatus=0;

	// Init vsg card
	istatus = vsgInit("");
	if (istatus)
	{
		cerr << "Error in vsgInit. Another VSG program may be running!" << endl;
		return 1;
	}

	// check args
	if (args(argc, argv))
	{
		return 1;
	}

	// setup vsg
	vsgSetDrawOrigin(0,0);
	vsgSetDrawMode(vsgSOLIDFILL);			// default is vsgCENTREXY! This makes it top-left-hand-corner origin
	vsgSetCommand(vsgPALETTERAMP);
	vsgSetCommand(vsgVIDEOCLEAR);

	// aperture location will be in f_iapXCorner, f_iapYCorner
	// I assume that the position given was in pixels (and hence is in integers). 
	f_iapXCorner = vsgGetScreenWidthPixels()/2 + f_iapX - (f_iCols * f_iDot)/2;
	f_iapYCorner = vsgGetScreenHeightPixels()/2 - f_iapY - (f_iRows * f_iDot)/2;
	if (f_verbose)
	{
		cout << "aperture center (" << f_iapX << ", " << f_iapY << ")" << endl;
		cout << "aperture corner (" << f_iapXCorner << ", " << f_iapYCorner << ")" << endl;
	}

	// Now draw pages.....
	prepareOverlay();
	blankPage();

	// draw the msequence into videomemory
	if (draw_mseq(f_sequence, f_iOrder, f_iRows, f_iCols, f_iDot))
	{
		// error drawing msequence. Exit now.
		return -1;
	}

	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);


	if (f_testing)
	{
		testing_loop();
	}
	else
	{
		prepare_cycling();
		cout << "Hit S  to start msequence." << endl;
		while (true)
		{
			if (_kbhit() && ('S'==_getch()))
			{
				break;
			}
			Sleep(100);
		}
		vsgSetCommand(vsgVIDEODRIFT+vsgOVERLAYDRIFT);			// allows us to move the offset of video memory
		vsgSetCommand(vsgCYCLEPAGEENABLE);
		Sleep(1000);
		cout << "Hit q to abort msequence." << endl;
		while (vsgGetSystemAttribute(vsgPAGECYCLINGSTATE) >= 0) 
		{
			if (_kbhit() && ('q'==_getch()))
			{
				vsgSetCommand(vsgCYCLEPAGEDISABLE);
				vsgSetZoneDisplayPage(vsgOVERLAYPAGE, NO_APERTURE_PAGE);
				cout << "Aborted by user." << endl;
				break;
			}
			Sleep(1000);
		}
		cout << "Done with msequence." << endl;
	}


	return 0;
}


void testing_loop()
{
	// testing loop...
	std::string s;
	bool bQuit = false;
	while (!bQuit)
	{
		// Get a new "trigger" from user
		cout << "Enter key: ";
		cin >> s;

		switch(s[0])
		{
		case 'q':
		case 'Q':
			{
				bQuit = true;
				break;
			}
		case 'a':
			{
				// OK activate the aperture page
				vsgSetZoneDisplayPage(vsgOVERLAYPAGE, APERTURE_PAGE);
				break;
			}
		case 'n':
			{
				// OK activate the aperture page
				vsgSetZoneDisplayPage(vsgOVERLAYPAGE, NO_APERTURE_PAGE);
				break;
			}
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			{
				int irow, icol;
				int xterm, yterm;
				int iterm = atoi(s.c_str());
				int p;
				int h;
				int w;

				p =  (int)pow(2.0f, (float)f_iOrder)/(f_iRows*f_iCols);
				w = vsgGetScreenWidthPixels();
				h = vsgGetScreenHeightPixels();

				// compute corner position for this term. 
				irow = (iterm % p)*f_iRows + iterm/(p*f_iRows);
				icol = (iterm/p)%f_iCols;
				xterm = (icol * f_iDot) - (w/2 - (f_iCols*f_iDot)/2);
				yterm = (irow * f_iDot) - (h/2 - (f_iRows*f_iDot)/2);
				if (f_verbose)
				{
					cout << "term " << iterm << " irow, icol = (" << irow << ", " << icol << ")" << endl;
					cout << "     " << " xterm, yterm = (" << xterm << ", " << yterm << ")" << endl;
				}

				vsgSetZoneDisplayPage(vsgOVERLAYPAGE, APERTURE_PAGE);
				vsgMoveScreen(xterm, yterm);
				break;
			}
		case 'm':
			{
				prepare_cycling();
				cout << "Start cycling ...." << endl;
				vsgSetCommand(vsgVIDEODRIFT+vsgOVERLAYDRIFT);			// allows us to move the offset of video memory
				vsgSetCommand(vsgCYCLEPAGEENABLE);
				Sleep(1000);
				while (vsgGetSystemAttribute(vsgPAGECYCLINGSTATE) >= 0) Sleep(1000);
				cout << "Done cycling." << endl;
				break;
			}
		default:
			{
				cout << "Unknown entry. Try again." << endl;
				break;
			}
		}
	}

}


void prepare_cycling()
{
	int iterm;
	int irow, icol;
	int xterm, yterm;
	int p;
	int h;
	int w;
	int M;

	p =  (int)pow(2.0f, (float)f_iOrder)/(f_iRows*f_iCols);
	w = vsgGetScreenWidthPixels();
	h = vsgGetScreenHeightPixels();
	M = (int)pow(2.0f, (float)f_iOrder) - 1;

	for (iterm=0; iterm<M; iterm++)
	{
		irow = (iterm % p)*f_iRows + iterm/(p*f_iRows);
		icol = (iterm/p)%f_iCols;
//		xterm = (icol * f_iDot) - (w/2 - (f_iCols*f_iDot)/2);
//		yterm = (irow * f_iDot) - (h/2 - (f_iRows*f_iDot)/2);
		xterm = (icol * f_iDot) - f_iapXCorner;
		yterm = (irow * f_iDot) - f_iapYCorner;
		mpos[iterm].Frames = f_iFramesPerTerm;
		mpos[iterm].ovPage = APERTURE_PAGE;
		mpos[iterm].ovXpos = 0;
		mpos[iterm].ovYpos = 0;
		mpos[iterm].Page = 0 + vsgDUALPAGE + vsgTRIGGERPAGE;
		mpos[iterm].Stop = 0;
		mpos[iterm].Xpos = xterm;
		mpos[iterm].Ypos = yterm;
	}
	mpos[M].Page = 0 + vsgDUALPAGE + vsgTRIGGERPAGE;
	mpos[M].ovPage = NO_APERTURE_PAGE;
	mpos[M].Stop = 1;

	vsgPageCyclingSetup(M+1, mpos);
}

bool blankPage()
{
	bool bvalue=true;
	vsgSetCommand(vsgCYCLEPAGEDISABLE);
	vsgSetDrawPage(vsgOVERLAYPAGE, NO_APERTURE_PAGE, 1);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, NO_APERTURE_PAGE);

	// write trigger to indicate stim and dot have stopped. 
//	vsgIOWriteDigitalOut(0, TRIGGERLINE_DOT | TRIGGERLINE_STIM);

	return bvalue;
}


// msequence -f filename -o order -r rows -c columns -R repeats,startframe,#s/#f

int args(int argc, char **argv)
{	
	bool have_o=false;
	bool have_r=false;
	bool have_c=false;
	bool have_xy=false;
	bool have_F=false;
	bool have_m = false;

	bool have_t=false;
	bool have_d=false;

	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "t:r:c:o:vp:d:am:D:K")) != -1)
	{
		switch (c) 
		{
		case 'K':
			f_testing = true;
			break;
		case 'p':
			s.assign(optarg);
			if (parse_int_pair(s, f_iapX, f_iapY))
			{
				cerr << "Cannot parse pixel position (" << s << "): expecting integer pixel positions x,y, no spaces." <<endl;
			}
			else
			{
				have_xy = true;
			}
			break;
		case 'm':
			f_sFilename.assign(optarg);
			have_m = true;
			break;
		case 'o':
			s.assign(optarg);
			if (parse_integer(s, f_iOrder))
			{
				cerr << "Cannot parse order (" << s << "): must be an integer." << endl;
				errflg++;
			}
			else 
			{
				have_o = true;
			}
			break;
		case 'd':
			s.assign(optarg);
			if (parse_integer(s, f_iDot))
			{
				cerr << "Cannot parse dot size (" << s << "): must be an integer." << endl;
				errflg++;
			}
			else
			{
				have_d = true;
			}
			break;
		case 'r':
			s.assign(optarg);
			if (parse_integer(s, f_iRows))
			{
				cerr << "Cannot parse rows (" << s << "): must be an integer." << endl;
				errflg++;
			}
			else
			{
				have_r = true;
			}
			break;
		case 'c':
			s.assign(optarg);
			if (parse_integer(s, f_iCols))
			{
				cerr << "Cannot parse columns (" << s << "): must be an integer." << endl;
				errflg++;
			}
			else
			{
				have_c = true;
			}
			break;
		case 'v':
			f_verbose = true;
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
		case 'h':
			errflg++;
			break;
		case '?':
            errflg++;
			break;
		default:
			cerr << "Unknown argument (" << c << ")" << endl;
			errflg++;
			break;
		}
	}

	if (!have_m) 
	{
		cerr << "No sequence file specified!" << endl; 
		errflg++;
	}
	if (!have_o)
	{
		cerr << "Sequence order not specified!" << endl; 
		errflg++;
	}
	if (!have_r || !have_c)
	{
		cerr << "Both rows and columns must be specified!" << endl; 
		errflg++;
	}
	if (!have_t)
	{
		cerr << "No frames_per_term value specified!" << endl;
		errflg++;
	}
	if (!have_xy)
	{
		cerr << "No position (-p) specified!" << endl;
		errflg++;
	}
	if (errflg) 
	{
		usage();
	}
	else
	{
		if (load_mseq(f_sFilename))
		{
			errflg++;
			cerr << "Error loading mseq file " << f_sFilename << endl;
		}
	}
	return errflg;
}

void usage()
{
	cerr << "usage: msequence -f filename -o order -r rows -c columns -d dotsize -p x,y -t frames_per_term" << endl;
}




