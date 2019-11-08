#if !defined(_ALERTLIB_H_)
#define _ALERTLIB_H_

#pragma warning(disable:4786)

//#include "VSGEX2.H"
#include "VSGV8.H"
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


// This macro returns true for a Visage, false for a 2/5 card. 
#define IS_VISAGE (vsgGetSystemAttribute(vsgDEVICECLASS)==7)


// output values from trigger or callbacks to ask for a master/slave present
// Apps must look at TriggerFunc::deferred() value to discern these requests.
#define PLEASE_PRESENT_MASTER 0x100
#define PLEASE_PRESENT_SLAVE  0x200
#define PLEASE_PRESENT_MASTER_ON_TRIGGER 0x400
#define PLEASE_PRESENT_SLAVE_ON_TRIGGER  0x800

#define GET_MASTER_TRIGGER_BITS(a) ((24>>a)&0xff)
#define GET_SLAVE_TRIGGER_BITS(a) ((16>>a)&0xff)

#define SET_MASTER_TRIGGER_BITS(a) (24<<(a&0xff))
#define SET_SLAVE_TRIGGER_BITS(a)  (16<<(a&0xff))


// These typedefs are used in all the specs. 
typedef enum colorvectorenum { unknown_color_vector=0, b_w, l_cone, m_cone, s_cone, custom_color_vector } COLOR_VECTOR_ENUM;
typedef enum colortype { unknown_color=0, black=1, white=2, red=3, green=4, blue=5, gray=6, custom=7 } COLOR_ENUM;
typedef enum patterntype { unknown_pattern=0, sinewave, squarewave } PATTERN_TYPE;
typedef enum aperturetype { unknown_aperture=0, ellipse, rectangle } APERTURE_TYPE;
typedef int PIXEL_LEVEL;

typedef struct color_vector_struct
{
	COLOR_VECTOR_ENUM type;
	VSGTRIVAL from;
	VSGTRIVAL to;
} COLOR_VECTOR_TYPE;

/*
typedef struct color_struct
{
	COLOR_ENUM type;
	VSGTRIVAL color;
} COLOR_TYPE;
*/

class COLOR_TYPE
{
	COLOR_ENUM m_type;
	VSGTRIVAL m_color;

public:
	COLOR_TYPE();
	COLOR_TYPE(COLOR_ENUM t);
	COLOR_TYPE(const COLOR_TYPE& ct);
	virtual ~COLOR_TYPE() {};
	COLOR_TYPE& operator=(const COLOR_TYPE& ct);
	COLOR_TYPE& operator=(const COLOR_ENUM &t);
	COLOR_ENUM type() const { return m_type; };
	VSGTRIVAL trival() const { return m_color; };
	void setType(COLOR_ENUM t);
	void setCustom(double a, double b, double c);
	void setCustom(double abc);
};

bool operator==(const COLOR_TYPE& lhs, const COLOR_TYPE& rhs);


typedef struct xywh_struct 
{
	double x, y, w, h;
} XYWH;

// useful helper functions
int parse_color(std::string s, COLOR_TYPE& c);
// SEE COLOR_TYPE::trival() int get_color(COLOR_TYPE c, VSGTRIVAL& trival);
int get_colorvector(COLOR_VECTOR_TYPE& cv, VSGTRIVAL& from, VSGTRIVAL& to);
int parse_colorvector(std::string s, COLOR_VECTOR_TYPE& v);
int parse_pattern(std::string s, PATTERN_TYPE& p);
int parse_aperture(std::string s, APERTURE_TYPE& a);
int parse_distance(std::string s, int& dist);
int parse_integer(std::string s, int& i);
int parse_ulong(std::string s, unsigned long& l);
int parse_uint(std::string s, unsigned int& l);
int parse_double(std::string s, double& d);
int parse_int_list(std::string s, std::vector<int>& list);
int parse_int_list(std::vector<std::string>& tokens, std::vector<int>& list);
int parse_contrast_triplet(std::string s, int& i_iContrastDown, int& i_iContrastBase, int& i_iContrastUp);
int parse_int_pair(std::string s, int& i_i1, int& i_i2);
int parse_sequence_pair(std::string s, int& i_i1, int& i_i2);
int parse_tuning_triplet(std::string s, double& i_dMin, double& i_dMax, int& i_iSteps);
int parse_tuning_list(std::string s, std::vector<double>& tuning_list, int& i_iSteps);
int parse_tuning_list(std::vector<std::string>& tokens, std::vector<double>& tuning_list, int& i_iSteps);
int parse_number_list(std::string s, std::vector<double>& number_list);
int parse_number_list(std::vector<std::string>& tokens, std::vector<double>& number_list);
int parse_xy(std::string s, double& x, double& y);
int parse_triplet(std::string s, double& d1, double& d2, double &d3);
int parse_sequence(std::string s, std::string& seq);
int parse_string(std::string s, std::string& scleaned);
void tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters);
void make_argv(const std::string& str, int &argc, char **argv);
void make_argv(std::vector<std::string>tokens, int& argc, char** argv);
void make_argv(std::ifstream& ifs, int& argc, char **argv);
void free_argv(int& argc, char **argv);
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





// convenient operators
std::ostream& operator<<(std::ostream& out, const COLOR_TYPE& c);
std::ostream& operator<<(std::ostream& out, const COLOR_VECTOR_TYPE& v);
std::ostream& operator<<(std::ostream& out, const APERTURE_TYPE& a);
std::ostream& operator<<(std::ostream& out, const PATTERN_TYPE& p);



/* Initialize vsg card. Leaves card with palette ramp, page 0 cleared to background color (use vsgBACKGROUND). 
 * Clears video page 0 and displays it. Suitable for initialization on startup of a stimulus app, 
 * or for cleanup on exit (or use clear_vsg()). If use_overlay is true, sets OVERLAYMASKMODE, initializes
 * the overlay palette to use bg color on level 1 (level 0 in overlay palette is reserved for "transparent".
 */
int init_vsg(int screenDistanceMM, COLOR_TYPE i_background, bool use_overlay);


/* Convenience */
void clear_vsg();

typedef void (*voidfunc)(int, void *);

namespace alert
{

	class ARvsg;

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
		void init(PIXEL_LEVEL first, int numlevels);
		void init(ARvsg& vsg, PIXEL_LEVEL first, int numlevels);
		void init(int numlevels);
		void init(ARvsg& vsg, int numlevels);
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
		ARContrastFixationPointSpec(const ARContrastFixationPointSpec& fixpt) : ARFixationPointSpec(fixpt) {};
		~ARContrastFixationPointSpec() {};
		ARContrastFixationPointSpec& operator=(const ARContrastFixationPointSpec& fixpt);
		ARContrastFixationPointSpec& operator=(const ARFixationPointSpec& fixpt);
		int draw();
		int drawOverlay();
	};


	// Circle, single pixel wide, visibility controlled by contrast. 

	class ARContrastCircleSpec: public ARContrastFixationPointSpec
	{
	public:
		ARContrastCircleSpec(): ARContrastFixationPointSpec(), linewidth(1) {};
		ARContrastCircleSpec(const ARContrastCircleSpec& c);
		~ARContrastCircleSpec() {};
		ARContrastCircleSpec& operator=(const ARContrastCircleSpec& c);
		int draw();
		int drawOverlay();
		int linewidth;
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
		ARGratingSpec() : bDrawInitDone(false), phase(0), wd(0), hd(0) {};

		// Copy constructor only copies grating parameters, not vsg object properties. 
		// A grating initialize with this constructor must still be initialized, and it will 
		// be a different vsg object than the original!

		ARGratingSpec(const ARGratingSpec& g);
		virtual ~ARGratingSpec() {};

		ARGratingSpec& operator=(const ARGratingSpec& g);

		double x,y,w,h;
		double wd, hd;	
		double sf, tf;
		double orientation;
		int contrast;
		double phase;	// initial phase
		PATTERN_TYPE pattern;
		APERTURE_TYPE aperture;
		COLOR_VECTOR_TYPE cv;
		bool bDrawInitDone;


		// gets needed for const objs
		const COLOR_VECTOR_TYPE& getCV() const { return cv; };


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
std::ostream& operator<<(std::ostream& out, const alert::ARRectangleSpec& arrect);
std::ostream& operator<<(std::ostream& out, const alert::ARGratingSpec& args);
std::ostream& operator<<(std::ostream& out, const alert::ARXhairSpec& arx);

// instead of input operators, methods
int parse_fixation_point(const std::string& s, alert::ARFixationPointSpec& afp);
int parse_rectangle(const std::string& s, alert::ARRectangleSpec& arrect);
int parse_grating(const std::string& s, alert::ARGratingSpec& ag);
int parse_xhair(const std::string& s, alert::ARXhairSpec& axh);

#endif