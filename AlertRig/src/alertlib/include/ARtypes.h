#pragma once
#include "VSGV8.H"
#include <iostream>
#include <vector>

// This macro returns true for a Visage, false for a 2/5 card. 
#define IS_VISAGE (vsgGetSystemAttribute(vsgDEVICECLASS)==7)

// Set this bit in the out_val of a trigger to indicate it should be toggled.
// The given out_val (without the toggle bit) will be the first value triggered.
// After that the value will toggle.
#define AR_TRIGGER_TOGGLE 0x8000

// set this bit in the in_val of a trigger to indicate it is NOT to be tested as a binary
// trigger. This would be for an ascii-only trigger (which has no corresponding binary trigger)
// added to MISOC trigger (multi-input-single-output-callback)
#define AR_TRIGGER_ASCII_ONLY 0x4000

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
typedef enum { unknown_color_vector, b_w, l_cone, m_cone, s_cone, custom_color_vector } COLOR_VECTOR_ENUM;
typedef enum { unknown_color = 0, black = 1, white = 2, red = 3, green = 4, blue = 5, gray = 6, custom = 7 } COLOR_ENUM;
typedef enum waveform_type { unknown_spatial_waveform = 0, sinewave, squarewave } WAVEFORM_TYPE;
typedef enum aperturetype { unknown_aperture = 0, ellipse, rectangle, oriented_rectangle } APERTURE_TYPE;
typedef int PIXEL_LEVEL;

//typedef struct color_vector_struct
//{
//	COLOR_VECTOR_ENUM type;
//	VSGTRIVAL from;
//	VSGTRIVAL to;
//} COLOR_VECTOR_TYPE;

class COLOR_VECTOR_TYPE
{
	COLOR_VECTOR_ENUM m_type;
	VSGTRIVAL m_from;
	VSGTRIVAL m_to;

public:
	COLOR_VECTOR_TYPE();
	COLOR_VECTOR_TYPE(COLOR_VECTOR_ENUM type);
	COLOR_VECTOR_TYPE(const COLOR_VECTOR_TYPE& cv);
	COLOR_VECTOR_TYPE& operator=(const COLOR_VECTOR_TYPE& cv);
	COLOR_VECTOR_TYPE& operator=(const COLOR_VECTOR_ENUM& t);
	COLOR_VECTOR_ENUM type() const { return m_type; };
	VSGTRIVAL from() const { return m_from; };
	VSGTRIVAL to() const { return m_to; };
	void setType(COLOR_VECTOR_ENUM t);
	void setCustom(const VSGTRIVAL& from, const VSGTRIVAL& to);
};

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
	COLOR_TYPE(double a, double b, double c);
	virtual ~COLOR_TYPE() {};
	COLOR_TYPE& operator=(const COLOR_TYPE& ct);
	COLOR_TYPE& operator=(const COLOR_ENUM& t);
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

typedef void (*voidfunc)(int, void*);

// convenient operators
std::ostream& operator<<(std::ostream& out, const COLOR_TYPE& c);
std::istream& operator>>(std::istream& in, COLOR_TYPE& c);
std::ostream& operator<<(std::ostream& out, const COLOR_VECTOR_TYPE& v);
std::istream& operator>>(std::istream& in, COLOR_VECTOR_TYPE& v);
std::ostream& operator<<(std::ostream& out, const APERTURE_TYPE& a);
std::istream& operator>>(std::istream& in, APERTURE_TYPE& a);
std::ostream& operator<<(std::ostream& out, const WAVEFORM_TYPE& p);

// useful helper functions
int parse_color(std::string s, COLOR_TYPE& c);
// SEE COLOR_TYPE::trival() int get_color(COLOR_TYPE c, VSGTRIVAL& trival);
int get_colorvector(COLOR_VECTOR_TYPE& cv, VSGTRIVAL& from, VSGTRIVAL& to);
int parse_colorvector(std::string s, COLOR_VECTOR_TYPE& v);
int parse_colorvector_list(std::string s, std::vector<COLOR_VECTOR_TYPE>& colorvector_list);
int parse_colorvector_list(std::vector<std::string>& tokens, std::vector<COLOR_VECTOR_TYPE>& colorvector_list);
int parse_waveform_types(std::string s, WAVEFORM_TYPE& swt, WAVEFORM_TYPE& twt);
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
int parse_color_list(std::string s, std::vector<COLOR_TYPE>& color_list);
int parse_color_list(std::vector<std::string>& tokens, std::vector<COLOR_TYPE>& tuning_list);
int parse_number_list(std::string s, std::vector<double>& number_list);
int parse_number_list(std::vector<std::string>& tokens, std::vector<double>& number_list);
int parse_xy(std::string s, double& x, double& y);
int parse_triplet(std::string s, double& d1, double& d2, double& d3);
int parse_sequence(std::string s, std::string& seq);
int parse_string(std::string s, std::string& scleaned);
void tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters);
void make_argv(const std::string& str, int& argc, char** argv);
void make_argv(std::vector<std::string>tokens, int& argc, char** argv);
void make_argv(std::ifstream& ifs, int& argc, char** argv);
void free_argv(int& argc, char** argv);
