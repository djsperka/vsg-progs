#include "MelStimSet.h"
#include "AttentionStimSet.h"
#include "AlertUtil.h"
#include "fetcher.h"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <algorithm>
#include <chrono>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;


void dumpPalette(const std::string& s, VSGLUTBUFFER& buffer, int N, int startN)
{
	for (int i = startN; i<startN + N; i++)
		cerr << s << "[" << i << "] " << buffer[i].a << ", " << buffer[i].b << ", " << buffer[i].c << endl;

}

void dumpHWPalette(const std::string& s, int N, int startN)
{
	VSGLUTBUFFER palVSG;
	vsgPaletteRead(&palVSG);
	dumpPalette(s, palVSG, N, startN);
//	for (int i=startN; i<startN+N; i++)
//		cerr << "palVSG[" << i << "] " << palVSG[i].a << ", " << palVSG[i].b << ", " << palVSG[i].c << endl;
}


RectanglePool::~RectanglePool()
{
	for (auto p : m_vec)
	{
		delete p.second;
	}
}

void RectanglePool::reset()
{
	vector<ColorRectPair>& vec = RectanglePool::instance().vec();
	for (auto p : vec)
	{
		vsgObjDestroy(p.second->handle());
		delete p.second;
	}
	vec.clear();
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
		cerr << "New rect, color " << c << endl;
		rect->init(2);
		rect->color = c;
		vec.push_back(make_pair(c, rect));
	}
	return rect;
}


#ifdef HOST_PAGE_TEST
void MelStimSet::cleanup(std::vector<int> pages)
{
	vsgPAGEDelete(m_hostPageHandle);
	RectanglePool::reset();
}
#else
void MelStimSet::cleanup(std::vector<int> pages)
{
}
#endif

int MelStimSet::init(std::vector<int> pages)
{
	m_pagesAvailable = pages;

#ifdef HOST_PAGE_TEST
	// Create a page
	m_hostPageHandle = vsgPAGECreate(vsgHOSTPAGE, vsgGetScreenWidthPixels(), vsgGetScreenHeightPixels(), vsg8BITPALETTEMODE);
	cerr << "Got host page handle " << m_hostPageHandle << endl;
#endif

	// first page is always blank
	vsgSetDrawPage(vsgVIDEOPAGE, m_pagesAvailable[0], vsgBACKGROUND);
	m_pageBlank = m_pagesAvailable[0];

	//PIXEL_LEVEL m_levelWhite;
	//ARvsg::instance().request_single(m_levelWhite);
	//cerr << "got dummy level " << m_levelWhite << endl;


	// Initialize color table
	PIXEL_LEVEL first;
	ARvsg::instance().request_range(130, first);

	//cerr << "First level is " << first << endl;
	//arutil_color_to_palette(COLOR_TYPE(gray), first);
	//arutil_color_to_palette(COLOR_TYPE(red), first+1);
	//arutil_color_to_palette(COLOR_TYPE(white), first+2);
	//arutil_color_to_palette(COLOR_TYPE(green), first+3);
	//arutil_color_to_palette(COLOR_TYPE(blue), first+4);

	cerr << "There are " << m_vecFixpts.size() << " fixpts to init." << endl;
	std::for_each(m_vecFixpts.begin(), m_vecFixpts.end(), [](alert::ARContrastFixationPointSpec& f) { f.init(2); f.setContrast(100); });
	//m_fixpt.init(2);
	//m_fixpt.setContrast(100);

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

#ifndef HOST_PAGE_TEST 
	vsgSetDrawPage(vsgVIDEOPAGE, page, vsgBACKGROUND);	
#else
	// Draw on the host page
	vsgSetDrawPage(vsgHOSTPAGE, m_hostPageHandle, vsgBACKGROUND);

	// load image to this page
	//vsgDrawImage(vsgPALETTELOAD, 0, 0, "d:\\work\\usrey\\src\\fixstim\\rect-test.bmp");
#endif

	//cerr << "drawCurrent - start" << endl;
	//cerr << "drawCurrent - fixpt page " << m_pageFixpt << endl;

	// The input file was parsed and separated into trials, and is saved
	// as a vector<MelTrialSpec>. The MelTrialSpec consists of the grid spec
	// that applies to the trial, and a vector of these:
	//
	// typedef std::pair<unsigned int, MelFrame > FrameRectVecPair;
	//
	// and a MelFrame is a couple of vectors:
	//	vector<MelBmpSpec> vecBmps;
	//	vector<ARContrastRectangleSpec> vecRects;
	//
	// The first element of the pair is the frame number to which the vector of rects 'belongs' - 
	// i.e. the frame number at which the list of rects should first appear. 
	//
	// The algorithm below is as follows:
	// A blank page is prepared.
	// for each FrameRectVecPair, test whether the frame number is different than the frame for which 
	// the most recent page was written. (* I've changed the parse method so this should happen every 
	// time - in other words, each FrameRectVecPair has a unique frame number, but the algorithm allows
	// for multiple pairs to have the same frame number. 
	// If the frame number is different, then the page currently being drawn is nearly complete, all that 
	// remains is for its fixpt(s) to be drawn. They are drawn and the page is complete. The cycling array
	// is updated with this page and its duration (which is known from the value of the frame for the 'current' 
	// pair in the loop), and a new draw page is initialized and made the current draw page.
	// Finally, the rects for the current element in the loop are drawn on the current draw page. 
	// Note that the drawing step follows, and is separate from, the frame number test (which may or may
	// not have forced the completion of the previous page). Thus if there are two or more FrameRectVecPairs
	// that share the same frame number, their rectVec contents will all be drawn on the same page. 
	// The fixpts are drawn after all rects for the page have been drawn, so they are always on top. 

	VSGCYCLEPAGEENTRY cycle[16];	// should  be plenty
	int ncycle = 0;
	for(auto melpair: m_trialSpecs[m_uiCurrentTrial].vecFrames)
	{

		// if the frame number differs from the last frame (it always should due to changes in parsing)
		// then finish drawing this page by drawing fixpt(s) and updating the cycling array.

		if (melpair.first > frameLast)
		{
			cerr << "Finish page " << page << endl;
			dumpHWPalette("VSGpAL", 8, 0);
			dumpHWPalette("VSGpal", 8, 123);

			// draw fixpts
			std::for_each(m_vecFixpts.begin(), m_vecFixpts.end(), [](alert::ARContrastFixationPointSpec& f) { cout << "Draw fixpt " << f << " levels " << f.getFirstLevel() << "/" << f.getNumLevels() << endl;  f.draw(); });

			// set up cycling element here. 
			cycle[ncycle].Xpos = cycle[ncycle].Ypos = 0;
			cycle[ncycle].Page = page + vsgTRIGGERPAGE;
			cycle[ncycle].Frames = melpair.first - frameLast;
			cycle[ncycle].Stop = 0;
			ncycle++;

			frameLast = melpair.first;

			// Get a new page number for the next copy
			page = m_pagesAvailable[nPages++];
			cerr << "New page to draw " << page << endl;

			// clear a new page
			vsgSetDrawPage(vsgVIDEOPAGE, page, vsgBACKGROUND);
		}

		// If requested, load images to this page
		cerr << "Draw " << melpair.second.vecBmps.size() << " bmps on page " << page << endl;
		for (auto bmp : melpair.second.vecBmps)
		{
			char f[256];
			strcpy(f, bmp.filename.c_str());

			cerr << "bmp file " << bmp.filename << endl;

			if (bmp.copyPalette)
			{
				vsgSetDrawMode(vsgCENTREXY);
				vsgDrawImage(0, bmp.x, bmp.y, f);
				VSGLUTBUFFER palette;
				vsgImageGetPalette(0, f, &palette);
				vsgPaletteWrite((VSGLUTBUFFER*)palette, bmp.startCopyLevel, bmp.numCopyLevel);
				cerr << "copy palette " << bmp.startCopyLevel << "/" << bmp.numCopyLevel << endl;
				dumpPalette("imagePAL", palette, 8, 0);
			}
			else if (bmp.drawMode == vsgTRANSONSOURCE)
			{
				DWORD saveMode = vsgGetDrawMode();
				vsgSetDrawMode(vsgCENTREXY | vsgTRANSONSOURCE);
				vsgSetPen2(bmp.level);
				vsgDrawImage(0, bmp.x, bmp.y, f);
				vsgSetDrawMode(saveMode);
				cerr << "transOnSource " << bmp.level << endl;
			}
			else if (bmp.drawMode == vsgTRANSONDEST)
			{
				DWORD saveMode = vsgGetDrawMode();
				vsgSetDrawMode(vsgCENTREXY | vsgTRANSONDEST);
				vsgSetPen2(bmp.level);
				vsgDrawImage(0, bmp.x, bmp.y, f);
				vsgSetDrawMode(saveMode);
				cerr << "transOnDest " << bmp.level << endl;
			}
			else
			{
				vsgSetDrawMode(vsgCENTREXY);
				vsgDrawImage(0, bmp.x, bmp.y, f);
				cerr << "No draw mode" << endl;
			}
		}

		// draw rects
		// different colored rects have to be unique objects
		cerr << "Draw " << melpair.second.vecRects.size() << " rects on page " << page << endl;
		for (auto rect : melpair.second.vecRects)
		{
			ARContrastRectangleSpec *drawrect = RectanglePool::getRect(rect.color);

			// Assign coordinates (after transforming them)
			applyTransform(*drawrect, rect, m_trialSpecs[m_uiCurrentTrial].grid);
			//cout << "drawrect " << *drawrect << endl;
			drawrect->draw();
		}
	}

	// draw fixpt
	//m_fixpt.draw();
	std::for_each(m_vecFixpts.begin(), m_vecFixpts.end(), [](alert::ARContrastFixationPointSpec& f) { f.draw(); });


#ifdef HOST_PAGE_TEST
	// Blit(copy) the page to the VSG video area
	vsgSetDrawPage(vsgVIDEOPAGE, page, vsgNOCLEAR);
	vsgDrawMoveRect(vsgHOSTPAGE, m_hostPageHandle, 0, 0, vsgGetScreenWidthPixels(), vsgGetScreenHeightPixels(), 0, 0, vsgGetScreenWidthPixels(), vsgGetScreenHeightPixels());
#endif

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

	//// Record end time
	//auto finish = std::chrono::high_resolution_clock::now();

	//// get duration
	//std::chrono::duration<double> elapsed = finish - start;

	//cerr << "drawCurrent: done. " << ncycle << " pages, " << m_trialSpecs[m_uiCurrentTrial].vecFrames.size() << " frame/rectvec pairs, Elapsed time: " << elapsed.count() << endl;

	return status;
}

// assume that the rectangle coords are grid coords. 
// The unit square CENTERED at (0,0) is drawn from (0,0) to (1,1). 
// With scaling, unit square is scaled by w,h. In pixel coords it is drawn shifted by 0.5,0.5
// So we need to rotate the center point of the rect, and add the grid center to that. 
// The vsg will rotate around the center point. 


void MelStimSet::applyTransform(ARContrastRectangleSpec& result, const ARContrastRectangleSpec& original, const MelGridSpec& grid)
{
	double ctheta = cos(grid.oriDegrees * M_PI / 180.0);
	double stheta = sin(grid.oriDegrees * M_PI / 180.0);
	double x0 = grid.wGrid*(original.x + 0.5);
	double y0 = grid.hGrid*(original.y + 0.5);
	result.x = x0 * ctheta - y0 * stheta;
	result.y = x0 * stheta + y0 * ctheta;
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
	return oss.str();
}


//void addOrAppendfrvPair(vector<FrameRectVecPair>& vecPairs, const FrameRectVecPair& frvPair)
//{
//	// find the frame number if we can...
//	auto it = std::find_if(vecPairs.begin(), vecPairs.end(), [&frvPair](FrameRectVecPair& p) { return p.first == frvPair.first; });
//	if (it != vecPairs.end())
//	{
//		it->second.insert(std::end(it->second), std::begin(frvPair.second), std::end(frvPair.second));
//	}
//	else
//	{
//		vecPairs.push_back(frvPair);
//	}
//	return;
//}


using boost::filesystem::path;

class MyPathMatcher : public PathMatcher<string>
{
	string m_extension;
public:
	MyPathMatcher(const string& ext) : m_extension(ext) { to_lower(m_extension); };
	bool operator()(const path& p, string& key)
	{
		bool b = false;
		//cerr << p.string() << endl;
		if (to_lower_copy(p.extension().string()) == m_extension)
		{
			b = true;
			key = p.stem().string();
		}
		return b;
	}
};


int parse_bmp_spec(const string& bmparg, const std::map<std::string, boost::filesystem::path>& fileMap, MelBmpSpec& bmpSpec)
{
	int status = 0;
	vector<string> tokens;
	tokenize(bmparg, tokens, ", ");
	if (tokens.size() == 3 || tokens.size() == 5)
	{
		string key = tokens[0];
		// look up key in fetched list of images
		if (fileMap.count(key) > 0)
		{
			bmpSpec.filename = fileMap.find(key)->second.string();

			if (parse_double(tokens[1], bmpSpec.x) || parse_double(tokens[2], bmpSpec.y))
			{
				cerr << "Error parsing x,y for bitmap image: " << bmparg << endl;
				status = 1;
			}
			else
			{
				bmpSpec.copyPalette = false;
				bmpSpec.drawMode = 0;
				if (tokens.size() == 5)
				{
					// hack - accept two extra args:
					// p,level = copy palette from 0 - (level-1)
					// s,level = TRANSONSOURCE
					// d,level = TRANSONDEST
					DWORD l;
					if (parse_ulong(tokens[4], l))
					{
						cerr << "Error parsing bmp level : " << bmparg << endl;
						status = 1;
					}
					else
					{
						if (boost::iequals(tokens[3], "s"))
						{
							bmpSpec.copyPalette = false;
							bmpSpec.drawMode = vsgTRANSONSOURCE;
							bmpSpec.level = l;
						}
						else if (boost::iequals(tokens[3], "d"))
						{
							bmpSpec.copyPalette = false;
							bmpSpec.drawMode = vsgTRANSONDEST;
							bmpSpec.level = l;
						}
						else if (boost::iequals(tokens[3], "p"))
						{
							bmpSpec.copyPalette = true;
							bmpSpec.startCopyLevel = 0;
							bmpSpec.numCopyLevel = l;
						}
						else
						{
							cerr << "Error parsing bmp mode: " << bmparg << endl;
							status = 1;
						}
					}
				}
			}
		}
		else
		{
			cerr << "Error - cannot find bmp using key " << key << " : " << bmparg << endl;
			status = 1;
		}
	}
	else
	{
		cerr << "Error in bmp input (expect 3 or 5 args): " << bmparg << endl;
		status = 1;
	}
	return status;
}


int parse_mel_params(const std::string& filename, vector<MelTrialSpec>& trialSpecs)
{
	int status = 0;
	trialSpecs.clear();
	std::ifstream myfile(filename.c_str());
	MyPathMatcher mpm(".bmp");
	FetcherWithFunctor<string> withFunctor(mpm);

	if (myfile.is_open())
	{
		// open file, read line-by-line and parse
		string line;
		int linenumber = 0;
		int iTrialStep = 0;
		vector<string> tokens;
		MelTrialSpec spec;
		MelGridSpec grid;
		MelFramePair melPair;
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

					// Expecting "trial" or "folder"
					if (string::npos != line.find("trial"))
					{
						//cerr << "Start of trial found" << endl;
						iTrialStep = 1;

						// initialize trial spec
						spec.lastFrame = 0;
						spec.vecFrames.clear();
						spec.grid = grid;
						melPair.first = 0;
						melPair.second.vecBmps.clear();
						melPair.second.vecRects.clear();
					}
					else if (string::npos != line.find("folder"))
					{
						string folder(line.substr(line.find("folder") + 6));
						boost::trim(folder);
						cerr << "Got folder:" << folder << endl;
						withFunctor.addFolder(folder);
					}
					else
					{
						cerr << "Unexpected input at line " << linenumber << " - looking for \"trial\": " << line << endl;
						status = 1;
					}
					break;

				case 1:
					// at this point we expect a "grid" or "time" or "frames" or "rect" or "bmp"
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
					else if (string::npos != line.find("time") || string::npos != line.find("frames"))
					{
						bool bFrames = (string::npos != line.find("frames"));

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
								if (bFrames) frames = t;
								else frames = SECONDS_TO_FRAMES(t);

								// The "time" line ends the last "time" block. Push that frvPair onto the
								// current spec. 
								spec.vecFrames.push_back(melPair);

								// clear the placeholder
								melPair.second.vecBmps.clear();
								melPair.second.vecRects.clear();

								if (tokens.size() == 2)
								{
									// save the number of frames in the melPair placeholder 
									melPair.first = frames;
								}
								else
								{
									if (tokens.size() == 3 && boost::iequals(tokens[2], "end"))
									{
										// this signifies the end of the trial. 
										// save the end frame in the current trial spec, and push the whole thing 
										// onto the trialSpecs vector. Return to trialStep 0 - look for "trial"
										spec.lastFrame = frames;
										spec.grid = grid;
										std::sort(spec.vecFrames.begin(), spec.vecFrames.end(), [](MelFramePair& a, MelFramePair& b) { return a.first < b.first; });
										trialSpecs.push_back(spec);
										iTrialStep = 0;
									}
									else
									{
										cerr << "Error in input at line " << linenumber << " - expecting time|frame t [end]: " << line << endl;
										status = 1;
									}
								}
							}
						}
						else
						{
							cerr << "Error on line " << linenumber << " : expecting time|frame value" << endl;
							status = 1;
						}
					}
//					else if (string::npos != line.find("rect"))
					else if (0 == line.find("rect"))
					{
						//cerr << "Found rect line: " << line << endl;
						if (parse_rectangle(line.substr(line.find("rect") + 4), rect))
						{
							cerr << "Error parsing rect on line " << linenumber << ": " << line << endl;
							status = 1;
						}
						else
						{
							melPair.second.vecRects.push_back(rect);
						}
					}
					else if (string::npos != line.find("bmp"))
					{
						// expect 3 parameters:  key,x,y
						tokens.clear();
						string bmparg(line.substr(line.find("bmp") + 3));
						boost::trim(bmparg);
						MelBmpSpec bmpSpec;
						if (!parse_bmp_spec(bmparg, withFunctor, bmpSpec))
							melPair.second.vecBmps.push_back(bmpSpec);
						else
						{
							cerr << "Error parsing bmp spec at line " << linenumber << endl;
							status = 1;
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
