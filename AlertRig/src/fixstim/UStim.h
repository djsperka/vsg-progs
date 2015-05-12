/* $Id: UStim.h,v 1.1 2015-05-12 17:27:01 devel Exp $ */

#ifndef _USTIM_H_
#define _USTIM_H_

#include <string>
#include "alertlib.h"

class UStim
{
public:
	UStim(): m_quit(false) {};
	virtual ~UStim() {};	// may want to destroy all vsg objects here?

	// parse input string. return true if OK, false if not (and stim should fail to run).
	virtual bool parses(const std::string& s);
	virtual bool parse(int argc, char ** argv) = 0;

	// run stimulus. Should initialize and run loop here.
	virtual void run_stim(alert::ARvsg& vsg) = 0;

	// call to set quit enabled. 
	// subclasses should check the value of quitEnabled() and 
	// react accordingly.
	void stop_stim() { m_quit = true; };
	bool quit_enabled() const { return m_quit; };

	static int initialize(alert::ARvsg& vsg, int iScreenDistanceMM, const COLOR_TYPE& background);

private:
	bool m_quit;
	alert::TriggerVector m_triggers;
protected:
	alert::TriggerVector& triggers() { return m_triggers; };
};

#endif