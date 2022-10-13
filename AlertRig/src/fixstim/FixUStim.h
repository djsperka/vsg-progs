/* $Id: FixUStim.h,v 1.5 2016-05-05 18:25:16 devel Exp $*/
#ifndef _FIXUSTIM_H_
#define _FIXUSTIM_H_


#include "UStim.h"
#include "alertlib.h"
#include "AlertUtil.h"
#include "StimSet.h"
#include "EQStimSet.h"
#include "StimSetCRG.h"
#include "FGGXStimSet.h"
#include "AttentionStimSet.h"
#include "MultiParameterFXMultiGStimSet.h"
#include "BufferedAsyncSerial.h"
#include <vector>

template <class T> StimSet* create_stimset(bool bHaveFixpt, ARContrastFixationPointSpec& fixpt, bool bHaveXhair, ARXhairSpec& xhair, ARGratingSpec& grating, vector<double> params);
MultiParameterFXMultiGStimSet* create_multiparameter_stimset(bool bHaveFixpt, ARContrastFixationPointSpec& fixpt, bool bHaveXhair, ARXhairSpec& xhair);
MultiParameterFXMultiGStimSet* create_multiparameter_stimset(bool bHaveFixpt, ARContrastFixationPointSpec& fixpt, bool bHaveXhair, ARXhairSpec& xhair, ARGratingSpec& grating);


// for interacting with argp 
struct fixstim_arguments
{
	bool bBinaryTriggers;
	bool bVerbose;
	COLOR_TYPE bkgdColor;
	int iDistanceToScreenMM;
	bool bHaveDistance;
	int iReadyPulseDelay;
	int iPulseBits;
	bool bPresentOnTrigger;
	string sTriggeredTriggers;
	unsigned long ulTriggerArmed;
	ARContrastFixationPointSpec fixpt;
	bool bHaveFixpt;
	bool bHaveStim;
	ARGratingSpec grating;
	bool bHaveGrating;
	ARXhairSpec xhair;
	bool bHaveXhair;
	bool bDisableGammaCorrection;
	bool bDigoutTests;

	// These 'last_was' vars tell us how to assign a stim set when we receive one.
	bool bLastWasFixpt;
	bool bLastWasGrating;
	bool bLastWasDistractor;
	bool bUseCueCircles;
	bool bUseCuePoints;
	bool bCuePointIsDot;

	// tentative introduction to multiple page stimuli
	int nStimPages;

	// store all the parsed specs
	std::vector<alert::ARContrastFixationPointSpec> vecFixpts;
	std::vector<alert::ARGratingSpec> vecGratings;
	std::vector<alert::ARGratingSpec> vecDistractors;
	std::vector<AttentionCue> vecAttentionCues;
	FlashyParamVectorVector vecFlashies;
	std::vector<alert::ARContrastFixationPointSpec> vecDots;

	StimSet* pStimSet;
	bool bUsingMultiParameterStimSet;
	bool bHaveSequence;
	string sequence;

	string serial_port;
	bool bUsingSerial;

	fixstim_arguments()
		: bBinaryTriggers(true)
		, bVerbose(false)
		, bkgdColor(gray)
		, iDistanceToScreenMM(0)
		, bHaveDistance(false)
		, iReadyPulseDelay(0)
		, iPulseBits(0x40)
		, bPresentOnTrigger(false)
		, sTriggeredTriggers("")
		, ulTriggerArmed(0)
		, fixpt()
		, bHaveFixpt(false)
		, grating()
		, bHaveGrating(false)
		, xhair()
		, bHaveXhair(false)
		, bDisableGammaCorrection(false)
		, bHaveStim(false)
		, bLastWasFixpt(false)
		, bLastWasGrating(false)
		, bLastWasDistractor(false)
		, bUseCueCircles(false)
		, bUseCuePoints(false)
		, bCuePointIsDot(false)
		, nStimPages(1)
		, vecFixpts()
		, pStimSet(nullptr)
		, bUsingMultiParameterStimSet(false)
		, bHaveSequence(false)
		, sequence("")
		, serial_port("")
		, bUsingSerial(false)
	{};

};



// Implementation of UStim interface for the fixstim app. This embodies the 
// original fixstim behavior.

class FixUStim: public UStim
{
public:
	FixUStim();
	virtual ~FixUStim();

	bool parse(int argc, char **argv);
	void run_stim(alert::ARvsg& vsg);
	int process_arg(int option, std::string& arg);

	// This acts as the callback function for the triggers
	int callback(int &output, const FunctorCallbackTrigger* ptrig, const std::string&);

private:
	struct fixstim_arguments m_arguments;
	BufferedAsyncSerial m_serial_port;

// These are the args allowed and which are handled by prargs. Do not use 'F' - it is reserved for 
// passing a command file.
	static const string m_allowedArgs;

	void init_triggers(TSpecificFunctor<FixUStim>* pfunctor, int npages=1);
	bool parseImageArg(const std::string& arg, std::string& filename, double& x, double& y, double& duration, double& delay, int& nlevels);
};

#endif
