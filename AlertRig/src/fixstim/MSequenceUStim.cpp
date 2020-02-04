#include "MSequenceUStim.h"

#include <iostream>
using namespace std;

const string MSequenceUStim::m_allowedArgs("t:r:c:o:vp:d:am:T:R:K");

VSGCYCLEPAGEENTRY *f_mpos = NULL;

MSequenceUStim::MSequenceUStim()
	: m_bTesting(false)
	, m_binaryTriggers(true)
	, m_nRepeats(1)
	, m_iapX(0), m_iapY(0)
	, m_iapXCorner(0), m_iapYCorner(0)
	, m_iOrder(15)
	, m_iDot(4)
	, m_iRows(16), m_iCols(16)
	, m_iFramesPerTerm(1)
	, m_sequence(NULL)
	, m_nterms(0)
	, m_errflg(0)
	, m_pulse(0x2)
	, m_iFirst(0)
	, m_iLast(-1)
{

}

MSequenceUStim::~MSequenceUStim()
{

}

void MSequenceUStim::run_stim(alert::ARvsg& vsg)
{

	// initialize triggers
	TSpecificFunctor<MSequenceUStim> functor(this, &MSequenceUStim::callback);
	init_triggers(&functor);

	// setup vsg
	vsgSetDrawOrigin(0, 0);
	vsgSetDrawMode(vsgSOLIDFILL);			// default is vsgCENTREXY! This makes it top-left-hand-corner origin
	vsgSetCommand(vsgPALETTERAMP);
	vsgSetPen2(0);
	vsgSetCommand(vsgVIDEOCLEAR);
	//vsgSetCommand(vsgDISABLELUTANIM);

	// specify pixel coords
	vsgSetSpatialUnits(vsgPIXELUNIT);

	// aperture location will be in f_iapXCorner, f_iapYCorner
	// I assume that the position given was in pixels (and hence is in integers). 
	m_iapXCorner = vsgGetScreenWidthPixels() / 2 + m_iapX - (m_iCols * m_iDot) / 2;
	m_iapYCorner = vsgGetScreenHeightPixels() / 2 - m_iapY - (m_iRows * m_iDot) / 2;
	if (m_verbose)
	{
		cout << "aperture center (" << m_iapX << ", " << m_iapY << ")" << endl;
		cout << "aperture corner (" << m_iapXCorner << ", " << m_iapYCorner << ")" << endl;
	}

	// Now draw pages.....
	prepare_overlay();
	blank_page();

	// draw the msequence into videomemory
	if (draw_mseq(get_configured_seq(), m_iOrder, m_iRows, m_iCols, m_iDot))
	{
		// error drawing msequence. Exit now.
		return;
	}

	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);


	if (m_bTesting)
	{
		testing_loop();
	}
	else
	{
		prepare_cycling();
		vsg.ready_pulse(100, m_pulse);

		// reset all triggers if using binary triggers
		if (m_binaryTriggers) triggers().reset(vsgIOReadDigitalIn());

		// All right, start monitoring triggers........
		int last_output_trigger = 0;
		long input_trigger = 0;
		string s;

		while (!quit_enabled())
		{
			// If user-triggered, get a trigger entry. 
			if (!m_binaryTriggers)
			{
				// Get a new "trigger" from user
				cout << "Enter trigger/key: ";
				cin >> s;
			}
			else
			{
				input_trigger = vsgIOReadDigitalIn();
			}

			TriggerFunc	tf = std::for_each(triggers().begin(), triggers().end(),
				(m_binaryTriggers ? TriggerFunc(input_trigger, last_output_trigger, false) : TriggerFunc(s, last_output_trigger)));

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
	vsg.clear();

	if (f_mpos) free(f_mpos);	// cleanup
	//vsgSetCommand(vsgOVERLAYDISABLE);
	//vsgSetDrawMode(vsgSOLIDFILL + vsgCENTREXY);
	//vsgSetDrawOrigin(vsgGetScreenWidthPixels()/2, vsgGetScreenHeightPixels()/2);
	//vsgSetSpatialUnits(vsgDEGREEUNIT);
	//vsgSetCommand(vsgVIDEOCLEAR);

	return;






}

void MSequenceUStim::blank_page()
{
	vsgSetCommand(vsgCYCLEPAGEDISABLE);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE, NO_APERTURE_PAGE);
	return;
}

void MSequenceUStim::prepare_overlay()
{
	// prepare overlay
	VSGTRIVAL ovcolor;
	ovcolor.a = ovcolor.b = ovcolor.c = 0.5;
	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&ovcolor, 1, 1);
	vsgSetCommand(vsgOVERLAYMASKMODE);
	vsgSetDrawPage(vsgOVERLAYPAGE, BIG_APERTURE_PAGE, 0);
	vsgSetDrawPage(vsgOVERLAYPAGE, NO_APERTURE_PAGE, 1);
	vsgSetDrawPage(vsgOVERLAYPAGE, APERTURE_PAGE, 1);
	vsgSetPen1(0);	// that's clear on the overlay page!
	vsgDrawRect(m_iapXCorner, m_iapYCorner, m_iDot*m_iCols, m_iDot*m_iRows);
	cerr << "prepare_overlay: " << m_iapXCorner << ", " << m_iapYCorner << ", " << m_iDot * m_iCols << ", " << m_iDot * m_iRows << endl;
}


const char *MSequenceUStim::get_configured_seq()
{
	if (m_sequence) return m_sequence;
	else return get_msequence();
}

void MSequenceUStim::init_triggers(TSpecificFunctor<MSequenceUStim>* pfunctor)
{
	triggers().clear();
	triggers().addTrigger(new FunctorCallbackTrigger("F", 0x2, 0x2, 0x2, 0x2, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("f", 0x2, 0x0, 0x2, 0x0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("S", 0x4, 0x4, 0x4, 0x4, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("s", 0x4, 0x0, 0x4, 0x0, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("a", 0x8, 0x8 | AR_TRIGGER_TOGGLE, 0x8, 0x8 | AR_TRIGGER_TOGGLE, pfunctor));
	triggers().addTrigger(new FunctorCallbackTrigger("X", 0x6, 0x0, 0x6, 0x0, pfunctor));
	triggers().addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));

	// Put quit trigger before cycling trigger
	triggers().addTrigger(m_ptrigCycling = new PageCyclingTrigger("C", m_nRepeats));

	return;
}

int MSequenceUStim::callback(int &output, const FunctorCallbackTrigger* ptrig)
{
	int ival = 1;
	string key = ptrig->getKey();

	if (key == "S")
	{
		vsgSetCommand(vsgVIDEODRIFT + vsgOVERLAYDRIFT);			// allows us to move the offset of video memory
		vsgSetCommand(vsgCYCLEPAGEENABLE);
		m_ptrigCycling->started();
	}
	else if (key == "X")
	{
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		m_ptrigCycling->stopped();
		vsgSetZoneDisplayPage(vsgOVERLAYPAGE, NO_APERTURE_PAGE);
	}

	return ival;
}


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


int MSequenceUStim::draw_mseq(const char *seq, int order, int r, int c, int d)
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
	p = (int)pow(2.0f, (float)order) / (r*c);
	N = vsgGetSystemAttribute(vsgNUMVIDEOPAGES);
	w_v = vsgGetSystemAttribute(vsgPAGEWIDTH);
	h_v = vsgGetSystemAttribute(vsgPAGEHEIGHT);
	w_s = vsgGetScreenWidthPixels();
	h_s = vsgGetScreenHeightPixels();

	if (m_verbose)
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
	if ((2 * c - 1)*d > w_v)
	{
		cout << "ERROR: The mseq will not fit in video memory." << endl;
		cout << "       Total page width required is (2c-1)*d = " << (2 * c - 1)*d << endl;
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
	arutil_color_to_palette(COLOR_TYPE(black), 1);
	arutil_color_to_palette(COLOR_TYPE(white), 2);
//	vsgSetCommand(vsgPALETTERAMP);
	vsgSetPen1(2);	// "ON" is white
	vsgSetPen2(1);		// "OFF" is black
	vsgSetCommand(vsgVIDEOCLEAR);	// clear all video pages to pen2 (black)

	// start drawing. 
	for (j = 0; j < (p + 1); j++)
	{
		// what page does this term start on?
		pg = (j*r*d) / h_v;
		vsgSetDrawPage(vsgVIDEOPAGE, pg, vsgNOCLEAR);

		// offset from top of page?
		offset = (j * r * d) % h_v;

		// draw dots
		for (x = 0; x < (2 * c - 1); x++)
		{
			for (y = 0; y < r; y++)
			{
				ind = (j + p * x + p * c*y) % M;
				if (seq[ind] == '1')
				{
					xrect = x * d;
					yrect = offset + y * d;
					vsgDrawRect(xrect, yrect, d, d);
				}
			}
		}
	}

	return 0;
}

void MSequenceUStim::prepare_cycling()
{
	int iterm;
	int irow, icol;
	int xterm, yterm;
	int p;
	int h;
	int w;
	int M;

	p = (int)pow(2.0f, (float)m_iOrder) / (m_iRows*m_iCols);
	w = vsgGetScreenWidthPixels();
	h = vsgGetScreenHeightPixels();
	M = (int)pow(2.0f, (float)m_iOrder) - 1;

	// Allocate page cycle structure
	f_mpos = (VSGCYCLEPAGEENTRY *)calloc(m_iLast-m_iFirst+1 + 1, sizeof(VSGCYCLEPAGEENTRY));
	memset(f_mpos, 0, sizeof(VSGCYCLEPAGEENTRY)*(m_iLast - m_iFirst + 1));


	for (iterm = m_iFirst; iterm < m_iLast; iterm++)
	{
		irow = (iterm % p)*m_iRows + iterm / (p*m_iRows);
		icol = (iterm / p) % m_iCols;
		//		xterm = (icol * f_iDot) - (w/2 - (f_iCols*f_iDot)/2);
		//		yterm = (irow * f_iDot) - (h/2 - (f_iRows*f_iDot)/2);
		xterm = (icol * m_iDot) - m_iapXCorner;
		yterm = (irow * m_iDot) - m_iapYCorner;
		f_mpos[iterm].Frames = m_iFramesPerTerm;
		f_mpos[iterm].ovPage = APERTURE_PAGE;
		f_mpos[iterm].ovXpos = 0;
		f_mpos[iterm].ovYpos = 0;
		f_mpos[iterm].Page = 0 + vsgDUALPAGE + vsgTRIGGERPAGE;
		f_mpos[iterm].Stop = 0;
		f_mpos[iterm].Xpos = xterm;
		f_mpos[iterm].Ypos = yterm;

		if (m_verbose)
		{
			if (iterm % p == 0)
			{
			}
		}
	}
	f_mpos[m_iLast].Page = 0 + vsgDUALPAGE + vsgTRIGGERPAGE;
	f_mpos[m_iLast].ovPage = NO_APERTURE_PAGE;
	f_mpos[m_iLast].Stop = 1;

	cerr << "Max cycling pages " << vsgGetSystemAttribute(vsgPAGECYCLEARRAYSIZE) << endl;
	vsgPageCyclingSetup(m_iLast-m_iFirst+1, f_mpos);

}


void MSequenceUStim::testing_loop()
{
	// testing loop...
	std::string s;
	int ipg;
	bool bQuit = false;
	while (!bQuit)
	{
		// Get a new "trigger" from user
		cout << "Enter key [qQabpn0-9]: ";
		cin >> s;

		switch (s[0])
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
		case 'b':
		{
			// OK activate the aperture page
			vsgSetZoneDisplayPage(vsgOVERLAYPAGE, BIG_APERTURE_PAGE);
			break;
		}
		case 'p':
		{
			cout << "Enter page number: ";
			cin >> ipg;
			vsgSetZoneDisplayPage(vsgVIDEOPAGE, ipg);
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

			p = (int)pow(2.0f, (float)m_iOrder) / (m_iRows*m_iCols);
			w = vsgGetScreenWidthPixels();
			h = vsgGetScreenHeightPixels();

			// compute corner position for this term. 
			irow = (iterm % p)*m_iRows + iterm / (p*m_iRows);
			icol = (iterm / p) % m_iCols;
			xterm = (icol * m_iDot) - (w / 2 - (m_iCols*m_iDot) / 2);
			yterm = (irow * m_iDot) - (h / 2 - (m_iRows*m_iDot) / 2);
			if (m_verbose)
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
			vsgSetCommand(vsgVIDEODRIFT + vsgOVERLAYDRIFT);			// allows us to move the offset of video memory
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

bool MSequenceUStim::parse(int argc, char **argv)
{
	bool b = false;
	int status;
	status = prargs(argc, argv, (process_args_func)NULL, m_allowedArgs.c_str(), 'F', this);
	if (!status) b = true;
	return b;
}

int MSequenceUStim::process_arg(int option, std::string& arg)
{
	static bool have_o = false;	// order
	static bool have_r = false;	// rows
	static bool have_c = false;	// columns
	static bool have_xy = false;	// center xy pos of grid
	static bool have_m = false;	// msequence filename
	static bool have_t = false;	// frames per term
	static bool have_d = false; // dot size in pixels
	static bool have_T = false;	// subsequence of terms

	switch (option)
	{
	case 'K':
		m_bTesting = true;
		break;
	case 'a':
		m_binaryTriggers = false;
		break;
	case 'R':
		if (parse_integer(arg, m_nRepeats))
		{
			cerr << "Cannot parse repeats (" << arg << "): must be an integer." << endl;
			m_errflg++;
		}
		break;
	case 'p':
		if (parse_int_pair(arg, m_iapX, m_iapY))
		{
			cerr << "Cannot parse pixel position (" << arg << "): expecting integer pixel positions x,y, no spaces." << endl;
			m_errflg++;
		}
		else
		{
			have_xy = true;
		}
		break;
	case 'm':
		m_sFilename.assign(arg);
		have_m = true;
		break;
	case 'o':
		if (parse_integer(arg, m_iOrder))
		{
			cerr << "Cannot parse order (" << arg << "): must be an integer." << endl;
			m_errflg++;
		}
		else
		{
			have_o = true;
		}
		break;
	case 'd':
		if (parse_integer(arg, m_iDot))
		{
			cerr << "Cannot parse dot size (" << arg << "): must be an integer." << endl;
			m_errflg++;
		}
		else
		{
			have_d = true;
		}
		break;
	case 'r':
		if (parse_integer(arg, m_iRows))
		{
			cerr << "Cannot parse rows (" << arg << "): must be an integer." << endl;
			m_errflg++;
		}
		else
		{
			have_r = true;
		}
		break;
	case 'c':
		if (parse_integer(arg, m_iCols))
		{
			cerr << "Cannot parse columns (" << arg << "): must be an integer." << endl;
			m_errflg++;
		}
		else
		{
			have_c = true;
		}
		break;
	case 'v':
		m_verbose = true;
		break;
	case 't':
		if (parse_integer(arg, m_iFramesPerTerm))
		{
			cerr << "Cannot parse frames per term (" << arg << "): must be an integer." << endl;
			m_errflg++;
		}
		else
		{
			have_t = true;
		}
		break;
	case 'T':
		{
			// parse a pair of ints first,stop
			// Terms numbered 0, 1, 2, 3, .... 
			// term 'first' is the first term shown
			// term 'stop' is NOT shown
			// To show 100 terms with term n as the first term, use args
			// "n,n+100" - e.g. "0,100" plays the first 100 terms, and
			// "1049,1149" plays 100 terms, starting at 1049. 
			if (parse_sequence_pair(arg, m_iFirst, m_iLast))
			{
				cerr << "Cannot parse subsequence pair (" << arg << ")." << endl;
				m_errflg++;
			}
			else
			{
				have_T = true;
			}
			break;
		}
	case 'h':
		m_errflg++;
		break;
	case '?':
		m_errflg++;
		break;
	case 0:

		// If a seq file was specified, then you must also specify order. 
		if (have_m)
		{
			if (!have_o)
			{
				cerr << "When specifying a sequence file, the sequence order (-o) must be specified!" << endl;
				m_errflg++;
			}
			else
			{
				if (load_mseq(m_sFilename))
				{
					m_errflg++;
					cerr << "Error loading mseq file " << m_sFilename << endl;
				}
			}
		}
		else
		{
			m_nterms = (int)pow(2.0f, m_iOrder) - 1;
		}

		// When no subsequence specified, then play the entire sequence. 
		if (!have_T)
		{
			m_iFirst = 0;
			m_iLast = m_nterms;
		}

		if (!have_r || !have_c)
		{
			cerr << "Both rows (-r) and columns (-c) must be specified!" << endl;
			m_errflg++;
		}
		if (!have_t)
		{
			cerr << "No frames_per_term value (-t) specified!" << endl;
			m_errflg++;
		}
		if (!have_xy)
		{
			cerr << "No position (-p) specified!" << endl;
			m_errflg++;
		}
		if (!have_d)
		{
			cerr << "No dot size (-d) specified!" << endl;
			m_errflg++;
		}


		break;
	default:
		cerr << "Unknown argument (" << option << ")" << endl;
		m_errflg++;
		break;
	}
	return m_errflg;
}

int MSequenceUStim::load_mseq(string& filename)
{
	FILE *fp = NULL;
	int istatus = 0;
	m_nterms = (int)pow(2.0f, m_iOrder) - 1;

	// Open mseq file
	if (fopen_s(&fp, filename.c_str(), "r"))
	{
		istatus = 1;
		cerr << "Cannot open sequence file " << filename << endl;
	}
	else
	{
		m_sequence = (char *)malloc(m_nterms + 1);
		memset(m_sequence, 0, m_nterms + 1);
		if (!fread(m_sequence, sizeof(char), m_nterms, fp))
		{
			istatus = 2;
			cerr << "Expected " << m_nterms << " terms in seq. Check mseq file " << filename << endl;
		}
		else if ((int)strlen(m_sequence) != m_nterms)
		{
			istatus = 3;
			cerr << "Expected " << m_nterms << " terms in seq. Found " << strlen(m_sequence) << ". Check mseq file." << endl;
		}
		else
			cerr << "Loaded " << m_nterms << " from seq file " << filename << endl;

		fclose(fp);
	}

	return istatus;
}




