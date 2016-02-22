#if !defined(_EQPARAMS_H_)
#define _EQPARAMS_H_

#include "StimSet.h"
#include <string>

class ClientNetwork;

struct EQParams
{
	std::string cueFile;
	double tQ0, tQ1;	// cue on/off
	double tS;			// Stim on
	double tCC;			// contrast change time
	double tE;			// stim off
	int iAL, iCR;	// attend location, correct response (unused here)
	std::vector<std::pair<int, int> > contrastPairs;
	ARContrastFixationPointSpec *fixpt;
	ARGratingSpec *g0;
	ARGratingSpec *g1;
};

int parse_eqparams(const string& s, int nstim, struct EQParams& params);

class EQStimSet: public FXMultiGStimSet
{
public:
	EQStimSet(ARContrastFixationPointSpec& fixpt, vector<alert::ARGratingSpec>& vecGratings, std::vector<EQParams>& params, const char *pIPPort);
	virtual ~EQStimSet();
	virtual int num_pages() {return 7;};
	virtual int num_overlay_pages() {return 0;};
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int drawCurrent();
	vector<struct EQParams> m_vecParams;
	unsigned int m_current;
	int m_pageBlank;
	int m_pageFixpt;
	int m_pageFixptCue;
	int m_pageFixptCueStim;
	int m_pageFixptCueStimCC;
	int m_pageFixptStim;
	int m_pageFixptStimCC;
	PIXEL_LEVEL m_levelImage;
	ClientNetwork *m_pEEGClient;
	bool m_bEEGClient;
};

#endif