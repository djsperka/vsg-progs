//#include "VSGEX2.H"
#include "VSGEX2.H"
#include <stdio.h>
#include <math.h>

#pragma comment(lib, "vsgex2.lib")


#define DOT_INDEX 255
#define TEXT_INDEX 1

typedef struct mseq_struct
{
	int n;		// order of m seq
	int r,c;	// rows and cols in 2-d grid
	int p;
	int nterms;
	char *S;	// The m seq itself - ASCII '1' and '0'
} MSEQ;

typedef struct view_struct
{
	int z;		// zoom factor
	int d;		// dot size in pixels
	double x,y;	// coords of center of grid
	double w,h;	// width and height of each term "grid"
	MSEQ * m;	// pointer to mseq struct
} VIEW;

char error[256];
bool dbg_text=false;
int FramesPerTerm=100;
VSGCYCLEPAGEENTRY MPositions[32768];
MSEQ M;
VIEW V;
double x_offset=0;
double y_offset=0;

// usage
// prog mseqfile n r c d x y


#define NumberOfTerms 32767

int draw_mseq2()
{
	int rindex, cindex, term;
	printf("using old mseq drawing technique\n");


	// Set level 1 to red for text
	VSGLUTBUFFER buf;
	buf[1].a=1; buf[1].b=buf[1].c=0;
	vsgPaletteWrite((VSGLUTBUFFER*)buf, TEXT_INDEX, 1);

	vsgSetDrawPage(vsgVIDEOPAGE,0,0);
	vsgSetPen1(255);
	vsgSetPen2(0);
	vsgSetCommand(vsgVIDEOCLEAR);




	for (rindex=0; rindex<128*16+15; rindex++)
	{
		for (cindex=0; cindex<31; cindex++)
		{
			term=(16*128*rindex+128*cindex) % NumberOfTerms;
			if (M.S[term]=='1') 
			{
				vsgDrawRect(V.d*(cindex+.5),V.d/V.z*(rindex+.5),V.d,V.d/V.z);
			}

			{
				double a=V.d*(cindex+.5);
				double b=V.d/V.z*(rindex+.5);
				double c=V.d;
				double d=V.d/V.z;
				if (rindex==0 && cindex<5) 
				{
					printf("r,c=%d,%d xy=%lf, %lf\n", rindex, cindex, a, b);
				}
//				vsgDrawRect(PixelWidth*(cindex+.5),PixelHeight*(rindex+.5),PixelWidth,PixelHeight);
			}
		}		
	}
	return 0;
}





int load_mseq(int argc, char *argv[])
{
	int istatus=0;

	// init storage
	memset(&M, 0, sizeof(MSEQ));


	// interpret input args
	M.n = atoi(argv[2]);
	M.r = atoi(argv[3]);
	M.c = atoi(argv[4]);
	M.p = (int)(pow(2, M.n)/(M.r*M.c));
	M.nterms = (int)pow(2, M.n)-1;

	// r and c should be powers of 2!!!
	// TODO: ENFORCE THIS REQUIREMENT

	// Open mseq file
	FILE* fp=fopen(argv[1], "r");
	if (!fp) 
	{
		istatus=1;
		sprintf(error, "Cannot open file %s", argv[1]);
	}
	else
	{
		// Allocate buffer for mseq
		int N = (int)pow(2, M.n);
		M.S = (char *)malloc(N);
		memset(M.S, 0, N);
		if (!fread(M.S, sizeof(char), N-1, fp))
		{
			istatus=2;
			sprintf(error, "Expected %d terms in seq. Check mseq file.", N-1);
		}
		else if ((int)strlen(M.S) != N-1)
		{
			istatus=3;
			sprintf(error, "Expected %d terms in seq. Found %d. Check mseq file.", N-1, strlen(M.S));
		}
		fclose(fp);
	}

	return istatus;
}

int load_view(int argc, char *argv[])
{
	memset(&V, 0, sizeof(VIEW));
	V.d = atoi(argv[5]);
	V.x = atof(argv[6]);
	V.y = atof(argv[7]);
	return 0;
}

int init_vsg(void)
{
	int istatus=0;

	// Init vsg card
	istatus = vsgInit("");
	vsgSetDrawOrigin(0,0);
	vsgSetDrawMode(0);	// default after init is vsgCENTREXY - this turns 
									// it off. 
	vsgSetCommand(vsgVIDEODRIFT);
	vsgSetVideoMode(vsgPANSCROLLMODE);
	
	// Determine best zoom mode
	if (!istatus)
	{
		if (!(V.d%16))
		{
			V.z = 16;
			vsgSetVideoMode(vsgZOOM16);
		}
		else if (!(V.d%8))
		{
			V.z = 8;
			vsgSetVideoMode(vsgZOOM8);
		}
		else if (!(V.d%4))
		{
			V.z = 4;
			vsgSetVideoMode(vsgZOOM4);
		}
		else if (!(V.d%2))
		{
			V.z = 2;
			vsgSetVideoMode(vsgZOOM2);
		}
		else
		{
			V.z=1;
		}

		// Now compute V.h and V.w
		V.w = M.c * V.d;
		V.h = M.r * V.d/V.z;
	}

	// Check that the mseq will fit on the card, and decide whether
	// PANSCROLL mode is required. 
	// Modified 4-26 djs Force PANSCROLL mode. Required -- zoom modes
	// will fail when using MoveWindow cmd. 

	if (!istatus)
	{
		int width = V.d * (2*M.c -1);
		if (width > 2048)
		{
			istatus=1;
			sprintf(error, "Dot size of %d requires video memory at least %d wide.", V.d, width);
		}
	}

	
	if (!istatus)
	{
		// Get Number of video pages in this video mode. The PANSCROLL setting will affect this, so we must
		// do it after PANSCROLL is set (if needed to set) 
		int pages = vsgGetSystemAttribute(vsgNUMVIDEOPAGES);
		int height = (M.p*M.r + M.r - 1)*V.d/V.z;

		// each page is 1024 pixels high regardless of PANSCROLL. 
		if (height > pages*1024)
		{
			istatus = 1;
			sprintf(error, "p=%d, r=%d, c=%d, d=%d, Z=%d requires at least %d lines.", M.p, M.r, M.c, V.d, V.z);
		}
	}

	return istatus;
}


int init(int argc, char *argv[])
{
	int istatus = load_mseq(argc, argv);
	if (!istatus)
	{
		istatus = load_view(argc, argv);
	}
	if (!istatus)
	{
		istatus = init_vsg();
	}

	return istatus;
}




int draw_overlay(bool useOutline)
{
	int istatus=0;
	double aperture_width, aperture_height;
	double aperture_x, aperture_y;
	double screen_width = vsgGetScreenWidthPixels();
	double screen_height = vsgGetScreenHeightPixels();
	short apx, apy;

	aperture_width = V.w;
	aperture_height = V.h*V.z;
	aperture_x = screen_width/2 - aperture_width/2 + V.x;
	aperture_y = screen_height/2 - aperture_height/2 - V.y;
	apx = (short)screen_width/2 - (short)aperture_width/2 + (short)V.x;
	apy = (short)screen_height/2 - (short)aperture_height/2 + (short)V.y;

	printf("apx=%d aperture_x=%f\n", apx, aperture_x);
	printf("apy=%d aperture_y=%f\n", apy, aperture_y);

	// Set OVERLAYMASK mode
	vsgSetCommand(vsgOVERLAYMASKMODE);

	// overlay LUT buffer position 0 is "clear". Use position 1 for background color.
	VSGLUTBUFFER overlayLUT;
	overlayLUT[1].a=overlayLUT[1].b=overlayLUT[1].c=.5;
	overlayLUT[2].a=overlayLUT[2].b=0; overlayLUT[2].c=1;
	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&overlayLUT, 0, 3);

	if (!useOutline)
	{

		// Overlay page 1 will have no aperture. It will serve as a blank page before and after stimulus starts. 
		vsgSetDrawPage(vsgOVERLAYPAGE, 1, 1);
		vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1 );

		vsgSetPen1(0);	// that's clear on the overlay page!
		vsgDrawRect(aperture_x, aperture_y, aperture_width, aperture_height);
		printf("aperture %lf, %lf %lfx%lf\n", aperture_x, aperture_y, aperture_width, aperture_height);

	}
	else
	{

		// Overlay page 1 will have no aperture. It will serve as a blank page before and after stimulus starts. 
		vsgSetDrawPage(vsgOVERLAYPAGE, 1, 1);
		vsgSetDrawPage(vsgOVERLAYPAGE, 0, 0);

		vsgSetDrawMode(vsgPIXELPEN);	// want outline only
		vsgSetPen1(2);
		vsgDrawRect(aperture_x, aperture_y, aperture_width, aperture_height);
		vsgSetDrawMode(vsgSOLIDFILL);

	}
	return istatus;
}


void get_mpositions(int first, int last)
{
	int index;
	double x,y;
	for (index=first; index<last; index++)
	{
		get_window_pos(index, &x, &y);
		MPositions[index].Page=0+vsgDUALPAGE+vsgTRIGGERPAGE ;
		MPositions[index].Xpos=(short)x;
		MPositions[index].Ypos=(short)y;
		MPositions[index].Frames=FramesPerTerm;
		MPositions[index].Stop=0;
		MPositions[index].ovPage=0;
		MPositions[index].ovXpos=0;
		MPositions[index].ovYpos=0;
		if (index%128 == 0) printf("Term %d (%d,%d)\n", index, (short)x, (short)y);
	}
	index=last;
	MPositions[index].Page=0+vsgDUALPAGE;
	MPositions[index].Xpos=0;
	MPositions[index].Ypos=0;
	MPositions[index].Frames=1;
	MPositions[index].ovPage=1;
	MPositions[index].ovXpos=0;
	MPositions[index].ovYpos=0;
	MPositions[index].Stop=1;
}















int main(int argc, char* argv[])
{
	int istatus=0;
	error[0]=0;


	if (argc != 8)
	{
		istatus=1;
		sprintf(error, "Args mseqfile n r c d x y");
	}
	else 
	{
		istatus = init(argc, argv);
		printf("Using dot size %d.\nAperture center at %lf,%lf.\nZOOM=%d, w,h=%lf,%lf\n", V.d, V.x, V.y, V.z, V.w, V.h);
		printf("Screen size %d,%d\n",vsgGetScreenWidthPixels(), vsgGetScreenHeightPixels());
	}


	// If no errors, prepare video memory -- first do overlay
	if (!istatus) 
	{
		char buf[128];
		printf("Overlay? ");
		gets(buf);
		if (!strcmpi(buf, "o"))
		{
			istatus = draw_overlay(true);
		}
		if (!strcmpi(buf, "y"))
		{
			istatus = draw_overlay(false);
		}
		else 
		{
			istatus=0;
		}
	}


	// Now draw the mseq pattern
	if (!istatus) 
	{
		istatus = draw_mseq2();
	}


	if (istatus)
	{
		printf("%s\n", error);
	}
	else
	{
		char buf[128];

		vsgSetZoneDisplayPage(vsgOVERLAYPAGE,0);
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);	

		// Get term, then show it. 
		printf("Enter term or coords: ");
		while (gets(buf))
		{
			double xscreen=0, yscreen=0;
			if (buf[0]=='c')
			{
				sscanf(buf+2, "%lf %lf", &xscreen, &yscreen);
				printf("screen pos (%lf,%lf)\n", xscreen, yscreen);
				vsgMoveScreen(xscreen, yscreen);
			}
			else if (isdigit(buf[0]))
			{
				int iterm = atoi(buf);
				if (iterm >=0 && iterm <=M.nterms)
				{
					// Where to move window? 
					// It appears that the zoom mode switches off when the origin for the 
					// move screen is outside the valid video coords?
					get_term_pos(iterm, &xscreen, &yscreen);
					printf("Term pos for term %d is (%lf,%lf)\n", iterm, xscreen, yscreen);
					vsgMoveScreen(xscreen, yscreen);
					printf("Hit enter to position window for this term");
					gets(buf);
					get_window_pos(iterm, &xscreen, &yscreen);
					printf("screen pos for term %d is (%lf,%lf)\n", iterm, xscreen, yscreen);
					vsgMoveScreen(xscreen, yscreen);
				}
			}
			else if (buf[0]=='t')
			{
				// read first-last term
				int first, last;
				sscanf(buf+1, " %d %d", &first, &last);
				printf("First,last=%d,%d\n",first,last);
				
				get_mpositions(first, last);

				// Set up page cycling
				vsgSetCommand(vsgVIDEODRIFT);
				vsgPageCyclingSetup(last-first+1,&MPositions[first]);
				vsgSetCommand(vsgCYCLEPAGEENABLE);

				printf("Hit enter to stop...");
				gets(buf);
				vsgSetCommand(vsgCYCLEPAGEDISABLE);
			}
			else break;

			printf("Enter term or coords: ");
		} 		
	}

	return istatus;
}





#if 0

void get_term_pos(int iterm, double *pxterm, double *pyterm)
{
	// The ULHC of each term lies somewhere in the first column of rxc terms. 
	// Those terms are drawn in a column on the LHS of video memory -- p terms 
	// in the column. Call each of these terms a "block" and determine which 
	// block the starting pos of 'iterm' is.....
	int block = iterm%M.p;

	// This tells us the position (0-rc) of the terms start point within the 
	// block specified by 'block'. The positions are numbered 0-c across the 
	// first row, and so on. The lower right corner is position 'rc'.
	int block_pos = (iterm-block)/M.p;
	int block_row = block_pos/M.r;
	int block_col = block_pos%M.c;

	*pxterm = x_offset + block_col * V.d;
	*pyterm = y_offset + (block*M.r + block_row)*V.d/V.z;
}

void get_dot_pos_relative(int icol, int irow, double x, double y, double *pxdot, double *pydot)
{
//	*pxdot = x + icol*V.d;
//	*pydot = y + irow*V.d/V.z;
	*pxdot = x + (icol+0.5)*V.d;
	*pydot = y + (irow+0.5)*V.d/V.z;
}

void get_dot_pos(int iterm, int icol, int irow, double *pxdot, double *pydot)
{
	double xterm, yterm;
	get_term_pos(iterm, &xterm, &yterm);
	get_dot_pos_relative(icol, irow, xterm, yterm, pxdot, pydot);
}

void get_window_pos(int iterm, double *pxscreen, double *pyscreen)
{
	double pxterm, pyterm;
	get_term_pos(iterm, &pxterm, &pyterm);
	*pxscreen = pxterm - vsgGetScreenWidthPixels()/2 - V.x + V.w/2;
	*pyscreen = pyterm - vsgGetScreenHeightPixels()/(2*V.z) + V.y/V.z +V.h/2;
}



void draw_term_dot_at(int iterm, int irow, int icol, double x, double y)
{
	// Determine if this dot is "on" or "off". 
	int D = (M.p*icol + M.p*M.c*irow);
	int iterm_offset = (iterm + D) % M.nterms;
	if (M.S[iterm_offset]=='1')
	{
		double xdot, ydot;
		get_dot_pos_relative(icol, irow, x, y, &xdot, &ydot);
		vsgSetPen1(DOT_INDEX);
		vsgDrawRect(xdot, ydot, V.d, V.d/V.z);
		if (iterm==0) printf("Draw term %d (%d,%d),(%lf,%lf)\n",iterm, irow, icol, xdot, ydot);
		// DRAW TEXT
		if (dbg_text && iterm<256) 
		{
			vsgSetStringMode(0, 12, vsgALIGNRIGHTTEXT, vsgALIGNTOPTEXT, 0, vsgFONTNORMAL);
			vsgSetPen1(TEXT_INDEX);
			char cstr[12];
			sprintf(cstr, "%d", iterm);
			vsgDrawString(xdot, ydot, cstr);
		}
	}
}


void draw_term_column_at(int iterm, int icol, double x, double y)
{
	for (int i=0; i<M.r; i++)
		draw_term_dot_at(iterm, i, icol, x, y);
}


void draw_term_row_at(int iterm, int irow, double x, double y)
{
	for (int j=0; j<M.c; j++)
		draw_term_dot_at(iterm, irow, j, x, y);
}

void draw_term_at(int iterm, double x, double y)
{
	for (int i=0; i<M.r; i++)
		draw_term_row_at(iterm, i, x, y);
}

void draw_term(int iterm)
{
	// figure out where the term should be drawn
	double x, y;
	get_term_pos(iterm, &x, &y);

	// Now draw the term 
	draw_term_at(iterm, x, y);
}


// draw_mseq - draws the msequence pattern into memory. 
//
// The video memory is cleared and we set pen1 to 255 --
// assumed to be the end of a palette ramp (i.e. white)
// We draw a term with a white rect if its "on" and we don't
// bother if its "off". The end result is a pattern of 
// "on" and "off" dots. 
// 
// The M sequence and associated parameters are stored in 
// the global struct MSEQ M. The members of this struct
// tell us the m-seq itself (the actual terms) as well as 
// the paremeters n, r(rows), c(columns), etc used to 
// determine the drawing pattern for the stimulus. 
//
// The drawing details are contained in the global struct 
// VIEW V. The members of V include z (hw zoom factor), d 
// (dot width in pixels), x,y (center of grid on screen, in
// pixels relative to center of screen -- with y up!), w,h
// ( the width and height of each rxc term in video memory. 
// Thus if a hw zoom mode is in use, the height h = M.r*V.d/V.z. 
// This is important in navigating video memory space to position
// the video window properly. 
//
// Notation: The m-sequence itself is a single sequence of 1/0's. 
// A "term" in the m-seq is a single value from that large sequence. 
// A term's value is 0 or 1. The "stimulus" will consist of a series
// of video frames, where each frame has a "grid" which is "r" rows
// by "c" columns. 
//
// This routine will use a technique described in the "m-sequence paper"
// to draw a pattern of dots into video memory. Then by judiscious
// use of mask and video window moves we can show the stimulus' frames
// as described in the paper. 
//
// The algorithm works thusly:
//
// Each grid in the sequence has as its upper-right-hand-corner
// term the value of that term from the original m-sequence. The "paper" 
// gives an offset formula to place the other dots in the grid, all based
// on an offset from that upper-left-hand-corner term. A column of these
// grids is drawn along the left hand side of video memory (for n=15 and 
// r=c=16 we draw 128 of them down this column). Note that the grids are
// drawn taking into account the video zoom. Thus each "dot" is drawn 
// to be V.d pixels wide, but V.d/V.z pixels tall. 
// After the first grid column is drawn, we iterate through all the remaining
// terms of the m sequence (128-32767), but for each we only need to draw the
// right-most column (this is because of the way the grids are overlaid -- see
// Table 2 in the "paper". After these are done, we only have to draw a 
// strip of dots at the bottom and the right side. 
// 
// AAAAARGH!!!!
// The vsg has the following quirk. 
// If you are NOT in PANSCROLL mode, and you are using a ZOOM factor, AND you try
// to move the window (vsgMoveWindow) using coordinates that are negative (i.e. not 
// on the physical video memory area), the results are bad. The zoom factor appears
// to turn off, or there is a partial overlay of the resulting image on the screen.
// The same thing in PANSCROLL mode, however, has no problems. 
// There are two possible solutions. 
// 1) Draw the m seq offset from (0,0) in video memory, by the same amount 
//    required to move the window. That way the first term will require a MoveWindow(0,0), 
//    and all subsequent terms will use coords within the physical confines of the window. 
// 2) Use PANSCROLL mode for all m sequences. This might not be the best way to go -- I 
//    think it will force changes on the algorithm used to draw the mseq in memory. 
//    Update - wrong-amundo #2. The limitation is only with 8Mb cards - they will have
//    just dot sizes 2,4,8,16 available to them. 

int draw_mseq(void)
{
	int istatus=0;
	int i,j;

// TEST TEST TEST TEST
	x_offset = 0; // vsgGetScreenWidthPixels()/2 + V.x - V.w/2;
	y_offset = 0; // vsgGetScreenHeightPixels()/(2*V.z) + V.y/V.z +V.h/2;
//TEST TEST TEST TEST


	// Set level 1 to red for text
	VSGLUTBUFFER buf;
	buf[1].a=1; buf[1].b=buf[1].c=0;
	vsgPaletteWrite((VSGLUTBUFFER*)buf, TEXT_INDEX, 1);

	vsgSetDrawPage(vsgVIDEOPAGE,0,0);
	vsgSetPen1(255);
	vsgSetPen2(0);
	vsgSetCommand(vsgVIDEOCLEAR);


	// Draw one column of terms 0 through p-1

//	char b[10];
//	printf("first column 0-%d ", M.p);
//	gets(b);

	for (i=0; i<M.p; i++)
		draw_term(i);

	// Draw last column of terms across the page

//	printf("columns for terms %d-%d ", M.p, M.nterms);
//	gets(b);

//	for (i=M.p; i<M.nterms; i++)
	for (i=M.p; i<M.p*M.c; i++)
	{
		double x, y;
		get_term_pos(i, &x, &y);
//		if (i%128 == 0) printf("%d %lf %lf\n", i, x, y);
		draw_term_column_at(i, M.c-1, x, y);
	}

	// Do the bottom rows. A little overlap here....
	for (i=M.p*M.c*(M.r-1)+(M.r-1); i<M.nterms; i+=M.r)
	{
		double x, y;
//		printf("Term %d\n", i);
		get_term_pos(i, &x, &y);
		for (j=1; j<4; j++)
		{
			draw_term_row_at(i, j, x, y);
		}
	}

	return istatus;
}




#endif