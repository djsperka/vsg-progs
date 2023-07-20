#ifndef _CONTEUSTIM_H_
#define _CONTEUSTIM_H_

#include "UStim.h"
#include "alertlib.h"
#include "AlertUtil.h"
#include <vector>
#include <algorithm>
#include <boost/tuple/tuple.hpp>


class ConteCueDotSupply
{
	unsigned int m_blocksize;	// number of dots per patch
	std::vector<double> m_xy;
public:
	ConteCueDotSupply() : m_blocksize(0) {};
	virtual ~ConteCueDotSupply() {};

	// only when loading
	void set_blocksize(unsigned int n) { m_blocksize = n; };
	std::vector<double>& xy() { return m_xy; };

	// safe for use during trials
	const double& x(unsigned int block, unsigned int index) const;
	const double& y(unsigned int block, unsigned int index) const;
	unsigned int blocksize() const { return m_blocksize; };
	unsigned int nblocks() const { if (m_blocksize) return (unsigned int)m_xy.size() / m_blocksize; else return 0; }
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
struct conte_stim_spec
{
	double x, y, w, h, ori, sf, phase, dev;
	unsigned int lwt;
	COLOR_TYPE color;
};

// a single trial specified here
struct conte_trial_spec
{
	double cue_x, cue_y, cue_w, cue_h;
	double cue_prob_a;
	COLOR_TYPE cue_color_a, cue_color_b;
	struct conte_stim_spec s0, s1, t0, t1;	// sample and target stim
	unsigned int cue_fpt;		// frames per term
	unsigned int cue_nterms;	// will be specified as duration, not terms, probably
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
	static DWORD cOvPageBkgd;
	static DWORD cOvPageAperture;
	static DWORD cOvPageClear;
	static DWORD cPageCue;
	static DWORD cPageProbe;
	static DWORD cPageTest;

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
	void draw_dot_patches();

	// draw a single 3-panel stim thingy
	void draw_conte_stim(const struct conte_stim_spec& stim);
};

#endif
