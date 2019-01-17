#pragma once

#include "StimSet.h"
#include <vector>
#include <algorithm>
#include <map>
#include <boost/filesystem.hpp>

//#define HOST_PAGE_TEST
#define HOST_PAGE_COPY

void dumpPalette(const std::string& s, VSGLUTBUFFER& buffer, int N, int startN=0);
void dumpHWPalette(const std::string& s, int N, int startN=0);

typedef std::pair<COLOR_TYPE, ARContrastRectangleSpec* > ColorRectPair;

class RectanglePool
{
	vector<ColorRectPair> m_vec;
	RectanglePool() {};
	static RectanglePool& instance() { static RectanglePool pool; return pool; };
	vector<ColorRectPair>& vec() { return m_vec; };
public:
	virtual ~RectanglePool();
	static ARContrastRectangleSpec *getRect(const COLOR_TYPE& c);
	static void reset();	// delete vsg objects, empty pool
};


// the ui is a frame number. The vector is a list of rectangles to be drawn on that frame
// Its a pair, with a frame and a rect vec.
typedef std::pair<unsigned int, vector<ARContrastRectangleSpec> > FrameRectVecPair;


typedef struct
{
	double x, y;
	std::string filename;
	DWORD drawMode;			// D = vsgCOPYONSOURCE, S = vsgCOPYONDEST
	DWORD level;			// setPen2
	bool copyPalette;		// if true, copy levels from image file to HW palette
	DWORD startCopyLevel;	// first level to copy
	DWORD numCopyLevel;		// copy this many levels
} MelBmpSpec;


// Updated to allow for loading bmp images. 
// Now we have a pair, where the first piece of it is a frame number, as before. 
// The second part of the pair is a struct that contains a vector of rectangles (as before), and
// a string that, if not null, contains a filename that is loaded with vsgImageDraw. The coords x,y are
// used in that call (and are the point where the center of the image goes). 

typedef struct 
{
	vector<MelBmpSpec> vecBmps;
	vector<ARContrastRectangleSpec> vecRects;
} MelFrame;
typedef std::pair<unsigned int, MelFrame> MelFramePair;


typedef struct
{
	double xGridCenter, yGridCenter, wGrid, hGrid, oriDegrees;
} MelGridSpec;


typedef struct
{
	vector<MelFramePair> vecFrames;
	unsigned int lastFrame;	// all off on this frame;
	MelGridSpec grid;
} MelTrialSpec;

int parse_bmp_spec(const string& bmparg, const std::map<std::string, boost::filesystem::path>& fileMap, MelBmpSpec& bmpSpec);
int parse_mel_params(const std::string& filename, vector<MelTrialSpec>& trialSpecs);

// The vector of pairs may have one with the same frame value as the frvPair supplied. If that's the case, append
// the elements of frvPair to that pair. If not, push the frvPair onto vecPairs. 
// This allows for rects being added at different positions in the trial spec. 
void addOrAppendfrvPair(vector<FrameRectVecPair>& vecPairs, const FrameRectVecPair& frvPair);

class MelStimSet : public StimSet
{
private:
	std::vector<ARContrastFixationPointSpec> m_vecFixpts;
	vector<MelTrialSpec> m_trialSpecs;
	PIXEL_LEVEL m_levelWhite;
	unsigned int m_uiCurrentTrial;
	std::vector<int> m_pagesAvailable;
	int m_pageFixpt;
	int m_pageBlank;

#ifdef HOST_PAGE_TEST
	int m_hostPageHandle;
#endif
#ifdef HOST_PAGE_COPY
	int m_hostPageHandle;
#endif


	int drawCurrent();
	void applyTransform(ARContrastRectangleSpec& result, const ARContrastRectangleSpec& original, const MelGridSpec& grid);
	void copyBmpFile(char *filename, const MelBmpSpec& bmp, int page);

public:
	MelStimSet(const ARContrastFixationPointSpec& fixpt, const vector<MelTrialSpec>& trialSpecs) : m_trialSpecs(trialSpecs), m_uiCurrentTrial(0) { m_vecFixpts.push_back(fixpt); };
	MelStimSet(const std::vector<alert::ARContrastFixationPointSpec>& vecFixpt, const vector<MelTrialSpec>& trialSpecs) : m_vecFixpts(vecFixpt), m_trialSpecs(trialSpecs), m_uiCurrentTrial(0) {};
	
	virtual ~MelStimSet() {};

	// subclasses should return the number of pages they will need.
	virtual int num_pages() {
		return 32;
	};

	// subclasses should return the number of pages they will need.
	virtual int num_overlay_pages() {
		return 0;
	};

	// initialize the pages indicated by the elements of the vector. Subclasses may 
	// call vsgPresent() if needed. The first version has a default implementation 
	// that uses the regular vsg card. The second version should be used for master/slave
	// cases. 
	virtual int init(std::vector<int> pages);
	virtual int init(ARvsg& vsg, std::vector<int> pages) {
		return -1;
	};

	// clean up any messes created in init() - esp settings in VSG
	virtual void cleanup(std::vector<int> pages);

	// handle the trigger indicated by the string s. Do not call vsgPresent! return value of 
	// 1 means vsgPresent() will be called. 
	virtual int handle_trigger(std::string& s);

	virtual std::string toString() const;

};
