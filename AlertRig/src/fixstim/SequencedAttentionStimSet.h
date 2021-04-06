#pragma once

#include <string>
#include <algorithm>
#include <map>
#include <list>
#include "Alertlib.h"
#include "StimSet.h"
#include "AttentionCue.h"

const unsigned int FixptIndex = 100;
const unsigned int CueIndex = 101;
const unsigned int EndIndex = 999;	// sets all stim to given contrast, makes this the stop frame.
const unsigned int FirstImageIndex = 1000;
// image indices will start at EndIndex + 1.

typedef std::tuple<double, double> ImageXY;
typedef std::tuple<std::vector<std::string>, std::vector<ImageXY> > ImageFilesPositions;

typedef std::tuple<std::string, double, double> ImageInfo;

// index, contrast pair
typedef std::pair< unsigned int, int > ICPair;

// map of index, contrast pairs, with key = frames
typedef std::multimap<unsigned int, ICPair> ICPairMap;


// Resource pool for gratings
class GratingPool
{
	std::list<ARGratingSpec* > not_in_use;
	std::list<ARGratingSpec* > in_use;

	GratingPool() {};
public:
	static GratingPool& instance();	// singleton
	~GratingPool() {};
	void populate(int n, int nlevels);	// dangerous, call just once

	ARGratingSpec *getGrating();
	void returnGrating(ARGratingSpec *);
	void printPoolStatus();
};


// When putting together a trial, this class represents a grating at a position. During a
// trial, this grating may be drawn at a different contrast, then again at the original contrast. 
// This is what happens for a grating that undergoes a contrast change. When this happens, the VSG
// requires separate objects. When you set the contrast of this object to a particular value,
// it will clone a new grating object if it hasn't already configured one at this contrast. 
// When draw() is called, the one with the last-set contrast is drawn. Contrast=0 is handled differently,  
// we don't configure a new one, and we don't draw anything. 

class SequenceHelper
{
	int m_index;
	int m_contrast;
	virtual void setContrastPriv() = 0;
public: 
	SequenceHelper(int index, int contrast): m_index(index), m_contrast(contrast) {};
	virtual ~SequenceHelper() {};
	int contrast() const { return m_contrast; };
	virtual void setContrast(int contrast) { m_contrast = contrast;  setContrastPriv(); };
	virtual void draw(double initial_phase) = 0;
	ICPair makeICPair() const { return make_pair(m_index, m_contrast); };
};

class FixptSequenceHelper: public SequenceHelper
{
	ARContrastFixationPointSpec& m_fixpt;
	void setContrastPriv() {};
public:
	FixptSequenceHelper(int index, int defaultContrast, ARContrastFixationPointSpec& fixpt): SequenceHelper(index, defaultContrast), m_fixpt(fixpt) {};
	virtual ~FixptSequenceHelper() {};
	void draw(double initial_phase) { draw(); };
	void draw();
};

class CueSequenceHelper : public SequenceHelper
{
	int m_ngratings;
	std::vector<alert::ARContrastCueCircleSpec> m_circles;
	void setContrastPriv() {};

public:
	CueSequenceHelper(int index, int defaultContrast, int ngratings, const std::vector<alert::ARContrastCueCircleSpec>& circles)
		: SequenceHelper(index, defaultContrast), m_ngratings(ngratings), m_circles(circles) {};
	virtual ~CueSequenceHelper() {};
	void draw(double initial_phase);	// no-op
	void draw_cues(int offbits, bool bCircles);
	//void draw_cue_points(int offbits);
};

class GratingSequenceHelper: public SequenceHelper
{
	std::map<int, ARGratingSpec* > m_gratingMap;
	ARGratingSpec m_gratingDefault;
	int m_defaultContrast;
	void setContrastPriv();
public:
	GratingSequenceHelper(int index, int defaultContrast, const ARGratingSpec& grating);
	~GratingSequenceHelper();

	// draw a grating at current contrast
	void draw(double initial_phase);

	// return any used gratings to pool
	void reset();
};

class ImageSequenceHelper : public SequenceHelper
{
	std::map<int, ARGratingSpec* > m_gratingMap;
	ARGratingSpec m_gratingDefault;
	int m_defaultContrast;
	void setContrastPriv();
public:
	ImageSequenceHelper(int index, int defaultContrast, const ARGratingSpec& grating);
	~ImageSequenceHelper();

	// draw a grating at current contrast
	void draw(double initial_phase);

	// return any used gratings to pool
	void reset();
};


// vector of index,contrast pairs, represents a page. The index is either FixptIndex or CueIndex, or an integer 0,1,... which represents an
// index into m_vecOSH.
class PageVec : public std::vector<ICPair>
{
public:
	PageVec() : std::vector<ICPair>() {};
	PageVec(const PageVec& pv) : std::vector<ICPair>(pv) {};
	void update(const ICPair& p);	// throws if unknown index
};

//  a vector of all pages configured for a trial
typedef std::vector< std::pair<int, PageVec> > PageVecs;

typedef struct
{
	COLOR_TYPE color;
	int offbits;
	vector<double> initialPhase;
	ICPairMap icpm;
} AttentionSequenceTrialSpec;



int parse_sequenced_params(const std::string& arg, unsigned int ngratings, std::vector<AttentionSequenceTrialSpec>& trialSpecs, ImageFilesPositions& filespos);


#define SECONDS_TO_FRAMES(t) (int)((t) * 1000000.0 /vsgGetSystemAttribute(vsgFRAMETIME))
#define FRAMES_TO_SECONDS(n) ((n) * vsgGetSystemAttribute(vsgFRAMETIME)/1000000.0)

class SequencedAttentionStimSet : public StimSet
{
public:
	SequencedAttentionStimSet(ARContrastFixationPointSpec& fixpt, vector<alert::ARGratingSpec>& vecGratings, vector<AttentionCue>& vecCuePairs, bool bCueCircles, bool bCuePoints, bool bCueIsDot, vector<AttentionSequenceTrialSpec>& trialSpecs);
	~SequencedAttentionStimSet() {};

	int num_pages() { return 24; };
	int num_overlay_pages() { return 0; };
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;

private:
	void updateHelper(const ICPair& icp);
	int drawCurrent();
	void draw_fixpt();
	bool findConfiguredPage(const PageVec& pv, const PageVecs& pagesConfigured, int& pageNumber);
	int drawPageUsingPageVec(const PageVec& pv, int page, int offbits, const vector<double>& initial_phase);
	int drawPageUsingPageVec(const PageVec& pv, int page, int offbits, double initial_phase);
	PageVec makePageVec();	// current values in fixpt, cue, gratings. 

	vector<AttentionSequenceTrialSpec> m_trialSpecs;
	alert::ARContrastFixationPointSpec m_fixpt;
	bool m_bUseCueCircles;
	bool m_bUseCuePoints;
	bool m_bCuePointIsDot;
	int m_pageBlank;
	int m_pageFixpt;
	vector<alert::ARGratingSpec> m_vecGratings;
	vector<alert::ARContrastCueCircleSpec> m_vecCueCircles;
	//vector<alert::ARContrastRectangleSpec> m_vecCueRects;
	unsigned int m_current;
	std::vector<int> m_pages;
	vector<GratingSequenceHelper *> m_gratingHelpers;
	FixptSequenceHelper *m_pFixptHelper;
	CueSequenceHelper *m_pCueHelper;
};
