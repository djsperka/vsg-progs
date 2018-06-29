#ifndef _ATTENTIONSTIMSET_H_
#define _ATTENTIONSTIMSET_H_

#include "StimSet.h"
#include "AttentionCue.h"

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
int parse_attparams(const string& s, int nstim, vector<struct AttParams>& vecTrialParams, double& tMax);



typedef struct flashy_params
{
	int nk;	// distractor number to use, index [0,...,distractor.size-1]
	double x, y, w, h;
	//double ton, toff;
	int fon, foff;	// ton and toff converted to frames. 
} FlashyParams;

typedef vector<struct flashy_params> FlashyParamVector;
typedef vector< FlashyParamVector > FlashyParamVectorVector;

// helper function for loading params from a comma-separated string
int parse_flashyparams(const string& s, FlashyParamVectorVector& vecFlashyParamVector);
void dump_flashyparams(const FlashyParamVectorVector& vecFlashyParamVector);
int checkFlashyTimes(const vector<AttParams>& vecInput, const FlashyParamVectorVector& vecFlashies, double tMax);

struct InterleavedParams
{
	COLOR_TYPE color;
	double dInitialPhase;
	double times[5];
	std::vector<std::pair<int, int> > contrastPairs;
	int iOffBits;
};

int parse_interleaved_params(const string& s, int nstim, vector<InterleavedParams>& params);
void my_print_interleaved_trials(const vector<InterleavedParams>& trials);

int parse_contrast_params(const string& s, int nstim);


// note = WORD is unsigned short. The cast here makes all conversions positive numbers. Bad when using as a test.
//#define SECONDS_TO_FRAMES(t) ((WORD)((t) * 1000000.0 /vsgGetSystemAttribute(vsgFRAMETIME)))
#define SECONDS_TO_FRAMES(t) (int)((t) * 1000000.0 /vsgGetSystemAttribute(vsgFRAMETIME))
#define FRAMES_TO_SECONDS(n) ((n) * vsgGetSystemAttribute(vsgFRAMETIME)/1000000.0)

class AttentionStimSet: public StimSet
{
public:
	AttentionStimSet(ARContrastFixationPointSpec& fixpt, double tMax, vector<alert::ARGratingSpec>& vecGratings, vector<AttParams>& params);
	AttentionStimSet(ARContrastFixationPointSpec& fixpt, double tMax, vector<alert::ARGratingSpec>& vecGratings, vector<AttentionCue>& vecCuePairs, bool bCueCircles, bool bCuePoints, vector<AttParams>& params);
	AttentionStimSet(ARContrastFixationPointSpec& fixpt, double tMax, vector<alert::ARGratingSpec>& vecGratings, vector<AttParams>& params, vector<alert::ARGratingSpec>& vecDistractors, FlashyParamVectorVector& vecFlashies);
	AttentionStimSet(ARContrastFixationPointSpec& fixpt, double tMax, vector<alert::ARGratingSpec>& vecGratings, vector<AttentionCue>& vecCuePairs, bool bCueCircles, bool bCuePoints, vector<AttParams>& params, vector<alert::ARGratingSpec>& vecDistractors, FlashyParamVectorVector& vecFlashies);
	AttentionStimSet(ARContrastFixationPointSpec& fixpt, vector<alert::ARGratingSpec>& vecGratings, vector<AttentionCue>& vecCuePairs, bool bCueCircles, bool bCuePoints, vector<InterleavedParams>& params);

	bool setFlashies(const vector<alert::ARGratingSpec>& vecGratings, const FlashyParamVectorVector& vecFlashies);
	virtual int num_pages();
	virtual int num_overlay_pages() {return 0;};
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int drawCurrent();
	int drawCurrentAttParams();
	int drawCurrentInterleaved();
	void draw_stim_gratings(bool bIsCC, int iOffBits, double dInitialPhase, const std::vector<std::pair<int, int> >& contrastPairs);
	void draw_cues(int offBits);
	void draw_cue_points(int offBits);
	void draw_fixpt();
	void draw_flashy(const FlashyParams& params);
	alert::ARContrastFixationPointSpec m_fixpt;
	double m_tMax;
	bool m_bUseCueCircles;
	bool m_bUseCuePoints;
	vector<alert::ARGratingSpec> m_vecGratings;
	vector<alert::ARGratingSpec> m_vecGratingsCC;
	vector<struct AttParams> m_vecParams;
	vector<alert::ARContrastCircleSpec> m_vecCues;
	vector<alert::ARContrastFixationPointSpec> m_vecCuePoints;
	vector<alert::ARContrastRectangleSpec> m_vecCueRects;
	FlashyParamVectorVector  m_vecFlashies;		// one vector element per trial
	vector<alert::ARGratingSpec> m_vecDistractors;
	vector<InterleavedParams> m_vecInterleaved;
	unsigned int m_current;
	int m_pageBlank;
	int m_pageFixpt;
	int m_pageStim;
	int m_pageChg;	// after CC
	int m_pageD;
	int m_pageFixptQ;
	int m_pageQStim;
	int m_pageQStimCC;
};

#endif