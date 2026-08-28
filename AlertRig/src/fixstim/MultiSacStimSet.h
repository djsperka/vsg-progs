#pragma once
#include "Alertlib.h"
#include "StimSet.h"

class MultiSacStimSet : public StimSet
{
private:
	ARContrastFixationPointSpec m_fixpt;

	int drawCurrent();

public:

	MultiSacStimSet(double x, double y, double size0, double size1, COLOR_TYPE c0, COLOR_TYPE c1, vector<int>& stim);
	MultiSacStimSet(const ARContrastFixationPointSpec& fixpt);

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
