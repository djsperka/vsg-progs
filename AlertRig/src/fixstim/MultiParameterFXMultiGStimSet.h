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
	MultiParameterFXMultiGStimSet();
	MultiParameterFXMultiGStimSet(ARGratingSpec& grating);
	MultiParameterFXMultiGStimSet(ARGratingSpec& grating, ARContrastFixationPointSpec& fixpt);
	MultiParameterFXMultiGStimSet(ARContrastFixationPointSpec& fixpt);
	MultiParameterFXMultiGStimSet(ARContrastFixationPointSpec& fixpt, ARXhairSpec& xhair);
	MultiParameterFXMultiGStimSet(ARGratingSpec& grating, ARXhairSpec& xhair);
	MultiParameterFXMultiGStimSet(ARGratingSpec& grating, ARContrastFixationPointSpec& fixpt, ARXhairSpec& xhair);
	virtual ~MultiParameterFXMultiGStimSet() {};
	void setCyclingDelay(int ndelay);
	void setStimDuration(double seconds);
	void setPursuitParameters(double durSeconds, double dirDegrees, double degPerSecond);
	bool setUseCycling(bool bUseCycling, bool bSweepCycling = true);	// This can be called once, but throws error if called a second time.
	//ZZZZZZ void setSweepNotPursuit(bool bSweep = true) { m_bSweepNotPursuit = bSweep; };
	virtual int num_pages() { return 5; };
	virtual int num_overlay_pages() { return 0; };
	virtual int init(std::vector<int> pages, int num_stim_pages);
	virtual int handle_trigger(const std::string& s, const std::string& args);
	virtual std::string toString() const;
	void cleanup(std::vector<int> pages);

	void setBmpImageSpec(std::vector<alert::ARImageSpec>& vec, unsigned int nlevels)
	{
		m_vecBmpImageSpec = vec;
		m_bHaveBmpImageSpec = true;
		m_uiNBmpImageLevels = nlevels;
	}
	bool hasImageOverride() const { return m_bHaveBmpImageSpec && m_iBmpImageOverride > -1 && m_iBmpImageOverride < m_vecBmpImageSpec.size(); }
	ARImageSpec& getImageOverride() { return m_vecBmpImageSpec[m_iBmpImageOverride]; }
	void setBmpImageOverride(int i) { m_iBmpImageOverride = i; }

private:
	int m_blank_page;
	int m_fixpt_page;
	int m_fixpt_dot_page;
	int m_stim_page;
	int m_alt_page;
	int m_num_stim_pages;
	bool m_bUseDrawGroups;
	bool m_bResetPhaseOnTrigger;
	//bool m_bSweepNotPursuit;	// when using cycling, is it sweep cycling (fixpt stationary) or smooth pursuit (fixpt and stim move)
	bool m_bUseCycling;		// we always init for _not_ using cycling. 
	bool m_bUseSweepCycling;	// if true, fixpt stationary (sweep cycling); if false, smoot pursuit (fixpt and stim move together). Ignored if !m_bUseCycling.
	int m_iCyclingType;			// this is set at the start of each trial. 
	int m_iCyclingDelay;
	int m_iStimDuration;	// if > 0, the number of frames stim lasts until blank. If <=0, no blank. 
	double m_dxPursuit;
	double m_dyPursuit;
	// These added for page flipping machinations as in the D (serial) command.
	size_t m_pageflipindex;
	int m_pageflippages[2];
	std::vector<COLOR_TYPE> m_dotColorsSaved;
	std::vector<alert::ARImageSpec> m_vecBmpImageSpec;
	unsigned int m_uiNBmpImageLevels;
	bool m_bHaveBmpImageSpec;
	int m_iBmpImageOverride;	// if > 0 and m_bHaveBmpImageSpec==true, then img is drawn instead of stimuli. 

	static const int m_max_cycle_count = 32768;
	VSGCYCLEPAGEENTRY m_cycle_params[m_max_cycle_count];	// warning! No check on usage. You have been warned. 
	int m_fixpt_cycle_start;
	int m_fixpt_cycle_count;
	int m_stim_cycle_start;
	int m_stim_cycle_count;
	int m_clear_cycle_start;
	int m_clear_cycle_count;

	void setup_cycling();
	void draw_current();
	void draw_stuff_on_page(int pagenumber, bool bFixpt, bool bDistractor, bool bGrating, bool bDots, bool bRectangles, int iDrawGroup = -1);
protected:
	void advance();
	void set_current(size_t index);
	void set_initial_parameters();
};
