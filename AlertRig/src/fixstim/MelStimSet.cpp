#include "MelStimSet.h"

int MelStimSet::init(std::vector<int> pages)
{
	return 0;
}

// handle the trigger indicated by the string s. Do not call vsgPresent! return value of 
// 1 means vsgPresent() will be called. 
int MelStimSet::handle_trigger(std::string& s)
{
	return 0;
}

std::string MelStimSet::toString() const
{
	return std::string("MelStimSet: toString() not implemented.");
}
