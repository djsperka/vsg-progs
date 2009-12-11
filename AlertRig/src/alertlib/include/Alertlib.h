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
#include <algorithm>


// Set this bit in the out_val of a trigger to indicate it should be toggled.
// The given out_val (without the toggle bit) will be the first value triggered.
// After that the value will toggle.
#define AR_TRIGGER_TOGGLE 0x8000

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
int parse_contrast_triplet(std::string s, int& i_iContrastDown, int& i_iContrastBase, int& i_iContrastUp);
int parse_int_pair(std::string s, int& i_i1, int& i_i2);
int parse_tuning_triplet(std::string s, double& i_dMin, double& i_dMax, int& i_iSteps);
int parse_tuning_list(std::string s, std::vector<double>& tuning_list, int& i_iSteps);
int parse_xy(std::string s, double& x, double& y);
void tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters);

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
	class ARvsg
	{
	public:
		~ARvsg() { clear(); };

		int init(int screenDistanceMM, COLOR_TYPE i_bg);
		int init_video();
		int init_overlay();

		int init_video_pages(voidfunc func_before_objects, voidfunc func_after_objects, void *data);

		/* Clear any page and display it. */
		void clear(int i);

		/* Clear page 0 and display it. */
		void clear();

		static ARvsg& instance() 
		{
			static ARvsg vsg;
			return vsg; 
		};

		COLOR_TYPE background_color() { return m_background_color; };
		PIXEL_LEVEL background_level() { return m_background_level; };

		long getScreenHeightPixels() { return m_heightPixels; };
		long getScreenWidthPixels() { return m_widthPixels; };
		double getScreenHeightDegrees() { return m_heightDegrees; };
		double getScreenWidthDegrees() { return m_widthDegrees; };

	private:
		ARvsg() : m_initialized(false) {};
		bool m_initialized;
		VSGOBJHANDLE m_handle;
		PIXEL_LEVEL m_background_level;
		COLOR_TYPE m_background_color;
		long m_heightPixels;
		long m_widthPixels;
		double m_heightDegrees;
		double m_widthDegrees;
	};



	// Base class that encapsulates VSG objects. All spec classes should inherit from this
	class ARObject
	{
	public:
		ARObject() : m_initialized(false) {};
		virtual ~ARObject() {};
		void init(PIXEL_LEVEL first, int numlevels);
		void init(int numlevels);
		int select();
		virtual void setContrast(int contrast) { select(); vsgObjSetContrast(contrast); };
		VSGOBJHANDLE handle() { return m_handle; };
		bool initialized() { return m_initialized; };
		PIXEL_LEVEL getFirstLevel() { return m_first; };
		int getNumLevels() { return m_numlevels; };
	private:
		bool m_initialized;
		VSGOBJHANDLE m_handle;
		PIXEL_LEVEL m_first;
		int m_numlevels;
	};

	// Base class for specs. Objects drawn using overlay pages use AROverlaySpec

	class ARSpec : public ARObject
	{
	public:
		ARSpec() {};
		virtual ~ARSpec() {};

		
		/* Draw object on currently selected page. */
		virtual int draw() = 0;

		/* Draw object on current page. Assume the page is an overlay page, draw an aperture for the 
		 * object on level 0. 
		 */
		virtual int drawOverlay() = 0;
	};




	// Fixation Point Spec

	class ARFixationPointSpec: public ARSpec
	{
	public:
		ARFixationPointSpec() {};
		~ARFixationPointSpec() {};
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
		~ARContrastFixationPointSpec() {};
		int draw();
		int drawOverlay();
	};


	// Circle, single pixel wide, visibility controlled by contrast. 

	class ARContrastCircleSpec: public ARContrastFixationPointSpec
	{
	public:
		ARContrastCircleSpec() {};
		~ARContrastCircleSpec() {};
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
		~ARContrastLineSpec() {};
		int draw();
		int drawOverlay();
	};



	// Grating spec
	class ARGratingSpec: public ARSpec
	{
	public:
		ARGratingSpec() {};
		virtual ~ARGratingSpec() {};
		double x,y,w,h;
		double sf, tf;
		double orientation;
		int contrast;
		PATTERN_TYPE pattern;
		APERTURE_TYPE aperture;
		COLOR_VECTOR_TYPE cv;
		int draw(bool useTransOnLower);
		virtual int draw();
		virtual int drawOverlay();
		int redraw(bool useTransOnLower);
		int drawOnce();
		int drawBackground();
		int setOrientation(double orientation);
		virtual void setContrast(int contrast) { select(); this->contrast = contrast; vsgObjSetContrast(contrast); };
		virtual void setTemporalFrequency(double tf) { select(); this->tf = tf; vsgObjSetDriftVelocity(tf); };
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


	class LevelManager
	{
	public:
		static LevelManager& instance() { static LevelManager m_instance; return m_instance; };
		~LevelManager() {};
		int request_single(PIXEL_LEVEL& level);
		int request_range(int num, PIXEL_LEVEL& first);
		int remaining() { return 251-m_next; }
	private:
		LevelManager(): m_next(0) {};
		int m_next;
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
//				std::cout << "TOGGLE(input) " << m_key << " in_val " << i_in_val << std::endl;
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


		std::string toString() const
		{
			std::ostringstream oss;
			oss << "Trigger " << m_key << " in mask/val/toggle: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << "/" << m_btoggleIn << " out mask/val/toggle: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
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

		std::string toString() const
		{
			unsigned int i;
			std::ostringstream oss;
			oss << "Trigger with multiple inputs" << std::endl;
			for (i=0; i<this->size(); i++)
			{
				oss << "   " << i << ": " << (*this)[i].first << " in mask/val/toggle: 0x" << std::hex << m_in_mask << "/0x" << (*this)[i].second << "/" << m_btoggleIn << " out mask/val/toggle: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
			}
			return oss.str();
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

		void operator()(Trigger* pitem)
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
	private:
		bool m_quit;
		bool m_binary;	// if true, do a binary trigger test. Otherwise do ascii
		int m_itrigger;	// input trigger value to test against
		std::string m_skey;
		bool m_present;	// if true, at least one trigger requires vsgPresent()
		int m_otrigger;	// if m_present is true, this is the output trigger value
		int m_page;
		int m_ideferred;	// flag set to indicate deferred processing of some sort is needed.
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



#endif