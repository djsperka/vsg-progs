#pragma once

#include "StimSet.h"
#include <vector>
#include <algorithm>

// the ui is a frame number. The vector is a list of rectangles to be drawn on that frame
typedef std::pair<unsigned int, vector<ARRectangleSpec> > FrameRectVec;

typedef struct
{
	double xGridCenter, yGridCenter, wGrid, hGrid, oriDegrees;
	vector<FrameRectVec> frv;
	unsigned int lastFrame;	// all off on this frame
} MelTrialSpec;

int parse_mel_params(const std::string& filename, vector<MelTrialSpec>& trialSpecs);


class MelStimSet : public StimSet
{
private:
	ARFixationPointSpec m_fixpt;

public:
	MelStimSet(const ARFixationPointSpec& fixpt) : m_fixpt(fixpt) {};
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
	virtual void cleanup(std::vector<int> pages) {};

	// handle the trigger indicated by the string s. Do not call vsgPresent! return value of 
	// 1 means vsgPresent() will be called. 
	virtual int handle_trigger(std::string& s);

	virtual std::string toString() const;

};
