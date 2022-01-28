#pragma once
#include "SequencedAttentionStimSet.h"

// this class is a copy of FixptSequenceHelper, but it uses ARFixationPointSpec (which will work 
// correctly when using vsgTRUECOLOURMODE video mode.
class ImFixptSequenceHelper : public SequenceHelper
{
	ARFixationPointSpec& m_fixpt;
	void setContrastPriv() {};
public:
	ImFixptSequenceHelper(int index, int defaultContrast, ARFixationPointSpec& fixpt) : SequenceHelper(index, defaultContrast), m_fixpt(fixpt) {};
	virtual ~ImFixptSequenceHelper() {};
	void draw(double initial_phase) { draw(); };
	void draw();
};

// this class is a copy of CueSequenceHelper, but it uses ARCueCircleSpec (which will work 
// correctly when using vsgTRUECOLOURMODE video mode.

// TODO - must create ARCueCircleSpec
class ImCueSequenceHelper : public SequenceHelper
{
	int m_ngratings;
	std::vector<alert::ARCueCircleSpec> m_circles;
	void setContrastPriv() {};

public:
	ImCueSequenceHelper(int index, int defaultContrast, int ngratings, const std::vector<alert::ARCueCircleSpec>& circles)
		: SequenceHelper(index, defaultContrast), m_ngratings(ngratings), m_circles(circles) {};
	virtual ~ImCueSequenceHelper() {};
	void draw(double initial_phase);	// no-op
	void draw_cues(int offbits, bool bCircles);
	//void draw_cue_points(int offbits);
};


// ImageSequenceHelper uses contrast() as an index into m_imageFiles.
// Setting contrast to a negative value turns off the image - will not be drawn.
class ImageSequenceHelper : public SequenceHelper
{
	double m_x, m_y;
	int m_imageIndex;
	const vector<string>& m_imageFiles;
	void setContrastPriv() {};		// use contrast() to get image index. 
public:
	ImageSequenceHelper(int index, double x, double y, const vector<string>& vecImageFiles)
		: SequenceHelper(index, -1)
		, m_x(x)
		, m_y(y)
		, m_imageFiles(vecImageFiles)
	{};
	virtual ~ImageSequenceHelper() {};

	// draw a grating at current contrast
	void draw(double initial_phase = 0);
};




class SequencedImagesAttentionStimSet : public StimSet
{
	ARFixationPointSpec m_fixpt;
	ImageFilesPositions m_ifp;
	vector<AttentionCue> m_vecCuePairs;
	vector<AttentionSequenceTrialSpec> m_trialSpecs;
	bool m_bUseCueCircles;
	bool m_bUseCuePoints;
	bool m_bCuePointIsDot;
	int m_pageBlank;
	int m_pageFixpt;
	unsigned int m_current;
	std::vector<int> m_pages;
	ImFixptSequenceHelper *m_pFixptHelper;
	ImCueSequenceHelper *m_pCueHelper;
	vector<ImageSequenceHelper *> m_imageHelpers;
	vector<ARCueCircleSpec> m_vecCueCircles;

	// Draw the current trial, i.e. m_trialSpecs[m_current]
	void drawCurrent();
	void drawPage(int pageNumber, int offbits);
	void updateHelper(const ICPair& icp);


public:
	SequencedImagesAttentionStimSet(ARContrastFixationPointSpec& fixpt, const ImageFilesPositions& ifp, vector<AttentionCue>& vecCuePairs, bool bUseCueCircles, bool bUseCuePoints, bool bCuePointIsDot, vector<AttentionSequenceTrialSpec>& trialSpecs);
	int num_pages() { return 24; };
	int num_overlay_pages() { return 0; };
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(const std::string& s, const std::string&);
	virtual std::string toString() const;
	virtual void cleanup(std::vector<int> pages);
};