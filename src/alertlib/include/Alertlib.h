#if !defined(_ALERTLIB_H_)
#define _ALERTLIB_H_

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
void tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters);

/* Initialize vsg card. Leaves card with palette ramp, page 0 cleared to background color (use vsgBACKGROUND). 
 * Clears video page 0 and displays it. Suitable for initialization on startup of a stimulus app, or for cleanup on exit (or use clear_vsg()).
 */
int init_vsg(int screenDistanceMM, COLOR_TYPE i_background);

/* Convenience */
void clear_vsg();


// this struct holds parameters for a fixation point
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


class AlertGrating
{
public:
	AlertGrating(): m_handle(false) {};
	~AlertGrating() {};
	double x,y,w,h;
	double sf, tf;
	double orientation;
	int contrast;
	PATTERN_TYPE pattern;
	APERTURE_TYPE aperture;
	COLOR_VECTOR_TYPE cv;
	void draw(PIXEL_LEVEL first, PIXEL_LEVEL last);
private:
	VSGOBJHANDLE handle;
	bool m_handle;
};


std::ostream& operator<<(std::ostream& out, const AlertFixationPoint& afp);
std::ostream& operator<<(std::ostream& out, const AlertGrating& ag);
std::ostream& operator<<(std::ostream& out, const COLOR_TYPE& c);
std::ostream& operator<<(std::ostream& out, const COLOR_VECTOR_TYPE& v);
std::ostream& operator<<(std::ostream& out, const APERTURE_TYPE& a);
std::ostream& operator<<(std::ostream& out, const PATTERN_TYPE& p);


int parse_fixation_point(const std::string& s, AlertFixationPoint& afp);
int parse_grating(const std::string& s, AlertGrating& ag);



class LevelManager
{
public:
	static LevelManager& instance() { static LevelManager m_instance; return m_instance; };
	~LevelManager() {};
	int request_single(PIXEL_LEVEL& level);
	int request_range(int num, PIXEL_LEVEL& first, PIXEL_LEVEL& last);
	int remaining() { return 251-m_next; }
private:
	LevelManager(): m_next(0) {};
	int m_next;
};


class Trigger
{
public:
	Trigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_page) 
	{
		m_key = i_key;
		m_in_mask = i_in_mask;
		m_in_val = i_in_val;
		m_out_mask = i_out_mask;
		m_out_val = i_out_val;
		m_page = i_page;
	};

	~Trigger() {};
	bool isQuit() { return m_page < 0; };
	int page() { return m_page; };
	virtual bool checkAscii(std::string input);
	virtual bool checkBinary(int input);
	virtual void execute(int& output);
private:
	std::string m_key;
	int m_in_mask;
	int m_in_val;
	int m_in_last;	// last value of this trigger's input&m_in_mask (initial = 0) 
	int m_out_mask;
	int m_out_val;
	int m_page;
};

//typedef std::vector<Trigger> TriggerVector;
class TriggerVector: public std::vector<Trigger>
{
public:
	TriggerVector() {};
	virtual ~TriggerVector() {};
	void addTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_page) 
	{
		Trigger t(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val, i_page);
		push_back(t);
	}
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
	void operator()(Trigger& item)
	{
		bool bTest=false;
		if (m_binary) bTest = item.checkBinary(m_itrigger);
		else bTest = item.checkAscii(m_skey);

		if (bTest)
		{
			if (item.isQuit()) m_quit = true;
			else
			{
				item.execute(m_otrigger);
				if (item.page()>=0)
				{
					m_present = true;
					if (m_page<0)
					{
						m_page = item.page();
					}
					else if (m_page != item.page())
					{
						std::cerr << "Warning: changing triggered page from " << m_page << " to " << item.page() << std::endl;
						m_page = item.page();
					}
				}
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

#endif