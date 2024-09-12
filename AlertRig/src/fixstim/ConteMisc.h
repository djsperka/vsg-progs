#pragma once
#include "Alertlib.h"
#include <vector>
#include <istream>
#include <ostream>
#include <string>
#include "alert-argp.h"

// A Patch is a set of dots, two colors. A series of these patches is used to make up the Cue in the Conte stimulus. 
class ContePatch
{
	unsigned int m_n0, m_n1;
	std::vector<double> m_x;
	std::vector<double> m_y;
public:
	ContePatch(unsigned int n0, unsigned int n1, double* p);
	ContePatch(unsigned int n0, unsigned int n1, const std::vector<double>& x, const std::vector<double>& y);
	virtual ~ContePatch() {};
	void draw(PIXEL_LEVEL level0, PIXEL_LEVEL level1, double patch_width, double patch_height, double dot_diameter) const;
	void make_outline_rect(alert::ARRectangleSpec& r, const COLOR_TYPE& color = COLOR_TYPE(1,1,0));
};

// 
class ConteCueDotSupply : public std::vector<ContePatch>
{
	size_t m_index;
public:
	ConteCueDotSupply() { m_index = 0; };
	virtual ~ConteCueDotSupply() {};

	// Add a patch. Each patch has two colors of dots, with n0, n1 of each. The array p[] should contain (n0+n1)*2 values 
	// in [-0.5, 0.5]. They are assigned as x0, y0, x1, y1, ....
	void add_patch(unsigned int n0, unsigned int n1, double* p) { this->push_back(ContePatch(n0, n1, p)); };
	void add_patch(unsigned int n0, unsigned int n1, const std::vector<double>& x, const std::vector<double>& y) { this->push_back(ContePatch(n0, n1, x, y)); };

	// how many patches do we have?
	size_t npatches() const { return this->size(); };

	// get next patch
	const ContePatch& next_patch()
	{
		if (m_index >= this->size())
		{
			std::cerr << "ERROR - out of dot patches! Starting over at the beginning" << std::endl;
			m_index = 0;
		}
		return this->at(m_index++);
	};
	static void generate_dot_supply(ConteCueDotSupply& supply, int nPatches, int nptsPerPatch);
};

// one of the 3-panel stim + distractor patches
struct conte_stim_params
{
	double x, y, w, h, ori, sf, phase, divisor;
	int iHorizontal;
	unsigned int lwt;
	unsigned int icolor;	// refers to cue_color_0 or cue_color_1, so should be 0|1
	double dGaborContrast;
	double dFlankerContrast;
	double dCueContrast;
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
	int cue_to_sample_delay_ms;
	int sample_display_ms;
	int sample_to_target_delay_ms;
	int target_display_ms;
	int saccade_response_time_ms;
	struct conte_stim_params s0, s1, t0, t1;	// sample and target stim
};
typedef struct conte_trial conte_trial_t;

typedef std::vector<conte_trial_t> conte_trial_list_t;

std::istream& operator>>(std::istream& ins, conte_trial_list_t& file);
std::istream& operator>>(std::istream& ins, conte_trial_t& trial);
std::istream& operator>>(std::istream& in, conte_stim_params_t& stim);

std::ostream& operator<<(std::ostream& ins, const conte_trial_t& trial);

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
	bool bShowCueRects;
	bool bGenerateDots;
	int iGenerateDotsNPts;
	bool bShowAperture;
	alert::ARFixationPointSpec fixpt;
	bool bHaveFixpt;
	bool bShowBorder;
	COLOR_TYPE borderColor;
	int iBorderLineWidth;
	bool bFixptCutoff;
	std::string dot_supply_filename;
	ConteCueDotSupply dot_supply;
	conte_trial_list_t trials;
	std::vector<COLOR_TYPE> colors;
	conte_arguments()
		: bBinaryTriggers(true)
		, bVerbose(false)
		, bkgdColor(gray)
		, iDistanceToScreenMM(0)
		, bHaveDistance(false)
		, iReadyPulseDelay(0)
		, iPulseBits(0x2)
		, bShowCueRects(false)
		, bGenerateDots(false)
		, iGenerateDotsNPts(0)
		, bShowAperture(false)
		, bHaveFixpt(false)
		, bShowBorder(false)
		, bFixptCutoff(false)
	{};
};






// callback for arg parser
error_t parse_conte_opt(int key, char* carg, struct argp_state* state);

// parse trials file
bool parse_trials_file(const std::string& filename, conte_trial_list_t& trials);

// parse xy file
bool parse_dot_supply_file(const std::string& filename, ConteCueDotSupply& dotsupply);

// input stuff
bool parse_conte_stim_params(const std::vector<std::string>& tokens, unsigned int first, conte_stim_params_t& stim);

// parse border argument
int parse_border(const std::string& sarg, COLOR_TYPE& borderColor, int& iBorderLineWidth);
