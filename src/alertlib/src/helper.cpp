#include "helper.h"
#include <iostream>
#include <string>
#include <vector>


using namespace std;

#pragma warning(disable:4786)


int getcolorvector(std::string s, COLOR_VECTOR_TYPE& v)
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


int getcolor(std::string s, COLOR_TYPE& c)
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

int getpattern(std::string s, PATTERN_TYPE& p)
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

int getaperture(std::string s, APERTURE_TYPE& a)
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

