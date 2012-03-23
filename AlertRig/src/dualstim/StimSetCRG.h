#include "DualStimSet.h"
#include <string>
#include <vector>

using namespace std;
using namespace alert;



class StimSequenceList : public vector<string>
{
private:
	vector<string>::const_iterator m_iter;
	StimSequenceList() {};
public:
	StimSequenceList(vector<string> sequences): vector<string>(sequences) { m_iter = begin(); };
	virtual ~StimSequenceList() {};
	void advance()
	{
		m_iter++;
		if (m_iter == end()) m_iter = begin();
		return;
	}

	string get_current_sequence()
	{
		return *m_iter;
	}
};


class StimSetCRG: public StimSetBase, public StimSequenceList
{
private:
	int m_contrast;
	ARGratingSpec m_grating0;		// this will be the base grating, with opposite contrast. 
	unsigned int m_fpt;
public:
	StimSetCRG(unsigned int ifpt, vector<string> sequences): m_fpt(ifpt), StimSequenceList(sequences) {};
	virtual ~StimSetCRG() {};

	void set_initial_parameters();
	int setup_cycling(int firstpage, int stim1page, int stim0page, int lastpage);

	virtual int init(ARvsg& vsg);
	virtual int handle_trigger(std::string& s);
};

