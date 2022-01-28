#pragma once
#include "Alertlib.h"
#include "StimSet.h"


// bit patterns 
#define SZ_BIT		0x10
#define LC_BIT		0x08
#define ORI_MASK	0x07
#define ORI_0		0x00
#define ORI_45		0x01
#define ORI_90		0x02
#define ORI_135		0x03
#define ORI_180		0x04
#define ORI_225		0x05
#define ORI_270		0x06
#define ORI_315		0x07

class BorderStimSet : public StimSet
{
private:
	double m_xrf, m_yrf;	// receptive field center
	COLOR_TYPE m_color0, m_color1;
	double m_size0, m_size1;
	std::vector<int> m_vecStim;
	PIXEL_LEVEL m_levelColor0, m_levelColor1;
	unsigned int m_uiCurrentTrial;
	int m_pageBackground;
	int m_pageStimulus;
	int m_pageStimulusFixpt;
	ARContrastRectangleSpec m_rect;
	bool m_bHasFixpt;
	ARContrastFixationPointSpec m_fixpt;

	int drawCurrent();

public:

	BorderStimSet(double x, double y, double size0, double size1, COLOR_TYPE c0, COLOR_TYPE c1, vector<int>& stim);
	BorderStimSet(const ARContrastFixationPointSpec& fixpt, double x, double y, double size0, double size1, COLOR_TYPE c0, COLOR_TYPE c1, vector<int>& stim);

	virtual ~BorderStimSet() {};

	// subclasses should return the number of pages they will need.
	virtual int num_pages() {
		return 3;
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
	virtual int handle_trigger(const std::string& s, const std::string&);

	virtual std::string toString() const;

};

BorderStimSet* parseBorderStimSet(const std::string& s);