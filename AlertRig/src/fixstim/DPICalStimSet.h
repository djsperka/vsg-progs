#pragma once
#include "StimSet.h"
#include "alertlib.h"

class DPICalStimSet : public FXStimSet
{
private:
	int m_pages[2];
	size_t m_ipage;
	void drawCurrent();
public:
	DPICalStimSet() : FXStimSet() {};
	DPICalStimSet(alert::ARContrastFixationPointSpec& fixpt) : FXStimSet(fixpt) {};
	int num_pages() { return 2; };
	int num_overlay_pages() { return 0; };
	int init(std::vector<int> pages, int num_stim_pages);
	void cleanup(std::vector<int> pages);
	int handle_trigger(const std::string& s, const std::string& args = std::string());
	std::string toString() const;
};






