#include "MelStimSet.h"
#include "AttentionStimSet.h"
#include <boost/algorithm/string.hpp>

int MelStimSet::init(std::vector<int> pages)
{
	return 0;
}

// handle the trigger indicated by the string s. Do not call vsgPresent! return value of 
// 1 means vsgPresent() will be called. 
int MelStimSet::handle_trigger(std::string& s)
{
	return 0;
}

std::string MelStimSet::toString() const
{
	return std::string("MelStimSet: toString() not implemented.");
}


int parse_mel_params(const std::string& filename, vector<MelTrialSpec>& trialSpecs)
{
	int status = 0;
	std::ifstream myfile(filename.c_str());
	if (myfile.is_open())
	{
		// open file, read line-by-line and parse
		string line;
		int linenumber = 0;
		int iTrialStep = 0;
		vector<string> tokens;
		MelTrialSpec spec;

		while (!status && getline(myfile, line))
		{
			linenumber++;

			boost::trim(line);
			boost::to_lower(line);

			if (line.length() > 0 && line[0] != '#')
			{
				switch (iTrialStep) {
				case 0:

					// Can have grid or trial
					if (string::npos != line.find("grid"))
					{
						cerr << "Found grid line: " << line << endl;
					}
					else if (string::npos != line.find("trial"))
					{
						cerr << "Start of trial found" << endl;
						iTrialStep = 1;

						// initialize trial spec
						spec.xGridCenter = spec.yGridCenter = spec.wGrid = spec.hGrid = 0;
						spec.lastFrame = 0;
						spec.oriDegrees = 0;
						spec.frv.clear();
					}
					break;

				case 1:
ZZZZZZZZZZZZZZZZZZZ					tokens.clear();
					tokenize(line, tokens, ",");
					if (tokens.size() == 3)
					{
						if (parse_color(tokens[0], spec.color))
						{
							cerr << "Error reading color at line " << linenumber << ": " << tokens[0] << endl;
							status = 1;
							break;
						}
						if (parse_double(tokens[1], spec.initialPhase))
						{
							cerr << "Error reading initial phase at line " << linenumber << ": " << tokens[1] << endl;
							status = 1;
							break;
						}
						if (parse_integer(tokens[2], spec.offbits))
						{
							cerr << "Error reading off bits at line " << linenumber << ": " << tokens[2] << endl;
							status = 1;
							break;
						}

						cerr << "Success! " << spec.color << ", " << spec.initialPhase << ", " << spec.offbits << endl;
						iTrialStep = 2;
					}
					else
					{
						cerr << "Error reading first line of trial spec, expecting three tokens color,initphase,offbits at line " << linenumber << ": " << line << endl;
					}
					break;
				case 2:
					tokens.clear();
					tokenize(line, tokens, ",");
					if (tokens.size() == 3)
					{
						unsigned int index;
						double t;
						int contrast;

						// index
						if (tokens[0].at(0) == 'F') index = FixptIndex;
						else if (tokens[0].at(0) == 'Q') index = CueIndex;
						else if (tokens[0].at(0) == '*') index = EndIndex;
						else if (parse_uint(tokens[0], index))
						{
							cerr << "Error reading stim index (expect \"F\", \"Q\", or 0<int<#gratings) line " << linenumber << ": " << tokens[0] << endl;
							status = 1;
							break;
						}

						// time
						if (parse_double(tokens[1], t))
						{
							cerr << "Error reading time from second token at line " << linenumber << ": " << line << endl;
							status = 1;
							break;
						}

						if (parse_integer(tokens[2], contrast))
						{
							cerr << "Error reading contrast from third token at line " << linenumber << ": " << line << endl;
							status = 1;
							break;
						}

						// convert the time value to frames. 
						unsigned long frames = SECONDS_TO_FRAMES(t);

						// Add this bad boy to the map
						spec.icpm.insert(make_pair(frames, ICPair(index, contrast)));

						// If this was the end index, the trial is complete. 
						// Push the map for this trial onto the vector of trial specs,
						// and reset the step so we look for another "Trial" line. 
						if (EndIndex == index)
						{
							cerr << "Finished parsing trial " << trialSpecs.size() + 1 << endl;
							trialSpecs.push_back(spec);
							iTrialStep = 0;
						}
					}
					else
					{
						status = 1;
						cerr << "Error reading second line of trial spec, expecting three tokens [FQ01...],t,contrast at line " << linenumber << ": " << line << endl;
					}
					break;
				default:
					cerr << "Error - bad trial step!!! " << iTrialStep << endl;
					status = 1;
				}
			}
		}
		myfile.close();
		cerr << "Read " << linenumber << " lines from " << filename << " and found " << trialSpecs.size() << " trials" << endl;
	}
	else
	{
		cerr << "Cannot open input file: " << filename << endl;
		status = 1;
	}
	return status;
}
