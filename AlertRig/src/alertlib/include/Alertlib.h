#if !defined(_ALERTLIB_H_)
#define _ALERTLIB_H_

#pragma warning(disable:4786)

//#include "VSGEX2.H"
// this macro will stop winsock from being included automatically.
#define WIN32_LEAN_AND_MEAN
#include "VSGV8.H"
#include "ARtypes.h"
#include "ARvsg.h"
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__
#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <ostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>
#include <boost/algorithm/string.hpp>


const char* get_msequence();


/*
 * prargs
 * 
 * Process command line args. Automatic handling of response files. 
 * Caller must pass a callback function pfunc(int c, string arg), which 
 * is called for each option/arg found. The callback function must
 * do whatever processing of the option (and arg if relevant) is necessary.
 * The callback should return 0 if all is well, nonzero return will halt
 * command line processing and prargs will return that value. 
 * On successful handling of all args prargs will return 0. 
 * 
 * options is the same gnu getopt style of options, colon (:) means option has an arg. 
 * Response_file_char is the char in the options that represents the response file. 
 * Pass 0 (the default) for no response file option. When you pass a response file 
 * arg, the contents of the response file are inserted in to the argv passed at the 
 * point where the response file is found. The response file option and arg are not 
 * used when the callback function is called for each arg found. 
 *
 * The callback is called once after all args have been processed. The callback
 * should look for the option c==0 to handle post-arg processing. Can return nonzero here
 * on errors. 
 */

typedef int (*process_args_func)(int option, std::string& arg);

// Alternate, for when you want the callback to be in a class. 
class prargs_handler
{
public:
	virtual int process_arg(int option, std::string& arg) = 0;
};



int prargs(int argc, char **argv, process_args_func pfunc, const char *options, int response_file_char=0, prargs_handler* handler = 0);

int tokenize_response_file(char *filename, std::vector<std::string> &tokens);








/* Initialize vsg card. Leaves card with palette ramp, page 0 cleared to background color (use vsgBACKGROUND). 
 * Clears video page 0 and displays it. Suitable for initialization on startup of a stimulus app, 
 * or for cleanup on exit (or use clear_vsg()). If use_overlay is true, sets OVERLAYMASKMODE, initializes
 * the overlay palette to use bg color on level 1 (level 0 in overlay palette is reserved for "transparent".
 */
int init_vsg(int screenDistanceMM, COLOR_TYPE i_background, bool use_overlay);


/* Convenience */
void clear_vsg();

namespace alert
{

	// Base class that encapsulates VSG objects. All spec classes should inherit from this. 
	// djs 3-23-10 Add init special case. If using numlevels = vsgFIXATION, that particular level is 
	// used for this object. This special level can be used in conjunction with the object animation
	// system. 
	class ARObject
	{
	public:
		ARObject();
		ARObject(const ARObject& obj);
		virtual ~ARObject();
		void init(PIXEL_LEVEL first, int numlevels, bool bcreate=true);
		void init(ARvsg& vsg, PIXEL_LEVEL first, int numlevels, bool bcreate = true);
		void init(int numlevels, bool bcreate = true);
		void init(ARvsg& vsg, int numlevels, bool bcreate = true);
		// copy VSGObject stuff only - handle, levels, etc. 
		// Use this when one obj is already init'd, but you want
		// to draw another. 
		void init(const ARObject& obj);
		void destroy();
		int select();
		virtual void setContrast(int contrast);
		VSGOBJHANDLE handle() const;
		bool initialized() const;
		PIXEL_LEVEL getFirstLevel() const;
		int getNumLevels() const;
		void setDrawGroups(int iDrawGroups) { m_iDrawGroups = iDrawGroups; };
		bool inDrawGroup(int iGroup) const { return (bool)(m_iDrawGroups < 1 || (m_iDrawGroups & (1 << (iGroup - 1)))); };
		bool getUseMaster() const { return m_use_master; };
		bool getUseSlave() const { return m_use_slave; };
		ARObject& operator=(const ARObject& obj);
	protected:
		ARvsg& getVSG();
	private:
		bool m_initialized;
		VSGOBJHANDLE m_handle;
		PIXEL_LEVEL m_first;
		int m_numlevels;
		int m_iDrawGroups;
		bool m_use_master;
		bool m_use_slave;
	};

	// Base class for specs. Objects drawn using overlay pages use AROverlaySpec

	class ARSpec : public ARObject
	{
	public:
		ARSpec() {};
		ARSpec(const ARSpec& s) : ARObject(s) {};
		virtual ~ARSpec() {};
		
		/* Draw object on currently selected page. */
		virtual int draw() = 0;

		/* Draw object on current page. Assume the page is an overlay page, draw an aperture for the 
		 * object on level 0. 
		 */
		virtual int drawOverlay() = 0;
	};

	// Rectangle Spec
	class ARRectangleSpec: public ARSpec
	{
	public:
		ARRectangleSpec(DWORD mode=0, DWORD line_width=1): ARSpec(), x(0), y(0), orientation(0), drawmode(mode), linewidth(line_width) {};
		virtual ~ARRectangleSpec() {};
		double x, y;	// Center
		double w, h;	// width, height. Units depend on how vsg is initialized
		double orientation;
		COLOR_TYPE color;
		DWORD drawmode;
		DWORD linewidth;
		virtual int draw();
		virtual int drawOverlay();
	};

	class ARContrastRectangleSpec: public ARRectangleSpec
	{
	public:
		ARContrastRectangleSpec(DWORD mode=0, DWORD line_width = 1): ARRectangleSpec(mode, line_width) {};
		~ARContrastRectangleSpec() {};
		int draw();
		int drawOverlay();
	};

	// Rectangle Spec
	class ARMultiContrastRectangleSpec: public ARSpec, public std::vector<XYWH>
	{
	public:
		COLOR_TYPE color;
		ARMultiContrastRectangleSpec() {};
		~ARMultiContrastRectangleSpec() {};
		virtual int draw();
		virtual int drawOverlay();
	};

	// Xhair Spec
	class ARXhairSpec: public ARSpec
	{
	public:
		ARXhairSpec() {};
		~ARXhairSpec() {};
		double x, y;	// Center
		double ri, rm, ro;	// inner, middle, outer radii
		int nc;				// number of divisions
		double r1, r2;		// inner and outer radii for tick lines
		virtual int draw();
		virtual int drawOverlay();
	private:
		int drawPie(int nc, PIXEL_LEVEL first, PIXEL_LEVEL second, double x, double y, double r);
	};

	// Conte stim
	class ARConteSpec : public ARSpec
	{
	private:
		PIXEL_LEVEL m_cue_level;
		PIXEL_LEVEL m_ramp_low;		// black
		PIXEL_LEVEL m_ramp_mid;		// s/b gray
		PIXEL_LEVEL m_ramp_high;	// white
	public:
		ARConteSpec() {};
		virtual ~ARConteSpec() {};
		double x, y, w, h;
		double orientation;
		double sf;		// spatial frequency
		double dev;		// gaussian e**(r**2/dev**2)
		double phase;	// initial phase
		bool bHorizontal;
		double distractor_factor;
		DWORD cue_line_width;
		COLOR_TYPE cue_color;

		// call this instead of init()
		void do_init(int nlevels);

		virtual int draw();
		virtual int drawOverlay() { return 0; };	// no-op do not use
	};


	// random grid Spec
	class ARRandomGridSpec: public ARSpec
	{
	public:
		ARRandomGridSpec() {};
		~ARRandomGridSpec() {};
		double x, y;	// Center
		double w, h;	// width, height
		int nr, nc;		// number of rows, columns
		double tf;
		virtual int draw();
		virtual int drawOverlay();
	};

	// alternating chessboard spec
	class ARChessboardSpec: public ARSpec
	{
	public:
		ARChessboardSpec(): nr(8), nc(8), tf(1.0) {};
		~ARChessboardSpec() {};
		double x, y;	// Center
		double w, h;	// width, height
		int nr, nc;		// number of rows, columns
		double tf;
		virtual int draw();
		virtual int drawOverlay();
	};


	// Fixation Point Spec

	class ARContrastFixationPointSpec;
	class ARFixationPointSpec: public ARSpec
	{
	public:
		ARFixationPointSpec();
		ARFixationPointSpec(const ARFixationPointSpec& fixpt);
		ARFixationPointSpec(const ARContrastFixationPointSpec& fixpt);
		~ARFixationPointSpec() {};
		ARFixationPointSpec& operator=(const ARFixationPointSpec& fixpt);
		double x, y;
		double d;
		COLOR_TYPE color;
		virtual int draw();
		virtual int drawOverlay();
	};

	// Fixation point for cases where its visibility is controlled by contrast setting 

	class ARContrastFixationPointSpec: public ARFixationPointSpec
	{
	public:
		ARContrastFixationPointSpec(): ARFixationPointSpec() {};
		ARContrastFixationPointSpec(const ARFixationPointSpec& fixpt) : ARFixationPointSpec(fixpt) {};
		ARContrastFixationPointSpec(const ARContrastFixationPointSpec& fixpt) : ARFixationPointSpec(fixpt) {};
		~ARContrastFixationPointSpec() {};
		ARContrastFixationPointSpec& operator=(const ARContrastFixationPointSpec& fixpt);
		ARContrastFixationPointSpec& operator=(const ARFixationPointSpec& fixpt);
		int draw();
		int drawOverlay();
	};

	// dots. Do not use contrast, but can have multi-dot. Each dot costs one level. 
	class ARDotSpec : public ARFixationPointSpec
	{
		bool bIsMulti;
		std::vector<ARFixationPointSpec> m_multi;

	public:
		ARDotSpec() : ARFixationPointSpec(), bIsMulti(false) {};
		ARDotSpec(const ARDotSpec& dot) : ARFixationPointSpec(dot), bIsMulti(dot.bIsMulti), m_multi(dot.m_multi) {};
		ARDotSpec(const ARFixationPointSpec& fixpt) : ARFixationPointSpec(fixpt), bIsMulti(false) {};
		~ARDotSpec() {};
		ARDotSpec& operator=(const ARFixationPointSpec& fixpt);
		int draw();
		int drawOverlay();

		void setMulti(const std::vector<ARFixationPointSpec>& vec) { bIsMulti = true; m_multi = vec; };
		void setMulti() { bIsMulti = false; m_multi.clear(); };
		std::vector<ARFixationPointSpec>& getMulti() { return m_multi; };
		bool isMulti() { return bIsMulti; };
	};

	// Circle, single pixel wide, visibility controlled by contrast. 

	class ARContrastCircleSpec : public ARContrastFixationPointSpec
	{
	public:
		ARContrastCircleSpec() : ARContrastFixationPointSpec(), linewidth(1) {};
		ARContrastCircleSpec(const ARContrastCircleSpec& c);
		~ARContrastCircleSpec() {};
		ARContrastCircleSpec& operator=(const ARContrastCircleSpec& c);
		int draw();
		int drawOverlay();
		int linewidth;
	};


	// Circle, single pixel wide, visibility controlled by contrast. 

	class ARContrastCueCircleSpec: public ARContrastCircleSpec
	{
	public:
		ARContrastCueCircleSpec(): ARContrastCircleSpec(), dCentral(0), bCentralIsDot(true), bCircleEnabled(true) {};
		ARContrastCueCircleSpec(const ARContrastCueCircleSpec& c);
		~ARContrastCueCircleSpec() {};
		ARContrastCueCircleSpec& operator=(const ARContrastCueCircleSpec& c);
		int draw();
		int drawOverlay();
		int drawCircle();
		int drawPoint();
		double dCentral;	// if > 0, this is diam of fixpt at center, same color
		bool bCentralIsDot;	// if dCentral, then this says if center pt is dot or square
		bool bCircleEnabled;	// if false, do not draw cue circle, still may draw dot if d>0
	};

	// Circle, single pixel wide, visibility controlled by drawing it or not. No need to call init()
	// and use a vsg object

	class ARCircleSpec : public ARFixationPointSpec
	{
	public:
		ARCircleSpec() : ARFixationPointSpec(), linewidth(1) {};
		ARCircleSpec(const ARCircleSpec& c);
		~ARCircleSpec() {};
		ARCircleSpec& operator=(const ARCircleSpec& c);
		int draw();
		int drawOverlay();
		int linewidth;
	};

	// Circle, single pixel wide
	class ARCueCircleSpec : public ARCircleSpec
	{
	public:
		ARCueCircleSpec() : ARCircleSpec(), dCentral(0), bCentralIsDot(true), bCircleEnabled(true) {};
		ARCueCircleSpec(const ARCueCircleSpec& c);
		~ARCueCircleSpec() {};
		ARCueCircleSpec& operator=(const ARCueCircleSpec& c);
		int draw();
		int drawOverlay();
		int drawCircle();
		int drawPoint();
		double dCentral;	// if > 0, this is diam of fixpt at center, same color
		bool bCentralIsDot;	// if dCentral, then this says if center pt is dot or square
		bool bCircleEnabled;	// if false, do not draw cue circle, still may draw dot if d>0
	};


	// Circle, single pixel wide, visibility controlled by contrast. 

	class ARContrastLineSpec: public ARSpec
	{
	public:
		double x0, y0, x1, y1;
		COLOR_TYPE color;
		ARContrastLineSpec() {};
		ARContrastLineSpec(const ARContrastLineSpec& line);
		~ARContrastLineSpec() {};
		ARContrastLineSpec& operator=(const ARContrastLineSpec& line);
		int draw();
		int drawOverlay();
	};



	// Grating spec
	class ARGratingSpec: public ARSpec
	{
	public:
		ARGratingSpec() : bIsMulti(false), phase(0), wd(0), hd(0), ttf(0), swt(sinewave), twt(sinewave) {};

		// Copy constructor only copies grating parameters, not vsg object properties. 
		// A grating initialize with this constructor must still be initialized, and it will 
		// be a different vsg object than the original!

		ARGratingSpec(const ARGratingSpec& g);
		virtual ~ARGratingSpec() {};

		ARGratingSpec& operator=(const ARGratingSpec& g);

		double x,y,w,h;
		double wd, hd;	
		double sf;		// spatial frequency
		double tf;		// drift velocity (modifies spatial waveform). Calling it tf (temporal freq) is a misnomer.
		double orientation;
		int contrast;
		double phase;	// initial phase
		WAVEFORM_TYPE swt;
		WAVEFORM_TYPE twt;
		double ttf;		// temporal frequency - for the temporal waveform. 
		APERTURE_TYPE aperture;
		COLOR_VECTOR_TYPE cv;

		// by default this is NOT multi. Call setMulti to start it off. 
		bool bIsMulti;
		std::vector< std::tuple<double, double, double> > m_multi;
		void setMulti(const std::vector< std::tuple<double, double, double> >& vec) { bIsMulti = true; m_multi = vec; };
		void setMulti() { bIsMulti = false; m_multi.clear(); };

		// reset spatial phase to the initial phase
		virtual int resetSpatialPhase() { return setSpatialPhase(phase); };

		virtual int setSpatialPhase(double p) { phase = p;  select(); return vsgObjSetSpatialPhase(p); };

		// If useTransOnHigher is true then the page should have been cleared with a LOW pixel level.
		// If its false, then the grating is drawn as a rectangle.
		virtual int draw(bool useTransOnHigher);

		// drawMode should be either vsgTRANSONLOWER or vsgTRANSONHIGHER (all other modes are ignored). The
		// actual grating is drawn with the mode supplied and vsgCENTREXY. 
		// This function has a long and tortured history, so its usage is a little tricky. There's lots
		// of ways to get a circular grating and this is one. If using 
		virtual int draw(long drawMode, int apertureLevel = -1);		// 

		// This just calls draw(true)
		virtual int draw();

		// This draws an aperture on level 0. You must set the current draw page to an OVERLAYPAGE prior to 
		// calling this function!
		virtual int drawOverlay();

		virtual int drawOnce();
		virtual int drawBackground();
		virtual void setContrast(int contrast);
		virtual void setTemporalFrequency(double tf);
		virtual void resetDriftPhase();
	};


	// Grating that is drawn full screen. Width, height and aperture are used to 
	// draw a level 0 shape. Before calling drawOverlay(), be sure to switch drawing page
	// to an overlay page. 
	class ARApertureGratingSpec: public ARGratingSpec
	{
	public:
		ARApertureGratingSpec(): ARGratingSpec() {};
		virtual ~ARApertureGratingSpec() {};
		virtual int draw();
	};

};	// end namespace alert;


// Operators for these
std::ostream& operator<<(std::ostream& out, const alert::ARFixationPointSpec& arfps);
std::istream& operator>>(std::istream& in, alert::ARFixationPointSpec& arfps);
std::ostream& operator<<(std::ostream& out, const alert::ARRectangleSpec& arrect);
std::ostream& operator<<(std::ostream& out, const alert::ARGratingSpec& args);
std::ostream& operator<<(std::ostream& out, const alert::ARXhairSpec& arx);
std::ostream& operator<<(std::ostream& out, const alert::ARConteSpec& arconte);

// instead of input operators, methods
int parse_fixation_point(const std::string& s, alert::ARFixationPointSpec& afp);
int parse_fixation_point_list(std::string s, std::vector<alert::ARFixationPointSpec>& fixpt_list);
int parse_fixation_point_list(std::vector<std::string>& tokens, std::vector<alert::ARFixationPointSpec>& fixpt_list);
int parse_rectangle(const std::string& s, alert::ARRectangleSpec& arrect);
int parse_grating(const std::string& s, alert::ARGratingSpec& ag);
int parse_xhair(const std::string& s, alert::ARXhairSpec& axh);
int parse_conte(const std::string& s, alert::ARConteSpec & conte);
#endif