#include "MelStimSet.h"
#include "AttentionStimSet.h"
#include "AlertUtil.h"
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

RectanglePool::~RectanglePool()
{
	for (auto p : m_vec)
	{
		delete p.second;
	}
}

ARContrastRectangleSpec *RectanglePool::getRect(const COLOR_TYPE& c)
{
	ARContrastRectangleSpec *rect = NULL;
	vector<ColorRectPair>& vec = RectanglePool::instance().vec();
	auto it = std::find_if(vec.begin(), vec.end(), [&c](ColorRectPair& p) { return p.first == c; });
	if (it != vec.end())
	{
		rect = it->second;
	}
	else
	{
		rect = new ARContrastRectangleSpec();
		rect->init(2);
		rect->color = c;
		vec.push_back(make_pair(c, rect));
	}
	return rect;
}



int MelStimSet::init(std::vector<int> pages)
{
	m_pagesAvailable = pages;

	// first page is always blank
	vsgSetDrawPage(vsgVIDEOPAGE, m_pagesAvailable[0], vsgBACKGROUND);
	m_pageBlank = m_pagesAvailable[0];

	// initialize fixpt
	PIXEL_LEVEL m_levelWhite;
	ARvsg::instance().request_single(m_levelWhite);
	cerr << "got dummy level " << m_levelWhite << endl;
	cerr << "init fixpt" << endl;
	m_fixpt.init(2);
	m_fixpt.setContrast(100);

	// Initialize color for rects
	//ARvsg::instance().request_single(m_levelWhite);
	//arutil_color_to_palette(COLOR_TYPE(white), m_levelWhite);
	//cerr << "rect color level " << m_levelWhite << endl;
	
	return drawCurrent();
}

int MelStimSet::drawCurrent()
{
	int status = 0;
	int pagesUsed = 1;	// DON'T MESS WITH THE FIRST PAGE
	unsigned int frameLast = 0;		// the last frame value for which we drew a page.
	unsigned int nPages = 1;
	int page = m_pagesAvailable[nPages++];
	m_pageFixpt = page;
	vsgSetDrawPage(vsgVIDEOPAGE, page, vsgBACKGROUND);	

	//cerr << "drawCurrent - start" << endl;
	//cerr << "drawCurrent - fixpt page " << m_pageFixpt << endl;

	// loop over each pair. Assume fixpt always on.
	VSGCYCLEPAGEENTRY cycle[16];	// should  be plenty
	int ncycle = 0;
	for(auto frvpair: m_trialSpecs[m_uiCurrentTrial].vecPairs)
	{
		if (frvpair.first > frameLast)
		{
			// draw fixpt
			m_fixpt.draw();

			// set up cycling element here. 
			cycle[ncycle].Xpos = cycle[ncycle].Ypos = 0;
			cycle[ncycle].Page = page + vsgTRIGGERPAGE;
			cycle[ncycle].Frames = frvpair.first - frameLast;
			cycle[ncycle].Stop = 0;
			ncycle++;

			frameLast = frvpair.first;

			// clear a new page
			page = m_pagesAvailable[nPages++];
			vsgSetDrawPage(vsgVIDEOPAGE, page, vsgBACKGROUND);
		}

		// draw rects
		// different colored rects have to be unique objects
		for (auto rect : frvpair.second)
		{
			ARContrastRectangleSpec *drawrect = RectanglePool::getRect(rect.color);

			// Assign coordinates (after transforming them)
			applyTransform(*drawrect, rect, m_trialSpecs[m_uiCurrentTrial].grid);
			drawrect->draw();
		}
	}

	// draw fixpt
	m_fixpt.draw();

	cycle[ncycle].Xpos = cycle[ncycle].Ypos = 0;
	cycle[ncycle].Page = page + vsgTRIGGERPAGE;
	cycle[ncycle].Frames = m_trialSpecs[m_uiCurrentTrial].lastFrame - frameLast;
	cycle[ncycle].Stop = 0;
	ncycle++;

	// now stop page
	cycle[ncycle].Xpos = cycle[ncycle].Ypos = 0;
	cycle[ncycle].Page = m_pageBlank + vsgTRIGGERPAGE;
	cycle[ncycle].Frames = m_trialSpecs[m_uiCurrentTrial].lastFrame - frameLast;
	cycle[ncycle].Stop = 1;
	ncycle++;

	// set up cycling
	// Set up page cycling
	vsgPageCyclingSetup(ncycle, &cycle[0]);

	cerr << "Cycling: Using " << ncycle << " pages" << endl;
	for (int i = 0; i<ncycle; i++)
	{
		cerr << i << ": page=" << (cycle[i].Page & vsgTRIGGERPAGE ? cycle[i].Page - vsgTRIGGERPAGE : cycle[i].Page) << " Frames=" << cycle[i].Frames << endl;
	}

	vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, vsgNOCLEAR);
	//vsgPresent();


	cerr << "drawCurrent: done." << endl;
	return status;
}

void MelStimSet::applyTransform(ARContrastRectangleSpec& result, const ARContrastRectangleSpec& original, const MelGridSpec& grid)
{
	double ctheta = cos(grid.oriDegrees * M_PI / 180.0);
	double stheta = sin(grid.oriDegrees * M_PI / 180.0);
	double x_scaled = (grid.xGridCenter + original.x * grid.wGrid);
	double y_scaled = (grid.yGridCenter + original.y * grid.hGrid);
	result.x = x_scaled * ctheta - y_scaled * stheta;
	result.y = x_scaled * stheta + y_scaled * ctheta;
	result.w = grid.wGrid * original.w;
	result.h = grid.hGrid * original.h;
	result.orientation = grid.oriDegrees + original.orientation;
	return;
}

// handle the trigger indicated by the string s. Do not call vsgPresent! return value of 
// 1 means vsgPresent() will be called. 
int MelStimSet::handle_trigger(std::string& s)
{
	int status = 0;

	if (s == "F")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixpt, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "S")
	{
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "a")
	{
		m_uiCurrentTrial++;
		if (m_uiCurrentTrial == m_trialSpecs.size())
		{
			m_uiCurrentTrial = 0;
		}
		drawCurrent();
	}
	else if (s == "X")
	{
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "A")
	{
		int ipage;
		string stmp;
		cout << "Enter page: ";
		cin >> ipage;
		vsgSetDrawPage(vsgVIDEOPAGE, ipage, vsgNOCLEAR);
		status = 1;
	}

	return status;
}

std::string MelStimSet::toString() const
{
	ostringstream oss;
	oss << "MelStimSet: " << endl;
	oss << "There are " << m_trialSpecs.size() << " trials";
	//for (auto ts : m_trialSpecs)
	//{
	//	oss << "Trial" << endl;
	//	for (auto frvpair : ts.vecPairs)
	//	{
	//		oss << " frame " << frvpair.first << endl;
	//		for (auto w : frvpair.second)
	//		{
	//			oss << "  rect " << w << endl;
	//		}
	//	}
	//	oss << " frame " << ts.lastFrame << " end" << endl;
	//}
	return oss.str();
}


void addOrAppendfrvPair(vector<FrameRectVecPair>& vecPairs, const FrameRectVecPair& frvPair)
{
	// find the frame number if we can...
	auto it = std::find_if(vecPairs.begin(), vecPairs.end(), [&frvPair](FrameRectVecPair& p) { return p.first == frvPair.first; });
	if (it != vecPairs.end())
	{
		it->second.insert(std::end(it->second), std::begin(frvPair.second), std::end(frvPair.second));
	}
	else
	{
		vecPairs.push_back(frvPair);
	}
	return;
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
		ARContrastRectangleSpec rect;

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
						//cerr << "Start of trial found" << endl;
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
						//cerr << "Found grid line: " << line << endl;

						// update grid with these values. 
						tokens.clear();
						string gridarg(line.substr(line.find("grid") + 4));
						boost::trim(gridarg);
						tokenize(gridarg, tokens, ", ");
						vector<double> vec;
						int nsteps;	// unused

						//cerr << "Grid line split: second half " << gridarg << endl;
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
						//cerr << "Found time marker: " << endl;
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
								addOrAppendfrvPair(spec.vecPairs, frvPair);
								//spec.vecPairs.push_back(frvPair);
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
										std::sort(spec.vecPairs.begin(), spec.vecPairs.end(), [](FrameRectVecPair& a, FrameRectVecPair& b) { return a.first < b.first; });
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
						//cerr << "Found rect line: " << line << endl;
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
