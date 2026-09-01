#pragma once
#include "Alertlib.h"
#include "StimSet.h"
#include <vector>
#include <istream>
#include <ostream>
#include <string>


// Specification of a single frame (a trial consists of a series of frames)
struct msac_frame
{
	std::vector<alert::ARGratingSpec> gratings;
};
typedef struct msac_frame msac_frame_t;

// a single trial specified here
struct msac_trial
{
	std::vector<msac_frame_t> frames;
};
typedef struct msac_trial msac_trial_t;

typedef std::vector<msac_trial_t> msac_trial_list_t;

std::istream& operator>>(std::istream& ins, msac_trial_list_t& trials);
std::istream& operator>>(std::istream& ins, msac_trial_t& trial);
std::istream& operator>>(std::istream& in, msac_frame_t& stim);

std::ostream& operator<<(std::ostream& ins, const msac_frame_t& frame);
std::ostream& operator<<(std::ostream& ins, const msac_trial_t& trial);

// parse trials file
bool parse_msac_trials_file(const std::string& filename, msac_trial_list_t& trials);



class MultiSacStimSet : public FXMultiGStimSet
{
private:
	ARContrastFixationPointSpec m_fixpt;
	msac_trial_list_t m_trials;
	int m_current;

	int drawCurrent();

public:

	MultiSacStimSet(ARContrastFixationPointSpec& fixpt, msac_trial_list_t& trials) : FXMultiGStimSet(fixpt), m_trials(trials) {};

	virtual ~MultiSacStimSet() {};

	// subclasses should return the number of pages they will need.
	virtual int num_pages();

	// subclasses should return the number of pages they will need.
	virtual int num_overlay_pages() {
		return 0;
	};

	virtual int init(std::vector<int> pages, int);

	// clean up any messes created in init() - esp settings in VSG
	virtual void cleanup(std::vector<int> pages) {};

	// handle the trigger indicated by the string s. Do not call vsgPresent! return value of 
	// 1 means vsgPresent() will be called. 
	virtual int handle_trigger(const std::string& s, const std::string&);

	virtual std::string toString() const;

};

// parse the trials file, create stim set. REturn NULL on failure.
MultiSacStimSet* createMultiSacStimSet(const std::string& filename, const ARContrastFixationPointSpec& fixpt);
