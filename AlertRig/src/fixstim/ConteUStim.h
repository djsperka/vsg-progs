#ifndef _CONTEUSTIM_H_
#define _CONTEUSTIM_H_

#include "UStim.h"
#include "alertlib.h"
#include "AlertUtil.h"
#include <vector>
#include <algorithm>
#include <boost/tuple/tuple.hpp>

class ContePatch
{
	unsigned int m_n0, m_n1;
	vector<double> m_x;
	vector<double> m_y;
public:
	ContePatch(unsigned int n0, unsigned int n1, double* p);
	virtual ~ContePatch() {};
	void draw(PIXEL_LEVEL level0, PIXEL_LEVEL level1, double diameter) const;
};

class ConteCueDotSupply
{
	std::vector<ContePatch> m_patches;
public:
	ConteCueDotSupply() {};
	virtual ~ConteCueDotSupply() {};

	// use while loading
	void add_patch(unsigned int n0, unsigned int n1, double* p) { m_patches.push_back(ContePatch(n0, n1, p)); };

	size_t npatches() const { return m_patches.size(); };
	const ContePatch& patch(unsigned int i) const { return m_patches[i]; };
};

// for interacting with argp 
struct conte_arguments
{
	bool bBinaryTriggers;
	bool bVerbose;
	COLOR_TYPE bkgdColor;
	int iDistanceToScreenMM;
	bool bHaveDistance;
	int iReadyPulseDelay;
	int iPulseBits;
	string dot_supply_filename;
	ConteCueDotSupply dot_supply;

	conte_arguments()
		: bBinaryTriggers(true)
		, bVerbose(false)
		, bkgdColor(gray)
		, iDistanceToScreenMM(0)
		, bHaveDistance(false)
		, iReadyPulseDelay(0)
		, iPulseBits(0x2)
	{};
};


// one of the 3-panel stim + distractor patches
struct conte_stim_params
{
	double x, y, w, h, ori, sf, phase, dev;
	unsigned int lwt;
	unsigned int icolor;	// refers to cue_color_0 or cue_color_1, so should be 0|1
};

// a single trial specified here
struct conte_trial_spec
{
	double cue_x, cue_y, cue_w, cue_h, cue_d;	// d is dot diam, w,h are patch width, height
	COLOR_TYPE cue_color_0, cue_color_1;
	struct conte_stim_params s0, s1, t0, t1;	// sample and target stim
	unsigned int cue_fpt;		// frames per term
	unsigned int cue_nterms;	// will be specified as duration, not terms, probably
};

class ConteXYHelper
{
	long m_WpixScr, m_HpixScr;	// screen width, height
	long m_WpixZone, m_HpixZone;	// zone w, h
	unsigned int m_nPatchPerRow;
	unsigned int m_nPatchRows;
	double m_wdeg, m_hdeg, m_ddeg, m_xdeg, m_ydeg;		// patch w,h; dot diam; patch screen position; all in degrees, origin in center, Ypos up
public:
	ConteXYHelper(double wdeg, double hdeg, double ddeg, double xdeg, double ydeg);
	virtual ~ConteXYHelper() {};

	// get drawing origin for patch 'i', values returned in degrees, use with setDrawOrigin
	void getDrawOrigin(unsigned int i, double& x_origin_deg, double& y_origin_deg) const;

	// get xy position for page, as used in cycling setup. Returned values in PIXELS, assign to .Xpos, .Ypos
	void getPageXYpos(unsigned int i, short& Xpos_pix, short& Ypos_pix) const;
};







// Implementation of UStim interface for the starstim app.
class ConteUStim: public UStim
{
public:
	ConteUStim();
	virtual ~ConteUStim();

	bool parse(int argc, char **argv);
	void run_stim(alert::ARvsg& vsg);
	int process_arg(int option, std::string& arg);

	// This acts as the callback function for the triggers
	int callback(int &output, const FunctorCallbackTrigger* ptrig, const std::string&);

private:
	struct conte_arguments m_arguments;

	bool m_quit;
	int m_errflg;

	PIXEL_LEVEL m_levelOverlayBackground;
	PIXEL_LEVEL m_levelColorA;
	PIXEL_LEVEL m_levelColorB;
	PIXEL_LEVEL m_levelTest;
	static WORD cOvPageBkgd;
	static WORD cOvPageAperture;
	static WORD cPageCue;
	static WORD cPageProbe;
	static WORD cPageTest;

	// These are the args allowed and which are handled by prargs. Do not use 'F' - it is reserved for 
	// passing a command file.
	static const string m_allowedArgs;

	// draw current trial into video memory
	void draw_current();

	// initialize pages at start of this stim
	void init();

	// cleanup anything from init() that needs cleaning.
	void cleanup();

	// setup triggers
	void init_triggers(TSpecificFunctor<ConteUStim>* pfunctor);

	// draw dot patches on single page
	void draw_dot_patches(const ConteXYHelper& xyhelper);

	// draw a single 3-panel stim thingy
	void draw_conte_stim(const struct conte_stim_spec& stim);

	// setup page cycling for current trial
	void setup_cycling(const ConteXYHelper& xyhelper);
};


#endif
