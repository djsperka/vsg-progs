#ifndef _TESTUSTIM_H_
#define _TESTUSTIM_H_

#include "UStim.h"
#include "alertlib.h"
#include "AlertUtil.h"
#include <vector>
#include <algorithm>
#include <boost/tuple/tuple.hpp>

// Implementation of UStim interface for the starstim app.

class TestUStim: public UStim, public prargs_handler
{
public:
	TestUStim();
	virtual ~TestUStim();

	bool parse(int argc, char **argv);
	void run_stim(alert::ARvsg& vsg);
	int process_arg(int option, std::string& arg);

	// This acts as the callback function for the triggers
	int callback(int &output, const FunctorCallbackTrigger* ptrig, const std::string&);

private:
	bool m_binaryTriggers;
	bool m_verbose;
	COLOR_TYPE m_background;
	alert::ARContrastFixationPointSpec m_fixpt;
	vector<ARGratingSpec*> m_gratings;

	int m_iDistanceToScreenMM;
	int m_pulse;
	bool m_quit;
	int m_errflg;
	int m_currentPage;
	int m_fixptStimPage;
	int m_fixptPage;

// These are the args allowed and which are handled by prargs. Do not use 'F' - it is reserved for 
// passing a command file.
	static const string m_allowedArgs;

	void draw_current();
	void init_pages();
	void init_triggers(TSpecificFunctor<TestUStim>* pfunctor);
};

#endif
