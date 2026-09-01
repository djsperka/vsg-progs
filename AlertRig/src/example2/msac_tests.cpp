#include "Alertlib.h"
#include <vector>
#include <istream>
#include <ostream>
#include <string>
#include <boost/algorithm/string/case_conv.hpp>
using namespace std;
using namespace boost::algorithm;


// Specification of a single frame (a trial consists of a series of frames)
struct msac_frame
{
	std::vector<alert::ARGratingSpec> gratings;
};
typedef struct msac_frame msac_frame_t;

// a single trial specified here
struct msac_trial
{
	std::vector<msac_frame_t> frames;
};
typedef struct msac_trial msac_trial_t;

typedef std::vector<msac_trial_t> msac_trial_list_t;

std::istream& operator>>(std::istream& ins, msac_trial_list_t& trials);
std::istream& operator>>(std::istream& ins, msac_trial_t& trial);
std::istream& operator>>(std::istream& in, msac_frame_t& stim);

std::ostream& operator<<(std::ostream& ins, const msac_frame_t& frame);
std::ostream& operator<<(std::ostream& ins, const msac_trial_t& trial);

// parse trials file
bool parse_msac_trials_file(const std::string& filename, msac_trial_list_t& trials);

std::ostream& operator<<(std::ostream& ins, const msac_trial_list_t& trials)
{
	ins << "Trial list:" << endl;
	for (auto trial : trials)
	{
		ins << trial << endl;
	}
	ins << "Trial list done" << endl;
	return ins;
}


std::ostream& operator<<(std::ostream& ins, const msac_frame_t& frame)
{
	ins << "\tFrame: ";
	for (auto grating : frame.gratings)
		ins << grating << " ";
	ins << "End";
	return ins;
}

std::ostream& operator<<(std::ostream& ins, const msac_trial_t& trial)
{
	ins << "Trial:" << endl;
	for (auto frame : trial.frames)
	{
		ins << frame << endl;
	}
	ins << "Trial done" << endl;
	return ins;
}



// Expecting ":" delimited list of gratings, or "BKGD". Terminated by "END"
std::istream& operator>>(std::istream& in, msac_frame_t& frame)
{
	string tmp;
	//std::stringstream buffer;
	//buffer << in.rdbuf();
	//tmp = buffer.str();
	while (std::getline(in, tmp, ':'))
	{
		// BKGD
		if (boost::algorithm::iequals(tmp, "BKGD"))
			break;
		else if (boost::algorithm::iequals(tmp, "END"))
			break;
		else
		{
			std::vector<std::string> result;
			boost::algorithm::split(result, boost::algorithm::trim_copy(tmp), boost::is_any_of(" "));
			if (result.size() == 2 && boost::algorithm::iequals(result[0], "G"))
			{
				alert::ARGratingSpec grating;
				if (parse_grating(result[1], grating))
					throw "Error parsing grating";
				else
				{
					frame.gratings.push_back(grating);
				}
			}
		}
	}

	// expecting either one or two strings in a single frame item. 
	// A single string can be BKGD (the frame has no gratings) or END (end of all frames for trial)

	//// tokenize/split at the pipe symbol
	//tokenize(line, tokens, "|");

	//// Each string between pipes is a frame
	//for (auto s_frame : tokens)
	//{
	//	msac_frame_t frame;
	//	fs.str(s_frame);
	//	fs.clear();
	//	fs >> frame;
	//	trial.frames.push_back(frame);
	//}
	//return ins;
	return in;
}


// A line in the input file corresponds to a single trial
std::istream& operator>>(istream& ins, msac_trial_t& trial)
{
	string line;
	stringstream fs;
	vector<string> tokens;


	// get a line nonzero length
	getline(ins, line);
	while (!ins.eof() && line.size() == 0)
		getline(ins, line);

	if (ins.eof())
		return ins;

	// tokenize/split at the pipe symbol
	tokenize(line, tokens, "|");

	// Each string between pipes is a frame
	for (auto s_frame : tokens)
	{
		trim(s_frame);
		if (iequals(s_frame, "END"))
			break;
		msac_frame_t frame;
		fs.str(s_frame);
		fs.clear();
		fs >> frame;
		trial.frames.push_back(frame);
	}
	return ins;
}


istream& operator>>(istream& ins, msac_trial_list_t& trials)
{
	trials.clear();
	msac_trial_t t;
	while (ins >> t)
	{
		trials.push_back(t);
		t.frames.clear();
	}
	return ins;
}

bool parse_msac_trials_file(const string& filename, msac_trial_list_t& trials)
{
	bool b = false;
	std::ifstream ifs(filename);

	std::cerr << "Open file " << filename << std::endl;
	if (ifs.is_open())
	{
		std::cerr << "Read trials" << std::endl;
		try
		{
			ifs >> trials;
			b = true;
		}
		catch (string s)
		{
			std::cerr << "Error: " << s << endl;
		}
		ifs.close();
	}
	return b;
}


int main(int argc, char* argv[])
{
	std::cerr << "Read file " << string(argv[1]) << endl;
	msac_trial_list_t trials;
	bool b = parse_msac_trials_file(argv[1], trials);
	std::cerr << "Read file " << string(argv[1]) << ": " << b << endl;
	std::cerr << trials << endl;
	return 0;
}
