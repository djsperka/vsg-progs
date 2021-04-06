#pragma once
#include "SequencedAttentionStimSet.h"

class SequencedImagesAttentionStimSet : public StimSet
{
	ARFixationPointSpec m_fixpt;
	ImageFilesPositions m_ifp;
	vector<AttentionCue>& m_vecCuePairs;
	vector<AttentionSequenceTrialSpec>& m_trialSpecs;
	int m_pageBlank;
	int m_pageFixpt;
	unsigned int m_current;
	std::vector<int> m_pages;

	// Draw the current trial, i.e. m_trialSpecs[m_current]
	void drawCurrent();

public:
	SequencedImagesAttentionStimSet(ARContrastFixationPointSpec& fixpt, const ImageFilesPositions& ifp, vector<AttentionCue>& vecCuePairs, bool bCueCircles, bool bCuePoints, bool bCueIsDot, vector<AttentionSequenceTrialSpec>& trialSpecs);
	int num_pages() { return 24; };
	int num_overlay_pages() { return 0; };
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
	virtual void cleanup(std::vector<int> pages);
};