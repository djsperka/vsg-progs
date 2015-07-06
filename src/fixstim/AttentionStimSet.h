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

typedef struct attention_cue
{
	double rdiff;
	int linewidth;
	COLOR_TYPE color;
} AttentionCue;

int parse_attparams(const string& s, int nstim, vector<struct AttParams>& vecTrialParams, double& tMax);
int parse_attcues(const string& s, int nstim, vector<AttentionCue>& vecCues);
void dump_attcues(const vector<AttentionCue>& vecCues);


typedef struct flashy_params
{
	int nk;	// distractor number to use, index [0,...,distractor.size-1]
	double x, y, w, h;
	double ton, toff;
} FlashyParams;

typedef vector<struct flashy_params> FlashyParamVector;
typedef vector< FlashyParamVector > FlashyParamVectorVector;

// helper function for loading params from a comma-separated string
int parse_flashyparams(const string& s, FlashyParamVectorVector& vecFlashyParamVector);
void dump_flashyparams(const FlashyParamVectorVector& vecFlashyParamVector);
int checkFlashyTimes(const vector<AttParams>& vecInput, const FlashyParamVectorVector& vecFlashies, double tMax);

// note = WORD is unsigned short. The cast here makes all conversions positive numbers. Bad when using as a test.
//#define SECONDS_TO_FRAMES(t) ((WORD)((t) * 1000000.0 /vsgGetSystemAttribute(vsgFRAMETIME)))
#define SECONDS_TO_FRAMES(t) ((t) * 1000000.0 /vsgGetSystemAttribute(vsgFRAMETIME))


class AttentionStimSet: public StimSet
{
public:
	AttentionStimSet(ARContrastFixationPointSpec& fixpt, double tMax, vector<alert::ARGratingSpec>& vecGratings, vector<AttParams>& params);
	AttentionStimSet(ARContrastFixationPointSpec& fixpt, double tMax, vector<alert::ARGratingSpec>& vecGratings, vector<AttentionCue>& vecCuePairs, vector<AttParams>& params);
	AttentionStimSet(ARContrastFixationPointSpec& fixpt, double tMax, vector<alert::ARGratingSpec>& vecGratings, vector<AttParams>& params, vector<alert::ARGratingSpec>& vecDistractors, FlashyParamVectorVector& vecFlashies);
	AttentionStimSet(ARContrastFixationPointSpec& fixpt, double tMax, vector<alert::ARGratingSpec>& vecGratings, vector<AttentionCue>& vecCuePairs, vector<AttParams>& params, vector<alert::ARGratingSpec>& vecDistractors, FlashyParamVectorVector& vecFlashies);
	bool setFlashies(const vector<alert::ARGratingSpec>& vecGratings, const FlashyParamVectorVector& vecFlashies);
	virtual int num_pages() {return 4 + m_vecDistractors.size();};
	virtual int num_overlay_pages() {return 0;};
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int drawCurrent();
	void draw_stim_gratings(bool bIsCC, const struct AttParams& params);
	void draw_cues(const struct AttParams& params);
	void draw_fixpt();
	void draw_flashy(const FlashyParams& params);
	alert::ARContrastFixationPointSpec m_fixpt;
	double m_tMax;
	vector<alert::ARGratingSpec> m_vecGratings;
	vector<alert::ARGratingSpec> m_vecGratingsCC;
	vector<struct AttParams> m_vecParams;
	vector<alert::ARContrastCircleSpec> m_vecCues;
	FlashyParamVectorVector  m_vecFlashies;		// one vector element per trial
	vector<alert::ARGratingSpec> m_vecDistractors;
	unsigned int m_current;
	int m_pageBlank;
	int m_pageFixpt;
	int m_pageStim;
	int m_pageChg;
	int m_pageD;
};

#endif