#if !defined(_ALERTLIB_H_)
#define _ALERTLIB_H_

#pragma warning(disable:4786)


#include "VSGEX2.H"
#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <ostream>

// These typedefs are used in all the specs. 
typedef enum colorvectortype { unknown_color_vector=0, b_w, l_cone, m_cone, s_cone } COLOR_VECTOR_TYPE;
typedef enum colortype { unknown_color=0, black=1, white=2, red=3, green=4, blue=5, gray=6 } COLOR_TYPE;
typedef enum patterntype { unknown_pattern=0, sinewave, squarewave } PATTERN_TYPE;
typedef enum aperturetype { unknown_aperture=0, ellipse, rectangle } APERTURE_TYPE;
typedef int PIXEL_LEVEL;


// useful helper functions
int parse_color(std::string s, COLOR_TYPE& c);
int get_color(COLOR_TYPE& c, VSGTRIVAL& trival);
int get_colorvector(COLOR_VECTOR_TYPE& cv, VSGTRIVAL& from, VSGTRIVAL& to);
int parse_colorvector(std::string s, COLOR_VECTOR_TYPE& v);
int parse_pattern(std::string s, PATTERN_TYPE& p);
int parse_aperture(std::string s, APERTURE_TYPE& a);
int parse_distance(std::string s, int& dist);
int parse_contrast_triplet(std::string s, int& i_iContrastDown, int& i_iContrastBase, int& i_iContrastUp);
void tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters);


std::ostream& operator<<(std::ostream& out, const COLOR_TYPE& c);
std::ostream& operator<<(std::ostream& out, const COLOR_VECTOR_TYPE& v);
std::ostream& operator<<(std::ostream& out, const APERTURE_TYPE& a);
std::ostream& operator<<(std::ostream& out, const PATTERN_TYPE& p);




/* Initialize vsg card. Leaves card with palette ramp, page 0 cleared to background color (use vsgBACKGROUND). 
 * Clears video page 0 and displays it. Suitable for initialization on startup of a stimulus app, or for cleanup on exit (or use clear_vsg()).
 */
int init_vsg(int screenDistanceMM, COLOR_TYPE i_background);

/* Convenience */
void clear_vsg();


namespace alert
{


	// Base class for specs

	class ARSpec
	{
	public:
		ARSpec() {};
		virtual ~ARSpec() {};
		virtual int draw() = 0;
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
	};

	// Fixation point for cases where its visibility is controlled by contrast setting 

	class ARContrastFixationPointSpec: public ARFixationPointSpec
	{
	public:
		ARContrastFixationPointSpec() : background(gray) {};
		~ARContrastFixationPointSpec() {};
		COLOR_TYPE background;
		int draw();
	};



	// Grating spec
	class ARGratingSpec: public ARSpec
	{
	public:
		ARGratingSpec() {};
		~ARGratingSpec() {};
		double x,y,w,h;
		double sf, tf;
		double orientation;
		int contrast;
		PATTERN_TYPE pattern;
		APERTURE_TYPE aperture;
		COLOR_VECTOR_TYPE cv;
		int draw();
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
			if (current != m_in_last && current == m_in_val)
			{
				bValue = true;
			}
			m_in_last = current;
			return bValue;
		};

		virtual void setMarker(int& output)
		{
			if (m_out_val > 0)
			{
				output = m_out_val | (output&(~m_out_mask));
			}
		};


		// Execute the triggers' action(s). Subclasses should return >0 if a vsgPresent() will be 
		// needed, 0 if no present(), and <0 if this trigger means quit (a call to vsgPresent() is 
		// is made in this case, too -- that way a quitting trigger can set the page to blank
		// or something similarly intelligent. 
		virtual int execute(int& output) { return 0; };

	private:
		std::string m_key;
		int m_in_mask;
		int m_in_val;
		int m_in_last;	// last value of this trigger's input&m_in_mask (initial = 0) 
		int m_out_mask;
		int m_out_val;
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
			for (int i=0; i<this->size(); i++)
			{
				vsgObjSelect((*this)[i].first);
				vsgObjSetContrast((*this)[i].second);
			}
			return 1;
		};
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
		TriggerFunc(std::string key) : m_binary(false), m_skey(key), m_present(false), m_otrigger(0), m_page(-1), m_quit(false) {};
		TriggerFunc(int itrigger) : m_binary(true), m_itrigger(itrigger), m_present(false), m_otrigger(0), m_page(-1), m_quit(false) {};

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

// instead of input operators, methods
int parse_fixation_point(const std::string& s, alert::ARFixationPointSpec& afp);
int parse_grating(const std::string& s, alert::ARGratingSpec& ag);











#if 0


// this struct holds parameters for a fixation point. The draw() method puts it on a 
// single level with no vsg object stuff. 
class AlertFixationPoint
{
public:
	AlertFixationPoint() {};
	~AlertFixationPoint() {};
	double x, y;
	double d;
	COLOR_TYPE color;
	void draw(PIXEL_LEVEL level);
};


class AlertFixationPointObj: public AlertFixationPoint, AlertObj
{
public:
	AlertFixationPointObj() : m_handle(false), m_levels(false), m_drawn(false) {};
	~AlertFixationPointObj() {};
	void setLevels(PIXEL_LEVEL first, PIXEL_LEVEL last, COLOR_TYPE endpoint);
	void draw(int contrast);
	void draw();
private:
	bool m_handle;
	VSGOBJHANDLE handle;
	bool m_levels;
	bool m_drawn;
};
	



class AlertGrating
{
public:
	AlertGrating(): m_handle(false), m_drawn(false) {};
	~AlertGrating() {};
	double x,y,w,h;
	double sf, tf;
	double orientation;
	int contrast;
	PATTERN_TYPE pattern;
	APERTURE_TYPE aperture;
	COLOR_VECTOR_TYPE cv;
	void draw(PIXEL_LEVEL first, PIXEL_LEVEL last);
	void draw(PIXEL_LEVEL first, PIXEL_LEVEL last, int contrast);
private:
	VSGOBJHANDLE handle;
	bool m_handle;
	bool m_drawn;
};





std::ostream& operator<<(std::ostream& out, const AlertFixationPoint& afp);
std::ostream& operator<<(std::ostream& out, const AlertGrating& ag);


int parse_fixation_point(const std::string& s, AlertFixationPoint& afp);
int parse_grating(const std::string& s, AlertGrating& ag);


#endif



#endif