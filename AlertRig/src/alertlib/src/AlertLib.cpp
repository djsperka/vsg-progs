#include "Alertlib.h"
#include "helper.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>



using namespace std;

#pragma warning(disable:4786)


std::ostream& operator<<(std::ostream& out, const AlertFixationPoint& afp)
{
	out << "(x,y): (" << afp.x << ", " << afp.y << ") diameter: " << afp.d << " color: " << afp.color;
	return out;
}

std::ostream& operator<<(std::ostream& out, const AlertGrating& ag)
{
	out << "(x,y): (" << ag.x << ", " << ag.y << ") (w,h): (" << ag.w << ", " << ag.h << ") con%: " << ag.contrast << " sf: " << ag.sf << " tf: " << ag.tf << " orient: " << ag.orientation << " cv: " << ag.cv << " patt: " << ag.pattern << " aper: " << ag.aperture;
	return out;
}

std::ostream& operator<<(std::ostream& out, const COLOR_TYPE& c)
{
	switch(c)
	{
	case black: out << "black"; break;
	case white: out << "white"; break;
	case red:	out << "red";	break;
	case green:	out << "green";	break;
	case blue:	out << "blue";	break;
	case gray:	out << "gray";	break;
	default:	out << "unknown"; break;
	}
	return out;
}


std::ostream& operator<<(std::ostream& out, const COLOR_VECTOR_TYPE& v)
{
	switch(v)
	{
	case b_w:	out << "black/white";	break;
	case l_cone:out << "l-cone";		break;
	case m_cone:out << "m-cone";		break;
	case s_cone:out << "s-cone";		break;
	default:	out << "unknown";		break;
	}
	return out;
}

std::ostream& operator<<(std::ostream& out, const APERTURE_TYPE& a)
{
	switch(a)
	{
	case rectangle:	out << "rectangle";	break;
	case ellipse:	out << "ellipse";	break;
	default:		out << "unknown";	break;
	}
	return out;
}

std::ostream& operator<<(std::ostream& out, const PATTERN_TYPE& p)
{
	switch(p)
	{
	case sinewave:		out << "sine";		break;
	case squarewave:	out << "square";	break;
	default:			out << "unknown";	break;
	}
	return out;
}




int parse_fixation_point(const std::string& s, AlertFixationPoint& afp)
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
			if (iss.fail()) cerr << "fail" << endl;
			if (iss.eof()) cerr << "eof" << endl;
			if (iss.bad()) cerr << "bad" << endl;
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
			afp.color = red;
		}
	}


	return status;
}



int parse_grating(const std::string& s, AlertGrating& ag)
{
	int status=0;
	vector<string> tokens;
	tokenize(s, tokens, ",");

	// Expected format for grating
	// x,y,w,h,contrast%,sf,tf,orientation,color_vector,s|q,r|e
	// x,y,w,h in degrees
	// contrast should be an integer from 0-100. 
	// 0 <= orientation < 360
	// color_vector should be b|w|black|white|gray|... for black/white,
	// l|L for l-cone, m|M for m-cone and s|S for s-cone. default is black/white
	// s|q indicates pattern type, s for sine wave, q for square wave
	// r|e indicates aperture type, r for rectangular (height h, width w), e for elliptical
	// the last three args (color_vector, pattern, aperture) can be omitted. 

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
		ag.cv = b_w;
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


int parse_colorvector(std::string s, COLOR_VECTOR_TYPE& v)
{
	int status=0;
	if (s=="black" || s=="BLACK" ||
		s=="b" || s=="B" ||
		s=="white" || s=="WHITE" ||
		s=="w" || s=="W" ||
		s=="gray" || s=="GRAY" ||
		s=="g" || s=="G") v = b_w;
	else if (s=="l" || s=="L") v = l_cone;
	else if (s=="m" || s=="M") v = m_cone;
	else if (s=="s" || s=="S") v = s_cone;
	else 
	{
		status=1;
		v = unknown_color_vector;
	}
	return status;
}


int parse_color(std::string s, COLOR_TYPE& c)
{
	int status=0;
	if (s=="black" || s=="BLACK") c = black;
	else if (s=="white" || s=="WHITE") c = white;
	else if (s=="gray" || s=="GRAY") c = gray;
	else if (s=="red" || s=="RED" || s=="r" || s=="R") c = red;
	else if (s=="green" || s=="GREEN" || s=="g" || s=="G") c = green;
	else if (s=="blue" || s=="BLUE" || s=="b" || s=="B") c = blue;
	else 
	{
		status=1;
		c = unknown_color;
	}
	return status;
}

int get_color(COLOR_TYPE& c, VSGTRIVAL& trival)
{
	int status=0;

	switch(c)
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
	default:
		status=1;
	}
	
	return status;
}


int get_colorvector(COLOR_VECTOR_TYPE& cv, VSGTRIVAL& from, VSGTRIVAL& to)
{
	int status=0;
	switch (cv)
	{
	case b_w:
		from.a = from.b = from.c=0;
		to.a = to.b = to.c=1;
		break;
	case l_cone:
		from.a = 0;
		from.b = .57926;
		from.c=.5;
		to.a = 1;
		to.b = .42074;
		to.c=.5;
		break;
	case m_cone:
		from.a = 1;
		from.b = .27593;
		from.c=.5137;
		to.a = 0;
		to.b = .72407;
		to.c=.4863;
		break;
	case s_cone:
		from.a = .38356;
		from.b = .63699;
		from.c=0;
		to.a = .61644;
		to.b = .36301;
		to.c=.1;
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


int LevelManager::request_single(PIXEL_LEVEL& level)
{
	int status=0;
	if (m_next < 250)
	{
		level = m_next++;
	}
	else status=1;
	return status;
}

int LevelManager::request_range(int num, PIXEL_LEVEL& first, PIXEL_LEVEL& last)
{
	int status=0;
	if (250-m_next >= num)
	{
		first = m_next;
		last = m_next + num - 1;
		m_next = last + 1;
	}
	else status=1;
	return status;
}


void AlertFixationPoint::draw(PIXEL_LEVEL level)
{
	vsgSetPen1(level);
	vsgDrawOval(x, y, d, d);
}

void AlertGrating::draw(PIXEL_LEVEL first, PIXEL_LEVEL last)
{
	VSGTRIVAL from, to;


	// create vsg object if necessary
	if (m_handle)
	{
		vsgObjSelect(handle);
	}
	else
	{
		handle = vsgObjCreate();
		m_handle = true;
	}


	// if ellipse, draw an ellipse on level 0 for TRANSONLOWER
	if (this->aperture == ellipse)
	{
		vsgSetPen1(0);
		vsgSetDrawMode(vsgCENTREXY);
		vsgDrawOval(x, y, w, h);
	}

	vsgObjSetDefaults();
	vsgObjSetPixelLevels(first, last);

	// Set spatial waveform
	if (this->pattern == sinewave)
	{
		vsgObjTableSinWave(vsgSWTABLE);
	}
	else
	{	
		// Set up standard 50:50 square wave
		vsgObjTableSquareWave(vsgSWTABLE, vsgObjGetTableSize(vsgSWTABLE)*0.25, vsgObjGetTableSize(vsgSWTABLE)*0.75);
	}

	// set temporal freq
	vsgObjSetDriftVelocity(tf);

	// set color vector
	if (get_colorvector(this->cv, from, to))
	{
		cerr << "Cannot get color vector for type " << this->cv << endl;
	}
	vsgObjSetColourVector(&from, &to, vsgBIPOLAR);

	// Set contrast
	vsgObjSetContrast(this->contrast);

	// Now draw
	if (this->aperture == ellipse)
	{
		vsgSetDrawMode(vsgTRANSONLOWER+vsgCENTREXY);
		vsgSetPen1(first);
		vsgSetPen2(last);
		vsgDrawGrating(this->x, this->y, this->w, this->h, this->orientation, this->sf);
	}
	else
	{
		vsgSetDrawMode(vsgCENTREXY);
		vsgSetPen1(first);
		vsgSetPen2(last);
		vsgDrawGrating(this->x, this->y, this->w, this->h, this->orientation, this->sf);
	}
	return;
}


bool Trigger::checkAscii(std::string input)
{
	return input == m_key;
}

bool Trigger::checkBinary(int input)
{
	bool bValue = false;
	int current = input&m_in_mask;
	if (current != m_in_last && current == m_in_val)
	{
		bValue = true;
	}
	m_in_last = current;
	return bValue;
}

void Trigger::execute(int& output)
{
	// Set trigger marker if required
	if (m_out_val > 0)
	{
		output = m_out_val | (output&(~m_out_mask));
	}

	return;
}


int init_vsg(int screenDistanceMM, COLOR_TYPE i_background)
{
	int status=0;
	VSGTRIVAL background;
	
	if (vsgInit(""))
	{
		cerr << "Error in vsgInit()." << endl;
		status=1;
	}
	else 
	{
		vsgSetViewDistMM(screenDistanceMM);
		vsgSetSpatialUnits(vsgDEGREEUNIT);
		vsgSetCommand(vsgPALETTERAMP);

		// set background color, er colour. 
		if (get_color(i_background, background))
		{
			cerr << "Cannot get trival for background color " << i_background << endl;
			status = 2;
		}
		vsgSetBackgroundColour(&background);

		clear_vsg();
	}

	// allocate a single vsgs object so triggers will work. This is redundant in cases
	// where you will be generating stuff like gratings, but just in case you don't you
	// need this. 
	vsgObjCreate();
	vsgObjSetPixelLevels(0, 1);

	return status;
}

void clear_vsg()
{
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);
}