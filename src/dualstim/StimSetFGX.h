#ifndef _STIMSETFGX_H_
#define _STIMSETFGX_H_

#include "DualStimSet.h"
#include "StimParameterList.h"
#include <string>
#include <boost/ptr_container/ptr_vector.hpp>

using namespace std;
using namespace alert;
using namespace boost;

class StimSetFGX: public StimSetSingleGrating, public ptr_vector<StimParameterList>
{
private:
	int m_contrast;

	void advance();
	void set_initial_parameters();
	int setup_cycling(int firstpage, int stimpage, int lastpage);
	int setup_fixpt_cycling(int firstpage, int fixptpage);

public:
	StimSetFGX() {};
	virtual ~StimSetFGX() {};

	virtual int init(ARvsg& vsg);
	virtual int handle_trigger(std::string& s);
};



#endif
