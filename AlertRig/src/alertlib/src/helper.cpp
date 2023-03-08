#include "Alertlib.h"
#include "coneiso.h"
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace alert;
using namespace boost::algorithm;

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
			afp.color.setType(red);
		}
	}


	return status;
}



int parse_fixation_point_list(vector<string>& tokens, vector<ARFixationPointSpec>& fixation_point_list)
{
	int status = 0;
	unsigned int i;
	istringstream iss;
	ARFixationPointSpec fixpt;
	for (i = 0; i < tokens.size(); i++)
	{
		iss.clear();
		iss.str(tokens[i]);
		cerr << "parsing token " << tokens[i] << endl;
		iss >> fixpt;
		if (!iss)
		{
			cerr << "bad fixpt value: " << tokens[i] << endl;
			status = 1;
		}
		else
		{
			fixation_point_list.push_back(fixpt);
		}
	}
	return status;
}




int parse_fixation_point_list(std::string s, vector<ARFixationPointSpec>& fixation_point_list)
{
	int status = 0;
	vector<string> tokens;
	tokenize(s, tokens, ";");
	return parse_fixation_point_list(tokens, fixation_point_list);
}


int parse_rectangle(const std::string& s, alert::ARRectangleSpec& ar)
{
	int status = 0;
	vector<string> tokens;
	tokenize(s, tokens, ",");

	// Expected format for rectangle is 
	// x,y[,w,h[,ori[,color]]]
	// w,h default to 1
	// ori default 0
	// color default white

	if (tokens.size() < 2 || tokens.size() > 6)
	{
		status = 1;	// bad format
	}
	else
	{
		ar.w = ar.h = 1;
		ar.orientation = 0;
		ar.color = COLOR_TYPE(white);

		istringstream iss;
		iss.str(tokens[0]);
		iss >> ar.x;
		if (!iss)
		{
			cerr << "bad x: " << tokens[0] << endl;
			status = 1;
		}
		iss.str(tokens[1]);
		iss.clear();
		iss >> ar.y;
		if (!iss)
		{
			cerr << "bad y: " << tokens[1] << endl;
			status = 1;
		}

		if (!status && tokens.size() > 2)
		{
			iss.str(tokens[2]);
			iss.clear();
			iss >> ar.w;
			if (!iss)
			{
				cerr << "bad width: " << tokens[2] << endl;
				status = 1;
			}
		}
		if (!status && tokens.size() > 3)
		{
			iss.str(tokens[3]);
			iss.clear();
			iss >> ar.h;
			if (!iss)
			{
				cerr << "bad height: " << tokens[3] << endl;
				status = 1;
			}
		}
		if (!status && tokens.size() > 4)
		{
			iss.str(tokens[4]);
			iss.clear();
			iss >> ar.orientation;
			if (!iss)
			{
				cerr << "bad orientation: " << tokens[4] << endl;
				status = 1;
			}
		}

		if (!status && tokens.size() > 5)
		{
			if (parse_color(tokens[5], ar.color))
			{
				cerr << "bad color: " << tokens[5] << endl;
				status = 1;
			}
		}
	}
	return status;
}



int parse_grating(const std::string& s, alert::ARGratingSpec& ag)
{
	int status=0;
	vector<string> tokens;
	vector<double> numbers;
	int num = 0;			// number of numbers
	double d;
	istringstream iss;

	// updated 1/18/2012 djs
	// Now allow for gratings and/or donuts.  

	// UPDATED 9/15/2011 djs
	// Added phase parameter to grating spec. In text form it fits after the orientation.
	// Now the long (short) form is 12 (9) args. 
	// For backwards compatibility we still allow 11/8 as below. 
	// 
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

	tokenize(s, tokens, ",");
	if (tokens.size() != 4  && (tokens.size() < 8 || tokens.size() > 15))
	{
		cerr << "Bad grating spec format." << endl;
		status=1;	// bad format
	}
	else
	{
		for (unsigned int i=0; i<tokens.size() && iss; i++)
		{
			iss.clear();
			iss.str(tokens[i]);
			iss >> d;
			if (iss) 
			{
				numbers.push_back(d);
			}
		}

		switch(numbers.size())
		{
		case 4:
			ag.x = numbers[0];
			ag.y = numbers[1];
			ag.w = numbers[2];
			ag.h = numbers[3];
			ag.contrast = 100;
			ag.sf = 0.5;
			ag.tf = 0.25;
			ag.orientation = 45;
			ag.phase = 0;
			ag.wd = 0;
			ag.hd = 0;
			break;
		case 8:
			ag.x = numbers[0];
			ag.y = numbers[1];
			ag.w = numbers[2];
			ag.h = numbers[3];
			ag.contrast = (int)numbers[4];
			ag.sf = numbers[5];
			ag.tf = numbers[6];
			ag.orientation = numbers[7];
			ag.phase = 0;
			ag.wd = 0;
			ag.hd = 0;
			break;
		case 9:
			ag.x = numbers[0];
			ag.y = numbers[1];
			ag.w = numbers[2];
			ag.h = numbers[3];
			ag.contrast = (int)numbers[4];
			ag.sf = numbers[5];
			ag.tf = numbers[6];
			ag.orientation = numbers[7];
			ag.phase = numbers[8];
			ag.wd = 0;
			ag.hd = 0;
			break;
		case 10:
			ag.x = numbers[0];
			ag.y = numbers[1];
			ag.w = numbers[2];
			ag.h = numbers[3];
			ag.wd = numbers[4];
			ag.hd = numbers[5];
			ag.contrast = (int)numbers[6];
			ag.sf = numbers[7];
			ag.tf = numbers[8];
			ag.orientation = numbers[9];
			ag.phase = 0;
			break;
		case 11:
			ag.x = numbers[0];
			ag.y = numbers[1];
			ag.w = numbers[2];
			ag.h = numbers[3];
			ag.wd = numbers[4];
			ag.hd = numbers[5];
			ag.contrast = (int)numbers[6];
			ag.sf = numbers[7];
			ag.tf = numbers[8];
			ag.orientation = numbers[9];
			ag.phase = numbers[10];
			break;
		default:
			cerr << "Bad number of tokens! parse_grating incorrectly configured!" << endl;
			status = 1;
			break;
		}

		// set defaults for the remaining items, then read if present
		ag.cv.setType(b_w);
		ag.swt = sinewave;
		ag.twt = sinewave;
		ag.aperture = ellipse;
		ag.ttf = 0;

		if (tokens.size() > numbers.size())
		{
			if (parse_colorvector(tokens[numbers.size()], ag.cv))
			{
				cerr << "bad colorvector: " << tokens[numbers.size()] << endl;
				status=1;
			}
		}
		if (tokens.size() > numbers.size()+1)
		{
			if (parse_waveform_types(tokens[numbers.size()+1], ag.swt, ag.twt))
			{
				cerr << "bad pattern: " << tokens[numbers.size()+1] << endl;
				status=1;
			}
		}
		if (tokens.size() > numbers.size()+2)
		{
			if (parse_aperture(tokens[numbers.size()+2], ag.aperture))
			{
				cerr << "bad aperture: " << tokens[numbers.size()+2] << endl;
				status=1;
			}
		}
		if (tokens.size() > numbers.size() + 3)
		{
			if (parse_double(tokens[numbers.size() + 3], ag.ttf))
			{
				cerr << "bad ttf: " << tokens[numbers.size() + 3] << endl;
				status = 1;
			}
		}
	}
	return status;
}


//int parse_multigrating(const std::string& s, alert::ARMultiGratingSpec& amg)
//{
//	int status = 1;
//
//	// Check if there is a '%' splitting the grating spec and the coord pairs. 
//	vector<string> tokens;
//	boost::split(tokens, s, boost::is_any_of("%"));
//	if (tokens.size() > 0)
//	{
//		status = parse_grating(tokens[0], amg);
//		if (!status)
//		{
//			if (tokens.size() > 1)
//			{
//				// parse x,y pairs
//				int iUnused;
//				vector<double> xy;
//				status = parse_tuning_list(tokens[1], xy, iUnused);
//				if (!status)
//				{
//					for (int i = 0; i < xy.size() - 2; i += 3)
//					{
//						amg.push_back(make_tuple(xy[i], xy[i + 1], xy[i + 2]));
//					}
//				}
//			}
//		}
//	}
//	return status;
//}

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






/*
 * prargs
 * 
 * Process command line args. Automatic handling of response files. 
 * Caller must pass a callback function pfunc(int c, string arg), which 
 * is called for each option/arg found. The callback function must
 * do whatever processing of the option (and arg if relevant) is necessary.
 * The callback should return 0 if all is well, nonzero return will halt
 * command line processing and prargs will return that value. 
 * After all args have been processed without trouble (the callback returned
 * 0 every time), then the callback is called one last time with arg = 0. 
 * The callback function can then do any final processing (check that all 
 * required args were provided, for example). As with a regular arg, the callback
 * should return 0 if all is well, or else return nonzero value. 
 *
 * On successful handling of all args prargs will return 0. 
 */

int prargs(int argc, char **argv, process_args_func pfunc, const char *options, int response_file_char, prargs_handler* handler)
{
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	char *local_argv[2048];
	int local_argc = 0;
	bool stop_processing = false;
	int status = 0;

	// Check that pfunc is not null, then test for response file.
	if (!pfunc && !handler)
	{
		cerr << "prargs: process_args_func and handler is NULL. Must provide a handler for args!" << endl;
		return -1;
	}

	// Tokenize command line args.
	vector<string> vecArgs;
	int i;
	for (i=0; i<argc; i++)
	{
		// Check for response_file_char -- don't put response file arg into the vector. 
		if (response_file_char &&
			argv[i][0] == '-' && argv[i][1] == response_file_char)
		{		
			// the next arg [i+1] is the response filename.
			// tokenize response_file will read that file and append tokens
			// to vecArgs

			// It is an error to have nested response file args!

			if (tokenize_response_file(argv[i+1], vecArgs) < 0)
			{
				return -1;
			}

			i += 1;
		}
		else
		{
			s.assign(argv[i]);
			vecArgs.push_back(s);
		}
	}

	// Now all tokens are in vecArgs. Make argc and argv....
	make_argv(vecArgs, local_argc, local_argv);


	// Now process them with getopt.
	optind = 0;
	while (!stop_processing && (c = getopt(local_argc, local_argv, options)) != -1)
	{
		stringstream ss;
		switch (c) 
		{
		case '?':
			ss.clear();
			ss << (char)c;
			cerr << "unknown arg letter (" << ss.str() << ") optind " << optind << endl;
			status = -1;
			stop_processing = true;
			break;
		default:
			if (optarg)	s.assign(optarg);
			else s.assign("");
			if (pfunc) status = pfunc(c, s);
			else if (handler) status = handler->process_arg(c, s);
			if (status)
				stop_processing = true;
			break;
		}
	}

	// If we made it through all options without complaint, make one more
	// call to the callback with the opt=0. 
	if (c == -1 && !status)
	{
		s.assign("");
		if (pfunc) status = pfunc(0, s);
		else if (handler) status = handler->process_arg(0, s);
	}

	// free the space taken up by our version of the args
	free_argv(local_argc, local_argv);

	return status;
}
