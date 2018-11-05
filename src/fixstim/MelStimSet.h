#pragma once

#include "StimSet.h"
#include <vector>
#include <algorithm>

#define HOST_PAGE_TEST

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
};


// the ui is a frame number. The vector is a list of rectangles to be drawn on that frame
// Its a pair, with a frame and a rect vec.
typedef std::pair<unsigned int, vector<ARContrastRectangleSpec> > FrameRectVecPair;

typedef struct
{
	double xGridCenter, yGridCenter, wGrid, hGrid, oriDegrees;
} MelGridSpec;

typedef struct
{
	vector<FrameRectVecPair> vecPairs;
	unsigned int lastFrame;	// all off on this frame;
	MelGridSpec grid;
} MelTrialSpec;

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
	unsigned int m_hostPageHandle;
#endif

	int drawCurrent();
	void applyTransform(ARContrastRectangleSpec& result, const ARContrastRectangleSpec& original, const MelGridSpec& grid);

public:
	MelStimSet(const ARContrastFixationPointSpec& fixpt, const vector<MelTrialSpec>& trialSpecs) : m_trialSpecs(trialSpecs), m_uiCurrentTrial(0) { m_vecFixpts.push_back(fixpt); };
	MelStimSet(const std::vector<alert::ARContrastFixationPointSpec>& vecFixpt, const vector<MelTrialSpec>& trialSpecs) : m_vecFixpts(vecFixpt), m_trialSpecs(trialSpecs), m_uiCurrentTrial(0) {};
	
	virtual ~MelStimSet() {};

	// subclasses should return the number of pages they will need.
	virtual int num_pages() {
		return 12;
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
