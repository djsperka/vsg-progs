#pragma once
#include "StimSet.h"
#include <string>
#include <boost/ptr_container/ptr_vector.hpp>

using namespace std;
using namespace alert;
using namespace boost;

class FXGStimParameterList;

class MultiParameterFXMultiGStimSet : public FXMultiGStimSet, public ptr_vector<FXGStimParameterList>
{
public:
	MultiParameterFXMultiGStimSet() : FXMultiGStimSet() {};
	MultiParameterFXMultiGStimSet(ARGratingSpec& grating) : FXMultiGStimSet() { add_grating(grating);  };
	MultiParameterFXMultiGStimSet(ARGratingSpec& grating, ARContrastFixationPointSpec& fixpt) : FXMultiGStimSet(fixpt) { add_grating(grating); };
	MultiParameterFXMultiGStimSet(ARContrastFixationPointSpec& fixpt) : FXMultiGStimSet(fixpt) {};
	MultiParameterFXMultiGStimSet(ARContrastFixationPointSpec& fixpt, ARXhairSpec& xhair) : FXMultiGStimSet(fixpt, xhair) {};
	MultiParameterFXMultiGStimSet(ARGratingSpec& grating, ARXhairSpec& xhair) : FXMultiGStimSet(xhair) { add_grating(grating); };
	MultiParameterFXMultiGStimSet(ARGratingSpec& grating, ARContrastFixationPointSpec& fixpt, ARXhairSpec& xhair) : FXMultiGStimSet(fixpt, xhair) { add_grating(grating); };
	virtual ~MultiParameterFXMultiGStimSet() {};
	void setCyclingDelay(int ndelay);
	void setStimDuration(double seconds);
	void setPursuitParameters(double durSeconds, double dirDegrees, double degPerSecond);
	void setSweepNotPursuit(bool bSweep = true) { m_bSweepNotPursuit = bSweep; };
	virtual int num_pages() { return 5; };
	virtual int num_overlay_pages() { return 0; };
	virtual int init(ARvsg& vsg, std::vector<int> pages, int num_stim_pages);
	virtual int handle_trigger(const std::string& s, const std::string& args);
	virtual std::string toString() const;
private:
	int m_blank_page;
	int m_fixpt_page;
	int m_fixpt_dot_page;
	int m_stim_page;
	int m_alt_page;
	int m_num_stim_pages;
	bool m_bUseDrawGroups;
	bool m_bResetPhaseOnTrigger;
	bool m_bSweepNotPursuit;
	int m_iCyclingType;
	int m_iCyclingDelay;
	int m_iStimDuration;	// if > 0, the number of frames stim lasts until blank. If <=0, no blank. 
	double m_dxPursuit;
	double m_dyPursuit;
	// These added for page flipping machinations as in the D (serial) command.
	size_t m_pageflipindex;
	int m_pageflippages[2];
	std::vector<COLOR_TYPE> m_dotColorsSaved;

	void setup_cycling();
	void draw_current();
	void draw_stuff_on_page(int pagenumber, bool bFixpt, bool bDistractor, bool bGrating, bool bDots, int iDrawGroup = -1);
protected:
	void advance();
	void set_current(size_t index);
	void set_initial_parameters();
};
