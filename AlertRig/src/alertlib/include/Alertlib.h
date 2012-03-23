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

// Set this bit in the out_val of a trigger to indicate it should be toggled.
// The given out_val (without the toggle bit) will be the first value triggered.
// After that the value will toggle.
#define AR_TRIGGER_TOGGLE 0x8000

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

typedef struct color_struct
{
	COLOR_ENUM type;
	VSGTRIVAL color;
} COLOR_TYPE;

typedef struct xywh_struct 
{
	double x, y, w, h;
} XYWH;

// useful helper functions
int parse_color(std::string s, COLOR_TYPE& c);
int get_color(COLOR_TYPE c, VSGTRIVAL& trival);
int get_colorvector(COLOR_VECTOR_TYPE& cv, VSGTRIVAL& from, VSGTRIVAL& to);
int parse_colorvector(std::string s, COLOR_VECTOR_TYPE& v);
int parse_pattern(std::string s, PATTERN_TYPE& p);
int parse_aperture(std::string s, APERTURE_TYPE& a);
int parse_distance(std::string s, int& dist);
int parse_integer(std::string s, int& i);
int parse_double(std::string s, double& d);
int parse_int_list(std::string s, std::vector<int>& list);
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
int prargs(int argc, char **argv, process_args_func pfunc, char *options, int response_file_char=0);
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

	// Singleton class representing the VSG. 
	// Modified 4-27-2010 djs
	// Changed to a "double singleton" class to accomodate the dual vsg setup. 
	class ARvsg
	{
	public:
		~ARvsg();
		int init(int screenDistanceMM, COLOR_TYPE i_bg, bool bUseLockFile=true, bool bSlaveSynch = false);

		// This function initializes all pages to background color. That color is set as 
		// level 0 - this func does not use vsgBACKGROUND. This func should be used if 
		// you draw gratings with draw(true). Why? Well, initially your pages are initialized
		// to a low level - 0 or 1. Then draw(true) draws an ellipse using level 250. Next, 
		// the grating is drawn using vsgTRANSONHIGHER, meaning that only the portion of the 
		// grating that overlays the ellipse will actually be drawn. 
		int init_video();

		int init_overlay();

		int init_video_pages(voidfunc func_before_objects, voidfunc func_after_objects, void *data);

		/* lock/unlock */
		bool acquire_lock();
		void release_lock();

		/* Clear any page and display it. */
		void clear(int i);

		/* Clear page 0 and display it. */
		void clear();

		/* Send ready pulse -- used for VSG/Spike2 expts. */
		void ready_pulse(int wait_msecs = 2000, unsigned int which_bit = vsgDIG6);

		/* 
		 * instance() will return a singleton instance of this class. Use this when you are using a single 
		 * vsg. When using dual vsg setup, use master() and slave() to get instances for each separately. 
		 */
		static ARvsg& instance();
		static ARvsg& master();
		static ARvsg& slave();
		static ARvsg& ARvsg::instance(bool is_master, bool is_slave);

		bool is_master();
		bool is_slave();

		/*
		 * Select specifies the vsg to which commands are directed. 
		 */
		void select();

		COLOR_TYPE background_color();
		PIXEL_LEVEL background_level();
		long getScreenHeightPixels();
		long getScreenWidthPixels();
		double getScreenHeightDegrees();
		double getScreenWidthDegrees();

//		LevelManager& getLevelManager();
		// Use these functions to get available levels
		int request_single(PIXEL_LEVEL& level);
		int request_range(int num, PIXEL_LEVEL& first);
		int remaining();

	private:
		ARvsg(bool bMaster=false, bool bSlave=false);
		ARvsg(ARvsg const&) {};
		ARvsg& operator=(ARvsg const&) {};
		bool m_initialized;
		bool m_is_master;
		bool m_is_slave;
		VSGOBJHANDLE m_handle;
		PIXEL_LEVEL m_background_level;
		COLOR_TYPE m_background_color;
		long m_heightPixels;
		long m_widthPixels;
		double m_heightDegrees;
		double m_widthDegrees;
		static bool c_bHaveLock;
		long m_device_handle;
		int m_next_available_level;
	};

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
		void destroy();
		int select();
		virtual void setContrast(int contrast);
		VSGOBJHANDLE handle();
		bool initialized();
		PIXEL_LEVEL getFirstLevel();
		int getNumLevels();
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
		ARRectangleSpec(): x(0), y(0), orientation(0) {};
		~ARRectangleSpec() {};
		double x, y;	// Center
		double w, h;	// width, height. Units depend on how vsg is initialized
		double orientation;
		COLOR_TYPE color;
		virtual int draw();
		virtual int drawOverlay();
	};

	
	class ARContrastRectangleSpec: public ARRectangleSpec
	{
	public:
		ARContrastRectangleSpec() {};
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


	// Fixation Point Spec

	class ARFixationPointSpec: public ARSpec
	{
	public:
		ARFixationPointSpec() {};
		ARFixationPointSpec(const ARFixationPointSpec& fixpt);
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
		ARContrastFixationPointSpec() {};
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
		ARContrastCircleSpec() {};
		ARContrastCircleSpec(const ARContrastCircleSpec& c);
		~ARContrastCircleSpec() {};
		ARContrastCircleSpec& operator=(const ARContrastCircleSpec& c);
		int draw();
		int drawOverlay();
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
		ARGratingSpec() : phase(0), wd(0), hd(0) {};

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

		// reset spatial phase to the initial phase
		virtual int resetSpatialPhase() { return setSpatialPhase(phase); };

		virtual int setSpatialPhase(double p) { select(); return vsgObjSetSpatialPhase(p); };

		// If useTransOnHigher is true then the page should have been cleared with a LOW pixel level.
		// If its false, then the grating is drawn as a rectangle.
		virtual int draw(bool useTransOnHigher);

		// drawMode should be either vsgTRANSONLOWER or vsgTRANSONHIGHER (all other modes are ignored). The
		// actual grating is drawn with the mode supplied and vsgCENTREXY. 
		// This function has a long and tortured history, so its usage is a little tricky. There's lots
		// of ways to get a circular grating and this is one. If using 
		virtual int draw(long drawMode);		// 

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
		ARApertureGratingSpec() {};
		virtual ~ARApertureGratingSpec() {};
		virtual int draw();
	};



// The trigger class (and its derivatives) perform a specific action upon receipt of a
// "trigger". Triggers can be ascii characters or bits (in a 32 bit integer).  

	class Trigger
	{
	public:
		Trigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val)
		{
			m_key = i_key;
			m_in_mask = i_in_mask;
			m_out_mask = i_out_mask;

			// Check if the output value should be toggled
			if (i_out_val & AR_TRIGGER_TOGGLE)
			{
//				std::cout << "TOGGLE(output) " << m_key << " out_val " << i_out_val << std::endl;
				m_btoggleOut = true;
				m_out_val = i_out_val & ~(AR_TRIGGER_TOGGLE);
				m_last_out_val = ~m_out_val & m_out_mask;
			}
			else
			{
				m_btoggleOut = false;
				m_out_val = i_out_val;
				m_last_out_val = m_out_val;
			}

			// Check if the input value will be toggled
			if (i_in_val & AR_TRIGGER_TOGGLE)
			{
//				std::cout << "Trigger(" << m_key << ")" << " TOGGLE(input) " << m_key << " in_val " << i_in_val << std::endl;
				m_btoggleIn = true;
				m_in_val = i_in_val & ~(AR_TRIGGER_TOGGLE);
//				m_in_last = ~m_in_val & m_in_mask;
//				m_in_last = 0;
			}
			else
			{
				m_btoggleIn = false;
				m_in_val = i_in_val;
//				m_in_last = m_in_val;
//				m_in_last = 0;
			}

			reset();	// this gives the m_in_val a value.
		
		};

		~Trigger() {};

		virtual bool checkAscii(std::string input)
		{
			return input == m_key;
		};

		virtual bool checkBinary(int input)
		{
			bool bValue = false;
			int current = input&m_in_mask;

			if (!m_btoggleIn)
			{
				if (current != m_in_last && current == m_in_val)
				{
					bValue = true;
				}
				m_in_last = current;
			}
			else
			{
				// We expect the trigger to be the last trigger toggled. If a trigger covers more than 
				// one bit, then all must be inverted. Note that the FIRST trigger expected is the value
				// given as i_in_val (without the AR_TRIGGER_TOGGLE bit). Subsequent triggers are expected
				// to be toggled. 
				if (((~current)&m_in_mask) == m_in_last)
				{
					bValue = true;
					m_in_last = current;	// Note that the last value is saved only if a toggled trigger
				}
//				std::cout << "Trigger(" << m_key << ")" <<  std::hex << " input " << input << " m_in_mask=" << m_in_mask << " m_in_val=" << m_in_val << " current=" << current << " last=" << m_in_last << " bval=" << bValue << std::endl;
			}
//			if (bValue) std::cerr << "Trigger(" << m_key << ")" << std::hex << " input " << input << " m_in_mask=" << m_in_mask << " m_in_val=" << m_in_val << " current=" << current << " bval=" << bValue << std::endl;
			return bValue;
		};

		virtual void setMarker(int& output)
		{
			if (!m_btoggleOut)
			{
				if (m_out_val >= 0)
				{
//					std::cout << "setMarker: output(in)=" << output;
					output = m_out_val | (output&(~m_out_mask));
//					std::cout << "setMarker: output(out)=" << output << std::endl;
				}
			}
			else
			{
				int temp;
				temp = ~m_last_out_val & m_out_mask;
				output = temp | (output&(~m_out_mask));
				m_last_out_val = temp;
			}
		};


		virtual std::string toString() const
		{
			std::ostringstream oss;
			oss << "Trigger " << m_key << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
			return oss.str();
		}

		// Execute the triggers' action(s). Subclasses should return >0 if a vsgPresent() will be 
		// needed, 0 if no present(), and <0 if this trigger means quit (a call to vsgPresent() is 
		// is made in this case, too -- that way a quitting trigger can set the page to blank
		// or something similarly intelligent. 
		virtual int execute(int& output) { return 0; };

		void reset()
		{
			reset(0);
		};


		// reset the trigger's notion of what the last input value was. 
		// This should be called in order to ensure that the first instance of the 
		// trigger will in fact fire the trigger. Remember that in order for the trigger
		// to fire the input must be DIFFERENT than the last time it was called, AND it
		// must match the in_val. Its best to call TriggerVector::reset(), not this
		// directly


		void reset(int input)
		{
			if (!m_btoggleIn)
			{
				m_in_last = (~input) & m_in_mask;
			}
			else
			{
				m_in_last = input & m_in_mask;
			}
		};


		std::string getKey() const { return m_key; };
		int outMask() const { return m_out_mask; };
	protected:
		std::string m_key;
		int m_in_mask;
		int m_in_val;
		int m_in_last;	// last value of this trigger's input&m_in_mask (initial = 0) 
		int m_out_mask;
		int m_out_val;
		int m_last_out_val;
		bool m_btoggleOut;
		bool m_btoggleIn;
	};


	// callback function type for CallbackTrigger
	class CallbackTrigger;
	typedef int (*TriggerCallbackFunc)(int &output, const CallbackTrigger* ptrig);

	class CallbackTrigger: public Trigger
	{
	public:
		CallbackTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, TriggerCallbackFunc tcf ) : 
		  Trigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val), m_callback(tcf)  {};
		~CallbackTrigger() {};
		virtual int execute(int& output)
		{
			setMarker(output);
			return m_callback(output, this);
		};
	
		virtual std::string toString() const
		{
			std::ostringstream oss;
			oss << "Callback " << m_key << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
			return oss.str();
		}

	protected:
		TriggerCallbackFunc m_callback;
	};



	class MultiInputSingleOutputCallbackTrigger : public CallbackTrigger, public std::vector< std::pair< std::string, int > >
	{
	private:
		int m_input_matched;	// index of input key/value matched in a call to checkAscii/checkBinary

	public:
		MultiInputSingleOutputCallbackTrigger(std::vector<std::pair<std::string, int> >& v, int i_in_mask, int i_out_mask, int i_out_val, TriggerCallbackFunc tcf) :
		  std::vector< std::pair < std::string, int> >(v), CallbackTrigger("", i_in_mask, 0, i_out_mask, i_out_val, tcf), m_input_matched(-1)
		  {};

		~MultiInputSingleOutputCallbackTrigger() {};

		virtual bool checkAscii(std::string input)
		{
			unsigned int i;
			m_input_matched = -1;
			m_key = "NO MATCH";
			for (i=0; i<this->size(); i++)
			{
				if (input == (*this)[i].first)
				{
					m_input_matched = i;
					m_key = (*this)[i].first;
					break;
				}
			}
			return (m_input_matched >= 0);
		};

		virtual bool checkBinary(int input)
		{
			unsigned int i;
			bool bValue = false;
			int current = input&m_in_mask;

			m_input_matched = -1;
			m_key = "NO MATCH";
			if (!m_btoggleIn)
			{
				if (current != m_in_last)
				{
					for (i=0; i<this->size(); i++)
					{
						if (current == (*this)[i].second)
						{
							m_input_matched = i;
							m_key = (*this)[i].first;
							break;
						}
					}
//					std::cout << "current " << std::hex << current << " matched " << m_input_matched << std::endl;
				}
				m_in_last = current;
				bValue = (m_input_matched >= 0);
			}
			else
			{
				// Toggling inputs is a bit ill-defined for this type of trigger. 
				// I'll just not try and define it now and call it an error. Always false. 
				std::cerr << "Cannot use toggled input trigger for MultiInputSingleOutputCallbackTrigger" << std::endl;
				bValue = false;
			}

			return bValue;
		};

		virtual int execute(int& output)
		{
			setMarker(output);
			return m_callback(output, this);
		};

		std::string getKey() const 
		{ 
//			std::cout << "getKey(): m_input_matched = " << m_input_matched << std::endl;
			if (m_input_matched < 0 || m_input_matched > (int)this->size()) return "ERROR";
			else return (*this)[m_input_matched].first;
		};

		virtual std::string toString() const
		{
			unsigned int i;
			std::ostringstream oss;
			oss << "MISOCallback " << std::endl; 
			for (i=0; i<this->size(); i++)
			{
				oss << "         " << (*this)[i].first << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << (*this)[i].second << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
				if (i<(this->size()-1)) oss << std::endl;
			}
			return oss.str();
		}

	};


	// Functor technique from http://www.newty.de/fpt/functor.html

	class FunctorCallbackTrigger;	// forward declaration needed

	// Abstract callback base class
	class TFunctor 
	{
	public:
		virtual int operator()(int &output, const FunctorCallbackTrigger* ptrig)=0;  // call using operator "()"
		virtual int call(int &output, const FunctorCallbackTrigger* ptrig)=0;        // call using function "Call"	
	};



	// Derived template for specific functor that can be used in CallbackFunctorTrigger
   template <class TClass> class TSpecificFunctor : public TFunctor
   {
   private:
      int (TClass::*fpt)(int &output, const FunctorCallbackTrigger* ptrig);   // pointer to member function
      TClass* pt2Object;                  // pointer to object

   public:

      // constructor - takes pointer to an object and pointer to a member and stores
      // them in two private variables
      TSpecificFunctor(TClass* _pt2Object, int(TClass::*_fpt)(int &output, const FunctorCallbackTrigger* ptrig))
         { pt2Object = _pt2Object;  fpt=_fpt; };

      // override operator "()"
      virtual int operator()(int &output, const FunctorCallbackTrigger* ptrig)
       { return (*pt2Object.*fpt)(output, ptrig);};              // execute member function

      // override function "Call"
      virtual int call(int &output, const FunctorCallbackTrigger* ptrig)
        { return (*pt2Object.*fpt)(output, ptrig);};             // execute member function
   };


   	class FunctorCallbackTrigger: public Trigger
	{
	public:
		FunctorCallbackTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, TFunctor *pfunc ) : 
		  Trigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val), m_pfunc(pfunc)  {};
		~FunctorCallbackTrigger() {};
		virtual int execute(int& output)
		{
			setMarker(output);
			return (*m_pfunc)(output, this);
		};
	
		virtual std::string toString() const
		{
			std::ostringstream oss;
			oss << "Callback " << m_key << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
			return oss.str();
		}

	protected:
		TFunctor *m_pfunc;
	};






	class PageTrigger: public Trigger
	{
	public:
		PageTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_page) : 
		  Trigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val), m_page(i_page) {};
		~PageTrigger() {};
		virtual int execute(int& output)
		{
			setMarker(output);
			vsgSetDrawPage(vsgVIDEOPAGE, m_page, vsgNOCLEAR);
			return 1;
		};
		virtual std::string toString() const
		{
			std::ostringstream oss;
			oss << "Page(" << m_page << ")" << m_key << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
			return oss.str();
		}
	protected:
		int m_page;
	};

	class MasterPageTrigger: public PageTrigger
	{
	public:
		MasterPageTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_page) : 
		  PageTrigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val, i_page) {};
		~MasterPageTrigger() {};

		// execute requires present.... but what vsg will receive that? 
		// possibly return diff't values for this trigger, e.g. MASTER_PRESENT or SLAVE_PRESENT?
		virtual int execute(int& output)
		{
			ARvsg::master().select();
			PageTrigger::execute(output);
			return PLEASE_PRESENT_MASTER;
		}
		virtual std::string toString() const
		{
			std::ostringstream oss;
			oss << "MasterPage(" << m_page << ")" << m_key << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
			return oss.str();
		}
	};

	class SlavePageTrigger: public PageTrigger
	{
	public:
		SlavePageTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_page) : 
		  PageTrigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val, i_page) {};
		~SlavePageTrigger() {};

		// execute requires present.... but what vsg will receive that? 
		// possibly return diff't values for this trigger, e.g. MASTER_PRESENT or SLAVE_PRESENT?
		virtual int execute(int& output)
		{
			ARvsg::slave().select();
			PageTrigger::execute(output);
			return PLEASE_PRESENT_SLAVE;
		}
		virtual std::string toString() const
		{
			std::ostringstream oss;
			oss << "SlavePage(" << m_page << ")" << m_key << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
			return oss.str();
		}
	};


	class TogglePageTrigger: public Trigger
	{
	public:
		TogglePageTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_pageA, int i_pageB) : 
		  Trigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val), m_pageA(i_pageA), m_pageB(i_pageB) {};
		~TogglePageTrigger() {};
		virtual int execute(int& output)
		{
			int iPage = vsgGetZoneDisplayPage(vsgVIDEOPAGE);
			if (iPage == m_pageA) iPage = m_pageB;
			else if (iPage == m_pageB) iPage = m_pageA;
			vsgSetDrawPage(vsgVIDEOPAGE, iPage, vsgNOCLEAR);
			setMarker(output);
			return 1;
		};
		virtual std::string toString() const
		{
			std::ostringstream oss;
			oss << "TogglePage(" << m_pageA << "/" << m_pageB << ")" << m_key << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
			return oss.str();
		}
	protected:
		int m_pageA;
		int m_pageB;
	};

	class MasterTogglePageTrigger: public TogglePageTrigger
	{
	public:
		MasterTogglePageTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_pageA, int i_pageB) : 
		  TogglePageTrigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val, i_pageA, i_pageB) {};
		  ~MasterTogglePageTrigger() {};
		virtual int execute(int& output)
		{
			ARvsg::master().select();
			TogglePageTrigger::execute(output);
			return PLEASE_PRESENT_MASTER;
		}
		virtual std::string toString() const
		{
			std::ostringstream oss;
			oss << "MasterTogglePage(" << m_pageA << "/" << m_pageB << ")" << m_key << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
			return oss.str();
		}
	};

	class SlaveTogglePageTrigger: public TogglePageTrigger
	{
	public:
		SlaveTogglePageTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_pageA, int i_pageB) : 
		  TogglePageTrigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val, i_pageA, i_pageB) {};
		  ~SlaveTogglePageTrigger() {};
		virtual int execute(int& output)
		{
			ARvsg::slave().select();
			TogglePageTrigger::execute(output);
			return PLEASE_PRESENT_SLAVE;
		}
		virtual std::string toString() const
		{
			std::ostringstream oss;
			oss << "SlaveTogglePage(" << m_pageA << "/" << m_pageB << ")" << m_key << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
			return oss.str();
		}
	};

	class OverlayPageTrigger: public Trigger
	{
	public:
		OverlayPageTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_page) : 
		  Trigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val), m_page(i_page) {};
		~OverlayPageTrigger() {};
		virtual int execute(int& output)
		{
			setMarker(output);
			vsgSetDrawPage(vsgOVERLAYPAGE, m_page, vsgNOCLEAR);
			return 1;
		};
		virtual std::string toString() const
		{
			std::ostringstream oss;
			oss << "OverlayPage(" << m_page << ")" << m_key << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
			return oss.str();
		}
	protected:
		int m_page;
	};


	class QuitTrigger: public PageTrigger
	{
	public:
		QuitTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_page) : 
		  PageTrigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val, i_page) {};
		~QuitTrigger() {};
		virtual int execute(int& output)
		{
			setMarker(output);
			vsgSetDrawPage(vsgVIDEOPAGE, m_page, vsgNOCLEAR);
			return -1;
		};
		virtual std::string toString() const
		{
			std::ostringstream oss;
			oss << "Quit    " << m_key << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
			return oss.str();
		}
	private:
	};



	class ContrastTrigger: public Trigger, public std::vector< std::pair< VSGOBJHANDLE, int > >
	{
	public:
		ContrastTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val) : 
		  Trigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val) {};
		  ~ContrastTrigger() { };
		virtual int execute(int& output)
		{
			setMarker(output);
			for (unsigned int i=0; i<this->size(); i++)
			{
				vsgObjSelect((*this)[i].first);
				vsgObjSetContrast((*this)[i].second);
			}
			return 1;
		};
		virtual std::string toString() const
		{
			unsigned int i;
			std::ostringstream oss;
			oss << "Contrast( ";
			for (i=0; i<this->size(); i++)
			{
				oss << (int)((*this)[i].first) << "->" << (*this)[i].second;
				if (i<this->size()-1) oss << ":";
			}
			oss << ") ";
			oss << m_key << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
			return oss.str();
		}
	};



	// callback for page cycling trigger
	class PageCyclingTrigger;
	typedef int (*PageCyclingTriggerCallbackFunc)(int icycle);

	class PageCyclingTrigger: public Trigger
	{
	public:
		PageCyclingTrigger(std::string i_key, int n_repeats, PageCyclingTriggerCallbackFunc callback = NULL) : 
		  Trigger(i_key, 0, 0, 0, 0), m_nrepeats(n_repeats), m_repeat_count(0), m_is_started(false), m_callback(callback) {};
		~PageCyclingTrigger() {};

		virtual bool checkAscii(std::string input)
		{
			return checkBinary(0);
		};

		virtual bool checkBinary(int input)
		{
			bool bValue = false;
			if (m_is_started)
			{
				// check if cycling is still running. If not, then set bValue = true.
				if (vsgGetSystemAttribute(vsgPAGECYCLINGSTATE) < 0)
				{
					m_repeat_count++;
					bValue = true;
				}
			}
			return bValue;
		};

		void started()
		{
			m_is_started = true;
		}

		void stopped()
		{
			m_is_started = false;
		}

		virtual int execute(int& output)
		{
			if (m_repeat_count < m_nrepeats)
			{
				if (m_callback)
				{
					if (m_callback(m_repeat_count))
					{
						m_is_started = false;
					}
					else 
					{
						setMarker(output);
						vsgSetCommand(vsgCYCLEPAGEENABLE);
					}
				}
				else
				{
					setMarker(output);
					vsgSetCommand(vsgCYCLEPAGEENABLE);
				}
			}
			else
			{
				m_is_started = false;
			}
			return 0;
		};
	protected:
		int m_nrepeats;
		int m_repeat_count;
		bool m_is_started;
		PageCyclingTriggerCallbackFunc m_callback;
	};


	// callback for file monitor trigger
	class FileChangedTrigger;
	typedef int (*FileChangedTriggerCallbackFunc)(bool created, bool deleted);


	// If an output trigger value and mask are supplied, then the output is fired when this trigger executes. 
	// We have to do this because there is no present() issued by this trigger. 
	// That could lead to problems if multiple triggers will fire at once, I suppose. 
	class FileChangedTrigger: public Trigger
	{
	public:
		FileChangedTrigger(std::string i_key, std::string filename, FileChangedTriggerCallbackFunc callback = NULL, int i_out_mask=0, int i_out_val=0) : 
		  Trigger(i_key, 0, 0, i_out_mask, i_out_val), m_filename(filename), m_callback(callback), m_exists(false), m_bCreated(false), m_bDeleted(false)
		  {
			  init();
		  }
		~FileChangedTrigger() {};

		virtual bool checkAscii(std::string input)
		{
			return checkBinary(0);
		};

		virtual bool checkBinary(int input)
		{
			struct _stat fileinfo;
			bool bValue = false;
			int status=0;

			status = _stat(m_filename.c_str(), &fileinfo);
			if (status)
			{
				// nonzero status value means the file was not found.
				if (m_exists)
				{
					bValue = true;
					m_bCreated = false;
					m_bDeleted = true;
					m_exists = false;
				}
			}
			else
			{
				if (!m_exists)
				{
					bValue = true;
					m_bCreated = true;
					m_bDeleted = false;
					m_exists = true;
					m_fileinfo = fileinfo;
				}
				else if (fileinfo.st_mtime != m_fileinfo.st_mtime)
				{
					bValue = true;
					m_bCreated = false;
					m_bDeleted = false;
					m_fileinfo = fileinfo;
				}
			}
			return bValue;
		};

		virtual int execute(int& output)
		{
			if (m_callback)
			{
				if (!m_callback(m_bCreated, m_bDeleted))
				{
					setMarker(output);
				}
			}
			else
			{
				setMarker(output);
			}
			// When writing via vsgIOWriteDigitalOut we must shift left 1 bit because vsgObjSetTriggers does. 
			if (m_out_mask) vsgIOWriteDigitalOut(output << 1, m_out_mask << 1);
			return 0;
		};
	private:
		void init()
		{
			if (_stat(m_filename.c_str(), &m_fileinfo)) m_exists = false;
			else m_exists = true;
		};
	protected:
		std::string m_filename;
		bool m_exists;
		bool m_bCreated;
		bool m_bDeleted;
		FileChangedTriggerCallbackFunc m_callback;
		struct _stat m_fileinfo;
	};


	class ResetTriggerFunc
	{
	public:
		ResetTriggerFunc(int val) : m_val(val) {};
		~ResetTriggerFunc() {};
		void operator()(Trigger* pitem)
		{
			pitem->reset(m_val);
		}
		int m_val;
	};



	class TriggerFunc
	{
	public:
		TriggerFunc(std::string key, int otrigger) : m_binary(false), m_skey(key), m_present(false), m_otrigger(otrigger), m_page(-1), m_quit(false), m_ideferred(0) {};
		TriggerFunc(int itrigger, int otrigger) : m_binary(true), m_itrigger(itrigger), m_present(false), m_otrigger(otrigger), m_page(-1), m_quit(false), m_ideferred(0) {};

		int page() { return m_page; };
		bool present() { return m_present; };
		int output_trigger() { return m_otrigger; };
		bool quit() { return m_quit; };
		int deferred() { return m_ideferred; };

		virtual void operator()(Trigger* pitem)
		{
			bool bTest=false;
			if (m_binary) bTest = pitem->checkBinary(m_itrigger);
			else bTest = pitem->checkAscii(m_skey);

			if (bTest)
			{
				int i;
				i = pitem->execute(m_otrigger);
				m_ideferred = i;
				if (i > 0) m_present = true;
				else if (i < 0) 
				{
					m_present = true;
					m_quit = true;
				}
			}
		};
	protected:
		bool m_quit;
		bool m_binary;	// if true, do a binary trigger test. Otherwise do ascii
		int m_itrigger;	// input trigger value to test against
		std::string m_skey;
		bool m_present;	// if true, at least one trigger requires vsgPresent()
		int m_otrigger;	// if m_present is true, this is the output trigger value
		int m_page;
		int m_ideferred;	// flag set to indicate deferred processing of some sort is needed.
	};


	class MasterSlaveTriggerFunc : public TriggerFunc
	{
	public:
		MasterSlaveTriggerFunc(std::string key, int otrigger) : TriggerFunc(key, otrigger) {};
		MasterSlaveTriggerFunc(int itrigger, int otrigger) : TriggerFunc(itrigger, otrigger) {};

		// This func differs from regular TriggerFunc's method in the handling of m_ideferred - because I'm scared
		// of breaking apps that use negative values in return values, or those where multiple triggers fire. 
		virtual void operator()(Trigger* pitem)
		{
			bool bTest=false;
			if (m_binary) bTest = pitem->checkBinary(m_itrigger);
			else bTest = pitem->checkAscii(m_skey);

			if (bTest)
			{
				int i;
				i = pitem->execute(m_otrigger);
				m_ideferred |= i;
				if (i > 0) m_present = true;
				else if (i < 0) 
				{
					m_present = true;
					m_quit = true;
				}
			}
		};
	};



	class TriggerVector: public std::vector<Trigger*>
	{
	public:
		TriggerVector() {};
		virtual ~TriggerVector() 
		{
			for (unsigned int i=0; i<size(); i++)
			{
				delete (*this)[i];
			}
		};
		void addTrigger(Trigger* t)
		{
			push_back(t);
		};

		void reset(int input)
		{
			std::for_each(this->begin(), this->end(), ResetTriggerFunc(input));
		}
	};



};	// end namespace alert;


// Operators for these
std::ostream& operator<<(std::ostream& out, const alert::ARFixationPointSpec& arfps);
std::ostream& operator<<(std::ostream& out, const alert::ARGratingSpec& args);
std::ostream& operator<<(std::ostream& out, const alert::Trigger& t);

// instead of input operators, methods
int parse_fixation_point(const std::string& s, alert::ARFixationPointSpec& afp);
int parse_grating(const std::string& s, alert::ARGratingSpec& ag);
int parse_xhair(const std::string& s, alert::ARXhairSpec& axh);

#endif