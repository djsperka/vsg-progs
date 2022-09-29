#pragma once
#include "StimSet.h"

class CycleTestStimSet : public FXGStimSet
{
public:
	CycleTestStimSet() {};
	virtual int num_pages() { return 4; };
	virtual int num_overlay_pages() { return 0; };
	virtual int init(ARvsg& vsg, std::vector<int> pages, int);
	virtual int handle_trigger(const std::string& s, const std::string&);
	virtual std::string toString() const {
		return std::string("CycleTestStimSet NI");
	}
private:
	int m_page_a, m_page_b, m_page_c, m_page_d;
};