#include "DualStimSet.h"
#include <string>
#include <vector>

using namespace std;
using namespace alert;



class StimSequenceList
{
private:
	vector<int>::const_iterator m_index;
	vector<string> m_sequences;
	vector<int> m_order;
	StimSequenceList() {};
public:
#if 0
	// WANT TO FORCE ORDER AS AN ARGUMENT TO AVOID ERROR IN OMITTING IT!
	StimSequenceList(vector<string> sequences): m_sequences(sequences) 
	{ 
		// init order - use 0,1,2,3,... 
		int i;
		m_order.clear();
		for (i=0; i<m_sequences.size(); i++) m_order.push_back(i);
		m_index = m_order.begin();
	}
#endif

	StimSequenceList(vector<string> sequences, vector<int> order): m_sequences(sequences), m_order(order) 
	{ 
		m_index = m_order.begin();
	}

	virtual ~StimSequenceList() {};
	void advance()
	{
		m_index++;
		if (m_index == m_order.end()) m_index = m_order.begin();
		return;
	}

	string get_current_sequence()
	{
		return m_sequences[*m_index];
	}
};


class StimSetCRG: public StimSetBase, public StimSequenceList
{
private:
	int m_contrast;
	ARGratingSpec m_grating0;		// this will be the base grating, with opposite contrast. 
	unsigned int m_fpt;
public:
	StimSetCRG(unsigned int ifpt, vector<string> sequences, vector<int> order): m_fpt(ifpt), StimSequenceList(sequences, order) {};
	virtual ~StimSetCRG() {};

	void set_initial_parameters();
	int setup_cycling(int firstpage, int stim1page, int stim0page, int lastpage);

	virtual int init(ARvsg& vsg);
	virtual int handle_trigger(std::string& s);
};

