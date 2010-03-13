#include "alertlib.h"
#include "VSGV8.H"
#include <stdio.h>
#include <math.h>

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")

int f_screenDistanceMM = 0;
COLOR_TYPE f_background = { gray, {} };
int f_iDotSize = 8;
int f_nRows = 16;
int f_nCols = 16;
int f_pixBoxX, f_pixBoxY;		// pixel coord of the single box drawn. For the white (black) box its relative to page 0 (2). 
int f_scrGridCenterX;
int f_scrGridCenterY;
int f_scrGridX, f_scrGridY;		// screen coord of origin (upper left corner) of grid
int f_scxGridX, f_scxGridY;		// screen pixel coord (measured from upper left corner, positive down and right) of grid origin.
int f_iFramesPerTerm;
bool f_verbose = false;

void usage();
int args(int argc, char **argv);

using namespace alert;
using namespace std;

int main(int argc, char **argv)
{
	string s;
	int scrBoxX, scrBoxY;
	bool bQuit = false;

	// Check input args
	if (args(argc, argv))
	{
		return 1;
	}

	// INit vsg
	if (ARvsg::instance().init(f_screenDistanceMM, f_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}

	// Pan/scroll mode makes video pages twice as wide as usual.
	vsgSetVideoMode(vsgPANSCROLLMODE);

	// clear all draw modes. In particular, this turns off vsgCENTREXY, so that primitives (like Rects) are 
	// drawn with their origins at upper left corner
	vsgSetDrawMode(0);

	// PALETTERAMP means black is color 0, white is 255, ramp between them. 
	vsgSetCommand(vsgPALETTERAMP);

	// clear video pages 0-3 to mean gray
	vsgSetDrawPage(vsgVIDEOPAGE, 0, 127);
	vsgSetDrawPage(vsgVIDEOPAGE, 1, 127);
	vsgSetDrawPage(vsgVIDEOPAGE, 2, 127);
	vsgSetDrawPage(vsgVIDEOPAGE, 3, 127);

	// Determine where to draw the boxes. 
	f_pixBoxX = vsgGetSystemAttribute(vsgPAGEWIDTH)/2;
	f_pixBoxY = vsgGetSystemAttribute(vsgPAGEHEIGHT);
	scrBoxX = f_pixBoxX - vsgGetScreenWidthPixels()/2;
	scrBoxY = f_pixBoxY - vsgGetScreenHeightPixels()/2;

	// Now draw the boxes
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	vsgSetPen1(255);
	vsgDrawRect(scrBoxX, scrBoxY, f_iDotSize, f_iDotSize);
	vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgNOCLEAR);
	vsgSetPen1(0);
	vsgDrawRect(scrBoxX, scrBoxY, f_iDotSize, f_iDotSize);
	if (f_verbose)
	{
		cerr << "Page dim wxh = " << vsgGetSystemAttribute(vsgPAGEWIDTH) << " x " << vsgGetSystemAttribute(vsgPAGEHEIGHT) << endl;
		cerr << "screen dim wxh = " << vsgGetScreenWidthPixels() << " x " << vsgGetScreenHeightPixels() << endl;
		cerr << "Box drawn at " << scrBoxX << ", " << scrBoxY << endl;
	}

	// compute other important numbers
	f_scrGridX = f_scrGridCenterX - f_nCols/2*f_iDotSize;
	f_scrGridY = f_scrGridCenterY + f_nRows/2*f_iDotSize;
	f_scxGridX = vsgGetScreenWidthPixels()/2 + f_scrGridX;
	f_scxGridY = vsgGetScreenHeightPixels()/2 - f_scrGridY;

	cout << "Enter command(qpm): ";
	cin >> s;
	while (!bQuit)
	{
		if (s=="q")
		{
			bQuit = true;
		}
		else if (s=="p")
		{
			int page;
			cout << "Enter page: ";
			cin >> s;
			if (!parse_integer(s, page))
			{
				vsgSetZoneDisplayPage(vsgVIDEOPAGE, page);
			}
		}
		else if (s=="m")
		{
			int ix, iy, i;
			cout << "Enter coords for move: ";
			cin >> s;
			if (!parse_int_pair(s, ix, iy))
			{
				i = vsgMoveScreen((double)ix, (double)iy);
				printf("MoveScreen returned %d\n", i);
			}
			else printf("Error in input\n");
		}
		else if (s=="t")
		{
			int ix, iy;
			int x, y;
			cout << "Enter row, column: ";
			cin >> s;
			if (!parse_int_pair(s, ix, iy))
			{
				x = f_pixBoxX - f_scxGridX - ix*f_iDotSize;
				y = f_pixBoxY - f_scxGridY - iy*f_iDotSize;
				vsgMoveScreen(x, y);
				cout << "x,y = " << x << ", " << y << endl;
			}
		}
		else if (s=="o")
		{
			// animation along outline of grid
			int i, j;
			int x, y;
			int count=0;
			string stmp;
			VSGCYCLEPAGEENTRY cycle[32768];

			j=0;
			for (i=0; i<f_nCols; i++)
			{
				x = f_pixBoxX - f_scxGridX - i*f_iDotSize;
				y = f_pixBoxY - f_scxGridY - j*f_iDotSize;
				cycle[count].Frames = f_iFramesPerTerm;
				cycle[count].Page = 0;
				cycle[count].Xpos = x;
				cycle[count].Ypos = y;
				count++;
			}
			i = 15;
			for (j=1; j<f_nCols; j++)
			{
				x = f_pixBoxX - f_scxGridX - i*f_iDotSize;
				y = f_pixBoxY - f_scxGridY - j*f_iDotSize;
				cycle[count].Frames = f_iFramesPerTerm;
				cycle[count].Page = 0;
				cycle[count].Xpos = x;
				cycle[count].Ypos = y;
				count++;
			}
			j = 15;
			for (i=f_nCols-1; i>=0; i--)
			{
				x = f_pixBoxX - f_scxGridX - i*f_iDotSize;
				y = f_pixBoxY - f_scxGridY - j*f_iDotSize;
				cycle[count].Frames = f_iFramesPerTerm;
				cycle[count].Page = 0;
				cycle[count].Xpos = x;
				cycle[count].Ypos = y;
				count++;
			}
			i = 0;
			for (j=f_nCols-1; j>=0; j--)
			{
				x = f_pixBoxX - f_scxGridX - i*f_iDotSize;
				y = f_pixBoxY - f_scxGridY - j*f_iDotSize;
				cycle[count].Frames = f_iFramesPerTerm;
				cycle[count].Page = 0;
				cycle[count].Xpos = x;
				cycle[count].Ypos = y;
				count++;
			}
			j=0;
			for (i=1; i<f_nCols; i++)
			{
				x = f_pixBoxX - f_scxGridX - i*f_iDotSize;
				y = f_pixBoxY - f_scxGridY - j*f_iDotSize;
				cycle[count].Frames = f_iFramesPerTerm;
				cycle[count].Page = 2;
				cycle[count].Xpos = x;
				cycle[count].Ypos = y;
				count++;
			}
			i = 15;
			for (j=1; j<f_nCols; j++)
			{
				x = f_pixBoxX - f_scxGridX - i*f_iDotSize;
				y = f_pixBoxY - f_scxGridY - j*f_iDotSize;
				cycle[count].Frames = f_iFramesPerTerm;
				cycle[count].Page = 2;
				cycle[count].Xpos = x;
				cycle[count].Ypos = y;
				count++;
			}
			j = 15;
			for (i=f_nCols-1; i>=0; i--)
			{
				x = f_pixBoxX - f_scxGridX - i*f_iDotSize;
				y = f_pixBoxY - f_scxGridY - j*f_iDotSize;
				cycle[count].Frames = f_iFramesPerTerm;
				cycle[count].Page = 2;
				cycle[count].Xpos = x;
				cycle[count].Ypos = y;
				count++;
			}
			i = 0;
			for (j=f_nCols-1; j>=0; j--)
			{
				x = f_pixBoxX - f_scxGridX - i*f_iDotSize;
				y = f_pixBoxY - f_scxGridY - j*f_iDotSize;
				cycle[count].Frames = f_iFramesPerTerm;
				cycle[count].Page = 2;
				cycle[count].Xpos = x;
				cycle[count].Ypos = y;
				count++;
			}
			vsgPageCyclingSetup(count, &cycle[0]);
			cout << "Hit any key to start." << endl;
			cin >> stmp;
			vsgSetCommand(vsgCYCLEPAGEENABLE);
			cout << "Hit any key to stop." << endl;
			cin >> stmp;
			vsgSetCommand(vsgCYCLEPAGEDISABLE);
		}

#if 0
		else if (s=="o")
		{
			int ix, iy;

			cout << "Enter rows, columns: ";
			cin >> s;
			if (!parse_int_pair(s, f_nrows, f_ncols))
			{
				cout << "Enter grid center, in screen coord: ";
				cin >> s;
				if (!parse_int_pair(s, ix, iy))
				{
					double x0, y0;

					// Figure out where the grid upper left corner is. Round it down to multiples of 4.
					x0 = ix - f_ncols/2*f_dotsize;
					y0 = iy - f_nrows/2*f_dotsize;

					f_scrOriginGridX = (int)(x0/4)*4;
					f_scrOriginGridY = (int)(y0/4)*4;
					cout << "Grid origin, screen coords: " << f_scrOriginGridX << ", " << f_scrOriginGridY << endl;

					f_pixOriginGridX = f_scrOriginGridX + 400;
					f_pixOriginGridY = 300 - f_scrOriginGridY;
					cout << "Grid origin, pixel coords: " << f_pixOriginGridX << ", " << f_pixOriginGridY << endl;
				}
			}
		}
		else if (s=="t")
		{
			int ix, iy;
			int x, y;
			cout << "Enter grid coord: ";
			cin >> s;
			if (!parse_int_pair(s, ix, iy))
			{
				x = ix*f_dotsize;
				y = iy*f_dotsize;

				// Now move window
				vsgMoveScreen(
			}
		}
		else if (s=="c")
		{
			int i, j;
			int x, y;
			int count=0;
			int F = 10;
			VSGCYCLEPAGEENTRY cycle[32768];
			string stmp;

			for (i=0; i<100; i++)
			{
				x = 1019 - i*f_dotsize;
				for (j=0; j<75; j++)
				{
					y = 1019 - j*f_dotsize;
					cycle[count].Frames = F;
					cycle[count].Page = 0;
					cycle[count].Xpos = x;
					cycle[count].Ypos = y;
					count++;
				}
			}
			for (i=0; i<100; i++)
			{
				x = 1019 - i*f_dotsize;
				for (j=0; j<75; j++)
				{
					y = 1019 - j*f_dotsize;
					cycle[count].Frames = F;
					cycle[count].Page = 2;
					cycle[count].Xpos = x;
					cycle[count].Ypos = y;
					count++;
				}
			}
			vsgPageCyclingSetup(count, &cycle[0]);
			cout << "Hit any key to start." << endl;
			cin >> stmp;
			vsgSetCommand(vsgCYCLEPAGEENABLE);
			cout << "Hit any key to stop." << endl;
			cin >> stmp;
			vsgSetCommand(vsgCYCLEPAGEDISABLE);
		}
#endif
		else
		{
			printf("Unknown input.\n");
			break;
		}
		if (bQuit) break;
		cout << "Enter command(qpm): ";
		cin >> s;
	}





	ARvsg::instance().release_lock();
	return 0;
}

int args(int argc, char **argv)
{	
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	bool have_p = false;
	bool have_d = false;
	bool have_r = false;
	bool have_c = false;
	bool have_t = false;

	while ((c = getopt(argc, argv, "d:p:r:c:t:hv")) != -1)
	{
		switch (c) 
		{
		case 'p':
			s.assign(optarg);
			if (parse_int_pair(s, f_scrGridCenterX, f_scrGridCenterY))
			{
				cerr << "Cannot parse grid center position (" << s << "): expecting integer positions, no spaces." <<endl;
			}
			else
			{
				have_p = true;
			}
			break;
		case 'd':
			s.assign(optarg);
			if (parse_integer(s, f_iDotSize))
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
			if (parse_integer(s, f_nRows))
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
			if (parse_integer(s, f_nCols))
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
			errflg++;
			break;
		}
	}

	if (!have_d) 
	{
		cerr << "No dot size specified!" << endl; 
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
	if (!have_p)
	{
		cerr << "No position (-p) specified!" << endl;
		errflg++;
	}
	if (errflg) 
	{
		usage();
	}
	return errflg;
}

void usage()
{
	cerr << "Usage: sparse -d dotsize -p gridCenterX,gridCenterY -r nrows -c ncolumns -t frames_per_term" << endl;
}