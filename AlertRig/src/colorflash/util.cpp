#include "util.h"
#include <sstream>
#include <vector>
#include <iostream>

using namespace std;

int parse_integer(std::string s, int& dist)
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
	mytokenize(s, tokens, ",");
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


void mytokenize(const string& str, vector<string>& tokens, const string& delimiters)
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


