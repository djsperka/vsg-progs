#ifndef _MULTIPARAMETERFXGSTIMSET_H_
#define _MULTIPARAMETERFXGSTIMSET_H_

#include "StimSet.h"
#include <string>
#include <boost/ptr_container/ptr_vector.hpp>

using namespace std;
using namespace alert;
using namespace boost;

class FXGStimParameterList;

class MultiParameterFXGStimSet: public FXGStimSet, public ptr_vector<FXGStimParameterList>
{
public:
	MultiParameterFXGStimSet(): FXGStimSet() {};
	MultiParameterFXGStimSet(ARGratingSpec& grating): FXGStimSet(grating) {};
	MultiParameterFXGStimSet(ARGratingSpec& grating, ARContrastFixationPointSpec& fixpt): FXGStimSet(grating, fixpt) {}; 
	MultiParameterFXGStimSet(ARContrastFixationPointSpec& fixpt): FXGStimSet(fixpt) {}; 
	MultiParameterFXGStimSet(ARContrastFixationPointSpec& fixpt, ARXhairSpec& xhair): FXGStimSet(fixpt, xhair) {}; 
	MultiParameterFXGStimSet(ARGratingSpec& grating, ARXhairSpec& xhair): FXGStimSet(grating, xhair) {}; 
	MultiParameterFXGStimSet(ARGratingSpec& grating, ARContrastFixationPointSpec& fixpt, ARXhairSpec& xhair): FXGStimSet(grating, fixpt, xhair) {}; 
	virtual ~MultiParameterFXGStimSet() {};
	void setCyclingDelay(int ndelay);

	virtual int num_pages() {return 3;};
	virtual int num_overlay_pages() {return 0;};
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

#endif
