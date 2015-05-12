#ifndef _ATTENTIONSTIMSET_H_
#define _ATTENTIONSTIMSET_H_

#include "StimSet.h"

// struct for holding parameters of a single attention trial
#define ATTPARAMS_MAX_CONTRASTS 16
struct AttParams
{
	COLOR_TYPE color;
	double dInitialPhase;
	double dTimeToCC;
	std::vector<std::pair<int, int> > contrastPairs;
	int iOffBits;
};

typedef std::pair<double, COLOR_TYPE> AttentionCue;

// helper function for loading params from a comma-separated string
int parse_attparams(const string& s, int nstim, vector<struct AttParams>& vecTrialParams, double& tMax);
int parse_attcues(const string& s, int nstim, vector<AttentionCue>& vecCues);

class AttentionStimSet: public StimSet
{
public:
	AttentionStimSet(ARContrastFixationPointSpec& fixpt, double tMax, vector<alert::ARGratingSpec>& vecGratings, vector<AttParams>& params);
	AttentionStimSet(ARContrastFixationPointSpec& fixpt, double tMax, vector<alert::ARGratingSpec>& vecGratings, vector<AttentionCue>& vecCuePairs, vector<AttParams>& params);
	virtual int num_pages() {return 4;};
	virtual int num_overlay_pages() {return 0;};
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int drawCurrent();
	alert::ARContrastFixationPointSpec m_fixpt;
	double m_tMax;
	vector<alert::ARGratingSpec> m_vecGratings;
	vector<alert::ARGratingSpec> m_vecGratingsCC;
	vector<struct AttParams> m_vecParams;
	vector<alert::ARContrastCircleSpec> m_vecCues;
	unsigned int m_current;
	int m_pageBlank;
	int m_pageFixpt;
	int m_pageStim;
	int m_pageChg;
};

#endif