#include "SequencedImagesAttentionStimSet.h"

SequencedImagesAttentionStimSet::SequencedImagesAttentionStimSet(ARContrastFixationPointSpec& fixpt, const ImageFilesPositions& ifp, vector<AttentionCue>& vecCuePairs, bool bCueCircles, bool bCuePoints, bool bCueIsDot, vector<AttentionSequenceTrialSpec>& trialSpecs)
	: m_fixpt(fixpt)
	, m_ifp(ifp)
	, m_vecCuePairs(vecCuePairs)
	, m_trialSpecs(trialSpecs)
	, m_current(0)
{
	cout << "SequencedImagesAttentionStimSet: have " << m_trialSpecs.size() << " trials." << endl;
}

std::string SequencedImagesAttentionStimSet::toString() const
{
	return string("SequencedImagesAttentionStimSet::toString() - not implemented");
}


int SequencedImagesAttentionStimSet::handle_trigger(std::string& s)
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
		m_current++;
		if (m_current == m_trialSpecs.size())
		{
			m_current = 0;
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


// reset video mode
void SequencedImagesAttentionStimSet::cleanup(std::vector<int> pages)
{
	vsgSetVideoMode(vsg8BITPALETTEMODE);
}

int SequencedImagesAttentionStimSet::init(ARvsg& vsg, std::vector<int> pages)
{
	int status = 0;
	m_pages = pages;
	cout << "have " << m_pages.size() << " pages";

	// Set the VSG to True colour mode
	// should this also set vsgNOGAMMACORRECT?
	vsgSetVideoMode(vsgTRUECOLOURMODE);

	//Disable all of the draw modes.
	vsgSetDrawMode(vsgCENTREXY);

	// draw 
	drawCurrent();

	return status;
}

void SequencedImagesAttentionStimSet::drawCurrent()
{
	VSGCYCLEPAGEENTRY cycle[16];	// should  be plenty
	int ncycle = 0;

	cerr << endl << endl << "Begin drawCurrent() - draw vsg pages and set up animation for trial" << endl;

	// Set color of fixpt...
	cout << "drawCurrent: draw " << m_current << " of " << m_trialSpecs.size() << endl;
	m_fixpt.color = m_trialSpecs[m_current].color;

	// blank page =================
	m_pageBlank = m_pages[0];
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, 127 * (256 + 256*256));
	cerr << "Configure page " << m_pageBlank << " background only" << endl;

	// fixpt
	m_pageFixpt = m_pages[1];
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixpt, 127 * (256 + 256 * 256));
	m_fixpt.draw();
}