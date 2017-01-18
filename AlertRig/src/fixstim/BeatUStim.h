#pragma once
#ifndef _BEATUSTIM_H_
#define _BEATUSTIM_H_

#include "UStim.h"
#include "alertlib.h"
#include "AlertUtil.h"
#include <vector>

// Implementation of UStim interface for the starstim app.

class BeatUStim : public UStim, public prargs_handler
{
public:
	BeatUStim();
	virtual ~BeatUStim();

	bool parse(int argc, char **argv);
	void run_stim(alert::ARvsg& vsg);
	int process_arg(int option, std::string& arg);

	// This acts as the callback function for the triggers
	int callback(int &output, const FunctorCallbackTrigger* ptrig);

private:
	bool m_binaryTriggers;
	bool m_verbose;
	COLOR_TYPE m_background;
	int m_pulse;
	bool m_quit;
	int m_errflg;
	unsigned long m_initial;	/// number of pulses at start of period
	unsigned long m_gap;		/// number of frames in a gap between pulses
	unsigned long m_period;		/// number of frames in a period

	// These are the args allowed and which are handled by prargs. Do not use 'F' - it is reserved for 
	// passing a command file.
	static const string m_allowedArgs;
	void init_triggers(TSpecificFunctor<BeatUStim>* pfunctor);
};

#endif
