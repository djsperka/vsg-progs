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
#include <vector>
#include <boost/filesystem.hpp>

// Implementation of UStim interface for the fixstim app. This embodies the 
// original fixstim behavior.

class FixUStim: public UStim, public prargs_handler
{
public:
	FixUStim(bool bStandAlone = false);
	virtual ~FixUStim();

	bool parse(int argc, char **argv);
	void run_stim(alert::ARvsg& vsg);
	int process_arg(int option, std::string& arg);

	// This acts as the callback function for the triggers
	int callback(int &output, const FunctorCallbackTrigger* ptrig);

private:
	bool m_bStandAlone;
	bool m_binaryTriggers;
	bool m_verbose;
	bool m_dumpStimSetsOnly;
	bool m_bPresentOnTrigger;
	std::string m_sTriggeredTriggers;
	DWORD m_ulTriggerArmed;
	bool m_bUseLock;
	COLOR_TYPE m_background;
	alert::ARContrastFixationPointSpec m_fixpt;
	std::vector<alert::ARContrastFixationPointSpec> m_vecFixpts;
	alert::ARGratingSpec m_grating;
	alert::ARXhairSpec m_xhair;
	std::vector<alert::ARGratingSpec> m_vecGratings;
	std::vector<alert::ARGratingSpec> m_vecDistractors;
	std::vector<AttentionCue> m_vecAttentionCues;
	FlashyParamVectorVector m_vecFlashies;
	StimSet *m_pStimSet;
	bool m_bUsingMultiParameterStimSet;
	int m_iDistanceToScreenMM;
	int m_iReadyPulseDelay;
	alert::TriggerVector m_triggers;
	int m_pulse;
	bool m_bDaemon;
	bool m_bClient;
	std::string m_sClientArgs;
	bool m_bClientSendQuit;
	int m_iDaemonPort;
	string m_sDaemonHostIP;
	bool m_quit;
	double m_dSlaveXOffset;
	double m_dSlaveYOffset;
	boost::filesystem::path m_pathCues;			// root path for cue files (see EQStimSet, 'W' arg)
	int m_errflg;

// These are the args allowed and which are handled by prargs. Do not use 'F' - it is reserved for 
// passing a command file.
	static const string m_allowedArgs;


	template <class T> StimSet* create_stimset(bool bHaveFixpt, ARContrastFixationPointSpec& fixpt, bool bHaveXhair, ARXhairSpec& xhair, ARGratingSpec& grating, vector<double> params);
	MultiParameterFXMultiGStimSet* create_multiparameter_stimset(bool bHaveFixpt, ARContrastFixationPointSpec& fixpt, bool bHaveXhair, ARXhairSpec& xhair);
	MultiParameterFXMultiGStimSet* create_multiparameter_stimset(bool bHaveFixpt, ARContrastFixationPointSpec& fixpt, bool bHaveXhair, ARXhairSpec& xhair, ARGratingSpec& grating);
	void init_triggers(TSpecificFunctor<FixUStim>* pfunctor);
	bool parseImageArg(const std::string& arg, std::string& filename, double& x, double& y, double& duration, int& low_water, int& hi_water);

};

#endif
