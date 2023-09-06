#ifndef _CONTEUSTIM_H_
#define _CONTEUSTIM_H_

#include "UStim.h"
#include "alertlib.h"
#include "AlertUtil.h"
#include <vector>
#include <algorithm>
#include <boost/tuple/tuple.hpp>

// A Patch is a set of dots, two colors. A series of these patches is used to make up the Cue in the Conte stimulus. 
class ContePatch
{
	unsigned int m_n0, m_n1;
	vector<double> m_x;
	vector<double> m_y;
public:
	ContePatch(unsigned int n0, unsigned int n1, double* p);
	virtual ~ContePatch() {};
	void draw(PIXEL_LEVEL level0, PIXEL_LEVEL level1, double patch_width, double patch_height, double dot_diameter) const;
};

// 
class ConteCueDotSupply: public std::vector<ContePatch>
{
public:
	ConteCueDotSupply() {};
	virtual ~ConteCueDotSupply() {};

	// Add a patch. Each patch has two colors of dots, with n0, n1 of each. The array p[] should contain (n0+n1)*2 values 
	// in [-0.5, 0.5]. They are assigned as x0, y0, x1, y1, ....
	void add_patch(unsigned int n0, unsigned int n1, double* p) { this->push_back(ContePatch(n0, n1, p)); };

	// how many patches do we have?
	size_t npatches() const { return this->size(); };

	// get a patch at an index
	const ContePatch& patch(unsigned int i) const { return this->at(i); };
};

// one of the 3-panel stim + distractor patches
struct conte_stim_params
{
	double x, y, w, h, ori, sf, phase, divisor;
	unsigned int isHorizontal;
	unsigned int lwt;
	unsigned int icolor;	// refers to cue_color_0 or cue_color_1, so should be 0|1
};
typedef struct conte_stim_params conte_stim_params_t;

// a single trial specified here
struct conte_trial
{
	double cue_x;	// cue patch center position x
	double cue_y;	// cue patch center position y
	double cue_w;	// cue patch width
	double cue_h;	// cue patch height
	double cue_d;	// cue dot diameter
	unsigned int cue_fpt;		// frames per term for cue
	unsigned int cue_nterms;	// number of terms for cue
	unsigned int cue_to_sample_delay_ms;
	unsigned int sample_display_ms;
	unsigned int sample_to_target_delay_ms;
	unsigned int target_display_ms;
	unsigned int saccade_response_time_ms;
	COLOR_TYPE cue_color_0; // conte_stim_params.icolor == 0
	COLOR_TYPE cue_color_1; // conte_stim_params.icolor == 1
	struct conte_stim_params s0, s1, t0, t1;	// sample and target stim
};
typedef struct conte_trial conte_trial_t;

typedef vector<conte_trial_t> conte_trial_list_t;

istream& operator>>(istream& ins, conte_trial_list_t& file);
istream& operator>>(istream& ins, conte_trial_t& trial);
//istream& operator>>(istream& ins, conte_stim_params_t& stim);

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
	ARContrastFixationPointSpec fixpt;
	bool bHaveFixpt;
	string dot_supply_filename;
	ConteCueDotSupply dot_supply;
	conte_trial_list_t trials;
	vector<COLOR_TYPE> colors;
	conte_arguments()
		: bBinaryTriggers(true)
		, bVerbose(false)
		, bkgdColor(gray)
		, iDistanceToScreenMM(0)
		, bHaveDistance(false)
		, iReadyPulseDelay(0)
		, iPulseBits(0x2)
		, bHaveFixpt(false)
	{};
};

class ConteXYHelper
{
	long m_WpixScr, m_HpixScr;	// screen width, height
	long m_WpixZone, m_HpixZone;	// zone w, h
	unsigned int m_nPatchPerRow;
	unsigned int m_nPatchRows;
	unsigned int m_nRowsPerPage;
	unsigned int m_nPatchPages;
	double m_wdeg, m_hdeg, m_ddeg, m_xdeg, m_ydeg;		// patch w,h; dot diam; patch screen position; all in degrees, origin in center, Ypos up
public:
	ConteXYHelper(double wdeg, double hdeg, double ddeg, double xdeg, double ydeg, unsigned int npatches);
	virtual ~ConteXYHelper() {};

	// get drawing origin for patch 'i', values returned in degrees, use with setDrawOrigin
	void getPageIndDrawOrigin(unsigned int i, DWORD& page_ind, double& x_origin_deg, double& y_origin_deg) const;

	// get xy position for page, as used in cycling setup. Returned values in PIXELS, assign to .Xpos, .Ypos
	void getPageIndXYpos(unsigned int i, DWORD& page_ind, short& Xpos_pix, short& Ypos_pix) const;

	// how many pages will this take? 
	unsigned int getNumPages() const { return m_nPatchPages; };
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
	unsigned int m_itrial;
	bool m_quit;
	int m_errflg;

	PIXEL_LEVEL m_levelOverlayBackground;
	vector<PIXEL_LEVEL> m_levelCueColors;
	PIXEL_LEVEL m_levelTest;
	static const unsigned int m_max_cycle_count = 256;
	unsigned int m_cycle_params_count;
	VSGCYCLEPAGEENTRY m_cycle_params[m_max_cycle_count];	// warning! No check on usage. You have been warned. 
	VSGCYCLEPAGEENTRY m_cycle_clear_params[2];				// warning! No check on usage. You have been warned. 
	VSGCYCLEPAGEENTRY m_cycle_fixpt_params[2];				// warning! No check on usage. You have been warned. 

	static WORD cOvPageClear;
	static WORD cOvPageAperture;
	static WORD cPageBackground;
	static WORD cPageFixpt;
	static WORD cPageCue;
	static WORD cPageSample;
	static WORD cPageTarget;

	ARConteSpec m_sample0, m_sample1, m_target0, m_target1;

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
	void draw_dot_patches(const ConteXYHelper& xyhelper, unsigned int npatches);

	// copy stim parameters prior to drawing
	void copy_params_to_spec(const struct conte_stim_params& params, ARConteSpec& spec);

	// setup page cycling for current trial
	void setup_cycling(const ConteXYHelper& xyhelper, unsigned int nterms_in_cue);

	// setup page cycling to make an orderly trial ending and fixpt
	void ConteUStim::setup_cycling_clear_fixpt();

};


#endif
