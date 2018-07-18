#include "MelStimSet.h"
#include "AttentionStimSet.h"
#include "AlertUtil.h"
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <algorithm>
using namespace std;

int MelStimSet::init(std::vector<int> pages)
{
	m_pagesAvailable = pages;

	// first page is always blank
	vsgSetDrawPage(vsgVIDEOPAGE, m_pagesAvailable[0], vsgBACKGROUND);

	// Initialize color for rects
	m_levelWhite = ARvsg::instance().request_single();
	arutil_color_to_palette(COLOR_TYPE(white), m_levelWhite);

	// initialize fixpt
	m_fixpt.init(2);

	
	return drawCurrent();
}

int MelStimSet::drawCurrent()
{
	int status = 0;
	int pagesUsed = 1;	// DON'T MESS WITH THE FIRST PAGE

	// loop over each pair. Assume fixpt always on.
	for(auto frvpair: m_trialSpecs[m_uiCurrentTrial].vecPairs)
	{
		cerr << "drawCurrent: frames=" << frvpair.first << endl;
	}
	return status;
}

// handle the trigger indicated by the string s. Do not call vsgPresent! return value of 
// 1 means vsgPresent() will be called. 
int MelStimSet::handle_trigger(std::string& s)
{
	return 0;
}

std::string MelStimSet::toString() const
{
	ostringstream oss;
	oss << "MelStimSet: " << endl;
	oss << "There are " << m_trialSpecs.size() << " trials";
	for (auto ts : m_trialSpecs)
	{
		oss << "Trial" << endl;
		for (auto frvpair : ts.vecPairs)
		{
			oss << " frame " << frvpair.first << endl;
			for (auto w : frvpair.second)
			{
				oss << " rect " << w << endl;
			}
		}
	}
	return oss.str();
}


int parse_mel_params(const std::string& filename, vector<MelTrialSpec>& trialSpecs)
{
	int status = 0;
	trialSpecs.clear();
	std::ifstream myfile(filename.c_str());
	if (myfile.is_open())
	{
		// open file, read line-by-line and parse
		string line;
		int linenumber = 0;
		int iTrialStep = 0;
		vector<string> tokens;
		MelTrialSpec spec;
		MelGridSpec grid;
		FrameRectVecPair frvPair;
		ARRectangleSpec rect;

		// grid starts out with default values
		grid.xGridCenter = grid.yGridCenter = 0;
		grid.oriDegrees = 0;
		grid.wGrid = grid.hGrid = 1;

		while (!status && getline(myfile, line))
		{
			linenumber++;

			boost::trim(line);
			boost::to_lower(line);

			if (line.length() > 0 && line[0] != '#')
			{
				switch (iTrialStep) {
				case 0:

					// Expecting "trial"
					if (string::npos != line.find("trial"))
					{
						cerr << "Start of trial found" << endl;
						iTrialStep = 1;

						// initialize trial spec
						spec.lastFrame = 0;
						spec.vecPairs.clear();
						spec.grid = grid;
						frvPair.first = 0;
						frvPair.second.clear();
					}
					else
					{
						cerr << "Unexpected input at line " << linenumber << " - looking for \"trial\": " << line << endl;
						status = 1;
					}
					break;

				case 1:
					// at this point we expect a "grid" or "time" or "rect"
					if (string::npos != line.find("grid"))
					{
						cerr << "Found grid line: " << line << endl;

						// update grid with these values. 
						tokens.clear();
						string gridarg(line.substr(line.find("grid") + 4));
						boost::trim(gridarg);
						tokenize(gridarg, tokens, ", ");
						vector<double> vec;
						int nsteps;	// unused

						cerr << "Grid line split: second half " << gridarg << endl;
						if (parse_tuning_list(tokens, vec, nsteps))
						{
							cerr << "Error parsing grid line: " << line << endl;
						}
						else
						{
							if (vec.size() > 0)
								grid.xGridCenter = vec[0];
							if (vec.size() > 1)
								grid.yGridCenter = vec[1];
							if (vec.size() > 2)
								grid.wGrid = vec[2];
							if (vec.size() > 3)
								grid.hGrid = vec[3];
							if (vec.size() > 4)
								grid.oriDegrees = vec[4];
							if (vec.size() > 5)
							{
								cerr << "Error parsing grid at line " << linenumber << "  (expect <= 5 values): " << line << endl;
								status = 1;
							}
							else
							{
								spec.grid = grid;
							}
						}

						// do not change ste value - stay at 1

					}
					else if (string::npos != line.find("time"))
					{
						cerr << "Found time marker: " << endl;
						tokens.clear();
						tokenize(line, tokens, ", ");
						if (tokens.size() > 1)
						{
							double t;
							unsigned int frames = 0;
							if (parse_double(tokens[1], t))
							{
								cerr << "Error parsing time at line " << linenumber << endl;
								status = 1;
							}
							else
							{
								// The "time" line ends the last "time" block. Push that frvPair onto the
								// current spec. 
								spec.vecPairs.push_back(frvPair);
								frvPair.second.clear();

								if (tokens.size() == 2)
								{
									// save the number of frames in the frvPair placeholder 
									frvPair.first = SECONDS_TO_FRAMES(t);
								}
								else
								{
									if (tokens.size() == 3 && boost::iequals(tokens[2], "end"))
									{
										// this signifies the end of the trial. 
										// save the end frame in the current trial spec, and push the whole thing 
										// onto the trialSpecs vector. Return to trialStep 0 - look for "trial"
										spec.lastFrame = SECONDS_TO_FRAMES(t);
										spec.grid = grid;
										trialSpecs.push_back(spec);
										iTrialStep = 0;
									}
									else
									{
										cerr << "Error in input at line " << linenumber << " - expecting time t [end]: " << line << endl;
										status = 1;
									}
								}
							}
						}
						else
						{
							cerr << "Error on line " << linenumber << " : expecting time value" << endl;
							status = 1;
						}
					}
					else if (string::npos != line.find("rect"))
					{
						cerr << "Found rect line: " << line << endl;
						if (parse_rectangle(line.substr(line.find("rect") + 4), rect))
						{
							cerr << "Error parsing rect on line " << linenumber << ": " << line << endl;
							status = 1;
						}
						else
						{
							frvPair.second.push_back(rect);
						}
					}
					else
					{
						cerr << "Error input at line " << linenumber << " - expecting \"time\" or \"grid\" or \"rect\": " << line << endl;
						status = 1;
					}
					break;
				default:
					cerr << "Error - unknown state " << iTrialStep << endl;
					status = 1;
					break;
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
