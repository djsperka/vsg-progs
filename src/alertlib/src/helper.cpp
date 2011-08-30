#include "Alertlib.h"
#include "coneiso.h"
#include <vector>
#include <string>
#include <sstream>

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")


using namespace std;
using namespace alert;

int parse_fixation_point(const std::string& s, alert::ARFixationPointSpec& afp)
{
	int status=0;
	vector<string> tokens;
	tokenize(s, tokens, ",");

	// Expected format for fixation point is 
	// x,y,diameter,color
	// If color is omitted, it is assumed to be RED (configurable as a default?)
	// x,y,diameter are required, parsed as floating point. 

	if (tokens.size() < 3 || tokens.size() > 4)
	{
		status=1;	// bad format
	}
	else
	{
		istringstream iss;
		iss.str(tokens[0]);
		iss >> afp.x;
		if (!iss) 
		{
			cerr << "bad x: " << tokens[0] << endl;
			status=1;
		}
		iss.str(tokens[1]); 
		iss.clear();
		iss >> afp.y;
		if (!iss) 
		{
			cerr << "bad y: " << tokens[1] << endl;
			status=1;
		}
		iss.str(tokens[2]);
		iss.clear();
		iss >> afp.d;
		if (!iss) 
		{
			cerr << "bad diameter: " << tokens[2] << endl;
			status=1;
		}

		if (tokens.size() == 4)
		{
			if (parse_color(tokens[3], afp.color))
			{
				cerr << "bad color: " << tokens[3] << endl;
				status=1;
			}
		}
		else
		{
			afp.color.type = red;
		}
	}


	return status;
}


int parse_grating(const std::string& s, alert::ARGratingSpec& ag)
{
	int status=0;
	vector<string> tokens;
	tokenize(s, tokens, ",");

	// There are two allowed formats for gratings. The long form has 11 args, the short just 8. 
	// 
	// Long format for grating:
	// x,y,w,h,contrast%,sf,tf,orientation,color_vector,s|q,r|e
	// x,y,w,h in degrees
	// contrast should be an integer from 0-100. 
	// 0 <= orientation < 360
	// color_vector should be b|w|black|white|gray|... for black/white,
	// l|L for l-cone, m|M for m-cone and s|S for s-cone. default is black/white
	// s|q indicates pattern type, s for sine wave, q for square wave
	// r|e indicates aperture type, r for rectangular (height h, width w), e for elliptical
	//
	// Short form: the last three args (color_vector, pattern, aperture) can be omitted. 


	if (tokens.size() < 8 || tokens.size() > 11)
	{
		status=1;	// bad format
	}
	else
	{
		istringstream iss;
		iss.str(tokens[0]);
		iss >> ag.x;
		if (!iss) 
		{
			cerr << "bad x: " << tokens[0] << endl;
			status=1;
		}
		iss.str(tokens[1]);
		iss.clear();
		iss >> ag.y;
		if (!iss) 
		{
			cerr << "bad y: " << tokens[1] << endl;
			status=1;
		}
		iss.str(tokens[2]);
		iss.clear();
		iss >> ag.w;
		if (!iss)
		{
			cerr << "bad w: " << tokens[2] << endl;
			status=1;
		}
		iss.str(tokens[3]);
		iss.clear();
		iss >> ag.h;
		if (!iss)
		{
			cerr << "bad h: " << tokens[3] << endl;
			status=1;
		}
		iss.str(tokens[4]);
		iss.clear();
		iss >> ag.contrast;
		if (!iss)
		{
			cerr << "bad contrast: " << tokens[4] << endl;
			status=1;
		}
		iss.str(tokens[5]);
		iss.clear();
		iss >> ag.sf;
		if (!iss)
		{
			cerr << "bad sf: " << tokens[5] << endl;
			status=1;
		}
		iss.str(tokens[6]);
		iss.clear();
		iss >> ag.tf;
		if (!iss)
		{
			cerr << "bad tf: " << tokens[6] << endl;
			status=1;
		}
		iss.str(tokens[7]);
		iss.clear();
		iss >> ag.orientation;
		if (!iss)
		{
			cerr << "bad orientation: " << tokens[7] << endl;
			status=1;
		}

		// set defaults for the remaining items, then read if present
		ag.cv.type = b_w;
		ag.pattern = sinewave;
		ag.aperture = ellipse;

		if (tokens.size() > 8)
		{
			if (parse_colorvector(tokens[8], ag.cv))
			{
				cerr << "bad colorvector: " << tokens[8] << endl;
				status=1;
			}
		}
		if (tokens.size() > 9)
		{
			if (parse_pattern(tokens[9], ag.pattern))
			{
				cerr << "bad pattern: " << tokens[9] << endl;
				status=1;
			}
		}
		if (tokens.size() > 10)
		{
			if (parse_aperture(tokens[10], ag.aperture))
			{
				cerr << "bad aperture: " << tokens[10] << endl;
				status=1;
			}
		}
	
	}


	return status;
}



int parse_donut(const std::string& s, alert::ARDonutSpec& ag)
{
	int status=0;
	vector<string> tokens;
	tokenize(s, tokens, ",");

	// Expected format for donut: same as grating but with donut width,height following w,h
	// x,y,w,h,wd,hd,contrast%,sf,tf,orientation,color_vector,s|q,r|e
	// x,y,w,h,wd,hd in degrees
	// contrast should be an integer from 0-100. 
	// 0 <= orientation < 360
	// color_vector should be b|w|black|white|gray|... for black/white,
	// l|L for l-cone, m|M for m-cone and s|S for s-cone. default is black/white
	// s|q indicates pattern type, s for sine wave, q for square wave
	// r|e indicates aperture type, r for rectangular (height h, width w), e for elliptical
	// the last three args (color_vector, pattern, aperture) can be omitted. 

	if (tokens.size() < 10 || tokens.size() > 13)
	{
		status=1;	// bad format
	}
	else
	{
		istringstream iss;
		iss.str(tokens[0]);
		iss >> ag.x;
		if (!iss) 
		{
			cerr << "bad x: " << tokens[0] << endl;
			status=1;
		}
		iss.str(tokens[1]);
		iss.clear();
		iss >> ag.y;
		if (!iss) 
		{
			cerr << "bad y: " << tokens[1] << endl;
			status=1;
		}
		iss.str(tokens[2]);
		iss.clear();
		iss >> ag.w;
		if (!iss)
		{
			cerr << "bad w: " << tokens[2] << endl;
			status=1;
		}
		iss.str(tokens[3]);
		iss.clear();
		iss >> ag.h;
		if (!iss)
		{
			cerr << "bad h: " << tokens[3] << endl;
			status=1;
		}
		iss.str(tokens[4]);
		iss.clear();
		iss >> ag.wd;
		if (!iss)
		{
			cerr << "bad wd: " << tokens[4] << endl;
			status=1;
		}
		iss.str(tokens[5]);
		iss.clear();
		iss >> ag.hd;
		if (!iss)
		{
			cerr << "bad hd: " << tokens[5] << endl;
			status=1;
		}



		iss.str(tokens[6]);
		iss.clear();
		iss >> ag.contrast;
		if (!iss)
		{
			cerr << "bad contrast: " << tokens[6] << endl;
			status=1;
		}
		iss.str(tokens[7]);
		iss.clear();
		iss >> ag.sf;
		if (!iss)
		{
			cerr << "bad sf: " << tokens[7] << endl;
			status=1;
		}
		iss.str(tokens[8]);
		iss.clear();
		iss >> ag.tf;
		if (!iss)
		{
			cerr << "bad tf: " << tokens[8] << endl;
			status=1;
		}
		iss.str(tokens[9]);
		iss.clear();
		iss >> ag.orientation;
		if (!iss)
		{
			cerr << "bad orientation: " << tokens[9] << endl;
			status=1;
		}

		// set defaults for the remaining items, then read if present
		ag.cv.type = b_w;
		ag.pattern = sinewave;
		ag.aperture = ellipse;

		if (tokens.size() > 10)
		{
			if (parse_colorvector(tokens[10], ag.cv))
			{
				cerr << "bad colorvector: " << tokens[10] << endl;
				status=1;
			}
		}
		if (tokens.size() > 11)
		{
			if (parse_pattern(tokens[11], ag.pattern))
			{
				cerr << "bad pattern: " << tokens[11] << endl;
				status=1;
			}
		}
		if (tokens.size() > 12)
		{
			if (parse_aperture(tokens[12], ag.aperture))
			{
				cerr << "bad aperture: " << tokens[12] << endl;
				status=1;
			}
		}
	
	}


	return status;
}



int parse_xhair(const std::string& s, alert::ARXhairSpec& axh)
{
	int status=0;
	vector<string> tokens;
	tokenize(s, tokens, ",");

	// Expected format for xhair is 
	// x,y,r_inner,r_mid,r_outer,num_divisions,r1,r2
	// If r1 and r2 are omitted the crosshairs are not drawn. 
	// The check circle pattern is always drawn. 

	if (tokens.size() != 6 && tokens.size() != 8)
	{
		status=1;	// bad format
	}
	else
	{
		istringstream iss;
		iss.str(tokens[0]);
		iss >> axh.x;
		if (!iss) 
		{
			cerr << "bad x: " << tokens[0] << endl;
			status=1;
		}
		iss.str(tokens[1]); 
		iss.clear();
		iss >> axh.y;
		if (!iss) 
		{
			cerr << "bad y: " << tokens[1] << endl;
			status=1;
		}
		iss.str(tokens[2]);
		iss.clear();
		iss >> axh.ri;
		if (!iss) 
		{
			cerr << "bad r_inner: " << tokens[2] << endl;
			status=1;
		}
		iss.str(tokens[3]);
		iss.clear();
		iss >> axh.rm;
		if (!iss) 
		{
			cerr << "bad r_mid: " << tokens[3] << endl;
			status=1;
		}
		iss.str(tokens[4]);
		iss.clear();
		iss >> axh.ro;
		if (!iss) 
		{
			cerr << "bad r_outer: " << tokens[4] << endl;
			status=1;
		}
		iss.str(tokens[5]);
		iss.clear();
		iss >> axh.nc;
		if (!iss) 
		{
			cerr << "bad num_divisions: " << tokens[5] << endl;
			status=1;
		}

		if (tokens.size() == 8)
		{
			iss.str(tokens[6]);
			iss.clear();
			iss >> axh.r1;
			if (!iss) 
			{
				cerr << "bad r_xhair1: " << tokens[6] << endl;
				status=1;
			}
			iss.str(tokens[7]);
			iss.clear();
			iss >> axh.r2;
			if (!iss) 
			{
				cerr << "bad r_xhair2: " << tokens[7] << endl;
				status=1;
			}
		}
		else
		{
			axh.r1 = axh.r2 = -1;
		}
	}


	return status;
}


std::ostream& operator<<(std::ostream& out, const COLOR_TYPE& c)
{
	switch(c.type)
	{
	case black: out << "black"; break;
	case white: out << "white"; break;
	case red:	out << "red";	break;
	case green:	out << "green";	break;
	case blue:	out << "blue";	break;
	case gray:	out << "gray";	break;
	case custom: 
		out << "(" << (int)(c.color.a * 255) << "/" << (int)(c.color.b * 255) << "/" 
			<< (int)(c.color.c * 255) << ")";
		break;
	default:	out << "unknown"; break;
	}
	return out;
}


std::ostream& operator<<(std::ostream& out, const COLOR_VECTOR_TYPE& cv)
{
	switch(cv.type)
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

std::ostream& operator<<(std::ostream& out, const APERTURE_TYPE& a)
{
	switch(a)
	{
	case rectangle:	out << "r";	break;
	case ellipse:	out << "e";	break;
	default:		out << "unknown";	break;
	}
	return out;
}

std::ostream& operator<<(std::ostream& out, const PATTERN_TYPE& p)
{
	switch(p)
	{
	case sinewave:		out << "s";		break;
	case squarewave:	out << "q";	break;
	default:			out << "unknown";	break;
	}
	return out;
}

int parse_colorvector(std::string s, COLOR_VECTOR_TYPE& v)
{
	int status=0;
	if (s=="black" || s=="BLACK" ||
		s=="b" || s=="B" ||
		s=="white" || s=="WHITE" ||
		s=="w" || s=="W" ||
		s=="gray" || s=="GRAY" ||
		s=="g" || s=="G") v.type = b_w;
	else if (s=="l" || s=="L") v.type = l_cone;
	else if (s=="m" || s=="M") v.type = m_cone;
	else if (s=="s" || s=="S") v.type = s_cone;
	else 
	{
		int n;
		int fr, fg, fb, tr, tb, tg;
		// try and parse a custom color vector
		status=1;
		v.type = unknown_color_vector;
		n = sscanf_s(s.c_str(), "(%d/%d/%d)-(%d/%d/%d)", &fr, &fg, &fb, &tr, &tg, &tb);
		if (n==6)
		{
			if (fr>=0 && fr<256 && fg>=0 && fg<256 && fb>=0 && fb<256 && 
				tr>=0 && tr<256 && tg>=0 && tg<256 && tb>=0 && tb<256)
			{
				status = 0;
				v.type = custom_color_vector;
				v.from.a = (double)fr/255.0;
				v.from.b = (double)fg/255.0;
				v.from.c = (double)fb/255.0;
				v.to.a = (double)tr/255.0;
				v.to.b = (double)tg/255.0;
				v.to.c = (double)tb/255.0;
			}
		}
	}
	return status;
}


int parse_color(std::string s, COLOR_TYPE& c)
{
	int status=0;
	if (s=="black" || s=="BLACK") c.type = black;
	else if (s=="white" || s=="WHITE") c.type = white;
	else if (s=="gray" || s=="GRAY") c.type = gray;
	else if (s=="red" || s=="RED" || s=="r" || s=="R") c.type = red;
	else if (s=="green" || s=="GREEN" || s=="g" || s=="G") c.type = green;
	else if (s=="blue" || s=="BLUE" || s=="b" || s=="B") c.type = blue;
	else 
	{
		int n;
		int r, g, b;
		// try and parse a custom color vector
		status=1;
		c.type = unknown_color;
		n = sscanf_s(s.c_str(), "(%d/%d/%d)", &r, &g, &b);
		if (n==3)
		{
			if (r>=0 && r<256 && g>=0 && g<256 && b>=0 && b<256)
			{
				status = 0;
				c.type = custom;
				c.color.a = (double)r/255.0;
				c.color.b = (double)g/255.0;
				c.color.c = (double)b/255.0;
			}
		}
	}
	return status;
}



int parse_contrast_triplet(std::string s, int& i_iContrastDown, int& i_iContrastBase, int& i_iContrastUp)
{
	int status=0;
	vector<string> tokens;
	tokenize(s, tokens, ",");
	if (tokens.size() != 3)
	{
		cerr << "Bad contrast triplet format: " << s << endl;
		status=1;
	}
	else
	{
		istringstream iss;
		iss.str(tokens[0]);
		iss >> i_iContrastDown;
		if (!iss) 
		{
			cerr << "bad ContrastDown: " << tokens[0] << endl;
			status=1;
		}
		iss.str(tokens[1]);
		iss.clear();
		iss >> i_iContrastBase;
		if (!iss) 
		{
			cerr << "bad ContrastBase: " << tokens[1] << endl;
			status=1;
		}
		iss.str(tokens[2]);
		iss.clear();
		iss >> i_iContrastUp;
		if (!iss)
		{
			cerr << "bad ContrastUp: " << tokens[2] << endl;
			status=1;
		}
	}
	return status;
}


int parse_int_pair(std::string s, int& i_i1, int& i_i2)
{
	int status=0;
	vector<string> tokens;
	tokenize(s, tokens, ",");
	if (tokens.size() != 2)
	{
		cerr << "Bad format: " << s << endl;
		status=1;
	}
	else
	{
		istringstream iss;
		iss.str(tokens[0]);
		iss >> i_i1;
		if (!iss) 
		{
			cerr << "bad int: " << tokens[0] << endl;
			status=1;
		}
		iss.str(tokens[1]);
		iss.clear();
		iss >> i_i2;
		if (!iss) 
		{
			cerr << "bad int: " << tokens[1] << endl;
			status=1;
		}
	}
	return status;
}

int parse_sequence_pair(std::string s, int& i_i1, int& i_i2)
{
	int status=0;
	status = parse_int_pair(s, i_i1, i_i2);
	if (!status)
	{
		if (i_i2 < i_i1)
		{
			cerr << "bad sequence pair: second term < first_term" << endl;
			status = 1;
		}
		else if (i_i1 <= 0)
		{
			cerr << "bad sequence pair: both terms must be > 0!" << endl;
			status = 1;
		}
	}
	return status;
}


int parse_tuning_triplet(std::string s, double& i_dMin, double& i_dMax, int& i_iSteps)
{
	int status=0;
	vector<string> tokens;
	tokenize(s, tokens, ",");
	if (tokens.size() != 3)
	{
		cerr << "Bad tuning triplet format: " << s << endl;
		status=1;
	}
	else
	{
		istringstream iss;
		iss.str(tokens[0]);
		iss >> i_dMin;
		if (!iss) 
		{
			cerr << "bad tuning Min value: " << tokens[0] << endl;
			status=1;
		}
		iss.str(tokens[1]);
		iss.clear();
		iss >> i_dMax;
		if (!iss) 
		{
			cerr << "bad tuning Max value: " << tokens[1] << endl;
			status=1;
		}
		iss.str(tokens[2]);
		iss.clear();
		iss >> i_iSteps;
		if (!iss)
		{
			cerr << "bad tuning #steps value: " << tokens[2] << endl;
			status=1;
		}
	}
	return status;
}

int parse_int_list(std::string s, std::vector<int>& list)
{
	int status=0;
	unsigned int i;
	istringstream iss;
	int num;
	vector<string> tokens;
	tokenize(s, tokens, ",");
	for (i=0; i<tokens.size(); i++)
	{
		iss.clear();
		iss.str(tokens[i]);
		iss >> num;
		if (!iss) 
		{
			cerr << "bad tuning value: " << tokens[i] << endl;
			status=1;
		}
		else
		{
			list.push_back(num);
		}
	}
	return status;
}


int parse_tuning_list(std::string s, vector<double>& tuning_list, int& i_iSteps)
{
	int status=0;
	unsigned int i;
	istringstream iss;
	double d;
	vector<string> tokens;
	tokenize(s, tokens, ",");
	for (i=0; i<tokens.size(); i++)
	{
		iss.clear();
		iss.str(tokens[i]);
		iss >> d;
		if (!iss) 
		{
			cerr << "bad tuning value: " << tokens[i] << endl;
			status=1;
		}
		else
		{
			tuning_list.push_back(d);
		}
	}
	// Number of steps is the size of the list MINUS 1!
	i_iSteps = tuning_list.size() - 1;
	return status;
}



int get_color(COLOR_TYPE c, VSGTRIVAL& trival)
{
	int status=0;

	switch(c.type)
	{
	case black:
		trival.a=0;
		trival.b=0;
		trival.c=0;
		break;
	case white:
		trival.a=1;
		trival.b=1;
		trival.c=1;
		break;
	case gray:
		trival.a=0.5;
		trival.b=0.5;
		trival.c=0.5;
		break;
	case red:
		trival.a=1;
		trival.b=0;
		trival.c=0;
		break;
	case green:
		trival.a=0;
		trival.b=1;
		trival.c=0;
		break;
	case blue:
		trival.a=0;
		trival.b=0;
		trival.c=1;
		break;
	case custom:
		trival = c.color;
		break;
	default:
		status=1;
	}
	
	return status;
}


int get_colorvector(COLOR_VECTOR_TYPE& cv, VSGTRIVAL& from, VSGTRIVAL& to)
{
	int status=0;
	switch (cv.type)
	{
	case b_w:
		from.a = from.b = from.c=0;
		to.a = to.b = to.c=1;
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
		status=1;
		break;
	}

	return status;
}


int parse_pattern(std::string s, PATTERN_TYPE& p)
{
	int status=0;
	if (s == "q" || s == "Q") p = squarewave;
	else if (s=="s" || s=="S") p = sinewave;
	else
	{
		status = 1;
		p = unknown_pattern;
	}
	return status;
}

int parse_aperture(std::string s, APERTURE_TYPE& a)
{
	int status=0;
	if (s == "r" || s == "R") a = rectangle;
	else if (s == "e" || s == "E" ||
			 s == "c" || s == "C") a = ellipse;
	else
	{
		a = unknown_aperture;
		status=1;
	}
	return status;
}


int parse_distance(std::string s, int& dist)
{
	int status=0;
	istringstream iss(s);
	iss >> dist;
	if (!iss) 
	{
		cerr << "bad distance: " << s << endl;
		status=1;
	}
	return status;
}


int parse_xy(std::string s, double& x, double& y)
{
	int status=0;
	vector<string> tokens;
	tokenize(s, tokens, ",");
	if (tokens.size() != 2)
	{
		cerr << "Bad x,y format: " << s << endl;
		status=1;
	}
	else
	{
		istringstream iss;
		iss.str(tokens[0]);
		iss >> x;
		if (!iss) 
		{
			cerr << "bad x: " << tokens[0] << endl;
			status=1;
		}
		iss.str(tokens[1]);
		iss.clear();
		iss >> y;
		if (!iss) 
		{
			cerr << "bad y: " << tokens[1] << endl;
			status=1;
		}
	}
	return status;
}


int parse_integer(std::string s, int& i)
{
	return parse_distance(s, i);
}

int parse_double(std::string s, double& d)
{
	int status=0;
	istringstream iss(s);
	iss >> d;
	if (!iss) 
	{
		cerr << "bad format for double: " << s << endl;
		status=1;
	}
	return status;
}



void tokenize(const string& str, vector<string>& tokens, const string& delimiters = " ")
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

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

void make_argv(std::ifstream& ifs, int& argc, char **argv)
{
     // Read the whole file into a string
     stringstream ss;
     ss << ifs.rdbuf();
     string contents( ss.str() );
	 make_argv(contents, argc, argv);
	 return;
}


void make_argv(const std::string& str, int &argc, char **argv)
{
	vector<string> tokens;
	tokenize(str, tokens, " \n\t");
	make_argv(tokens, argc, argv);
	return;
}

void make_argv(vector<string>tokens, int& argc, char** argv)
{
	unsigned int i;
	argv[0] = (char *)malloc(9);
	strcpy_s(argv[0], 9, "PROGNAME");
	for (i=0; i<tokens.size(); i++)
	{
		argv[i+1] = (char *)malloc(tokens[i].length()+1);
		strcpy_s(argv[i+1], tokens[i].length()+1, tokens[i].c_str());
	}
	argc = (int)tokens.size()+1;
}

void free_argv(int& argc, char **argv)
{
	int i;
	for (i=0; i<argc; i++)
	{
		free(argv[i]);
		argv[i] = NULL;
	}
	argc = 0;
}
