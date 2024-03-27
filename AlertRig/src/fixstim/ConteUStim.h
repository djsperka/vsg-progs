#ifndef _CONTEUSTIM_H_
#define _CONTEUSTIM_H_

#include "UStim.h"
#include "ConteMisc.h"
#include "alertlib.h"
#include "AlertUtil.h"
#include <vector>
#include <queue>
#include <algorithm>
#include <boost/tuple/tuple.hpp>





// Implementation of UStim interface for the starstim app.
class ConteUStim: public UStim
{
public:
	ConteUStim();
	virtual ~ConteUStim();

	bool parse(int argc, char **argv);
	void run_stim(alert::ARvsg& vsg);
	int process_arg(int option, std::string& arg);

	// This acts as the callback function for the triggers
	int callback(int &output, const FunctorCallbackTrigger* ptrig, const std::string&);

private:
	struct conte_arguments m_arguments;
	unsigned int m_itrial;
	bool m_quit;
	int m_errflg;

	PIXEL_LEVEL m_levelOverlayBackground;
	PIXEL_LEVEL m_levelOverlayFixpt;
	PIXEL_LEVEL m_levelOverlayCueRect0;
	PIXEL_LEVEL m_levelOverlayCueRect1;
	PIXEL_LEVEL m_levelOverlayAperture;

	vector<PIXEL_LEVEL> m_levelCueColors;
	PIXEL_LEVEL m_levelTest;
	static const unsigned int m_max_cycle_count = 256;
	unsigned int m_cycle_params_count;
	VSGCYCLEPAGEENTRY m_cycle_params[m_max_cycle_count];	// warning! No check on usage. You have been warned. 
	VSGCYCLEPAGEENTRY m_cycle_clear_params[2];				// warning! No check on usage. You have been warned. 
	VSGCYCLEPAGEENTRY m_cycle_fixpt_params[2];				// warning! No check on usage. You have been warned. 

	static WORD cOvPageClear;
	static WORD cOvPageAperture;
	static WORD cPageBackground;
	static WORD cPageFixpt;
	static WORD cPageCue;
	static WORD cPageSample;
	static WORD cPageTarget;

	string sState;

	ARConteSpec m_sample0, m_sample1, m_target0, m_target1;

	// These are the args allowed and which are handled by prargs. Do not use 'F' - it is reserved for 
	// passing a command file.
	static const string m_allowedArgs;

	// draw current trial into video memory
	void draw_current();

	// restore drawing pages to IDLE state
	void pagesToIdleState();

	// initialize pages at start of this stim
	void init();

	// cleanup anything from init() that needs cleaning.
	void cleanup();

	// setup triggers
	void init_triggers(TSpecificFunctor<ConteUStim>* pfunctor);

	// draw dot patches on single page
	void draw_dot_patches(const ConteXYHelper& xyhelper, const conte_trial_t& trial);

	// copy stim parameters prior to drawing
	void copy_stim_params_to_spec(const struct conte_stim_params& params, ARConteSpec& spec);

	// setup page cycling for current trial
	void setup_cycling(const ConteXYHelper& xyhelper, const conte_trial_t& trial);

	// setup page cycling to make an orderly trial ending and fixpt
	void ConteUStim::setup_cycling_clear_fixpt();

};


#endif
