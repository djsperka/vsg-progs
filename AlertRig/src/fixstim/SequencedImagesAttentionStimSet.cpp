#include "SequencedImagesAttentionStimSet.h"

const DWORD truecolorGray = 127 * (1 + 256 + 256 * 256);



SequencedImagesAttentionStimSet::SequencedImagesAttentionStimSet(ARContrastFixationPointSpec& fixpt, const ImageFilesPositions& ifp, vector<AttentionCue>& vecCuePairs, bool bUseCueCircles, bool bUseCuePoints, bool bCuePointIsDot, vector<AttentionSequenceTrialSpec>& trialSpecs)
	: m_fixpt(fixpt)
	, m_ifp(ifp)
	, m_vecCuePairs(vecCuePairs)
	, m_trialSpecs(trialSpecs)
	, m_bUseCueCircles(bUseCueCircles)
	, m_bUseCuePoints(bUseCuePoints)
	, m_bCuePointIsDot(bCuePointIsDot)
	, m_current(0)
{
	int nStimPositions = std::get<1>(m_ifp).size();
	cout << "SequencedImagesAttentionStimSet: " << m_trialSpecs.size() << " trials." << endl;
	cout << "SequencedImagesAttentionStimSet: " << nStimPositions << " stim positions." << endl;
	cout << "SequencedImagesAttentionStimSet: " << vecCuePairs.size() << " cue specifications." << endl;
	cout << "SequencedImagesAttentionStimSet: " << get<0>(m_ifp).size() << " image files." << endl;

	for (unsigned int i = 0; i < vecCuePairs.size(); i++)
	{
		ARCueCircleSpec circle;
		int indImage = i % nStimPositions;

		// set up cue circle
		circle.x = std::get<1>(m_ifp)[indImage].first;
		circle.y = std::get<1>(m_ifp)[indImage].second;
		circle.d = 5;		// arbitrary! Need to get a reasonable diameter from image files????
		circle.linewidth = vecCuePairs[i].linewidth;
		circle.color = vecCuePairs[i].color;
		circle.dCentral = fixpt.d;
		if (m_bUseCueCircles)
			circle.bCircleEnabled = true;
		if (m_bUseCuePoints)
		{
			if (m_bUseCueCircles)
			{
				circle.dCentral = fixpt.d;
			}
			else
			{
				circle.dCentral = vecCuePairs[i].rdiff;
			}
			circle.bCentralIsDot = m_bCuePointIsDot;
		}
		m_vecCueCircles.push_back(circle);
	}


#if 0


	for (unsigned int i = 0; i < vecCuePairs.size(); i++)
	{
		ARContrastCueCircleSpec circle;
		int indGrating = i % m_vecGratings.size();

		// set up cue circle
		circle.x = m_vecGratings[indGrating].x;
		circle.y = m_vecGratings[indGrating].y;
		circle.d = m_vecGratings[indGrating].w + vecCuePairs[i].rdiff * 2;
		circle.linewidth = vecCuePairs[i].linewidth;
		circle.color = vecCuePairs[i].color;
		if (m_bUseCueCircles)
			circle.bCircleEnabled = true;
		if (m_bUseCuePoints)
		{
			if (m_bUseCueCircles)
			{
				circle.dCentral = fixpt.d;
			}
			else
			{
				circle.dCentral = vecCuePairs[i].rdiff;
			}
			circle.bCentralIsDot = m_bCuePointIsDot;
		}
		m_vecCueCircles.push_back(circle);
	}

#endif





	// create helpers
	m_pFixptHelper = new ImFixptSequenceHelper(FixptIndex, 100, m_fixpt);
	m_pCueHelper = new ImCueSequenceHelper(CueIndex, 0, nStimPositions, m_vecCueCircles);
	for (unsigned int i = 0; i < nStimPositions; i++)
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

	// Set color of fixpt...
	cout << "drawCurrent(): current trial index " << m_current << " total configured trials " << m_trialSpecs.size() << endl;
	m_fixpt.color = m_trialSpecs[m_current].color;

	// blank page =================
	m_pageBlank = m_pages[0];
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, truecolorGray);
	cerr << "drawCurrent(): Configure page " << m_pageBlank << " background only" << endl;

	// fixpt
	m_pageFixpt = m_pages[1];
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixpt, truecolorGray);
	cerr << "drawCurrent(): Configure page " << m_pageFixpt << " fixpt only" << endl;
	m_fixpt.draw();



	int f = 0;
	int nPagesConfigured = 2;

	// collect index/contrast pairs for a single page. When a new frame number is encountered, draw it, then clear
	// Each trial starts out with fixpt present, as we expect that the animation starts at a point where fixation is 
	// being maintained. 

	
	for (auto frame_icpair : m_trialSpecs[m_current].icpm)
	{
		cerr << "drawCurrent(): Got frame/icpair " << frame_icpair.first << "/" << frame_icpair.second.first << "," << frame_icpair.second.second << endl;
		if (frame_icpair.first == f)
		{
			//cerr << "same frame number, update helpers..." << endl;
			// Warn if index is EndIndex...
			if (frame_icpair.second.first == EndIndex)
			{
				cerr << "drawCurrent(): Warning! The end index \"*\" should not be accompanied by any other contrast changes (they are ignored)" << endl;
			}
		}
		else
		{
			int pageNumber = -1;

			// this is a new frame number, so the last page is finished. 
			// Look in pageVecs to see if we've already created an identical page. 
			// If we have we get back a VSG page number. 
			// If we haven't then we must draw a new page and push the page number and page vec onto vector.

			// In SequencedAttentionStimSet where we use gratings, things are more complicated
			// because of the need to reuse grating objects.
			// In this case, we are not doing anything like that. We don't use the "PageVector" stuff, 
			// and instead just draw each page without checking for a duplicate.

			if (nPagesConfigured == m_pages.size())
			{
				cerr << "drawCurrent(): Cannot draw another page - too many transitions!" << endl;
				throw std::runtime_error("Cannot draw another page - too many transitions!");
			}
			pageNumber = m_pages[nPagesConfigured++];
			drawPage(pageNumber, m_trialSpecs[m_current].offbits);

			// Now update cycling array using the page we just found or created
			cycle[ncycle].Frames = frame_icpair.first - f;
			cycle[ncycle].Page = pageNumber + vsgTRIGGERPAGE;
			cycle[ncycle].Stop = 0;
			cycle[ncycle].ovPage = cycle[ncycle].ovXpos = cycle[ncycle].ovYpos = cycle[ncycle].Xpos = cycle[ncycle].Ypos = 0;
			ncycle++;

		}

		// Now if the index of the pair is the end index, finish out the cycle and break out of here.
		// If it isn't, update the appropriate helper and continue. 
		if (frame_icpair.second.first != EndIndex)
		{
			updateHelper(frame_icpair.second);
		}
		else
		{
			cycle[ncycle].Frames = 1;
			cycle[ncycle].Page = m_pageBlank + vsgTRIGGERPAGE;
			cycle[ncycle].Stop = 1;
			cycle[ncycle].ovPage = cycle[ncycle].ovXpos = cycle[ncycle].ovYpos = cycle[ncycle].Xpos = cycle[ncycle].Ypos = 0;
			ncycle++;
			break;
		}

		f = frame_icpair.first;
	}

	// Set up page cycling
	vsgPageCyclingSetup(ncycle, &cycle[0]);

	cerr << "drawCurrent(): Cycling will use " << ncycle << " pages" << endl;
	for (int i = 0; i < ncycle; i++)
	{
		cerr << i << ": page=" << (cycle[i].Page & vsgTRIGGERPAGE ? cycle[i].Page - vsgTRIGGERPAGE : cycle[i].Page) << " Frames=" << cycle[i].Frames << endl;
	}
	cerr << "drawCurrent(): Done drawing pages for this trial." << endl << endl << endl;
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, vsgNOCLEAR);
	vsgPresent();




	cerr << endl << endl << "drawCurrent() - done" << endl;




}

void SequencedImagesAttentionStimSet::updateHelper(const ICPair& p)
{
	//cout << "updateHelper(): index " << p.first << " contrast " << p.second << endl;
	switch (p.first)
	{
	case FixptIndex:
		m_pFixptHelper->setContrast(p.second);
		break;
	case CueIndex:
		m_pCueHelper->setContrast(p.second);
		break;
	default:
		if (p.first >= 0 && p.first < std::get<1>(m_ifp).size())
			m_imageHelpers[p.first]->setContrast(p.second);
		else
			throw std::runtime_error("Cannot find index in page vec");
		break;
	}
}

void SequencedImagesAttentionStimSet::drawPage(int pageNumber, int offbits)
{
	cout << "drawPage(): page number " << pageNumber << " offbits " << std::hex << offbits << std::dec << endl;
	vsgSetDrawPage(vsgVIDEOPAGE, pageNumber, truecolorGray);

	// rely on the helpers to not draw when they're not supposed to be drawn
	if (m_vecCueCircles.size()>0) m_pCueHelper->draw_cues(offbits, false);
	for (auto& imageHelper : m_imageHelpers)
	{
		imageHelper->draw();
	}
	m_pFixptHelper->draw(0);	// arg is ignored for fixpt.

	//cout << "drawPage(): done" << endl;
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
	//cout << "CueSequenceHelper::draw_cues() - contrast " << contrast() << " iCueBase " << ((iOffBits & 0xff00) >> 8) << endl;
	if (contrast() == 0) return;

	// Draw cue circles.
	// One for each grating, but the set of cues used are taken from 
	// (iOffBits & 0xff00) >> 8
	int iCueBase = (iOffBits & 0xff00) >> 8;

	for (unsigned int i = 0; i < m_ngratings; i++)
	{
		//cout << "cue " << i << " (iOffBits & (1 << i)) " << (iOffBits & (1 << i)) << endl;
		// Check if this stim has an off bit set.
		if (iOffBits & (1 << i))
		{
			//cout << "Nothing to do." << endl;
			// do nothing
		}
		else
		{
			if (m_circles.size() > iCueBase*m_ngratings + i)
			{
				//cout << "Drawing cue index " << (iCueBase * m_ngratings + i) << endl;
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
	vsgDrawImage(vsgBMPPICTURE + vsgPALETTELOAD, m_x, -m_y, f);

}