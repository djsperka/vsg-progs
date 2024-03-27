#include "ConteMisc.h"
#include <boost/filesystem.hpp>
#include <boost/range/algorithm/random_shuffle.hpp>
#include <fstream>
#include <string>
#include <random>

using namespace std;


ostream& operator<<(ostream& out, const conte_trial_t& trial)
{
	out << "cue: " << trial.cue_x << "," << trial.cue_y << "," << trial.cue_w << "," << trial.cue_h << ","
		<< trial.cue_d << "," << trial.cue_fpt << "," << trial.cue_nterms
		<< ") timing: " << trial.cue_to_sample_delay_ms << "," << trial.sample_display_ms << ","
		<< trial.sample_to_target_delay_ms << "," << trial.target_display_ms << "," << trial.saccade_response_time_ms;
	return out;
}


istream& operator>>(istream& ins, conte_trial_t& trial)
{
	string line;
	stringstream fs;
	vector<string> tokens;

	// get a line nonzero length
	getline(ins, line);
	while (!ins.eof() && line.size() == 0)
		getline(ins, line);

	if (ins.eof())
		return ins;

	tokenize(line, tokens, ",");

	fs.str(tokens[0]);
	fs.clear();
	fs >> trial.cue_x;

	fs.str(tokens[1]);
	fs.clear();
	fs >> trial.cue_y;

	fs.str(tokens[2]);
	fs.clear();
	fs >> trial.cue_w;

	fs.str(tokens[3]);
	fs.clear();
	fs >> trial.cue_h;

	fs.str(tokens[4]);
	fs.clear();
	fs >> trial.cue_d;

	fs.str(tokens[5]);
	fs.clear();
	fs >> trial.cue_fpt;

	fs.str(tokens[6]);
	fs.clear();
	fs >> trial.cue_nterms;

	fs.str(tokens[7]);
	fs.clear();
	fs >> trial.cue_to_sample_delay_ms;

	fs.str(tokens[8]);
	fs.clear();
	fs >> trial.sample_display_ms;

	fs.str(tokens[9]);
	fs.clear();
	fs >> trial.sample_to_target_delay_ms;

	fs.str(tokens[10]);
	fs.clear();
	fs >> trial.target_display_ms;

	fs.str(tokens[11]);
	fs.clear();
	fs >> trial.saccade_response_time_ms;

	parse_conte_stim_params(tokens, 12, trial.s0);
	parse_conte_stim_params(tokens, 26, trial.s1);
	parse_conte_stim_params(tokens, 40, trial.t0);
	parse_conte_stim_params(tokens, 54, trial.t1);

	return ins;
}


istream& operator>>(istream& ins, conte_trial_list_t& trials)
{
	trials.clear();
	conte_trial_t t;
	while (ins >> t)
	{
		trials.push_back(t);
		std::cerr << t << std::endl;
	}
	return ins;
}



std::istream& operator>>(std::istream& in, conte_stim_params_t& stim)
{
	std::string s;
	std::vector<std::string> tokens;
	in >> s;
	tokenize(s, tokens, ",");
	if (!parse_conte_stim_params(tokens, 0, stim))
	{
		in.setstate(std::ios::failbit);
	}
	return in;
}


bool parse_conte_stim_params(const std::vector<string>& tokens, unsigned int first, conte_stim_params_t& stim)
{
	stringstream fs;
	
	fs.str(tokens[first]);
	fs.clear();
	fs >> stim.x;

	fs.str(tokens[first + 1]);
	fs.clear();
	fs >> stim.y;

	fs.str(tokens[first + 2]);
	fs.clear();
	fs >> stim.w;

	fs.str(tokens[first + 3]);
	fs.clear();
	fs >> stim.h;

	fs.str(tokens[first + 4]);
	fs.clear();
	fs >> stim.ori;

	fs.str(tokens[first + 5]);
	fs.clear();
	fs >> stim.sf;

	fs.str(tokens[first + 6]);
	fs.clear();
	fs >> stim.phase;

	fs.str(tokens[first + 7]);
	fs.clear();
	fs >> stim.divisor;

	fs.str(tokens[first + 8]);
	fs.clear();
	fs >> stim.iHorizontal;

	fs.str(tokens[first + 9]);
	fs.clear();
	fs >> stim.lwt;

	fs.str(tokens[first + 10]);
	fs.clear();
	fs >> stim.icolor;

	fs.str(tokens[first + 11]);
	fs.clear();
	fs >> stim.dGaborContrast;

	fs.str(tokens[first + 12]);
	fs.clear();
	fs >> stim.dFlankerContrast;

	fs.str(tokens[first + 13]);
	fs.clear();
	fs >> stim.dCueContrast;

	return true;
}


bool parse_trials_file(const std::string& filename, conte_trial_list_t& trials)
{
	bool b = false;
	ifstream ifs(filename);
	if (ifs.is_open())
	{
		ifs >> trials;
		ifs.close();
		b = true;
	}
	return b;
}

bool parse_dot_supply_file(const std::string& filename, ConteCueDotSupply& dotsupply)
{
	bool bReturn = false;

	boost::filesystem::path p(filename);
	if (!exists(p))
	{
		std::cerr << "Error: dot supply file does not exist: " << filename << endl;
		return false;
	}
	else
	{
		bReturn = true;
		boost::filesystem::path folder = p.parent_path();		// if file has relative pathnames to images, they are relative to dir file lives in
		std::cerr << "Found dot supply file " << p << " at path " << folder << std::endl;

		ifstream ifs(filename, ios::in | ios::binary);

		unsigned int N[2];
		double d[1000];				// hard limit to 500 points per patch

		while (ifs.read((char*)N, 2 * sizeof(int)) && ifs.read((char*)d, 2 * (N[0] + N[1]) * sizeof(double)))
		{
			dotsupply.add_patch(N[0], N[1], d);
		}
		ifs.close();
	}
	return bReturn;
}




error_t parse_conte_opt(int key, char* carg, struct argp_state* state)
{
	error_t ret = 0;
	struct conte_arguments* arguments = (struct conte_arguments*)state->input;
	COLOR_TYPE color;
	std::string sarg;
	if (carg) sarg = carg;
	switch (key)
	{
	case 'a':
		arguments->bBinaryTriggers = false;
		break;
	case 'v':
		arguments->bVerbose = true;
		break;
	case 'b':
		if (parse_color(sarg, arguments->bkgdColor))
			ret = EINVAL;
		break;
	case 'd':
		if (parse_distance(sarg, arguments->iDistanceToScreenMM))
			ret = EINVAL;
		else
			arguments->bHaveDistance = true;
		break;
	case 'c':
		if (parse_color(sarg, color))
			ret = EINVAL;
		else
			arguments->colors.push_back(color);
		break;
	case 'l':
		if (parse_integer(sarg, arguments->iReadyPulseDelay))
			ret = EINVAL;
		break;
	case 'p':
		if (parse_integer(sarg, arguments->iPulseBits))
			ret = EINVAL;
		break;
	case 'f':
		if (parse_fixation_point(sarg, arguments->fixpt))
			ret = EINVAL;
		else
		{
			arguments->bHaveFixpt = true;
		}
		break;
	case 701:
		if (!boost::filesystem::exists(sarg))
		{
			cerr << "Cannot find dot supply file" << endl;
			ret = EINVAL;
		}
		else
		{
			if (!parse_dot_supply_file(sarg, arguments->dot_supply))
			{
				cerr << "Error parsing dot supply file" << endl;
				ret = EINVAL;
			}
			else
			{
				cerr << "Found " << arguments->dot_supply.npatches() << " patches " << endl;
			}
		}
		break;
	case 702:
		// parse conte stim spec
		if (!boost::filesystem::exists(sarg))
		{
			cerr << "Cannot find trials spec file" << endl;
			ret = EINVAL;
		}
		else
		{
			if (parse_trials_file(sarg, arguments->trials))
			{
				cerr << "Number of trials: " << arguments->trials.size() << endl;
			}
			else
			{
				cerr << "Cannot read trials file: " << sarg << endl;
				ret = EINVAL;
			}
		}
		break;
	case 703:
		arguments->bShowCueRects = true;
		break;
	case 704:
		arguments->bGenerateDots = true;
		if (parse_integer(sarg, arguments->iGenerateDotsNPts))
		{
			cerr << "Expecting integer arg - should be number of dots per generated patch.";
			ret = EINVAL;
		}
	case 705:
		arguments->bShowAperture = true;
		break;
	case ARGP_KEY_END:
		{
			// check that everything needed has been received
			size_t num_patches = arguments->dot_supply.size();
			size_t num_patches_needed = 0;
			for (auto tr : arguments->trials)
				num_patches_needed += tr.cue_nterms;
			cerr << "Trials require " << num_patches_needed << " patches, dot supply has " << num_patches << endl;
			if (num_patches == 0 && arguments->bGenerateDots)
			{
				ConteCueDotSupply::generate_dot_supply(arguments->dot_supply, num_patches_needed, arguments->iGenerateDotsNPts);
				num_patches = arguments->dot_supply.size();
			}
			if (num_patches_needed > num_patches)
			{
				ret = EINVAL;
				cerr << "NOT ENOUGH PATCHES!" << endl;
			}
			else if (num_patches_needed < num_patches)
			{
				cerr << endl << "WARNING: There are more patches than are needed!" << endl << endl;
			}
		}
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return ret;
}



ConteXYHelper::ConteXYHelper(double w, double h, double d, double x, double y, unsigned int npatches)
	: m_wdeg(w)
	, m_hdeg(h)
	, m_ddeg(d)
	, m_xdeg(x)
	, m_ydeg(y)
{
	m_WpixScr = vsgGetScreenWidthPixels();
	m_HpixScr = vsgGetScreenHeightPixels();
	m_WpixZone = vsgGetSystemAttribute(vsgVIDEOZONEWIDTH);
	m_HpixZone = vsgGetSystemAttribute(vsgVIDEOZONEHEIGHT);

	// Figure out how many patches can be drawn on a page..........
	double WdegZone, HdegZone;
	vsgUnit2Unit(vsgPIXELUNIT, m_WpixZone, vsgDEGREEUNIT, &WdegZone);
	vsgUnit2Unit(vsgPIXELUNIT, m_HpixZone, vsgDEGREEUNIT, &HdegZone);
	m_nPatchPerRow = (unsigned int)trunc(WdegZone / (m_ddeg + m_wdeg));
	m_nRowsPerPage = (unsigned int)trunc(HdegZone / (m_ddeg + m_wdeg));
	m_nPatchRows = (unsigned int)ceil((double)npatches / (double)m_nPatchPerRow);
	m_nPatchPages = (unsigned int)ceil((double)npatches / (double)(m_nRowsPerPage * m_nPatchPerRow));
	cerr << "ConteXYHelper: nPatchPerRow " << m_nPatchPerRow << " rows per page " << m_nRowsPerPage << endl;
	cerr << "num patches for cue: " << npatches << " pages needed: " << m_nPatchPages << endl;
}

void ConteXYHelper::getPageIndDrawOrigin(unsigned int i, DWORD& page_ind, double& x_origin_deg, double& y_origin_deg) const
{
	page_ind = (DWORD)(i / (m_nRowsPerPage * m_nPatchPerRow));
	unsigned int ipage = i % (m_nRowsPerPage * m_nPatchPerRow);
	x_origin_deg = (ipage % m_nPatchPerRow + 0.5) * (m_wdeg + m_ddeg);
	y_origin_deg = (ipage / m_nPatchPerRow + 0.5) * (m_wdeg + m_ddeg);
	return;
}

void ConteXYHelper::getPageIndXYpos(unsigned int i, DWORD& page_ind, short& Xpos_pix, short& Ypos_pix) const
{
	page_ind = (DWORD)(i / (m_nRowsPerPage * m_nPatchPerRow));
	unsigned int ipage = i % (m_nRowsPerPage * m_nPatchPerRow);

	double x_origin_degrees, y_origin_degrees;
	double x_origin_pixels, y_origin_pixels;

	// draw origin relative to upper left corner of page, with +y = down. 
	// convert to pixels
	DWORD dummy;
	getPageIndDrawOrigin(i, dummy, x_origin_degrees, y_origin_degrees);
	//getDrawOrigin(i, x_origin_degrees, y_origin_degrees);
	vsgUnit2Unit(vsgDEGREEUNIT, x_origin_degrees, vsgPIXELUNIT, &x_origin_pixels);
	vsgUnit2Unit(vsgDEGREEUNIT, y_origin_degrees, vsgPIXELUNIT, &y_origin_pixels);

	Xpos_pix = (short)(x_origin_pixels - m_WpixScr / 2);
	Ypos_pix = (short)(y_origin_pixels - m_HpixScr / 2);
	return;
};

ContePatch::ContePatch(unsigned int n0, unsigned int n1, double* p)
	: m_n0(n0)
	, m_n1(n1)
{
	for (unsigned int i = 0; i < (n0 + n1); i++)
	{
		m_x.push_back(p[2 * i]);
		m_y.push_back(p[2 * i + 1]);
	}
}

ContePatch::ContePatch(unsigned int n0, unsigned int n1, const std::vector<double>& x, const std::vector<double>& y)
	: m_n0(n0)
	, m_n1(n1)
	, m_x(x)
	, m_y(y)
{	
}


void ContePatch::draw(PIXEL_LEVEL level0, PIXEL_LEVEL level1, double patch_width, double patch_height, double dot_diam) const
{
	unsigned int i;
	vector < PIXEL_LEVEL> levels;
	vector <unsigned int> ind;
	for (i = 0; i < m_n0; i++)
		levels.push_back(level0);
	for (i = 0; i < m_n1; i++)
		levels.push_back(level1);
	// randomize the order
	for (i = 0; i < (m_n0 + m_n1); i++)
		ind.push_back(i);
	boost::range::random_shuffle(ind);

	// draw
	long mode_saved = vsgGetDrawMode();
	vsgSetDrawMode(vsgCENTREXY | vsgSOLIDFILL);
	for (i = 0; i < (m_n0 + m_n1); i++)
	{
		vsgSetPen1(levels[ind[i]]);
		vsgDrawOval(m_x[ind[i]] * patch_width, m_y[ind[i]] * patch_height, dot_diam, dot_diam);
	}
	vsgSetDrawMode(mode_saved);
}

//void ContePatch::drawOutlineRect(double patch_width, double patch_height, const COLOR_TYPE& color = COLOR_TYPE(1, 1, 0))
//{
//	alert::ARRectangleSpec r;
//	r.x = 0;
//	r.y = 0;
//	r.w = patch_width;
//	r.h = patch_height;
//	r.color = color;
//	r.drawmode = vsgSOLIDPEN + vsgCENTREXY;
//	r.linewidth = 2.0;
//	r.draw();
//}
//

void ConteCueDotSupply::generate_dot_supply(ConteCueDotSupply& supply, int nPatches, int nptsPerPatch)
{
	std::vector<double> x, y;
	int n0, n1;
	double f0[5] = { 0, .25, .5, .75, 1 };
	std::random_device rd;  // Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
	std::uniform_real_distribution<> dis(-0.5, 0.5);
	for (int i = 0; i < nPatches; i++)
	{
		x.clear();
		y.clear();
		for (int j = 0; j < nptsPerPatch; j++)
		{
			x.push_back(dis(gen));
			y.push_back(dis(gen));
		}
		n0 = nptsPerPatch * f0[i % 5];
		n1 = nptsPerPatch - n0;
		supply.add_patch(n0, n1, x, y);
	}
	return;
}
