/* $Id: StarUStim.h,v 1.1 2016-04-01 22:26:34 devel Exp $*/
#ifndef _STARUSTIM_H_
#define _STARUSTIM_H_

#include "UStim.h"
#include "alertlib.h"
#include "AlertUtil.h"
#include <vector>

// Implementation of UStim interface for the starstim app.

class StarUStim: public UStim, public prargs_handler
{
public:
	StarUStim();
	virtual ~StarUStim();

	bool parse(int argc, char **argv);
	void run_stim(alert::ARvsg& vsg);
	int process_arg(int option, std::string& arg);

	// This acts as the callback function for the triggers
	int callback(int &output, const FunctorCallbackTrigger* ptrig);

private:
	bool m_binaryTriggers;
	bool m_verbose;
	COLOR_TYPE m_background;
	alert::ARContrastFixationPointSpec m_fixpt;
	vector<ARContrastFixationPointSpec*> m_targets;
	vector<int> m_vecTargetOrder;
	vector<int>::const_iterator m_iterator;
	int m_iDistanceToScreenMM;
	int m_pulse;
	bool m_quit;
	int m_errflg;

// These are the args allowed and which are handled by prargs. Do not use 'F' - it is reserved for 
// passing a command file.
	static const string m_allowedArgs;

	void update_page();
	void init_pages();
	void init_triggers(TSpecificFunctor<StarUStim>* pfunctor);
};

#endif