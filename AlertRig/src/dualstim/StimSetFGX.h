#include "DualStimSet.h"
#include "StimParameterList.h"
#include <string>
#include <boost/ptr_container/ptr_vector.hpp>

using namespace std;
using namespace alert;
using namespace boost;

class StimSetFGX: public StimSetBase, public ptr_vector<StimParameterList>
{
private:
	int m_contrast;
public:
	StimSetFGX() {};
	virtual ~StimSetFGX() {};

	void advance();
	void set_initial_parameters();
	int setup_cycling(int firstpage, int stimpage, int lastpage);
	int setup_fixpt_cycling(int firstpage, int fixptpage);

	virtual int init(ARvsg& vsg);
	virtual int handle_trigger(std::string& s);
};



