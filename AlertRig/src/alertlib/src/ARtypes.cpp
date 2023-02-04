#include "ARtypes.h"
#include "coneiso.h"
#include <string>
#include <sstream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
using namespace boost::algorithm;
using namespace std;

COLOR_TYPE::COLOR_TYPE()
{
	m_type = gray;
	m_color.a = m_color.b = m_color.c = .5;
};

COLOR_TYPE::COLOR_TYPE(COLOR_ENUM t)
{
	setType(t);
}

void COLOR_TYPE::setType(COLOR_ENUM t)
{
	switch (t)
	{
	case white:
		m_type = white;
		m_color.a = m_color.b = m_color.c = 1;
		break;
	case gray:
		m_type = gray;
		m_color.a = m_color.b = m_color.c = .5;
		break;
	case red:
		m_type = red;
		m_color.a = 1;
		m_color.b = m_color.c = 0;
		break;
	case green:
		m_type = green;
		m_color.b = 1;
		m_color.a = m_color.c = 0;
		break;
	case blue:
		m_type = blue;
		m_color.c = 1;
		m_color.a = m_color.b = 0;
		break;
	case custom:
		m_type = custom;
		m_color.a = m_color.b = m_color.c = 0;
		break;
	case unknown_color:
	case black:
	default:
		m_type = black;
		m_color.a = m_color.b = m_color.c = 0;
		break;
	}
}


void COLOR_TYPE::setCustom(double a, double b, double c)
{
	m_type = custom;
	m_color.a = a;
	m_color.b = b;
	m_color.c = c;
}

void COLOR_TYPE::setCustom(double abc)
{
	m_type = custom;
	m_color.a = m_color.b = m_color.c = abc;
}


COLOR_TYPE::COLOR_TYPE(const COLOR_TYPE& ct)
	: m_type(ct.type())
	, m_color(ct.trival())
{};

std::ostream& operator<<(std::ostream& out, const COLOR_TYPE& c)
{
	switch (c.type())
	{
	case black: out << "black"; break;
	case white: out << "white"; break;
	case red:	out << "red";	break;
	case green:	out << "green";	break;
	case blue:	out << "blue";	break;
	case gray:	out << "gray";	break;
	case custom:
		break;
	default:	out << "unknown"; break;
	}

	out << "(" << (int)(c.trival().a * 255) << "/" << (int)(c.trival().b * 255) << "/"
		<< (int)(c.trival().c * 255) << ")";
	return out;
}

std::istream& operator>>(std::istream& in, COLOR_TYPE& c)
{
	std::string s;
	in >> s;
	if (parse_color(s, c))
	{
		in.setstate(std::ios::failbit);
	}
	return in;
}


std::ostream& operator<<(std::ostream& out, const COLOR_VECTOR_TYPE& cv)
{
	switch (cv.type)
	{
	case b_w:	out << "b";	break;
	case l_cone:out << "l";		break;
	case m_cone:out << "m";		break;
	case s_cone:out << "s";		break;
	case custom_color_vector:
		out << "(" << (int)(cv.from.a * 255) << "/" << (int)(cv.from.b * 255) << "/"
			<< (int)(cv.from.c * 255) << ")-(" << (int)(cv.to.a * 255) << "/"
			<< (int)(cv.to.b * 255) << "/" << (int)(cv.to.c * 255) << ")";
		break;
	default:	out << "unknown";		break;
	}
	return out;
}

std::istream& operator>>(std::istream& in, COLOR_VECTOR_TYPE& cv)
{
	std::string s;
	in >> s;
	if (parse_colorvector(s, cv))
	{
		in.setstate(std::ios::failbit);
	}
	return in;
}



std::ostream& operator<<(std::ostream& out, const APERTURE_TYPE& a)
{
	switch (a)
	{
	case rectangle:	out << "r";	break;
	case ellipse:	out << "e";	break;
	default:		out << "unknown";	break;
	}
	return out;
}

std::istream& operator>>(std::istream& in, APERTURE_TYPE& a)
{
	std::string s;
	in >> s;
	if (parse_aperture(s, a))
	{
		in.setstate(std::ios::failbit);
	}
	return in;
}

std::ostream& operator<<(std::ostream& out, const WAVEFORM_TYPE& p)
{
	switch (p)
	{
	case sinewave:		out << "s";		break;
	case squarewave:	out << "q";	break;
	default:			out << "unknown";	break;
	}
	return out;
}

int parse_colorvector(std::string s, COLOR_VECTOR_TYPE& v)
{
	int status = 0;
	if (s == "black" || s == "BLACK" ||
		s == "b" || s == "B" ||
		s == "white" || s == "WHITE" ||
		s == "w" || s == "W" ||
		s == "gray" || s == "GRAY" ||
		s == "g" || s == "G") v.type = b_w;
	else if (s == "l" || s == "L") v.type = l_cone;
	else if (s == "m" || s == "M") v.type = m_cone;
	else if (s == "s" || s == "S") v.type = s_cone;
	else
	{
		int n;
		int fr, fg, fb, tr, tb, tg;
		// try and parse a custom color vector
		status = 1;
		v.type = unknown_color_vector;
		n = sscanf_s(s.c_str(), "(%d/%d/%d)-(%d/%d/%d)", &fr, &fg, &fb, &tr, &tg, &tb);
		if (n == 6)
		{
			if (fr >= 0 && fr < 256 && fg >= 0 && fg < 256 && fb >= 0 && fb < 256 &&
				tr >= 0 && tr < 256 && tg >= 0 && tg < 256 && tb >= 0 && tb < 256)
			{
				status = 0;
				v.type = custom_color_vector;
				v.from.a = (double)fr / 255.0;
				v.from.b = (double)fg / 255.0;
				v.from.c = (double)fb / 255.0;
				v.to.a = (double)tr / 255.0;
				v.to.b = (double)tg / 255.0;
				v.to.c = (double)tb / 255.0;
			}
		}
		else
		{
			double lfr, lfg, lfb, ltr, ltb, ltg;
			n = sscanf_s(s.c_str(), "[%lf/%lf/%lf]-[%lf/%lf/%lf]", &lfr, &lfg, &lfb, &ltr, &ltg, &ltb);
			if (n == 6)
			{
				if (lfr >= 0 && lfr <= 1.0 && lfg >= 0 && lfg <= 1.0 && lfb >= 0 && lfb <= 1.0 &&
					ltr >= 0 && ltr <= 1.0 && ltg >= 0 && ltg <= 1.0 && ltb >= 0 && ltb <= 1.0)
				{
					status = 0;
					v.type = custom_color_vector;
					v.from.a = lfr;
					v.from.b = lfg;
					v.from.c = lfb;
					v.to.a = ltr;
					v.to.b = ltg;
					v.to.c = ltb;
				}
			}
		}
	}
	return status;
}

COLOR_TYPE& COLOR_TYPE::operator=(const COLOR_TYPE& ct)
{
	if (this == &ct) return *this;
	m_type = ct.type();
	m_color = ct.trival();
	return *this;
};

COLOR_TYPE& COLOR_TYPE::operator=(const COLOR_ENUM& t)
{
	setType(t);
	return *this;
}

bool operator==(const COLOR_TYPE& lhs, const COLOR_TYPE& rhs)
{
	return (lhs.type() == rhs.type() &&
		lhs.trival().a == rhs.trival().a &&
		lhs.trival().b == rhs.trival().b &&
		lhs.trival().c == rhs.trival().c);
}



int parse_color(std::string s, COLOR_TYPE& c)
{
	int status = 0;
	if (s == "black" || s == "BLACK") c.setType(black);
	else if (s == "white" || s == "WHITE") c.setType(white);
	else if (s == "gray" || s == "GRAY") c.setType(gray);
	else if (s == "red" || s == "RED" || s == "r" || s == "R") c.setType(red);
	else if (s == "green" || s == "GREEN" || s == "g" || s == "G") c.setType(green);
	else if (s == "blue" || s == "BLUE" || s == "b" || s == "B") c.setType(blue);
	else
	{
		int r, g, b;
		double dr, dg, db;
		boost::regex exprParen{ "\\s*[(](\\d+)[/](\\d+)[/](\\d+)[)]" };
		// floating point ((\d+\.?\d*)|(\.\d+))
		boost::regex exprBrack{ "\\s*[\\[]((\\d+\\.?\\d*)|(\\.\\d+))[/]((\\d+\\.?\\d*)|(\\.\\d+))[/]((\\d+\\.?\\d*)|(\\.\\d+))[\\]]\\s*" };
		boost::smatch what;
		status = 1;
		if (boost::regex_search(s, what, exprParen))
		{
			std::stringstream ss;
			ss << what[1] << " " << what[2] << " " << what[3];
			ss >> r >> g >> b;
			if (r > -1 && r<256 && g>-1 && g<256 && b>-1 && b < 256)
			{
				c.setCustom((double)r / 255.0, (double)g / 255.0, (double)b / 255.0);
				status = 0;
			}
		}
		else if (boost::regex_search(s, what, exprBrack))
		{
			std::stringstream ss;
			ss << what[1] << " " << what[4] << " " << what[7];
			ss >> dr >> dg >> db;
			if (dr>=0 && dr<=1.0 && dg>=0 && dg<=1.0 && db>=0 && db<=1.0)
			{
				c.setCustom(dr, dg, db);
				status = 0;
			}
		}
	}
	return status;
}



int parse_contrast_triplet(std::string s, int& i_iContrastDown, int& i_iContrastBase, int& i_iContrastUp)
{
	int status = 0;
	vector<string> tokens;
	tokenize(s, tokens, ",");
	if (tokens.size() != 3)
	{
		cerr << "Bad contrast triplet format: " << s << endl;
		status = 1;
	}
	else
	{
		istringstream iss;
		iss.str(tokens[0]);
		iss >> i_iContrastDown;
		if (!iss)
		{
			cerr << "bad ContrastDown: " << tokens[0] << endl;
			status = 1;
		}
		iss.str(tokens[1]);
		iss.clear();
		iss >> i_iContrastBase;
		if (!iss)
		{
			cerr << "bad ContrastBase: " << tokens[1] << endl;
			status = 1;
		}
		iss.str(tokens[2]);
		iss.clear();
		iss >> i_iContrastUp;
		if (!iss)
		{
			cerr << "bad ContrastUp: " << tokens[2] << endl;
			status = 1;
		}
	}
	return status;
}


int parse_int_pair(std::string s, int& i_i1, int& i_i2)
{
	int status = 0;
	vector<string> tokens;
	tokenize(s, tokens, ",");
	if (tokens.size() != 2)
	{
		cerr << "Bad format: " << s << endl;
		status = 1;
	}
	else
	{
		istringstream iss;
		iss.str(tokens[0]);
		iss >> i_i1;
		if (!iss)
		{
			cerr << "bad int: " << tokens[0] << endl;
			status = 1;
		}
		iss.str(tokens[1]);
		iss.clear();
		iss >> i_i2;
		if (!iss)
		{
			cerr << "bad int: " << tokens[1] << endl;
			status = 1;
		}
	}
	return status;
}

int parse_sequence_pair(std::string s, int& i_i1, int& i_i2)
{
	int status = 0;
	status = parse_int_pair(s, i_i1, i_i2);
	if (!status)
	{
		if (i_i2 < i_i1)
		{
			cerr << "bad sequence pair: second term < first_term" << endl;
			status = 1;
		}
		else if (i_i1 < 0)
		{
			cerr << "bad sequence pair: both terms must be >= 0!" << endl;
			status = 1;
		}
	}
	return status;
}


int parse_tuning_triplet(std::string s, double& i_dMin, double& i_dMax, int& i_iSteps)
{
	int status = 0;
	vector<string> tokens;
	tokenize(s, tokens, ",");
	if (tokens.size() != 3)
	{
		cerr << "Bad tuning triplet format: " << s << endl;
		status = 1;
	}
	else
	{
		istringstream iss;
		iss.str(tokens[0]);
		iss >> i_dMin;
		if (!iss)
		{
			cerr << "bad tuning Min value: " << tokens[0] << endl;
			status = 1;
		}
		iss.str(tokens[1]);
		iss.clear();
		iss >> i_dMax;
		if (!iss)
		{
			cerr << "bad tuning Max value: " << tokens[1] << endl;
			status = 1;
		}
		iss.str(tokens[2]);
		iss.clear();
		iss >> i_iSteps;
		if (!iss)
		{
			cerr << "bad tuning #steps value: " << tokens[2] << endl;
			status = 1;
		}
	}
	return status;
}

int parse_triplet(std::string s, double& i_d1, double& i_d2, double& i_d3)
{
	int status = 0;
	vector<string> tokens;
	tokenize(s, tokens, ",");
	if (tokens.size() != 3)
	{
		cerr << "Bad triplet format: " << s << endl;
		status = 1;
	}
	else
	{
		istringstream iss;
		iss.str(tokens[0]);
		iss >> i_d1;
		if (!iss)
		{
			cerr << "bad triplet value: " << tokens[0] << endl;
			status = 1;
		}
		iss.str(tokens[1]);
		iss.clear();
		iss >> i_d2;
		if (!iss)
		{
			cerr << "bad triplet value: " << tokens[1] << endl;
			status = 1;
		}
		iss.str(tokens[2]);
		iss.clear();
		iss >> i_d3;
		if (!iss)
		{
			cerr << "bad triplet value: " << tokens[2] << endl;
			status = 1;
		}
	}
	return status;
}

int parse_int_list(std::string s, std::vector<int>& list)
{
	int status = 0;
	unsigned int i;
	istringstream iss;
	int num;
	vector<string> tokens;
	tokenize(s, tokens, ",");
	for (i = 0; i < tokens.size(); i++)
	{
		iss.clear();
		iss.str(tokens[i]);
		iss >> num;
		if (!iss)
		{
			cerr << "bad tuning value: " << tokens[i] << endl;
			status = 1;
		}
		else
		{
			list.push_back(num);
		}
	}
	return status;
}

int parse_int_list(vector<string>& tokens, vector<int>& int_list)
{
	int status = 0;
	unsigned int i;
	istringstream iss;
	int n;
	for (i = 0; i < tokens.size() && status == 0; i++)
	{
		iss.clear();
		iss.str(tokens[i]);
		iss >> n;
		if (!iss)
		{
			status = 1;
			cerr << "Bad int value: " << tokens[i] << endl;
		}
		else
		{
			int_list.push_back(n);
		}
	}
	return status;
}



int parse_tuning_list(std::string s, vector<double>& tuning_list, int& i_iSteps)
{
	int status = 0;
	vector<string> tokens;
	tokenize(s, tokens, ",");
	return parse_tuning_list(tokens, tuning_list, i_iSteps);
}

int parse_tuning_list(vector<string>& tokens, vector<double>& tuning_list, int& i_iSteps)
{
	int status = 0;
	unsigned int i;
	istringstream iss;
	double d;
	for (i = 0; i < tokens.size(); i++)
	{
		iss.clear();
		iss.str(tokens[i]);
		iss >> d;
		if (!iss)
		{
			cerr << "bad tuning value: " << tokens[i] << endl;
			status = 1;
		}
		else
		{
			tuning_list.push_back(d);
		}
	}
	// Number of steps is the size of the list MINUS 1!
	i_iSteps = (int)tuning_list.size() - 1;
	return status;
}

int parse_color_list(std::string s, vector<COLOR_TYPE>& color_list)
{
	int status = 0;
	vector<string> tokens;
	tokenize(s, tokens, ",");
	return parse_color_list(tokens, color_list);
}

int parse_color_list(vector<string>& tokens, vector<COLOR_TYPE>& color_list)
{
	int status = 0;
	unsigned int i;
	istringstream iss;
	COLOR_TYPE color;
	for (i = 0; i < tokens.size(); i++)
	{
		iss.clear();
		iss.str(tokens[i]);
		iss >> color;
		if (!iss)
		{
			cerr << "bad color value: " << tokens[i] << endl;
			status = 1;
		}
		else
		{
			color_list.push_back(color);
		}
	}
	return status;
}

// Just like parse_tuning_list, but doesn't return error when it hits a bad value. 
// Used for parsing grating spec, where there may be a variable number of numbers
// before some characters. Fill the vec with numbers but don't err when a non-number 
// is found. Always return 0, since its not considered an error to have a 
// non-number in the list. 

int parse_number_list(std::string s, vector<double>& number_list)
{
	int status = 0;
	vector<string> tokens;
	tokenize(s, tokens, ",");
	parse_number_list(tokens, number_list);
	return 0;
}

// NOte: same as above - return 0. Stops at first non-number in list.
int parse_number_list(vector<string>& tokens, vector<double>& number_list)
{
	int status = 0;
	unsigned int i;
	istringstream iss;
	double d;
	for (i = 0; i < tokens.size() && status == 0; i++)
	{
		iss.clear();
		iss.str(tokens[i]);
		iss >> d;
		if (!iss)
		{
			status = 1;
		}
		else
		{
			number_list.push_back(d);
		}
	}
	return 0;
}


// input string is a filename or a character string. 
// If a filename the arg should be file=filename, contents should be a character string. 
// sequence strings should be 10101010-ish, no commas. 
// No restriction on characters allowed. 
int parse_sequence(std::string s, std::string& seq)
{
	int status = 0;
	std::size_t found = s.find_first_of("=");
	seq.clear();
	if (found == std::string::npos)
	{
		// expecting sequence on command line. Just copy the string
		seq = s;
	}
	else
	{
		std::ifstream input(s.substr(found + 1).c_str());
		if (input.is_open())
		{
			input >> seq;
		}
		else
		{
			cerr << "Cannot open file \"" << s.substr(found + 1) << "\"" << endl;
			status = 1;
		}
	}
	cerr << "Loaded " << seq.length() << " terms." << endl;
	return status;
}


// trim, remove quotes (if they enclose the string), and trim again.
int parse_string(std::string s, std::string& s2)
{
	s2 = s;

	// trim
	trim(s2);

	// remove enclosing quotes. If a leading quote is found it is removed. 
	// Same for trailing quote. If you get one and not the other, just one is 
	// removed. 
	trim_left_if(s2, is_any_of("\"'"));
	trim_right_if(s2, is_any_of("\"'"));

	// trim again
	trim(s2);

	return 0;
}

#if 0
int get_color(COLOR_TYPE c, VSGTRIVAL& trival)
{
	int status = 0;
	COLOR_TYPE

		switch (c.type())
		{
		case black:
			trival.a = 0;
			trival.b = 0;
			trival.c = 0;
			break;
		case white:
			trival.a = 1;
			trival.b = 1;
			trival.c = 1;
			break;
		case gray:
			trival.a = 0.5;
			trival.b = 0.5;
			trival.c = 0.5;
			break;
		case red:
			trival.a = 1;
			trival.b = 0;
			trival.c = 0;
			break;
		case green:
			trival.a = 0;
			trival.b = 1;
			trival.c = 0;
			break;
		case blue:
			trival.a = 0;
			trival.b = 0;
			trival.c = 1;
			break;
		case custom:
			trival = c.color;
			break;
		default:
			status = 1;
		}

	return status;
}
#endif


int get_colorvector(COLOR_VECTOR_TYPE& cv, VSGTRIVAL& from, VSGTRIVAL& to)
{
	int status = 0;
	switch (cv.type)
	{
	case b_w:
		from.a = from.b = from.c = 0;
		to.a = to.b = to.c = 1;
		break;
	case l_cone:
		if (!coneiso_l(from, to)) status = 1;
		break;
	case m_cone:
		if (!coneiso_m(from, to)) status = 1;
		break;
	case s_cone:
		if (!coneiso_s(from, to)) status = 1;
		break;
	case custom_color_vector:
		from = cv.from;
		to = cv.to;
		break;
	default:
		status = 1;
		break;
	}

	return status;
}

int parse_waveform_types(std::string s, WAVEFORM_TYPE& swt, WAVEFORM_TYPE& twt)
{
	int status = 0;
	if (s.length() == 0 || s.length() > 2)
	{
		return 1;
	}
	else
	{
		if (s[0] == 'q' || s[0] == 'Q') swt = squarewave;
		else if (s[0] == 's' || s[0] == 'S') swt = sinewave;
		if (s.length() == 1)
		{
			twt = sinewave;
		}
		else
		{
			if (s[1] == 'q' || s[1] == 'Q') twt = squarewave;
			else if (s[1] == 's' || s[1] == 'S') twt = sinewave;
		}
	}
	return status;
}


int parse_aperture(std::string s, APERTURE_TYPE& a)
{
	int status = 0;
	if (s == "r" || s == "R") a = rectangle;
	else if (s == "e" || s == "E" ||
		s == "c" || s == "C") a = ellipse;
	else
	{
		a = unknown_aperture;
		status = 1;
	}
	return status;
}


int parse_integer(std::string s, int& i)
{
	int status = 0;
	istringstream iss(s);
	iss >> i;
	if (!iss)
	{
		cerr << "bad integer: " << s << endl;
		status = 1;
	}
	return status;
}

int parse_ulong(std::string s, unsigned long& l)
{
	int status = 0;
	istringstream iss(s);
	iss >> l;
	if (!iss)
	{
		cerr << "bad unsigned long: " << s << endl;
		status = 1;
	}
	return status;
}

int parse_uint(std::string s, unsigned int& ui)
{
	int status = 0;
	istringstream iss(s);
	iss >> ui;
	if (!iss)
	{
		cerr << "bad unsigned int: " << s << endl;
		status = 1;
	}
	return status;
}


int parse_xy(std::string s, double& x, double& y)
{
	int status = 0;
	vector<string> tokens;
	tokenize(s, tokens, ",");
	if (tokens.size() != 2)
	{
		cerr << "Bad x,y format: " << s << endl;
		status = 1;
	}
	else
	{
		istringstream iss;
		iss.str(tokens[0]);
		iss >> x;
		if (!iss)
		{
			cerr << "bad x: " << tokens[0] << endl;
			status = 1;
		}
		iss.str(tokens[1]);
		iss.clear();
		iss >> y;
		if (!iss)
		{
			cerr << "bad y: " << tokens[1] << endl;
			status = 1;
		}
	}
	return status;
}


int parse_distance(std::string s, int& i)
{
	return parse_integer(s, i);
}

int parse_double(std::string s, double& d)
{
	int status = 0;
	istringstream iss(s);
	iss >> d;
	if (!iss)
	{
		cerr << "bad format for double: " << s << endl;
		status = 1;
	}
	return status;
}

#if 0

void tokenize(const string& str, vector<string>& tokens, const string& delimiters = " ")
{
	// Skip delimiters at beginning.
	string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	string::size_type pos = str.find_first_of(delimiters, lastPos);

	while (string::npos != pos || string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
}
#else

// djs - Update tokenize to handle quoted strings e.g. "this is a string"
void tokenize(const string& str, vector<string>& tokens, const string& delimiters = " ")
{
	//	size_t split(char *buffer, char *argv[], size_t argv_size)
	//{
	string::const_iterator it = str.begin();
	string::const_iterator itStartOfToken;
	int c;
	enum states { IN_DELIM, IN_WORD, IN_STRING } state = IN_DELIM;
	//	cout << "Tokenize str>>" << str << "<<" << endl;
	for (it = str.begin(); it != str.end(); it++)
	{
		c = (unsigned char)*it;

		//		cout << "char " << *it << " state " << state << endl;

		switch (state) {
		case IN_DELIM:
			if (delimiters.find_first_of(c) < string::npos)
			{
				continue;
			}

			if (c == '"')
			{
				state = IN_STRING;
				itStartOfToken = it + 1;
				//				cout << "start of string" << endl;
				continue;
			}
			state = IN_WORD;
			itStartOfToken = it;
			//			cout << "Start of word" << endl;
			continue;

		case IN_STRING:
			if (c == '"')
			{
				tokens.push_back(string(itStartOfToken, it));
				//				cout << "end of string: " << string(itStartOfToken, it) << endl;
				state = IN_DELIM;
			}
			continue;

		case IN_WORD:
			if (delimiters.find_first_of(c) == string::npos)
			{
				continue;
			}
			else
			{
				tokens.push_back(string(itStartOfToken, it));
				state = IN_DELIM;
				//				cout << "end of word: " << string(itStartOfToken, it) << endl;
			}
			continue;
		}
	}

	if (state != IN_DELIM)
	{
		tokens.push_back(string(itStartOfToken, str.end()));
		//		cout << "end of token at end of input: " << string(itStartOfToken, str.end()) << endl;
	}
	//	cout << "done." << endl;
	return;
}

#endif


void make_argv(std::ifstream& ifs, int& argc, char** argv)
{
	// Read the whole file into a string
	stringstream ss;
	ss << ifs.rdbuf();
	string contents(ss.str());
	make_argv(contents, argc, argv);
	return;
}

/* Warning! Make sure you prepend the progname to your string!!! */

void make_argv(const std::string& str, int& argc, char** argv)
{
	vector<string> tokens;
	tokenize(str, tokens, " \n\t");
	make_argv(tokens, argc, argv);
	return;
}


/*ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
All of these automatically add the program name, and assume that the first element
of args (the progname) has already been stripped.
*/

void make_argv(vector<string>tokens, int& argc, char** argv)
{
	unsigned int i;
	/*
	 * djs 9-4-2011 Change this to NOT add a program name entry prior to the passed tokens
	 * Instead, just copy all tokens. Caller MUST pre-pend the progname.
	argv[0] = (char *)malloc(9);
	strcpy_s(argv[0], 9, "PROGNAME");
	*/
	for (i = 0; i < tokens.size(); i++)
	{
		argv[i] = (char*)malloc(tokens[i].length() + 1);
		strcpy_s(argv[i], tokens[i].length() + 1, tokens[i].c_str());
	}
	argc = (int)tokens.size();
}

void free_argv(int& argc, char** argv)
{
	int i;
	for (i = 0; i < argc; i++)
	{
		free(argv[i]);
		argv[i] = NULL;
	}
	argc = 0;
}

// tokenize all lines in filename, separated on spaces. 
// Lines beginning with # are ignored and not counted. 
// Returns number of non-# lines read from file. Return -1 if file cannot be opened. 

int tokenize_response_file(char* filename, vector<string>& tokens)
{
	vector<string> tmpTokens;
	int count = 0;
	ifstream ifs(filename);
	string s;
	if (!ifs.is_open())
	{
		s.assign(filename);
		cerr << "ERROR: Cannot open response file " << s << endl;
		return -1;
	}

	getline(ifs, s);
	while (ifs)
	{
		if (s[0] != '#')
		{
			count++;
			tokenize(s, tokens, " ");
		}
		getline(ifs, s);
	}
	return count;
}
