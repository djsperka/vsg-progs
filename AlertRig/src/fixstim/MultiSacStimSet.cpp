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


std::ostream& operator<<(std::ostream& ins, const msac_page_t& page)
{
	ins << "\tpage: ";
	for (auto grating : page.gratings)
		ins << grating << " ";
	ins << "End";
	return ins;
}

std::ostream& operator<<(std::ostream& ins, const msac_trial_t& trial)
{
	ins << "Trial:" << endl;
	for (auto page : trial.pages)
	{
		ins << page << endl;
	}
	ins << "Trial done" << endl;
	return ins;
}



// Expecting ":" delimited list of gratings, or "BKGD". Terminated by "END"
std::istream& operator>>(std::istream& in, msac_page_t& page)
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
					page.gratings.push_back(grating);
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

	// Each string between pipes is a page
	for (auto s_page : tokens)
	{
		trim(s_page);
		if (iequals(s_page, "END"))
			break;
		msac_page_t page;
		fs.str(s_page);
		fs.clear();
		fs >> page;
		trial.pages.push_back(page);
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
		t.pages.clear();
	}
	return ins;
}

//bool parse_msac_trials_file(const string& filename, msac_trial_list_t& trials)
//{
//	bool b = false;
//	std::ifstream ifs(filename);
//
//	std::cerr << "Open file " << filename << std::endl;
//	if (ifs.is_open())
//	{
//		try
//		{
//			ifs >> trials;
//			b = true;
//		}
//		catch (string s)
//		{
//			std::cerr << "Error: " << s << endl;
//		}
//		ifs.close();
//	}
//	return b;
//}


MultiSacStimSet* createMultiSacStimSet(const std::string& filename, alert::ARContrastFixationPointSpec& fixpt)
{
	MultiSacStimSet* pStimSet = nullptr;
	msac_trial_list_t trials;
	ifstream ifs(filename);

	std::cerr << "Open file " << filename << std::endl;
	if (ifs.is_open())
	{
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
		if (trial.pages.size() > m)
			m = trial.pages.size();
	return m;
};

// Figure out how many gratings are needed (max number used in a trial)
// initialize that many gratings in gratings()

int MultiSacStimSet::init(std::vector<int> pages, int)
{
	int nGratings = 0;
	int nLevels = 0;

	// save the pages to use
	m_pages = pages;

	for (auto trial : m_trials)
	{
		int n = 0;
		for (auto page : trial.pages)
			n += page.gratings.size();
		nGratings = max(nGratings, n);
	}

	// figure out how many levels per grating. Max of 12 is arbitrary.
	nLevels = 40;
	if (nGratings <= 6)
		nLevels = 40;
	else if (nGratings > 6 && nGratings < 9)
		nLevels = 30;
	else if (nGratings < 13)
		nLevels = 20;
	else
	{
		cerr << "Too many gratings! Max 12.";
		return 1;
	}

	// now populate the gratings we need and init them
	ARGratingSpec g;
	for (int i = 0; i < nGratings; i++)
	{
		add_grating(g);
		this->grating(i).init(nLevels);
	}

	// init the fixpt
	this->fixpt().init(2);

	//// background page will not change
	//vsgSetDrawPage(vsgVIDEOPAGE, m_pageBackground, vsgBACKGROUND);

	m_uiCurrentTrial = 0;
	drawCurrent();
	vsgPresent();
	//vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	return 0;
}

int MultiSacStimSet::handle_trigger(const std::string& s, const std::string&)
{
	int status = 0;
	if (s == "F")
	{
		this->fixpt().setContrast(100);
		status = 1;
	}
	else if (s == "f")
	{
		this->fixpt().setContrast(0);
		status = 1;
	}
	else if (s == "S")
	{
		// restore original contrast for gratings
		for (int i = 0; i < m_nGratingsCurrentTrial; i++)
			this->grating(i).unhide();
		status = 1;
	}
	else if (s == "s")
	{
		// set contrast to 0 for all gratings
		for (int i = 0; i < m_nGratingsCurrentTrial; i++)
			this->grating(i).hide();
		status = 1;
	}
	else if (s == "v")
	{
		// move to next page
		m_uiCurrentPageIndex++;
		if (m_uiCurrentPageIndex < m_trials[m_uiCurrentTrial].pages.size())
			vsgSetDrawPage(vsgVIDEOPAGE, m_pages[m_uiCurrentPageIndex], vsgNOCLEAR);
		else
			cerr << "ERROR - cannot advance this far. Not enough pages in this trial." << endl;
		status = 1;
	}
	else if (s == "a")
	{
		m_uiCurrentTrial++;
		if (m_uiCurrentTrial >= m_trials.size())
		{
			m_uiCurrentTrial = 0;
			cerr << "WARNING - out of trials - starting over with first trial" << endl;
		}
		drawCurrent();
		m_uiCurrentPageIndex = 0;
		status = 1;		// will draw initial page for trial - which should be blank (all contrast=0) after drawCurrent() is called.
	}
	else if (s == "X")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		status = 1;
	}
	return status;
}


// draw the current trial onto vsg pages. 
// current trial is m_trials[m_uiCurrentTrial]
// trial contains one or more pages (a vector of pages).
// each page has zero or more gratings. 
// For each page, draw any gratings first, then the fixpt. 
// We're going to turn gratings on and off all at once (even if they're not on current page)
// If that changes, we will need to keep track of which gratings are drawn on which page. 
//
// All objects are drawn with contrast=0
// Current draw page is left at the first page for this trial
int MultiSacStimSet::drawCurrent()
{
	int nPages = m_trials[m_uiCurrentTrial].pages.size();
	int iPage = 0;
	m_nGratingsCurrentTrial = 0;
	for (auto page : m_trials[m_uiCurrentTrial].pages)
	{
		// clear vsg page
		vsgSetDrawPage(vsgVIDEOPAGE, m_pages[iPage], vsgBACKGROUND);

		// draw each grating specified for this page 
		for (auto g : page.gratings)
		{
			this->grating(m_nGratingsCurrentTrial).assignGratingProperties(g);
			this->grating(m_nGratingsCurrentTrial).draw();
			this->grating(m_nGratingsCurrentTrial).hide();
			m_nGratingsCurrentTrial++;
		}

		// draw fixpt
		this->fixpt().draw();
		this->fixpt().setContrast(0);

		// increment page
		iPage++;
	}
	vsgSetDrawPage(vsgVIDEOPAGE, m_pages[0], vsgNOCLEAR);
	return 0;
}

