#ifndef _BARUSTIM_H_
#define _BARUSTIM_H_

#include "UStim.h"
#include "alertlib.h"
#include "AlertUtil.h"
#include <vector>
#include <algorithm>
#include <boost/tuple/tuple.hpp>

// Implementation of UStim interface for the starstim app.

class BarUStim: public UStim, public prargs_handler
{
public:
	BarUStim();
	virtual ~BarUStim();

	bool parse(int argc, char **argv);
	void run_stim(alert::ARvsg& vsg);
	int process_arg(int option, std::string& arg);

	// This acts as the callback function for the triggers
	int callback(int &output, const FunctorCallbackTrigger* ptrig);

	enum class TargetType { fixpt, rectangle, grating };

private:
	bool m_binaryTriggers;
	bool m_verbose;
	COLOR_TYPE m_background;
	alert::ARContrastFixationPointSpec m_fixpt;
	vector<ARContrastFixationPointSpec*> m_dots;
	vector<ARGratingSpec*> m_gratings;
	vector<ARContrastRectangleSpec*> m_rectangles;
	//vector< pair<bool, unsigned int> > m_targets;
	vector< boost::tuple<TargetType, unsigned int, int> > m_targets;		//  < fixpt/rect/grating, index in m_gratings/m_fixpt, contrast in spec (gratings only) > 

	vector<int> m_vecTargetOrder;
	vector<int>::const_iterator m_iterator;
	int m_iDistanceToScreenMM;
	int m_pulse;
	bool m_quit;
	int m_errflg;
	int m_currentPage;
	ARGratingSpec* m_pBackgroundGrating;

// These are the args allowed and which are handled by prargs. Do not use 'F' - it is reserved for 
// passing a command file.
	static const string m_allowedArgs;

	void update_page();
	void init_pages();
	void init_triggers(TSpecificFunctor<BarUStim>* pfunctor);
};

#endif
