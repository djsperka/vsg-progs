#include "MultiSacStimSet.h"
#include "AlertUtil.h"
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
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


MultiSacStimSet* createMultiSacStimSet(const std::string& filename, alert::ARContrastFixationPointSpec& fixpt)
{
	MultiSacStimSet* pStimSet = nullptr;
	msac_trial_list_t trials;
	ifstream ifs(filename);

	std::cerr << "Open file " << filename << std::endl;
	if (ifs.is_open())
	{
		std::cerr << "Read trials" << std::endl;
		ifs >> trials;
		ifs.close();
		pStimSet = new MultiSacStimSet(fixpt, trials);
	}
	else
	{
		std::cerr << "Cannot open msac trials file " << filename << endl;
	}
	return pStimSet;
}

std::string MultiSacStimSet::toString() const
{
	std::string s("MultiSacStimSet");
	return s;
}

int MultiSacStimSet::num_pages() 
{
	int m = 0;
	for (auto trial : m_trials)
		if (trial.frames.size() > m)
			m = trial.frames.size();
	return m;
};

// Figure out how many gratings are needed (max number used in a trial)
// initialize that many gratings in gratings()

int MultiSacStimSet::init(std::vector<int> pages, int)
{
	int nGratings = 0;
	for (auto trial : m_trials)
	{
		int n = 0;
		for (auto frame : trial.frames)
			n += frame.gratings.size();
		nGratings = max(nGratings, n);
	}
ZZZZZZZZZZZZZZ
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

