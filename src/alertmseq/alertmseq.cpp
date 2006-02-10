#include "VSGV8.H"
#include "msequence.h"
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__
#include <iostream>
#include <cstdio>
#include <cmath>
#include "alertlib.h"

using namespace std;

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")



VSGCYCLEPAGEENTRY MPositions[32768];

int f_iRows=16;
int f_iCols=16;
bool f_verbose;
int f_iOrder=15;		// order of the m-sequence. Expect 2**order-1 terms in the sequence.
string f_sFilename;	// sequence filename
bool f_isRepeating = false;
int f_iSegmentLength=0;
int f_iFramesPerTerm=1;
int f_iDot = 4;
int f_iZoom = 4;
float f_apX=0;
float f_apY=0;
char *f_sequence=NULL;



void tokenize(const string& str, vector<string>& tokens, const string& delimiters = " ")
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}


int parse_xy(std::string s, double& x, double& y)
{
	int status=0;
	vector<string> tokens;
	tokenize(s, tokens, ",");
	if (tokens.size() != 2)
	{
		cerr << "Bad x,y format: " << s << endl;
		status=1;
	}
	else
	{
		istringstream iss;
		iss.str(tokens[0]);
		iss >> x;
		if (!iss) 
		{
			cerr << "bad x: " << tokens[0] << endl;
			status=1;
		}
		iss.str(tokens[1]);
		iss.clear();
		iss >> y;
		if (!iss) 
		{
			cerr << "bad y: " << tokens[1] << endl;
			status=1;
		}
	}
	return status;
}



int parse_double(std::string s, double& d)
{
	int status=0;
	istringstream iss(s);
	iss >> d;
	if (!iss) 
	{
		cerr << "bad format for double: " << s << endl;
		status=1;
	}
	return status;
}




int parse_integer(std::string s, int& dist)
{
	int status=0;
	istringstream iss(s);
	iss >> dist;
	if (!iss) 
	{
		cerr << "bad integer: " << s << endl;
		status=1;
	}
	return status;
}

int parse_repeating(string &s, RepeatingSpec &r)
{
	int status=0;
	string szTemp;
	vector<string> tokens;
	tokenize(s, tokens, ",");
	if (tokens.size() != 4)
	{
		cerr << "Bad repeating format: " << s << endl;
		status=1;
	}
	else
	{
		istringstream iss;
		iss.str(tokens[0]);
		iss >> r._iFramesPerTerm;
		if (!iss) 
		{
			cerr << "bad frames_per_term value: " << tokens[0] << endl;
			status=1;
		}
		iss.clear();
		iss.str(tokens[1]);
		iss >> r._iRepeats;
		if (!iss) 
		{
			cerr << "bad repeats value: " << tokens[1] << endl;
			status=1;
		}
		iss.clear();
		iss.str(tokens[2]);
		iss >> r._iFirst;
		if (!iss) 
		{
			cerr << "bad first frame value: " << tokens[2] << endl;
			status=1;
		}
		iss.clear();
		iss.str(tokens[3]);

		// get last character
		char c = tokens[3].c_str()[tokens[3].size()-1];
		if (c == 'f')
		{
			int icount;
			iss >> icount;
			if (!iss)
			{
				cerr << "bad frame count value: " << tokens[3] << endl;
				status=1;
			}
			else
			{
				r._iLast = r._iFirst + icount -1;
			}
		}
		else if (c == 's')
		{
			float sec;
			iss >> sec;
			if (!iss)
			{
				cerr << "bad frame duration value: " << tokens[3] << endl;
				status=1;
			}
			else
			{
				int nterms = (int)(1000000 / r._uspf / r._iFramesPerTerm * sec);
				r._iLast = r._iFirst + nterms -1;
			}
		}
		else if (c >= '0' && c<= '9')
		{
			iss >> r._iLast;
			if (!iss)
			{
				cerr << "bad last frame value: " << tokens[3] << endl;
				status=1;
			}
		}

	}
	return status;

}



// draw mseq, assuming that 
int draw_mseq()
{
	int rindex, cindex, term;
	int nterms = pow(2, f_iOrder) - 1;

	vsgSetDrawPage(vsgVIDEOPAGE,0,0);
	vsgSetCommand(vsgPALETTERAMP);
	vsgSetPen1(255);
	vsgSetPen2(0);
	vsgSetCommand(vsgVIDEOCLEAR);

	for (rindex=0; rindex<128*16+15; rindex++)
	{
		for (cindex=0; cindex<31; cindex++)
		{
			term=(16*128*rindex+128*cindex) % nterms;
			if (f_sequence[term]=='1') 
			{
				vsgDrawRect(f_iDot*(cindex+.5), f_iDot/f_iZoom*(rindex+.5), f_iDot, f_iDot/f_iZoom);
			}
		}		
	}
	return 0;
}



int load_mseq(string& filename)
{
	int istatus=0;
	int nterms = pow(2, f_iOrder) -1;

	// Open mseq file
	FILE* fp=fopen(filename.c_str(), "r");
	if (!fp) 
	{
		istatus=1;
		cerr << "Cannot open sequence file " << filename << endl;
	}
	else
	{
		f_sequence = (char *)malloc(nterms+1);
		memset(f_sequence, 0, nterms+1);
		if (!fread(f_sequence, sizeof(char), nterms, fp))
		{
			istatus=2;
			cerr << "Expected " << nterms << " terms in seq. Check mseq file " << filename << endl;
		}
		else if ((int)strlen(f_sequence) != nterms)
		{
			istatus=3;
			cerr << "Expected " << nterms << " terms in seq. Found " << strlen(f_sequence) << ". Check mseq file." << endl;
		}
		fclose(fp);
	}

	return istatus;
}



int main(int argc, char **argv)
{
	int istatus=0;

	// Init vsg card
	istatus = vsgInit("");


	// check args
	if (args(argc, argv))
	{
		return 1;
	}

	// setup vsg
	vsgSetDrawOrigin(0,0);
	vsgSetCommand(vsgPALETTERAMP);
	vsgSetVideoMode(vsgPANSCROLLMODE);
	getZoomFactor(f_iDot, f_iZoom);
	vsgSetCommand(vsgOVERLAYMASKMODE);		// makes overlay pages visible


	// aperture location
	float W = vsgGetScreenWidthPixels();
	float H = vsgGetScreenHeightPixels();
	float w = f_iRows * f_iDot;		// the width of the entire grid, as it should appear on the screen
	float h = f_iCols * f_iDot;		// the height of the entire grid, as it should appear on the screen

	
	// prepare overlay
	VSGLUTBUFFER overlayLUT;
	overlayLUT[1].a=overlayLUT[1].b=overlayLUT[1].c=.5;
	overlayLUT[2].a=overlayLUT[2].b=0; overlayLUT[2].c=1;
	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&overlayLUT, 0, 3);

	// Overlay page 1 will have no aperture. It will serve as a blank page before and after stimulus starts. 
	vsgSetDrawPage(vsgOVERLAYPAGE, 1, 1);
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 1);
	vsgSetPen1(0);	// that's clear on the overlay page!
	vsgDrawRect(W/2+f_apX, H/2-f_apY, w, h);


	// draw the msequence into videomemory
	draw_mseq();
	

	// Page cycling setup. 
	int index=0;
	for (int iterm = f_pr->getFirst(); iterm <= f_pr->getLast(); iterm++)
	{
		// WARNING : rows and columns are hardcoded here. 
		int irow = (index%128) * f_iRows + floor(index/(128*16));
		int icol = (int)(floor(index/128))%f_iCols;

		MPositions[index].Page = 0+vsgDUALPAGE+vsgTRIGGERPAGE;
		MPositions[index].Xpos=-W/2 + w/2 - f_apX + icol*f_iDot;
		MPositions[index].Ypos=-H/(2*f_iZoom) + h/(2*f_iZoom) + f_apY/f_iZoom + irow*f_iDot/f_iZoom;

		if (index==0)
		{
			cout << "xy=" << MPositions[index].Xpos << "," << MPositions[index].Ypos << endl;
		}
		MPositions[index].Frames=f_pr->getFramesPerTerm();
		MPositions[index].Stop=0;
		MPositions[index].ovPage=0;
		MPositions[index].ovXpos=0;
		MPositions[index].ovYpos=0;
		index++;
	}

	// index is now the totalnumber of positions
	vsgPageCyclingSetup(index, &MPositions[0]);

	// reset timer and start cycling

	cout << "time us = " << f_pr->getTimeUS() << endl;
	cout << "uspf = " << vsgGetSystemAttribute(vsgFRAMETIME) << endl;
	vsgResetTimer();
	vsgSetCommand(vsgVIDEODRIFT+vsgOVERLAYDRIFT);			// allows us to move the offset of video memory
	vsgSetCommand(vsgCYCLEPAGEENABLE);
	while (vsgGetTimer() < f_pr->getTimeUS())
	{
//		cout << "sleeping, timer=" << vsgGetTimer() << " < " << f_pr->getTimeUS() << endl;
		Sleep(1000);
	}
	vsgSetCommand(vsgCYCLEPAGEDISABLE);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 1);

	Sleep(5000);	// per Daniel request.

	return 0;
}



// msequence -f filename -o order -r rows -c columns -R repeats,startframe,#s/#f

int args(int argc, char **argv)
{	
	bool have_f=false;
	// djs - remove orc options. The prog is basically hard-coded to use a 16x16 msequence of
	// order 15. 
	bool have_o=true;
	bool have_r=true;
	bool have_c=true;
	bool have_xy=false;
	bool have_F=false;
	bool have_t=false;
	bool have_d=false;

	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "f:l:t:vp:d:")) != -1)
	{
		switch (c) 
		{
		case 'p':
			double x,y;
			s.assign(optarg);
			if (parse_xy(s, x, y))
			{
				cerr << "Cannot parse pixel position (" << s << "): expecting integer pixel positions x,y, no spaces." <<endl;
			}
			else
			{
				have_xy = true;
				f_apX = (float)x;
				f_apY = (float)y;
			}
			break;
		case 'f':
			f_sFilename.assign(optarg);
			have_f = true;
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
		case 'l':
			s.assign(optarg);
			if (parse_integer(s, f_iSegmentLength))
			{
				cerr << "Cannot parse segment length (" << s << "): must be an integer." << endl;
				errflg++;
			}
			else
			{
				have_l = true;
			}
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

	if (!have_f) 
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
	if (!have_mode)
	{
		cerr << "No mode specified!" << endl;
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
	cerr << "usage: msequence -f filename -o order -r rows -c columns -R repeats,first_frame,last_frame|<seconds>s|<frames>f" << endl;
}

int getZoomFactor(int idot, int& zoom)
{
	int status=0;

#if 0
	if (!(idot%16))
	{
		zoom = 16;
		vsgSetVideoMode(vsgZOOM16);
	}
	else if (!(idot%8))
	{
		zoom = 8;
		vsgSetVideoMode(vsgZOOM8);
	}
	else if (!(idot%4))
	{
#endif
		zoom = 4;
		vsgSetVideoMode(vsgZOOM4);
#if 0
	}
	else if (!(idot%2))
	{
		zoom = 2;
		vsgSetVideoMode(vsgZOOM2);
	}
	else
	{
		zoom=1;
	}
#endif
	return status;
}



