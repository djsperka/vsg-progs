#include "Alertlib.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>



using namespace std;
using namespace alert;

std::ostream& operator<<(std::ostream& out, const alert::ARFixationPointSpec& arfps)
{
	out << "(x,y): (" << arfps.x << ", " << arfps.y << ") diameter: " << arfps.d << " color: " << arfps.color;
	return out;
}

std::ostream& operator<<(std::ostream& out, const alert::ARGratingSpec& args)
{
	out << "(x,y): (" << args.x << ", " << args.y << ") (w,h): (" << args.w << ", " << args.h << ") con%: " << args.contrast << " sf: " << args.sf << " tf: " << args.tf << " orient: " << args.orientation << " cv: " << args.cv << " patt: " << args.pattern << " aper: " << args.aperture;
	return out;
}

std::ostream& operator<<(std::ostream& out, const alert::Trigger& t)
{
	out << t.toString();
	return out;
}


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
			afp.color = red;
		}
	}


	return status;
}


int parse_grating(const std::string& s, alert::ARGratingSpec& ag)
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


int alert::ARObject::select()
{
	if (m_initialized) 
	{
		vsgObjSelect(m_handle);
		return 0;
	}
	else return -1;
};


void alert::ARObject::init(int numlevels)
{
	PIXEL_LEVEL level;
	if (numlevels>1)
	{
		LevelManager::instance().request_range(numlevels, level);
		init(level, numlevels);
	}
	else if (numlevels==1)
	{
		LevelManager::instance().request_single(level);
		init(level, numlevels);
	}
	else
	{
		cerr << "Error - number of levels must be >0!" << endl;
	}
}



void alert::ARObject::init(PIXEL_LEVEL first, int numlevels)
{
	m_handle = vsgObjCreate();
	vsgObjSetDefaults();
	vsgObjSetContrast(100);
	vsgObjSetPixelLevels(first, numlevels);
	m_initialized = true;
	cerr << "init obj on level " << first << ", with " << numlevels << " levels" << endl;
}


int alert::ARFixationPointSpec::draw()
{
	select();
	vsgDrawOval(x, y, d, d);
	return 0;
}

int alert::ARFixationPointSpec::drawOverlay()
{
	vsgSetPen1(0);	// overlay page transparent
	vsgDrawOval(x, y, d, d);
	return 0;
}


int alert::ARContrastFixationPointSpec::draw()
{
	int status=0;
	VSGTRIVAL from, to;

	select();

	if (get_color(this->color, to))
	{
		cerr << "Cannot get color trival for point: " << this->color << endl;
		status=1;
	}
	else if (get_color(alert::ARvsg::instance().background_color(), from))
	{
		cerr << "Cannot get background color trival for point: " << alert::ARvsg::instance().background_color() << endl;
		status=1;
	}
	else 
	{
		vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);
		cerr << "from=" << from.a << "," << from.b << "," << from.c << " to=" << to.a << "," << to.b << "," << to.c << endl;
	}

	if (!status)
	{
		ARFixationPointSpec::draw();
	}
	return status;
}

int alert::ARContrastFixationPointSpec::drawOverlay()
{
	vsgSetPen1(0);
	vsgDrawOval(x, y, d, d);
	return 0;
}


int alert::ARGratingSpec::draw()
{
	return draw(false);
}

int alert::ARGratingSpec::drawOnce()
{
	return draw(true);
}

int alert::ARGratingSpec::draw(bool useTransOnLower)
{
	int status=0;
	VSGTRIVAL from, to;

	select();

	// We assume that the handle is created and selected. In order to make this grating appear, you still must
	// assign pixel levels (vsgObjSetPixels). Note also that the contrast is initially set to 100% by the call to 
	// vsgObjSetDefaults().

	if (useTransOnLower)
	{
		// djs TRANSONLOWER seems to leave artifacts on the screen when we turn the contrast off. !!!
		// if ellipse, draw an ellipse on level 0 for TRANSONLOWER
		// djs. Specifically setting the level seems to lead to artifacts. I found that leaving the levels
		// unassigned solves this. I suspect this may lead to trouble someday.....
		if (this->aperture == ellipse)
		{
//			vsgSetPen1(0);
//			vsgSetPen2(0);
			vsgSetDrawMode(vsgCENTREXY);
			vsgDrawOval(x, y, w, h);
		}
	}

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

	// Now draw
	if (this->aperture == ellipse)
	{
		if (useTransOnLower)
		{
			vsgSetDrawMode(vsgCENTREXY + vsgTRANSONLOWER);
			vsgDrawGrating(this->x, this->y, this->w, this->h, this->orientation, this->sf);
			vsgSetDrawMode(vsgCENTREXY);
		}
		else
		{
			vsgSetDrawMode(vsgCENTREXY);
			vsgDrawGrating(this->x, this->y, this->w, this->h, this->orientation, this->sf);
		}
	}
	else
	{
		vsgSetDrawMode(vsgCENTREXY);
		vsgDrawGrating(this->x, this->y, this->w, this->h, this->orientation, this->sf);
	}

	return 0;
}


int alert::ARGratingSpec::drawOverlay()
{
	vsgSetPen1(0);
	vsgSetDrawMode(vsgCENTREXY);
	if (this->aperture == ellipse)
	{
		vsgDrawOval(x, y, w, h);
	}
	else
	{
		vsgDrawRect(x, y, w, h);
	}
	return 0;
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





int get_color(COLOR_TYPE c, VSGTRIVAL& trival)
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


int alert::LevelManager::request_single(PIXEL_LEVEL& level)
{
	int status=0;
	if (m_next < 250)
	{
		level = m_next++;
	}
	else status=1;
	return status;
}

int alert::LevelManager::request_range(int num, PIXEL_LEVEL& first)
{
	int status=0;
	if (250-m_next >= num)
	{
		first = m_next;
		m_next += num;
	}
	else status=1;
	return status;
}


int alert::ARvsg::init(int screenDistanceMM, COLOR_TYPE i_bg)
{
	int status=0;
	if (!m_initialized)
	{
		if (vsgInit(""))
		{
			cerr << "Error in vsgInit()." << endl;
			status=1;
		}
		else
		{
			Sleep(2000);
			m_initialized = true;
			vsgSetViewDistMM(screenDistanceMM);
			vsgSetSpatialUnits(vsgDEGREEUNIT);
			vsgSetCommand(vsgPALETTERAMP);
			m_background_color = i_bg;
			// this level gets used later, but we request it now to insure we get level 0
			alert::LevelManager::instance().request_single(m_background_level);
		}
	}
	return status;
}



int alert::ARvsg::init_overlay()
{
	int status=0;
	VSGTRIVAL background;

	if (!m_initialized)
	{
		cerr << "init_overlay: Must call init first!" << endl;
		status=1;
	}
	else 
	{		
		vsgSetCommand(vsgOVERLAYMASKMODE);
		if (get_color(m_background_color, background))
		{
			cerr << "Cannot get trival for background color " << m_background_color << endl;
			status = 2;
		}
		else
		{
			vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&background, 1, 1);
			vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1);
		}
	}
	return status;
}



int alert::ARvsg::init_video()
{
	return init_video_pages(NULL, NULL);
}




int alert::ARvsg::init_video_pages(voidfunc func_before_objects, voidfunc func_after_objects)
{
	int i;
	int status=0;
	VSGTRIVAL background;
	PIXEL_LEVEL dummy_level;

	if (!m_initialized)
	{
		cerr << "Must call ARvsg::init first!" << endl;
		return 1;
	}
	else 
	{		
		// set background color, er colour, in palette
		// We'll set the vsgBACKGROUND color later, after the vsgObject is created. 
		// The background level was obtained in the init() call.
		if (get_color(m_background_color, background))
		{
			cerr << "Cannot get trival for background color " << m_background_color << endl;
			status = 2;
		}
		vsgPaletteWrite((VSGLUTBUFFER*)&background, m_background_level, 1);


		// Now clear all pages to background level, then call the before_objects callback, 
		// if it exists. 
		for (i=0; i<vsgGetSystemAttribute(vsgNUMVIDEOPAGES); i++)
		{
			vsgSetDrawPage(vsgVIDEOPAGE, i, m_background_level);
			if (func_before_objects)
			{
				func_before_objects(i);
			}
		}
		

		// Create a single vsg object and set vsgBACKGROUND color
		m_handle = vsgObjCreate();
		alert::LevelManager::instance().request_single(dummy_level);
		vsgObjSetPixelLevels(dummy_level, 1);
		vsgSetBackgroundColour(&background);


		// Now call after_objects callback if not null
		if (func_after_objects)
		{
			for (i=0; i<vsgGetSystemAttribute(vsgNUMVIDEOPAGES); i++)
			{
				vsgSetDrawPage(vsgVIDEOPAGE, i, vsgNOCLEAR);
				func_after_objects(i);
			}
		}

		// Finally, set page 0 as the current page and present. 
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		vsgPresent();
	}
	return status;
};

void alert::ARvsg::clear(int ipage)
{
	vsgSetDrawPage(vsgVIDEOPAGE, ipage, vsgBACKGROUND);
	vsgPresent();
}

void alert::ARvsg::clear()
{
	clear(0);
}



