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

	virtual int num_pages() { return 3; };
	virtual int num_overlay_pages() { return 0; };
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_page[2];
	int m_current_page;
	int m_fixpt_page;
	bool m_bUseCycling;
	int m_iCyclingDelay;

	void setup_cycling();

protected:
	void advance();
	void set_initial_parameters();
};
