#pragma once
#include "Alertlib.h"
#include "StimSet.h"

// A frame to be drawn can contain 0 or more gratings
typedef std::vector<ARGratingSpec> FrameVector;
typedef std::vector<FrameVector> SingleTrialVector;
typedef std::vector<SingleTrialVector> AllTrialsVector;

class MultiSacStimSet : public FXMultiGStimSet
{
private:
	ARContrastFixationPointSpec m_fixpt;
	AllTrialsVector m_atv;
	int m_current;

	int drawCurrent();

public:

	MultiSacStimSet(ARContrastFixationPointSpec& fixpt) : FXMultiGStimSet(fixpt) {};

	virtual ~MultiSacStimSet() {};

	// subclasses should return the number of pages they will need.
	virtual int num_pages() {
		return 5;
	};

	// subclasses should return the number of pages they will need.
	virtual int num_overlay_pages() {
		return 0;
	};

	virtual int init(std::vector<int> pages, int);

	// clean up any messes created in init() - esp settings in VSG
	virtual void cleanup(std::vector<int> pages) {};

	// handle the trigger indicated by the string s. Do not call vsgPresent! return value of 
	// 1 means vsgPresent() will be called. 
	virtual int handle_trigger(const std::string& s, const std::string&);

	virtual std::string toString() const;

};

MultiSacStimSet* parseMultiSacStimSet(const std::string& filename, alert::ARContrastFixationPointSpec& fixpt);

