#include "MultiSacStimSet.h"
#include "AlertUtil.h"

class MultiSacStimSet;
MultiSacStimSet* parseMultiSacStimSet(const std::string& s)
{
	// comma-separated
	std::vector<std::string> tokens;
	tokenize(s, tokens, ",");
	if (tokens.size() < 7)
	{
		cerr << "Error parsing BorderStimSet: must be at least 7 args" << endl;
		return nullptr;
	}

	double x, y;
	double s0, s1;
	COLOR_TYPE c0, c1;
	if (parse_double(tokens[0], x) || parse_double(tokens[1], y))
	{
		cerr << "parseBorderStimSet: bad x,y value(s) in args 0,1: " << tokens[0] << "," << tokens[1] << endl;
		return nullptr;
	}

	if (parse_double(tokens[2], s0) || s0 < .01 || parse_double(tokens[3], s1) || s1 < 0.01)
	{
		cerr << "parseBorderStimSet: bad size value(s) in args 2,3: " << tokens[2] << "," << tokens[3] << endl;
		return nullptr;
	}

	if (parse_color(tokens[4], c0) || parse_color(tokens[5], c1))
	{
		cerr << "parseBorderStimSet: bad color value(s) in args 4,5: " << tokens[4] << "," << tokens[5] << endl;
		return nullptr;
	}

	// parse remaining values as stim specs......
	std::vector<std::string> specs(tokens.cbegin() + 6, tokens.cend());
	std::vector<int> stim;
	if (parse_int_list(specs, stim))
	{
		cerr << "parseBorderStimSet: bad spec list: " << s << endl;
		return nullptr;
