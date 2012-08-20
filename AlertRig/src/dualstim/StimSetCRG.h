#ifndef _STIMSETCRG_H_
#define _STIMSETCRG_H_

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
	static const string empty_sequence;
public:

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

	const string& get_current_sequence()
	{
		if (*m_index < (int)m_sequences.size() && *m_index > -1)
			return m_sequences.at(*m_index);
		else
			return StimSequenceList::empty_sequence;
	}
};


class StimSetCRG: public StimSetMultipleGrating, public StimSequenceList
{
protected:
	int m_contrast;
	ARGratingSpec m_grating1;		// this will be the base grating, with opposite contrast. 
	unsigned int m_fpt;
public:
	StimSetCRG(unsigned int ifpt, vector<string> sequences, vector<int> order): m_fpt(ifpt), StimSequenceList(sequences, order) {};
	virtual ~StimSetCRG() {};

	void set_initial_parameters();
	int setup_cycling(int firstpage, int stim1page, int stim0page, int lastpage);

	virtual int init(ARvsg& vsg);
	virtual int handle_trigger(std::string& s);
};

#endif
