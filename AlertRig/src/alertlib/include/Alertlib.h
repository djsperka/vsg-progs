#if !defined(_ALERTLIB_H_)
#define _ALERTLIB_H_

#pragma warning(disable:4786)


#include "VSGEX2.H"
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__
#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <ostream>
#include <sstream>

// These typedefs are used in all the specs. 
typedef enum colorvectortype { unknown_color_vector=0, b_w, l_cone, m_cone, s_cone } COLOR_VECTOR_TYPE;
typedef enum colortype { unknown_color=0, black=1, white=2, red=3, green=4, blue=5, gray=6 } COLOR_TYPE;
typedef enum patterntype { unknown_pattern=0, sinewave, squarewave } PATTERN_TYPE;
typedef enum aperturetype { unknown_aperture=0, ellipse, rectangle } APERTURE_TYPE;
typedef int PIXEL_LEVEL;


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
int parse_tuning_triplet(std::string s, double& i_dMin, double& i_dMax, int& i_iSteps);
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

	private:
		ARvsg() : m_initialized(false) {};
		bool m_initialized;
		VSGOBJHANDLE m_handle;
		PIXEL_LEVEL m_background_level;
		COLOR_TYPE m_background_color;
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
		int setOrientation(double orientation);
		virtual void setContrast(int contrast) { select(); this->contrast = contrast; vsgObjSetContrast(contrast); };
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
			m_in_val = i_in_val;
			m_out_mask = i_out_mask;
			m_out_val = i_out_val;
			m_binitial = false;
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
			initial(input);
			if (current != m_in_last && current == m_in_val)
			{
				bValue = true;
			}
			m_in_last = current;
			if (bValue) std::cerr << "key: " << m_key << std::hex << " input " << input << " m_in_mask=" << m_in_mask << " m_in_val=" << m_in_val << " current=" << current << " bval=" << bValue << std::endl;
			return bValue;
		};

		virtual void setMarker(int& output)
		{
			if (m_out_val >= 0)
			{
				output = m_out_val | (output&(~m_out_mask));
			}
		};


		std::string toString() const
		{
			std::ostringstream oss;
			oss << "Trigger " << m_key << " in mask/val: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << " out mask/val: 0x" << m_out_mask << "/0x" << m_out_val;
			return oss.str();
		}

		// Execute the triggers' action(s). Subclasses should return >0 if a vsgPresent() will be 
		// needed, 0 if no present(), and <0 if this trigger means quit (a call to vsgPresent() is 
		// is made in this case, too -- that way a quitting trigger can set the page to blank
		// or something similarly intelligent. 
		virtual int execute(int& output) { return 0; };

		bool initial(int input)
		{
			if (!m_binitial)
			{
				m_in_initial = input;
				m_binitial = true;
				std::cerr << "key " << m_key << " initial=" << std::hex << input << std::endl;
			}
			return m_binitial;
		};

		std::string getKey() const { return m_key; };

	private:
		std::string m_key;
		int m_in_mask;
		int m_in_val;
		int m_in_last;	// last value of this trigger's input&m_in_mask (initial = 0) 
		int m_out_mask;
		int m_out_val;
		bool m_binitial;
		int m_in_initial;
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
	private:
		TriggerCallbackFunc m_callback;
	};


	class PageTrigger: public Trigger
	{
	public:
		PageTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_page) : 
		  Trigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val), m_page(i_page) {};
		~PageTrigger() {};
		virtual int execute(int& output)
		{
			std::cerr << "Set page " << m_page << std::endl;
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
			for (int i=0; i<this->size(); i++)
			{
				vsgObjSelect((*this)[i].first);
				vsgObjSetContrast((*this)[i].second);
			}
			return 1;
		};
	};

/*
	class TuningOnOffTrigger: public Trigger
	{
	public:
		TuningOnOffTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val,
								std::string i_keyON, int i_in_maskON, int i_in_valON, int i_out_maskON, int i_out_valON,
								std::string i_keyOFF, int i_in_maskOFF, int i_in_valOFF, int i_out_maskOFF, int i_out_valOFF) :
		Trigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val)
		{
			m_iTriggered = -1;
			m_ptrigON = new ContrastTrigger(i_keyON, i_in_maskON, i_in_valON, i_out_maskON, i_out_maskON);
			m_ptrigOFF = new ContrastTrigger(i_keyOFF, i_in_maskOFF, i_in_valOFF, i_out_maskOFF, i_out_maskOFF);
		};


		~TuningOnOffTrigger() 
		{
			delete m_ptrigON;
			delete m_ptrigOFF;
		};

		virtual bool checkAscii(std::string input)
		{
			bool bval=false;
			if (Trigger::checkAscii(input))
			{
				bval = true;
				m_iTriggered = 0;
			}
			else if (m_ptrigON->checkAscii(input))
			{
				bval = true;
				m_iTriggered = 1;
			}
			else if (m_ptrigOFF->checkAscii(input))
			{
				bval = true;
				m_iTriggered = 2;
			}
			return bval;
		};

		virtual bool checkBinary(int input)
		{
			bool bval = false;
			bool bval0, bval1, bval2;

			// check each class. This ensures that the current trigger value is stored. 
			bval0 = Trigger::checkBinary(input);
			bval1 = m_ptrigON->checkBinary(input);
			bval2 = m_ptrigOFF->checkBinary(input);
			if (bval0)
			{
				m_iTriggered = 0;
				return true;
			}
			if (bval1)
			{
				m_iTriggered = 1;
				return true;
			}
			if (bval2)
			{
				m_iTriggered = 2;
				return true;
			}
			return false;
		}


		// Execute the triggers' action(s). Subclasses should return >0 if a vsgPresent() will be 
		// needed, 0 if no present(), and <0 if this trigger means quit (a call to vsgPresent() is 
		// is made in this case, too -- that way a quitting trigger can set the page to blank
		// or something similarly intelligent. 
		virtual int execute(int& output)
		{
			int ival=0;
			switch (m_iTriggered)
			{
			case 0:
				setMarker(output);
				return executeTuning();
				break;
			case 1:
				return m_ptrigON->execute(output);
				break;
			case 2:
				return m_ptrigOFF->execute(output);
				break;
			default:
				std::cerr << "Error: No execute method for internal trigger " << m_iTriggered << std::endl;
			}
			return ival;
		}


		virtual int executeTuning() = 0;

	private:
		int m_iTriggered;
		ContrastTrigger *m_ptrigON;
		ContrastTrigger *m_ptrigOFF;
	};
*/



	class TuningTrigger: public Trigger
	{
	public:
		TuningTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val) :
		  Trigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val) {};

	    virtual ~TuningTrigger() {};

		virtual int executeTuning() = 0;

		// The trigger should set the initial value for its variable parameter in the grating here. 
		virtual void initialize() = 0;

		// Execute the triggers' action(s). Subclasses should return >0 if a vsgPresent() will be 
		// needed, 0 if no present(), and <0 if this trigger means quit (a call to vsgPresent() is 
		// is made in this case, too -- that way a quitting trigger can set the page to blank
		// or something similarly intelligent. 
		virtual int execute(int& output)
		{
			setMarker(output);
			return executeTuning();
		}
	};


	class OrientationTuningTrigger : public TuningTrigger
	{
	public:
		OrientationTuningTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, ARGratingSpec& i_gr, double i_minOri, double i_maxOri, int i_nsteps) :
		TuningTrigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val), m_gr(i_gr)
		{
			double step = (i_maxOri - i_minOri)/(double)i_nsteps;
			for (int i=0; i<i_nsteps; i++)
			{
				m_vecOri.push_back(i_minOri + i*step);
			}
			m_vecOri.push_back(i_maxOri);
			m_iCurrentOri = 0;
		}

		void initialize()
		{
			m_gr.orientation = m_vecOri[0];
			m_gr.drawOnce();
		}


		int executeTuning()
		{
			m_iCurrentOri++;
			if (m_iCurrentOri >= m_vecOri.size()) m_iCurrentOri = 0;
			m_gr.orientation = m_vecOri[m_iCurrentOri];
			m_gr.draw();
			return 1;
		}


	  private:
		  std::vector<double> m_vecOri;
		  int m_iCurrentOri;
		  ARGratingSpec& m_gr;
	};


	class TriggerVector: public std::vector<Trigger*>
	{
	public:
		TriggerVector() {};
		virtual ~TriggerVector() 
		{
			for (int i=0; i<size(); i++)
			{
				delete (*this)[i];
			}
		};
		void addTrigger(Trigger* t)
		{
			push_back(t);
		};
	};

	class TriggerFunc
	{
	public:
		TriggerFunc(std::string key, int otrigger) : m_binary(false), m_skey(key), m_present(false), m_otrigger(otrigger), m_page(-1), m_quit(false) {};
		TriggerFunc(int itrigger, int otrigger) : m_binary(true), m_itrigger(itrigger), m_present(false), m_otrigger(otrigger), m_page(-1), m_quit(false) {};

		int page() { return m_page; };
		bool present() { return m_present; };
		int output_trigger() { return m_otrigger; };
		bool quit() { return m_quit; };

		void operator()(Trigger* pitem)
		{
			bool bTest=false;
			if (m_binary) bTest = pitem->checkBinary(m_itrigger);
			else bTest = pitem->checkAscii(m_skey);

			if (bTest)
			{
				int i;
				i = pitem->execute(m_otrigger);
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