#include "ConteMisc.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <string>

using namespace std;

bool parse_conte_stim_params(const std::vector<string>& tokens, unsigned int first, conte_stim_params_t& stim);


istream& operator>>(istream& ins, conte_trial_list_t& trials)
{
	trials.clear();
	conte_trial_t t;
	while (ins >> t)
	{
		trials.push_back(t);
	}
	return ins;
}

istream& operator>>(istream& ins, conte_trial_t& trial)
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

	tokenize(line, tokens, ",");	

	fs.str(tokens[0]);
	fs.clear();
	fs >> trial.cue_x;

	fs.str(tokens[1]);
	fs.clear();
	fs >> trial.cue_y;

	fs.str(tokens[2]);
	fs.clear();
	fs >> trial.cue_w;

	fs.str(tokens[3]);
	fs.clear();
	fs >> trial.cue_h;

	fs.str(tokens[4]);
	fs.clear();
	fs >> trial.cue_d;

	fs.str(tokens[5]);
	fs.clear();
	fs >> trial.cue_fpt;

	fs.str(tokens[6]);
	fs.clear();
	fs >> trial.cue_nterms;

	fs.str(tokens[7]);
	fs.clear();
	fs >> trial.cue_to_sample_delay_ms;

	fs.str(tokens[8]);
	fs.clear();
	fs >> trial.sample_display_ms;

	fs.str(tokens[9]);
	fs.clear();
	fs >> trial.sample_to_target_delay_ms;

	fs.str(tokens[10]);
	fs.clear();
	fs >> trial.target_display_ms;

	fs.str(tokens[11]);
	fs.clear();
	fs >> trial.saccade_response_time_ms;

	fs.str(tokens[12]);
	fs.clear();
	fs >> trial.cue_color_0;

	fs.str(tokens[13]);
	fs.clear();
	fs >> trial.cue_color_1;

	parse_conte_stim_params(tokens, 14, trial.s0);
	parse_conte_stim_params(tokens, 25, trial.s1);
	parse_conte_stim_params(tokens, 36, trial.t0);
	parse_conte_stim_params(tokens, 47, trial.t1);

	return ins;
}

bool parse_conte_stim_params(const std::vector<string>& tokens, unsigned int first, conte_stim_params_t& stim)
{
	stringstream fs;
	
	fs.str(tokens[first]);
	fs.clear();
	fs >> stim.x;

	fs.str(tokens[first + 1]);
	fs.clear();
	fs >> stim.y;

	fs.str(tokens[first + 2]);
	fs.clear();
	fs >> stim.w;

	fs.str(tokens[first + 3]);
	fs.clear();
	fs >> stim.h;

	fs.str(tokens[first + 4]);
	fs.clear();
	fs >> stim.ori;

	fs.str(tokens[first + 5]);
	fs.clear();
	fs >> stim.sf;

	fs.str(tokens[first + 6]);
	fs.clear();
	fs >> stim.phase;

	fs.str(tokens[first + 7]);
	fs.clear();
	fs >> stim.divisor;

	fs.str(tokens[first + 8]);
	fs.clear();
	fs >> stim.isHorizontal;

	fs.str(tokens[first + 9]);
	fs.clear();
	fs >> stim.lwt;

	fs.str(tokens[first + 10]);
	fs.clear();
	fs >> stim.icolor;

	return true;
}


bool parse_trials_file(const std::string& filename, conte_trial_list_t& trials)
{
	bool b = false;
	ifstream ifs(filename);
	if (ifs.is_open())
	{
		ifs >> trials;
		ifs.close();
		b = true;
	}
	return b;
}

bool parse_dot_supply_file(const std::string& filename, ConteCueDotSupply& dotsupply)
{
	bool bReturn = false;

	boost::filesystem::path p(filename);
	if (!exists(p))
	{
		std::cerr << "Error: dot supply file does not exist: " << filename << endl;
		return false;
	}
	else
	{
		bReturn = true;
		boost::filesystem::path folder = p.parent_path();		// if file has relative pathnames to images, they are relative to dir file lives in
		std::cerr << "Found dot supply file " << p << " at path " << folder << std::endl;

		ifstream ifs(filename, ios::in | ios::binary);

		unsigned int N[2];
		double d[1000];				// hard limit to 500 points per patch

		while (ifs.read((char*)N, 2 * sizeof(int)) && ifs.read((char*)d, 2 * (N[0] + N[1]) * sizeof(double)))
		{
			dotsupply.add_patch(N[0], N[1], d);
		}
		ifs.close();
	}
	return bReturn;
}




error_t parse_conte_opt(int key, char* carg, struct argp_state* state)
{
	error_t ret = 0;
	struct conte_arguments* arguments = (struct conte_arguments*)state->input;
	COLOR_TYPE color;
	std::string sarg;
	if (carg) sarg = carg;
	switch (key)
	{
	case 'a':
		arguments->bBinaryTriggers = false;
		break;
	case 'v':
		arguments->bVerbose = true;
		break;
	case 'b':
		if (parse_color(sarg, arguments->bkgdColor))
			ret = EINVAL;
		break;
	case 'd':
		if (parse_distance(sarg, arguments->iDistanceToScreenMM))
			ret = EINVAL;
		else
			arguments->bHaveDistance = true;
		break;
	case 'c':
		if (parse_color(sarg, color))
			ret = EINVAL;
		else
			arguments->colors.push_back(color);
		break;
	case 'l':
		if (parse_integer(sarg, arguments->iReadyPulseDelay))
			ret = EINVAL;
		break;
	case 'p':
		if (parse_integer(sarg, arguments->iPulseBits))
			ret = EINVAL;
		break;
	case 'f':
		if (parse_fixation_point(sarg, arguments->fixpt))
			ret = EINVAL;
		else
		{
			arguments->bHaveFixpt = true;
		}
		break;
	case 701:
		if (!boost::filesystem::exists(sarg))
		{
			cerr << "Cannot find dot supply file" << endl;
			ret = EINVAL;
		}
		else
		{
			if (!parse_dot_supply_file(sarg, arguments->dot_supply))
			{
				cerr << "Error parsing dot supply file" << endl;
				ret = EINVAL;
			}
			else
			{
				cerr << "Found " << arguments->dot_supply.npatches() << " patches " << endl;
			}
		}
		break;
	case 702:
		// parse conte stim spec
		if (!boost::filesystem::exists(sarg))
		{
			cerr << "Cannot find trials spec file" << endl;
			ret = EINVAL;
		}
		else
		{
			if (parse_trials_file(sarg, arguments->trials))
			{
				cerr << "Number of trials: " << arguments->trials.size() << endl;
			}
			else
			{
				cerr << "Cannot read trials file: " << sarg << endl;
				ret = EINVAL;
			}
		}
		break;
	case ARGP_KEY_END:
		{
			// check that everything needed has been received
			unsigned int num_patches = arguments->dot_supply.size();
			unsigned int num_patches_needed = 0;
			for (auto tr : arguments->trials)
				num_patches_needed += tr.cue_nterms;
			cerr << "Trials require " << num_patches_needed << " patches, dot supply has " << num_patches << endl;
			if (num_patches_needed > num_patches)
			{
				ret = EINVAL;
				cerr << "NOT ENOUGH PATCHES!" << endl;
			}
			else if (num_patches_needed < num_patches)
			{
				cerr << endl << "WARNING: There are more patches than are needed!" << endl << endl;
			}
		}
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return ret;
}
