#include "MultiSacStimSet.h"
#include "AlertUtil.h"
#include <iostream>
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

MultiSacStimSet* parseMultiSacStimSet(const std::string& filename, alert::ARContrastFixationPointSpec& fixpt)
{
	MultiSacStimSet* pStimSet = nullptr;
	AllTrialsVector atv;

	boost::filesystem::path p(filename);
	if (!exists(p))
	{
		std::cerr << "Error: multisac trials file does not exist: " << filename << endl;
		return nullptr;
	}
	else
	{
		// open file, read line-by-line and parse
		string line;
		int linenumber = 0;		// count lines from 0

		std::ifstream myfile(filename.c_str());
		if (myfile.is_open())
		{
			while (getline(myfile, line))
			{
				linenumber++;

				// skip comment line
				if (line[0] == '#')
					continue;

				// skip empty lines
				boost::algorithm::trim(line);
				if (line.size() == 0)
					continue;

				// tokenize into pipe-separated chunks.
				// Each token is the content of a FRAME. 
				// Each FRAME can be BKGD, or a colon: separated sequence of gratings with a G prefix e.g. "G 1,2,.... : G 2,3,.... : G 4,5,...."
				// The FRAMES make up a trial. 

				SingleTrialVector trialvec;
				std::vector<std::string> frame_tokens;
				tokenize(line, frame_tokens, "|");
				std::cerr << "Got " << frame_tokens.size() << " frame tokens on line " << linenumber << std::endl;
				for (auto frame_token : frame_tokens)
				{
					// Each token represents a single screen FRAME that will be displayed. 
					// The simplest frame is blank - the text should be BKGD.
					// The last token on the line should be END
					// Each FRAME in a trial is represented by a GratingVector
					FrameVector gvec;
					boost::algorithm::trim(frame_token);
					std::cerr << "token " << frame_token << std::endl;

					if (boost::algorithm::iequals(frame_token, "BKGD"))
					{
						trialvec.push_back(gvec);	// pushing an empty vector means nothing gets drawn == BKGD
					}
					else if (boost::algorithm::iequals(frame_token, "END"))
					{
						// don't push anything else into the trial vector - this trial is finished with END
						break;
					}
					else
					{
						// tokenize on ":"
						std::vector<std::string> stim_tokens;
						tokenize(frame_token, stim_tokens, ":");
						std::cerr << "Got " << stim_tokens.size() << " stim tokens" << std::endl;
						for (auto stim_token : stim_tokens)
						{
							std::cerr << "Stim token " << stim_token << std::endl;
							std::string stmp;
							boost::algorithm::trim(stim_token);
							vector<string> strs;
							boost::split(strs, stim_token, boost::is_any_of(" "));
							if (strs.size() == 2 && boost::iequals(strs[0], "G"))
							{
								alert::ARGratingSpec grating;
								if (!parse_grating(strs[1], grating))
									gvec.push_back(grating);
								else
								{
									std::cerr << "Cannot read grating spec at line " << linenumber << ": " << stim_token << std::endl;
									return nullptr;
								}
							}
							else
							{
								std::cerr << "Cannot frame token at line " << linenumber << ": " << stim_token << std::endl;
								return nullptr;
							}
						}
						trialvec.push_back(gvec);
					}
					atv.push_back(trialvec);
				}
			}
			myfile.close();
		}
	}
	return pStimSet;
}
	//double x, y;
	//double s0, s1;
	//COLOR_TYPE c0, c1;
	//if (parse_double(tokens[0], x) || parse_double(tokens[1], y))
	//{
	//	cerr << "parseBorderStimSet: bad x,y value(s) in args 0,1: " << tokens[0] << "," << tokens[1] << endl;
	//	return nullptr;
	//}

	//if (parse_double(tokens[2], s0) || s0 < .01 || parse_double(tokens[3], s1) || s1 < 0.01)
	//{
	//	cerr << "parseBorderStimSet: bad size value(s) in args 2,3: " << tokens[2] << "," << tokens[3] << endl;
	//	return nullptr;
	//}

	//if (parse_color(tokens[4], c0) || parse_color(tokens[5], c1))
	//{
	//	cerr << "parseBorderStimSet: bad color value(s) in args 4,5: " << tokens[4] << "," << tokens[5] << endl;
	//	return nullptr;
	//}

	//// parse remaining values as stim specs......
	//std::vector<std::string> specs(tokens.cbegin() + 6, tokens.cend());
	//std::vector<int> stim;
	//if (parse_int_list(specs, stim))
	//{
	//	cerr << "parseBorderStimSet: bad spec list: " << s << endl;
	//	return nullptr;


std::string MultiSacStimSet::toString() const
{
	std::string s("MultiSacStimSet");
	return s;
}


int MultiSacStimSet::init(std::vector<int> pages, int)
{
	//// first page is for background only
	//m_pageBackground = pages[0];
	//m_pageStimulus = pages[1];
	//m_pageStimulusFixpt = pages[2];

	//// allocate a spot for the two colors we need.....
	//ARvsg::instance().request_single(m_levelColor0);
	//arutil_color_to_palette(m_color0, m_levelColor0);
	//ARvsg::instance().request_single(m_levelColor1);
	//arutil_color_to_palette(m_color1, m_levelColor1);

	//// init the rect levels, but don't bother setting x,y, color, size until drawCurrent()
	//m_rect.init(2);

	//// background page will not change
	//vsgSetDrawPage(vsgVIDEOPAGE, m_pageBackground, vsgBACKGROUND);

	//m_uiCurrentTrial = 0;
	//drawCurrent();

	//vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	return 0;
}

int MultiSacStimSet::handle_trigger(const std::string& s, const std::string&)
{
	int status = 0;
	//if (s == "F")
	//{
	//	cerr << "Fixpt not supported" << endl;
	//}
	//else if (s == "S")
	//{
	//	vsgSetDrawPage(vsgVIDEOPAGE, m_pageStimulus, vsgNOCLEAR);
	//	status = 1;
	//}
	//else if (s == "a")
	//{
	//	m_uiCurrentTrial++;
	//	if (m_uiCurrentTrial >= m_vecStim.size()) m_uiCurrentTrial = 0;
	//	drawCurrent();
	//	vsgSetDrawPage(vsgVIDEOPAGE, m_pageBackground, vsgNOCLEAR);
	//	status = 1;
	//}
	//else if (s == "X")
	//{
	//	vsgSetDrawPage(vsgVIDEOPAGE, m_pageBackground, vsgNOCLEAR);
	//	status = 1;
	//}
	return status;
}


int MultiSacStimSet::drawCurrent()
{
	return 0;
}

