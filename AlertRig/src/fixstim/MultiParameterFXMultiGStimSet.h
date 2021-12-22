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

	virtual int num_pages() { return 5; };
	virtual int num_overlay_pages() { return 0; };
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_blank_page;
	int m_fixpt_page;
	int m_fixpt_dot_page;
	int m_stim_page;
	//bool m_bUseCycling;
	int m_iCyclingType;
	int m_iCyclingDelay;
	int m_iStimDuration;	// if > 0, the number of frames stim lasts until blank. If <=0, no blank. 
	double m_dxPursuit;
	double m_dyPursuit;

	void setup_cycling();
	void draw_current();

protected:
	void advance();
	void set_initial_parameters();
};
