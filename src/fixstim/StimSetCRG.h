#ifndef _STIMSETCRG_H_
#define _STIMSETCRG_H_

#include "StimSet.h"
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

	int index() const 
	{
		return *m_index;
	}
};


class StimSetCRG: public FXMultiGStimSet, public StimSequenceList
{
protected:
//	int m_contrast;
//	ARGratingSpec m_grating0;		// this will be the base grating, -1*contrast. 
//	ARGratingSpec m_grating1;		// this will be the base grating. 
	unsigned int m_fpt;
	int m_page_f, m_page_1, m_page_0, m_page_x;

	void initialize(ARGratingSpec& g);
public:
	StimSetCRG(ARContrastFixationPointSpec& f, ARXhairSpec& h, ARGratingSpec& g, unsigned int ifpt, vector<string> sequences, vector<int> order) 
		: FXMultiGStimSet(f, h), StimSequenceList(sequences, order), m_fpt(ifpt) {initialize(g);};
	StimSetCRG(ARContrastFixationPointSpec& f, ARGratingSpec& g, unsigned int ifpt, vector<string> sequences, vector<int> order) 
		: FXMultiGStimSet(f), StimSequenceList(sequences, order), m_fpt(ifpt) {initialize(g);};
	StimSetCRG(ARGratingSpec& g, unsigned int ifpt, vector<string> sequences, vector<int> order) 
		: FXMultiGStimSet(), StimSequenceList(sequences, order), m_fpt(ifpt) {initialize(g);};

	virtual ~StimSetCRG() {};

	void set_initial_parameters();
	int setup_cycling(int firstpage, int stim1page, int stim0page, int lastpage);

	virtual int num_pages() { return 4; };
	virtual int num_overlay_pages() { return 0; };

	//virtual int init(ARvsg& vsg);
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const { return std::string("StimSetCRG - toString() N.I."); };
};


#endif
