#include "SequencedImagesAttentionStimSet.h"

const DWORD truecolorGray = 127 * (1 + 256 + 256 * 256);



SequencedImagesAttentionStimSet::SequencedImagesAttentionStimSet(ARContrastFixationPointSpec& fixpt, const ImageFilesPositions& ifp, vector<AttentionCue>& vecCuePairs, vector<AttentionSequenceTrialSpec>& trialSpecs)
	: m_fixpt(fixpt)
	, m_ifp(ifp)
	, m_vecCuePairs(vecCuePairs)
	, m_trialSpecs(trialSpecs)
	, m_current(0)
{
	cout << "SequencedImagesAttentionStimSet: have " << m_trialSpecs.size() << " trials." << endl;

	int nImages = std::get<1>(m_ifp).size();
	for (unsigned int i = 0; i < vecCuePairs.size(); i++)
	{
		ARCueCircleSpec circle;
		int indImage = i % nImages;

		// set up cue circle
		circle.x = std::get<1>(m_ifp)[indImage].first;
		circle.y = std::get<1>(m_ifp)[indImage].second;
		circle.d = 5;		// arbitrary! Need to get a reasonable diameter from image files????
		circle.linewidth = vecCuePairs[i].linewidth;
		circle.color = vecCuePairs[i].color;
		circle.bCircleEnabled = false;
		circle.dCentral = fixpt.d;
		circle.bCentralIsDot = true;
		m_vecCueCircles.push_back(circle);
	}

	// create helpers
	m_pFixptHelper = new ImFixptSequenceHelper(FixptIndex, 100, m_fixpt);
	m_pCueHelper = new ImCueSequenceHelper(CueIndex, 0, nImages, m_vecCueCircles);
	for (unsigned int i = 0; i < nImages; i++)
	{
		m_imageHelpers.push_back(new ImageSequenceHelper(i, std::get<1>(m_ifp)[i].first, std::get<1>(m_ifp)[i].second, std::get<0>(m_ifp)));
	}
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
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, truecolorGray);
	cerr << "Configure page " << m_pageBlank << " background only" << endl;

	// fixpt
	m_pageFixpt = m_pages[1];
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixpt, truecolorGray);
	m_fixpt.draw();



	int f = 0;
	int nPagesConfigured = 2;

	// collect index/contrast pairs for a single page. When a new frame number is encountered, draw it, then clear
	// Each trial starts out with fixpt present, as we expect that the animation starts at a point where fixation is 
	// being maintained. 

	
	for (auto frame_icpair : m_trialSpecs[m_current].icpm)
	{
		cerr << "Got frame/icpair " << frame_icpair.first << "/" << frame_icpair.second.first << "," << frame_icpair.second.second << endl;
		if (frame_icpair.first == f)
		{
			//cerr << "same frame number, update helpers..." << endl;
			// Warn if index is EndIndex...
			if (frame_icpair.second.first == EndIndex)
			{
				cerr << "Warning! The end index \"*\" should not be accompanied by any other contrast changes (they are ignored)" << endl;
			}
			else
			{
				updateHelper(frame_icpair.second);
			}
		}
		else
		{
			int pageNumber = -1;

			// this is a new frame number, so the last page is finished. 
			// Look in pageVecs to see if we've already created an identical page. 
			// If we have we get back a VSG page number. 
			// If we haven't then we must draw a new page and push the page number and page vec onto vector.
			cerr << "new frame number, finish current page..." << endl;


			// In SequencedAttentionStimSet where we use gratings, things are more complicated
			// because of the need to reuse grating objects.
			// In this case, we are not doing anything like that. We don't use the "PageVector" stuff, 
			// and instead just draw each page without checking for a duplicate.

			if (nPagesConfigured == m_pages.size())
			{
				cerr << "Cannot draw another page - too many transitions!" << endl;
				throw std::runtime_error("Cannot draw another page - too many transitions!");
			}
			pageNumber = m_pages[nPagesConfigured++];
			cerr << "Configure page " << pageNumber << endl;
			drawPage(pageNumber, m_trialSpecs[m_current].offbits);

			// Now update cycling array using the page we just found or created
			cycle[ncycle].Frames = frame_icpair.first - f;
			cycle[ncycle].Page = pageNumber + vsgTRIGGERPAGE;
			cycle[ncycle].Stop = 0;
			cycle[ncycle].ovPage = cycle[ncycle].ovXpos = cycle[ncycle].ovYpos = cycle[ncycle].Xpos = cycle[ncycle].Ypos = 0;
			ncycle++;

			// Now that page is drawn, update helper with the ICPair 
			updateHelper(frame_icpair.second);
		}

		//// Now if the index of the pair is the end index, finish out the cycle and get out of here.
		//if (frame_icpair.second.first == EndIndex)
		//{
		//	cycle[ncycle].Frames = 1;
		//	cycle[ncycle].Page = m_pageBlank + vsgTRIGGERPAGE;
		//	cycle[ncycle].Stop = 1;
		//	cycle[ncycle].ovPage = cycle[ncycle].ovXpos = cycle[ncycle].ovYpos = cycle[ncycle].Xpos = cycle[ncycle].Ypos = 0;
		//	ncycle++;
		//	break;
		//}

		f = frame_icpair.first;
	}

	//// Set up page cycling
	//vsgPageCyclingSetup(ncycle, &cycle[0]);

	//cerr << "Cycling: Using " << ncycle << " pages" << endl;
	//for (int i = 0; i < ncycle; i++)
	//{
	//	cerr << i << ": page=" << (cycle[i].Page & vsgTRIGGERPAGE ? cycle[i].Page - vsgTRIGGERPAGE : cycle[i].Page) << " Frames=" << cycle[i].Frames << endl;
	//}
	//cerr << "Done drawing pages for this trial." << endl << endl << endl;
	//vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, vsgNOCLEAR);
	//vsgPresent();




	cerr << endl << endl << "drawCurrent() - done" << endl;




}

void SequencedImagesAttentionStimSet::updateHelper(const ICPair& p)
{
	cout << "updateHelper() " << p.first << " contrast " << p.second << endl;
	switch (p.first)
	{
	case FixptIndex:
		m_pFixptHelper->setContrast(p.second);
		break;
	case CueIndex:
		m_pCueHelper->setContrast(p.second);
		break;
	default:
		cout << "files size " << std::get<1>(m_ifp).size() << endl;
		if (p.first >= 0 && p.first < std::get<1>(m_ifp).size())
			m_imageHelpers[p.first]->setContrast(p.second);
		else
			throw std::runtime_error("Cannot find index in page vec");
		break;
	}
}

void SequencedImagesAttentionStimSet::drawPage(int pageNumber, int offbits)
{
	cout << "drawPage() " << pageNumber << " offbits " << std::hex << offbits << endl;
	cout << "drawPage() done" << endl;
}


void ImFixptSequenceHelper::draw()
{
	if (contrast() == 0) return;
	m_fixpt.draw();
}

void ImCueSequenceHelper::draw(double initial_phase)
{
	cerr << "WARNING - CueSequenceHelper::draw is NO-OP. Use draw_cues, draw_cue_points instead" << endl;
}

void ImCueSequenceHelper::draw_cues(int iOffBits, bool bCircles)
{
	if (contrast() == 0) return;

	// Draw cue circles.
	// One for each grating, but the set of cues used are taken from 
	// (iOffBits & 0xff00) >> 8
	int iCueBase = (iOffBits & 0xff00) >> 8;

	cout << "CueSequenceHelper::draw_cues - iCueBase " << iCueBase << endl;

	cout << "There are " << m_circles.size() << " cues." << endl;
	for (unsigned int i = 0; i < m_ngratings; i++)
	{
		cout << "cue " << i << " (iOffBits & (1 << i)) " << (iOffBits & (1 << i)) << endl;
		// Check if this stim has an off bit set.
		if (iOffBits & (1 << i))
		{
			cout << "Nothing to do." << endl;
			// do nothing
		}
		else
		{
			if (m_circles.size() > iCueBase*m_ngratings + i)
			{
				cout << "Drawing cue." << endl;
				m_circles[iCueBase*m_ngratings + i].setContrast(this->contrast());
				if (bCircles)
				{
					m_circles[iCueBase*m_ngratings + i].drawCircle();
				}
				else
				{
					m_circles[iCueBase*m_ngratings + i].drawPoint();
				}
			}
		}
	}

	return;
}


void ImageSequenceHelper::draw(double ignore)
{
	if (contrast() < 0 || contrast() >= m_imageFiles.size()) return;
	// draw image
	char f[256];
	strcpy(f, m_imageFiles[contrast()].c_str());
	vsgDrawImage(vsgBMPPICTURE + vsgPALETTELOAD, m_x, m_y, f);

}