#include "SequencedAttentionStimSet.h"
#include "AttentionStimSet.h"
#include "AlertUtil.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <exception>
#include <algorithm>
using namespace std;


SequencedAttentionStimSet::SequencedAttentionStimSet(ARContrastFixationPointSpec& fixpt, vector<alert::ARGratingSpec>& vecGratings, vector<AttentionCue>& vecCuePairs, bool bCueCircles, bool bCuePoints, bool bCueIsDot, vector<AttentionSequenceTrialSpec>& trialSpecs)
: m_trialSpecs(trialSpecs)
, m_fixpt(fixpt)
, m_bUseCueCircles(bCueCircles)
, m_bUseCuePoints(bCuePoints)
, m_bCuePointIsDot(bCueIsDot)
, m_vecGratings(vecGratings)
, m_current(0)
{
	for (unsigned int i = 0; i<vecCuePairs.size(); i++)
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
};


PageVec SequencedAttentionStimSet::makePageVec()
{
	PageVec pv;
	pv.push_back(m_pFixptHelper->makeICPair());
	pv.push_back(m_pCueHelper->makeICPair());
	for_each(m_gratingHelpers.begin(), m_gratingHelpers.end(), [&pv](GratingSequenceHelper *pgsh) { pv.push_back(pgsh->makeICPair());  });
	std::sort(pv.begin(), pv.end());

	//cerr << "makePageVec" << endl;
	//std::for_each(pv.begin(), pv.end(), [=](const ICPair& icp) {std::cout << icp.first << "/" << icp.second << std::endl; });
	return pv;
}

int SequencedAttentionStimSet::init(ARvsg& vsg, std::vector<int> pages, int)
{
	int status = 0;
	size_t nlevels;

	m_pages = pages;

	m_fixpt.init(vsg, 2);
	m_fixpt.setContrast(100);

	// Change level mult factor from 2 to 4 for cues to accomodate cue points. If no cue points this
	// overestimates the number of levels. 
	//nlevels = (245 - 4*m_vecCues.size())/(m_vecGratings.size()*2 + m_vecDistractors.size());

	// If using cues, take 2 levels per stim for the cues. That's because the cue point AND cue circle use the
	// same VSG object. If there are multiple sets of cues/points, they share the same set of VSG 
	// objects. Thus, if there are 3 stim, and 18 sets of cues (54 in all), then we will only need 6 levels for 
	// the cue objects.

	nlevels = (245 - ((m_bUseCueCircles || m_bUseCuePoints) ? m_vecGratings.size() * 2 : 0)) / (m_vecGratings.size()*2);
	cerr << "Number of levels per grating " << nlevels << endl;


	// Below the cues and cue points share levels to save on the overall number of objects. 
	// The cues come in groups where each one corresponds to a stimulus grating. If there are 3 stim gratings, 
	// then the first cue corresponds to the first grating (uses its center point, and its radius is used for 
	// determining the radius offset of the cue circle), the second cue corresponds to the second grating, 
	// and the third cue corresponds to the third grating. After that, additional groups of cues correspond to 
	// the first, second, and third grating in the same way. 
	// The 'offBits' parameter for a given trial has the particular cue group encoded: The upper 8 bits are an 
	// index into the list of cues. Thus, if 'offBits' is 0x400, the cue group [4] (the fifth one, since we use
	// ordinary C indexing like we should) is used for that trial. 
	// 
	// The first group of cues is initialized (gets assigned a VSG object number, color levels) in the normal way. 
	// Subsequent groups of cues (and all the cue points) are initialized to use the same object number and levels. 

	if (m_vecCueCircles.size() > 0)
	{
		cerr << "Initialize " << m_vecCueCircles.size() << " cues." << endl;
		for (size_t i = 0; i < m_vecGratings.size(); i++)
		{
			m_vecCueCircles[i].init(vsg, 2);
		}
		for (size_t i = m_vecGratings.size(); i < m_vecCueCircles.size(); i++)
		{
			cerr << "init cue " << i << " using cue " << (i % m_vecGratings.size()) << endl;
			m_vecCueCircles[i].init(m_vecCueCircles[i % m_vecGratings.size()]);
		}
	}


	// create helpers. 
	m_pFixptHelper = new FixptSequenceHelper(FixptIndex, 100, m_fixpt);
	m_pCueHelper = new CueSequenceHelper(CueIndex, 0, (int)m_vecGratings.size(), m_vecCueCircles);
	for (unsigned int i = 0; i<m_vecGratings.size(); i++)
	{
		m_gratingHelpers.push_back(new GratingSequenceHelper(i, m_vecGratings[i].contrast, m_vecGratings[i]));
	}

	cerr << "Initialize grating pool" << endl;
	cerr << "Number of vsg objects " << vsgGetSystemAttribute(vsgNUMOBJECTS) << endl;
	cerr << "Number of vsg pages   " << vsgGetSystemAttribute(vsgNUMVIDEOPAGES) << endl;
	GratingPool::instance().populate(2*m_vecGratings.size(), nlevels);

	status = drawCurrent();

	return status;
}


std::string SequencedAttentionStimSet::toString() const
{
	return string("not implemented");
}

int SequencedAttentionStimSet::drawCurrent()
{
	PageVecs pagesConfigured;
	PageVec pv;
	VSGCYCLEPAGEENTRY cycle[16];	// should  be plenty
	int ncycle = 0;

	cerr << endl << endl << "Begin drawCurrent() - draw vsg pages and set up animation for trial" << endl;
	cerr << "Flush unused gratings..." << endl;
	for_each(m_gratingHelpers.begin(), m_gratingHelpers.end(), [](GratingSequenceHelper* phelper) { phelper->reset(); });

	cerr << "Pool status" << endl;
	GratingPool::instance().printPoolStatus();



	// Set color of fixpt...
	m_fixpt.color = m_trialSpecs[m_current].color;

	// blank page =================
	cerr << "Configure page " << m_pageBlank << " background only" << endl;

	// Get page number
	m_pageBlank = m_pages[pagesConfigured.size()];

	// set contrasts
	m_pFixptHelper->setContrast(0);
	m_pCueHelper->setContrast(0);
	for_each(m_gratingHelpers.begin(), m_gratingHelpers.end(), [](GratingSequenceHelper* phelper) { phelper->setContrast(0); });

	// get page vec, add it to configured pages
	pv = makePageVec();
	pagesConfigured.push_back(make_pair(m_pageBlank, pv));

	// draw the actual page
	drawPageUsingPageVec(pv, m_pageBlank, 0, 0);

	// fixpt only page ===========================================
	cerr << "Configure page " << m_pageFixpt << " fixpt only" << endl;

	// page number
	m_pageFixpt = m_pages[pagesConfigured.size()];

	// set contrasts, make page vec, etc. 
	m_pFixptHelper->setContrast(100);
	pv = makePageVec();
	pagesConfigured.push_back(make_pair(m_pageFixpt, pv));

	// draw
	drawPageUsingPageVec(pv, m_pageFixpt, 0, 0);

	// Now loop over each transition time (be careful - can have multiple things at same time.
	// The value of 'f' is always the last frame for which we've generated a transition. 
	// On each iteration of the loop, we check the frame number for the contrast change. If its the same 
	// as 'f', it means we're still operating on  that page, and we just push the icpair onto the PageVec. 
	// If its a new frame number, then we have to check if the current page has already been created. 
	// 

	//cerr << "Begin scanning transitions for this trial...." << endl;

	// The loop below is over the elements of m_trialSpec[m_current].icpm
	// .icpm is an ICPairMap, or std::multimap<unsigned int, ICPair>
	// ICPair is std::pair< unsigned int, int >, the two items are index (0,1,...) of grating and contrast. 
	// The key to the map is the frame number. 
	// 
	// We expect that as we iterate through the map, we'll get a blocks of 
	// frame_icpair - which denote transitions that should occur at this 
	// frame number. As new transitions are encountered, we call updateHelper(frame_icpair.second)
	// The 'second' is the index/contrast pair. 
	// 
	// When the index is a grating, updateHelper calls setContrast on the helper itself. For a 
	// GratingHelper, 

	size_t f = 0;
	for (auto frame_icpair : m_trialSpecs[m_current].icpm)
	{
		//cerr << "Got frame/icpair " << frame_icpair.first << "/" << frame_icpair.second.first << "," << frame_icpair.second.second << endl;
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
			//cerr << "new frame number, finish current page..." << endl;

			pv = makePageVec();

			if (!findConfiguredPage(pv, pagesConfigured, pageNumber))
			{
				//cerr << "This page not found, create new page..." << endl;

				// draw page from page vec
				if (pagesConfigured.size() == m_pages.size())
				{
					cerr << "Cannot draw another page - too many transitions!" << endl;
					throw std::runtime_error("Cannot draw another page - too many transitions!");
				}
				pageNumber = m_pages[pagesConfigured.size()];
				cerr << "Configure page " << pageNumber << endl;
				drawPageUsingPageVec(pv, pageNumber, m_trialSpecs[m_current].offbits, m_trialSpecs[m_current].initialPhase);
				pagesConfigured.push_back(make_pair(pageNumber, pv));
			}
			else
			{
				cerr << "Using previously configured page " << pageNumber << endl;
			}

			// Now update cycling array using the page we just found or created
			cycle[ncycle].Frames = (WORD)(frame_icpair.first - f);
			cycle[ncycle].Page = pageNumber + vsgTRIGGERPAGE;
			cycle[ncycle].Stop = 0;
			cycle[ncycle].ovPage = cycle[ncycle].ovXpos = cycle[ncycle].ovYpos = cycle[ncycle].Xpos = cycle[ncycle].Ypos = 0;
			ncycle++;

			// update helper
			if (frame_icpair.second.first != EndIndex)
			{
				updateHelper(frame_icpair.second);
			}

		}

		// Now if the index of the pair is the end index, finish out the cycle and get out of here.
		if (frame_icpair.second.first == EndIndex)
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

	cerr << "Cycling: Using " << ncycle << " pages" << endl;
	for (int i = 0; i<ncycle; i++)
	{
		cerr << i << ": page=" << (cycle[i].Page & vsgTRIGGERPAGE ? cycle[i].Page - vsgTRIGGERPAGE : cycle[i].Page) << " Frames=" << cycle[i].Frames << endl;
	}
	cerr << "Done drawing pages for this trial." << endl << endl << endl;
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, vsgNOCLEAR);
	vsgPresent();

	return 0;
}


int SequencedAttentionStimSet::drawPageUsingPageVec(const PageVec& pv, int page, int offbits, double initial_phase)
{
	vector<double> initialPhase;
	for (int i = 0; i < m_gratingHelpers.size(); i++)
		initialPhase.push_back(initial_phase);
	return drawPageUsingPageVec(pv, page, offbits, initialPhase);
}

int SequencedAttentionStimSet::drawPageUsingPageVec(const PageVec& pv, int page, int offbits, const vector<double>& initial_phase)
{
	bool bFixpt = false;
	bool bCues = false;
	bool bCuePoints = false;
	PageVec::const_iterator itFixpt = find_if(pv.begin(), pv.end(), [](const ICPair& icp) { return icp.first == FixptIndex; });
	PageVec::const_iterator itCues = find_if(pv.begin(), pv.end(), [](const ICPair& icp) { return icp.first == CueIndex; });

	vsgSetDrawPage(vsgVIDEOPAGE, page, vsgBACKGROUND);
	bCues = m_bUseCueCircles && (itCues != pv.end() && itCues->second != 0);
	bCuePoints = m_bUseCuePoints && (itCues != pv.end() && itCues->second != 0);
	bFixpt = (itFixpt != pv.end());

	if (bCuePoints) m_pCueHelper->draw_cues(offbits, false);
	for (auto& icp : pv)
	{
		// Ignore offbits for gratings! 
		if (icp.first >= 0 && icp.first < 8)
		{
			m_gratingHelpers[icp.first]->draw(initial_phase.at(icp.first));
		}
	}
	if (bCues) m_pCueHelper->draw_cues(offbits, true);
	if (bFixpt) m_pFixptHelper->draw(0);	// arg is ignored for fixpt.

	//GratingPool::instance().printPoolStatus();

	return 0;
}





bool SequencedAttentionStimSet::findConfiguredPage(const PageVec& pv, const PageVecs& pagesConfigured, int& pageNumber)
{
	bool b = false;
	for (auto n_pv_pair : pagesConfigured)
	{
		if (pv == n_pv_pair.second)
		{
			pageNumber = n_pv_pair.first;
			b = true;
			break;
		}
	}
	return b;
}

void SequencedAttentionStimSet::draw_fixpt()
{
	m_fixpt.draw();
}


int SequencedAttentionStimSet::handle_trigger(const std::string& s, const std::string&)
{
	int status = 0;

	if (s == "F")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixpt, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "S")
	{
		cerr << "WARNING - check initial phase" << endl;
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




int parse_sequenced_params(const std::string& filename, size_t ngratings, std::vector<AttentionSequenceTrialSpec>& trialSpecs, ImageFilesPositions& ifp)
{
	int status = 0;
	size_t nimages = 0;
	std::ifstream myfile(filename.c_str());
	if (myfile.is_open())
	{

		// open file, read line-by-line and parse
		string line;
		int linenumber = 0;
		int iTrialStep = 0;
		vector<string> tokens;
		AttentionSequenceTrialSpec spec;


		/*
		   iTrialStep
		   0 - 
		*/
		while (!status && getline(myfile, line))
		{
			linenumber++;
			boost::trim(line);
			if (line.length() > 0 && line[0] != '#')
			{
				switch(iTrialStep) {
				case 0:

					// The only thing we look for here is the word "trial" or "image". 
					boost::algorithm::to_lower(line);
					if (line.rfind("trial") == 0)
					{
						iTrialStep = 1;

						// initialize trial spec
						spec.color = COLOR_ENUM::unknown_color;
						spec.initialPhase.clear();
						spec.offbits = 0;
						spec.icpm.clear();
					}
					else if (line.rfind("images") == 0)
					{
						if (ngratings > 0)
						{
							cerr << "Cannot have images in spec file and gratings on command line." << endl;
							status = 1;
							break;
						}
						// parse remainder of line
						// there should be an ODD number of tokens: the word "images" followed by x,y pairs. 
						tokens.clear();
						tokenize(line, tokens, ",");
						if (((tokens.size() - 1) % 2) == 1)
						{
							cerr << "Expecting x,y pairs following \"images\" at line " << linenumber << endl;
							status = 1;
							break;
						}
						else
						{
							for (int i = 1; i < tokens.size(); i += 2)
							{
								double ximg, yimg;
								if (parse_double(tokens[i], ximg) || parse_double(tokens[i + 1], yimg))
								{
									cerr << "Error parsing x,y pair on line " << linenumber << ": " << tokens[i] << ", " << tokens[i + 1] << endl;
									status = 1;
									break;
								}
								else
								{
									std::get<1>(ifp).push_back(make_pair(ximg, yimg));
									cerr << "image x,y " << ximg << ", " << yimg << endl;
								}
							}
						}

						// change to image reading state
						iTrialStep = 10;
						break;
					}
					break;

				case 10:

					// expecting just a filename
					if (line[0] == '*')
					{
						// done with images
						iTrialStep = 0;
						nimages = std::get<0>(ifp).size();
					}
					else
					{
						// verify file exists
						boost::filesystem::path image(line);
						if (!exists(image))
						{
							cerr << "image file not found,line " << linenumber << " : " << image << endl;
							status = 1;
						}
						else
						{
							cerr << "image file found: " << line << endl;
							std::get<0>(ifp).push_back(line);
						}
					}
					break;

				case 1:
					tokens.clear();
					tokenize(line, tokens, ",");
					if (tokens.size() == 3 || tokens.size() == 2+ngratings)
					{
						if (parse_color(tokens[0], spec.color))
						{
							cerr << "Error reading color at line " << linenumber << ": " << tokens[0] << endl;
							status = 1;
							break;
						}
						if (parse_integer(tokens[1], spec.offbits))
						{
							cerr << "Error reading off bits at line " << linenumber << ": " << tokens[1] << endl;
							status = 1;
							break;
						}

						// When using gratings, expect either a single initial phase value, or one for each of the gratings. 
						// When using images, ignore the rest of the line.
						if (ngratings > 0)
						{
							if (tokens.size() == 3)
							{
								double phase;
								if (parse_double(tokens[2], phase))
								{
									cerr << "Error reading initial phase at line " << linenumber << ": " << tokens[2] << endl;
									status = 1;
									break;
								}
								else
								{
									for (int i = 0; i < ngratings; i++)
										spec.initialPhase.push_back(phase);
								}
							}
							else if (tokens.size() == (2 + ngratings))
							{
								double phase;
								for (int i = 2; i < 2 + ngratings; i++)
								{
									if (parse_double(tokens[i], phase))
									{
										cerr << "Error reading initial phase at line " << linenumber << ": " << tokens[i] << endl;
										status = 1;
										break;
									}
									else
									{
										spec.initialPhase.push_back(phase);
									}
								}
							}
							else
							{
								cerr << "Error reading initial phases at line " << linenumber << ": expecting a single phase, or one for each grating (" << ngratings << ")" << endl;
								status = 1;
								break;
							}
						}
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
						else
						{
							if (parse_uint(tokens[0], index))
							{
								cerr << "Error reading stim index (expect \"F\", \"Q\", or 0<int<(#gratings or #images)) line " << linenumber << ": " << tokens[0] << endl;
								status = 1;
								break;
							}
							else if (ngratings > 0)
							{
								if (index < 0 || index >= ngratings)
								{
									cerr << "Bad grating index (" << index << ") at line " << linenumber << " - expecting index in [0," << ngratings << ")";
									status = 1;
								}
							}
							else
							{
								if (index < 0 || index >= nimages)
								{
									cerr << "Bad image index (" << index << ") at line " << linenumber << " - expecting index in [0," << nimages << ")";
									status = 1;
								}
							}
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
							//cerr << "Finished parsing trial " << trialSpecs.size() + 1 << endl;
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
		cerr << "Found " << std::get<0>(ifp).size() << " image files." << endl;
		cerr << "Found " << get<0>(ifp).size() << " stim x,y pairs." << endl;
	}
	else
	{
		cerr << "Cannot open input file: " << filename << endl;
		status = 1;
	}
	return status;
}



GratingPool& GratingPool::instance()
{
	static GratingPool pool;
	return pool;
}

void GratingPool::populate(size_t n, size_t nlevels)
{
	ARGratingSpec *g;
	cerr << "Populate grating pool with " << n << " gratings, each using " << nlevels << " levels" << endl;
	not_in_use.clear();
	in_use.clear();
	for (int i = 0; i < n; i++)
	{
		g = new ARGratingSpec();
		g->init((int)nlevels);
		not_in_use.push_back(g);
	}
	cerr << "Grating pool status:" << endl;
	printPoolStatus();
}

ARGratingSpec *GratingPool::getGrating()
{
	ARGratingSpec *g = NULL;
	if (not_in_use.empty())
	{
		cerr << "Grating pool is empty!" << endl;
	}
	else
	{
		g = not_in_use.front();
		not_in_use.pop_front();
		in_use.push_back(g);
	}
	return g;
}

void GratingPool::returnGrating(ARGratingSpec *pG)
{
	// verify that this pointer is in use.
	// The test is done using the vsg object handle 
	auto it = std::find_if(in_use.begin(), in_use.end(), [&pG](ARGratingSpec *p) { return pG->handle() == p->handle(); });
	if (it != in_use.end())
	{
		not_in_use.push_back(*it);
		in_use.remove(*it);
	}
	else
	{
		cerr << "The grating passed was not in list of in-use gratings!" << endl;
	}
}


void GratingPool::printPoolStatus()
{
	cerr << "not_in_use size " << not_in_use.size() << endl;
	std::for_each(not_in_use.begin(), not_in_use.end(), [=](const ARGratingSpec* g) { std::cerr << "(" << g->handle() << "): " << *g << std::endl; });
	cerr << "in_use size " << in_use.size() << endl;
	std::for_each(in_use.begin(), in_use.end(), [=](const ARGratingSpec* g) { std::cerr << "(" << g->handle() << "): " << *g << std::endl; });
}




GratingSequenceHelper::GratingSequenceHelper(size_t index, int defaultContrast, const ARGratingSpec& grating)
	: SequenceHelper(index, defaultContrast)
	, m_defaultContrast(defaultContrast)
{
	m_gratingDefault = grating;
	m_gratingDefault.setContrast(defaultContrast);
	//m_gratingMap.insert(make_pair(defaultContrast, &m_gratingDefault));
}

GratingSequenceHelper::~GratingSequenceHelper() {};

void GratingSequenceHelper::reset()
{
	// return all gratings to pool except for original
	for (auto it = m_gratingMap.begin(); it != m_gratingMap.end(); )
	{
		cerr << "flushing grating handle " << it->second->handle() << " contrast " << it->first << endl;
		GratingPool::instance().returnGrating(it->second);
		it = m_gratingMap.erase(it);
	}
	setContrast(0);
}

void GratingSequenceHelper::setContrastPriv()
{
	// If contrast is zero, do nothing, we always have one of those. 
	// Otherwise, check if we have a grating at this contrast.
	if (contrast())
	{
		if (m_gratingMap.find(contrast()) == m_gratingMap.end())
		{
			ARGratingSpec* g = GratingPool::instance().getGrating();
			if (!g)
			{
				cerr << "ERROR! Grating pool is empty!" << endl;
				throw std::runtime_error("Grating Pool is empty!");
			}
			else
			{
				// can't use the = operator because it also copies the VSG handle, and that's NOT what we want. 
				// We need a grating with identical settings except for contrast -- and that will require a unique
				// range of levels, and hence a unique vsg object handle. 
				cerr << "new contrast " << contrast() << " configure obj " << g->handle() << " using " << m_gratingDefault.handle() << endl;
				g->x = m_gratingDefault.x;
				g->y = m_gratingDefault.y;
				g->w = m_gratingDefault.w;
				g->h = m_gratingDefault.h;
				g->wd = m_gratingDefault.wd;
				g->hd = m_gratingDefault.hd;
				g->contrast = contrast();		// set contrast
				g->sf = m_gratingDefault.sf;
				g->tf = m_gratingDefault.tf;
				g->orientation = m_gratingDefault.orientation;
				g->phase = m_gratingDefault.phase;
				g->cv = m_gratingDefault.cv;
				g->swt = m_gratingDefault.swt;
				g->twt = m_gratingDefault.twt;
				g->ttf = m_gratingDefault.ttf;
				g->aperture = m_gratingDefault.aperture;
				g->bDrawInitDone = false;

				m_gratingMap.insert(make_pair(contrast(), g));
			}
		}
	}
}

// draw a grating at current contrast
void GratingSequenceHelper::draw(double initial_phase)
{
	if (contrast() == 0) return;
	map<int, ARGratingSpec* >::iterator it = m_gratingMap.find(contrast());
	if (it == m_gratingMap.end())
		throw std::runtime_error("GratingSequenceHelper::draw cannot find grating with contrast required!");
	else
	{
		cerr << "Drawing grating contrast " << contrast() << " init phase " << initial_phase << " handle " << it->second->handle() << endl;
		it->second->phase = initial_phase;
		it->second->resetDriftPhase();
		it->second->draw();
	}
	return;
}

void FixptSequenceHelper::draw()
{
	if (contrast() == 0) return;
	m_fixpt.setContrast(contrast());	// just in case
	m_fixpt.draw();
}

void CueSequenceHelper::draw(double initial_phase)
{
	cerr << "WARNING - CueSequenceHelper::draw is NO-OP. Use draw_cues, draw_cue_points instead" << endl;
}

void CueSequenceHelper::draw_cues(int iOffBits, bool bCircles)
{
	if (contrast() == 0) return;

	// Draw cue circles.
	// One for each grating, but the set of cues used are taken from 
	// (iOffBits & 0xff00) >> 8
	int iCueBase = (iOffBits & 0xff00) >> 8;

	cout << "CueSequenceHelper::draw_cues - iCueBase " << iCueBase << endl;

	cout << "There are " << m_circles.size() << " cues." << endl;
	for (unsigned int i=0; i<m_ngratings; i++)
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


void SequencedAttentionStimSet::updateHelper(const ICPair& p)
{
	switch (p.first)
	{
	case FixptIndex:
		m_pFixptHelper->setContrast(p.second);
		break;
	case CueIndex:
		m_pCueHelper->setContrast(p.second);
		break;
	default:
		if (p.first >= 0 && p.first < m_vecGratings.size())
			m_gratingHelpers[p.first]->setContrast(p.second);
		else
			throw std::runtime_error("Cannot find index in page vec");
		break;
	}
}