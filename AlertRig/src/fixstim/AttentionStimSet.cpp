#include "AttentionStimSet.h"
#include "QCycleUtil.h"
#include <algorithm>
using namespace std;

// this undef tells stim set to use (unfilled) rects instead of (filled) points for cue points
#undef USE_CUE_POINTS



double get_fconfig(VSGCYCLEPAGEENTRY* cycle, int count)
{
	double f=0;
	for (int i=0; i<count; i++)
		f += FRAMES_TO_SECONDS(cycle[i].Frames);
	return f;
}









// helper function for loading params from a comma-separated string
int parse_attparams(const string& s, int nstim, vector<struct AttParams>& vecTrialParams, double& tMax)
{
	int i;
	int status = 0;
	struct AttParams params;
	vector<string> tokens;
	tokenize(s, tokens, ",");
	if (tokens.size() % (4+nstim*2) == 1)
	{
		istringstream iss;
		vector<string>::const_iterator it = tokens.begin();
		if (parse_double(*it, tMax))
		{
			cerr << "Error reading tMax - max time after contrast change (" << *it << ")" << endl;
			return 1;
		}
		it++;

		while (it != tokens.end())
		{
			// each pass through this loop will pickup the parameters for a single trial. 
			// The color, init phase, time to cc, and off bits are followed by a contrast 
			// pair for each stim.

			if (parse_color(*it, params.color))
			{
				cerr << "Error reading color: " << *it << endl;
				status = 1;
				break;
			}
			it++;
			iss.str(*it);
			if (parse_double(*it, params.dInitialPhase))
			{
				cerr << "Error reading initial phase: " << *it << endl;
				status = 1;
				break;
			}
			it++;
			if (parse_double(*it, params.dTimeToCC))
			{
				cerr << "Error reading time to CC: " << *it;
				status = 1;
				break;
			}
			it++;
			if (parse_integer(*it, params.iOffBits))
			{
				cerr << "Error reading off bits: " << *it;
				status = 1;
				break;
			}
			it++;

			// Pick up contrast pairs. Clear the vector that holds the pairs first, so we
			// don't just append the current trials' pairs to last trials' pairs. 
			params.contrastPairs.clear();
			for (i=0; i<nstim; i++)
			{
				int iBase, iChg;
				if (parse_integer(*it, iBase))
				{
					cerr << "Error reading base contrast: " << *it << endl;
					status = 1;
					return 1;
				}
				it++;
				if (parse_integer(*it, iChg))
				{
					cerr << "Error reading chg contrast: " << *it << endl;
					status = 1;
					return 1;
				}
				it++;
				params.contrastPairs.push_back(std::pair<int, int>(iBase, iChg));
			}
			vecTrialParams.push_back(params);
		}
	}
	else
	{
		cerr << "Error reading attention parameters. Check command line args." << endl;
		status = 1;
	}
	return status;
}

int parse_flashyparams(const string& s, FlashyParamVectorVector& vecFlashyParamVector)
{
	unsigned int itoken;
	int tcount=0;
	int nf;
	int status = 0;
	double ton, toff;
	FlashyParams params;
	FlashyParamVector vecFPTrial;
	vector<string> tokens;
	tokenize(s, tokens, ",");

	// note: incrementing itoken is taken care of inside loop.
	for (itoken = 0; !status && itoken<tokens.size(); )
	{
		if (parse_integer(tokens.at(itoken), nf))
		{
			cerr << "Error parsing numflashes for trial " << tcount+1 << endl;
			status = 1;
		}
		else
		{
			// check if there are enough tokens for all the flashes; 7 numbers per flash
			vecFPTrial.clear();
			if (itoken + nf*7 < tokens.size())
			{
				for (int iflash=0; iflash<nf; iflash++)
				{
					if (parse_integer(tokens.at(itoken + 1 + iflash*7), params.nk) || 
						parse_double(tokens.at(itoken + 1 + iflash*7 + 1), params.x) ||
						parse_double(tokens.at(itoken + 1 + iflash*7 + 2), params.y) ||
						parse_double(tokens.at(itoken + 1 + iflash*7 + 3), params.w) ||
						parse_double(tokens.at(itoken + 1 + iflash*7 + 4), params.h) ||
						parse_double(tokens.at(itoken + 1 + iflash*7 + 5), ton) ||
						parse_double(tokens.at(itoken + 1 + iflash*7 + 6), toff))
					{
						cerr << "Error parsing trial " << tcount+1 << " flash# " << iflash << endl;
						status = 1;
						break;
					}
					else
					{
						params.fon = SECONDS_TO_FRAMES(ton);
						params.foff = SECONDS_TO_FRAMES(toff);
						vecFPTrial.push_back(params);
					}
				}

				// Check that the flashies do NOT overlap with one another. 
				if (!status)
				{
					int f = -1;
					for (FlashyParamVector::const_iterator it = vecFPTrial.begin(); !status && it != vecFPTrial.end(); it++)
					{
						if (it->fon < f)
						{
							cerr << "Error parsing flashies for trial " << vecFlashyParamVector.size() << ": flashy overlap." << endl;
							status = 1;
						}
						else if (it->fon > it->foff)
						{
							cerr << "Error parsing flashies for trial " << vecFlashyParamVector.size() << ": flashy off before on." << endl;
							status = 1;
						}
						else
							f = it->foff;
					}
				}

				if (!status)
				{
					vecFlashyParamVector.push_back(vecFPTrial);
					itoken = itoken + 1 + nf*7;
				}
			}
			else
			{
				cerr << "Error parsing trial " << tcount+1 << " Expecting " << nf << " flashies, itoken=" << itoken << " tokens.size=" << tokens.size() << endl;
				status = 1;
			}
		}
	}

	return status;
}


void my_dump_flashyparamvector(FlashyParamVector vec)
{
	int i=0;
	for (FlashyParamVector::iterator it = vec.begin(); it != vec.end(); it++)
	{
		cout << i << ": " << it->nk << ":" << it->x << "," << it->y << "," << it->w << "," << it->h << "," << it->fon << "," << it->foff << endl;
		i++;
	}
}

void dump_flashyparams(const FlashyParamVectorVector& vecFlashyParamVector)
{
	for_each(vecFlashyParamVector.begin(), vecFlashyParamVector.end(), my_dump_flashyparamvector);
}

int checkFlashyTimes(const vector<AttParams>& vecInput, const FlashyParamVectorVector& vecFlashies, double tMax)
{
	int status = 0;
	if (vecInput.size() == vecFlashies.size())
	{
		for (unsigned int itrial = 0; !status && itrial<vecInput.size(); itrial++)
		{
			//double tLast = 0.0;
			//double dTimeToCC = vecInput.at(itrial).dTimeToCC;
			int fLast = 0;
			int fMax = SECONDS_TO_FRAMES(tMax);
			int fToCC = SECONDS_TO_FRAMES(vecInput.at(itrial).dTimeToCC);
			for (unsigned int j=0; !status && j<vecFlashies.at(itrial).size(); j++)
			{
				const FlashyParams& params = vecFlashies.at(itrial).at(j);
				if (params.foff < params.fon)
				{
					cerr << "checkFlashyTimes: trial " << itrial << " flashy# " << j << " flashy on " << params.fon << " off " << params.foff << " must be longer than 0s." << endl;
					status = 1;
				}
				else if (params.fon < fLast)
				{
					cerr << "checkFlashyTimes: trial " << itrial << " flashy# " << j << " overlaps previous." << endl;
					status = 1;
				}
				else if ((params.foff - fToCC) > fMax)
				{
					cerr << "checkFlashyTimes: trial " << itrial << " flashy# " << j << " goes beyond tMax for this trial." << endl;
					status = 1;
				}

				// now make sure last flashy doesn't go past tMax
				fLast = params.foff;
			}
		}
	}
	else if (vecFlashies.size() > 0)
	{
		cerr << "Error: if flashies used, must configure one for each trial." << endl;
		status = 1;
	}
	return status;
}

int parse_interleaved_params(const string& s, int nstim, vector<InterleavedParams>& params)
{
	int i;
	int status = 0;
	struct InterleavedParams trial;
	vector<string> tokens;
	tokenize(s, tokens, ",");
	if (tokens.size() % (8 + nstim * 2) == 0)
	{
		istringstream iss;
		vector<string>::const_iterator it = tokens.begin();
		while (it != tokens.end())
		{
			// each pass through this loop will pickup the parameters for a single trial. 

			if (parse_color(*it, trial.color))
			{
				cerr << "Error reading color: " << *it << endl;
				status = 1;
				break;
			}
			it++;
			iss.str(*it);
			if (parse_double(*it, trial.dInitialPhase))
			{
				cerr << "Error reading initial phase: " << *it << endl;
				status = 1;
				break;
			}
			it++;
			if (parse_integer(*it, trial.iOffBits))
			{
				cerr << "Error reading off bits: " << *it;
				status = 1;
				break;
			}
			it++;
			if (parse_double(*it, trial.times[0]))
			{
				cerr << "Error reading time to Q0: " << *it;
				status = 1;
				break;
			}
			it++;
			if (parse_double(*it, trial.times[1]))
			{
				cerr << "Error reading time to Q1: " << *it;
				status = 1;
				break;
			}
			it++;
			if (parse_double(*it, trial.times[2]))
			{
				cerr << "Error reading time to Stim: " << *it;
				status = 1;
				break;
			}
			it++;
			if (parse_double(*it, trial.times[3]))
			{
				cerr << "Error reading time to CC: " << *it;
				status = 1;
				break;
			}
			it++;
			if (parse_double(*it, trial.times[4]))
			{
				cerr << "Error reading time to End: " << *it;
				status = 1;
				break;
			}
			it++;

			// Pick up contrast pairs. Clear the vector that holds the pairs first, so we
			// don't just append the current trials' pairs to last trials' pairs. 
			trial.contrastPairs.clear();
			for (i = 0; i<nstim; i++)
			{
				int iBase, iChg;
				if (parse_integer(*it, iBase))
				{
					cerr << "Error reading base contrast: " << *it << endl;
					status = 1;
					return 1;
				}
				it++;
				if (parse_integer(*it, iChg))
				{
					cerr << "Error reading chg contrast: " << *it << endl;
					status = 1;
					return 1;
				}
				it++;
				trial.contrastPairs.push_back(std::pair<int, int>(iBase, iChg));
			}
			params.push_back(trial);
		}
	}
	else
	{
		cerr << "Error reading interleaved attention parameters. Check command line args." << endl;
		status = 1;
	}
	return status;
}

void my_print_interleaved_params(const InterleavedParams& trial)
{
	std::cout << trial.color << " , " << trial.dInitialPhase << " , " << std::hex << trial.iOffBits << std::dec << trial.times[0] << "/" << trial.times[1] << "/" << trial.times[2] << "/" << trial.times[3] << "/" << trial.times[4] << " Pairs "; 
	for (int i = 0; i < trial.contrastPairs.size(); i++)
		std::cout << "(" << trial.contrastPairs[i].first << "," << trial.contrastPairs[i].second << ") ";
	std::cout << std::endl;
}

void my_print_interleaved_trials(const vector<InterleavedParams>& trials)
{
	std::for_each(trials.begin(), trials.end(), my_print_interleaved_params);
}


AttentionStimSet::AttentionStimSet(ARContrastFixationPointSpec& fixpt, double tMax, std::vector<alert::ARGratingSpec>& vecGratings, vector<AttParams>& params)
: m_fixpt(fixpt)
, m_tMax(tMax)
, m_bUseCueCircles(false)
, m_bUseCuePoints(false)
, m_vecGratings(vecGratings)
, m_vecGratingsCC(vecGratings)
, m_vecParams(params)
, m_current(0)
{};

AttentionStimSet::AttentionStimSet(ARContrastFixationPointSpec& fixpt, double tMax, std::vector<alert::ARGratingSpec>& vecGratings, vector<AttParams>& params, vector<alert::ARGratingSpec>& vecDistractors, FlashyParamVectorVector& vecFlashies)
: m_fixpt(fixpt)
, m_tMax(tMax)
, m_bUseCueCircles(false)
, m_bUseCuePoints(false)
, m_vecGratings(vecGratings)
, m_vecGratingsCC(vecGratings)
, m_vecParams(params)
, m_vecDistractors(vecDistractors)
, m_vecFlashies(vecFlashies)
, m_current(0)
{};


AttentionStimSet::AttentionStimSet(ARContrastFixationPointSpec& fixpt, double tMax, vector<alert::ARGratingSpec>& vecGratings, vector<AttentionCue>& vecCuePairs, bool bCueCircles, bool bCuePoints, vector<AttParams>& params)
: m_fixpt(fixpt)
, m_tMax(tMax)
, m_bUseCueCircles(bCueCircles)
, m_bUseCuePoints(bCuePoints)
, m_vecGratings(vecGratings)
, m_vecGratingsCC(vecGratings)
, m_vecParams(params)
, m_current(0)
{
	for (unsigned int i=0; i<vecCuePairs.size(); i++)
	{
		ARContrastCircleSpec circle;
		int indGrating = i % m_vecGratings.size();

		// set up cue circle
		circle.x = m_vecGratings[indGrating].x;
		circle.y = m_vecGratings[indGrating].y;
		circle.d = m_vecGratings[indGrating].w + vecCuePairs[i].rdiff*2;
		circle.linewidth = vecCuePairs[i].linewidth;
		circle.color = vecCuePairs[i].color;
		m_vecCues.push_back(circle);

#ifdef USE_CUE_POINTS
		// set up cue point
		ARContrastFixationPointSpec f;
		f.color = vecCuePairs[i].color;
		f.d = fixpt.d;
		f.x = m_vecGratings[indGrating].x;
		f.y = m_vecGratings[indGrating].y;
		m_vecCuePoints.push_back(f);
#else
		ARContrastRectangleSpec r(vsgSOLIDPEN, vecCuePairs[i].linewidth);
		r.color = vecCuePairs[i].color;
		r.x = m_vecGratings[indGrating].x;
		r.y = m_vecGratings[indGrating].y;
		r.w = r.h = 2*fixpt.d;
		r.orientation = 0;
		m_vecCueRects.push_back(r);
#endif
	}
};

AttentionStimSet::AttentionStimSet(ARContrastFixationPointSpec& fixpt, double tMax, vector<alert::ARGratingSpec>& vecGratings, vector<AttentionCue>& vecCuePairs, bool bCueCircles, bool bCuePoints, vector<AttParams>& params, vector<alert::ARGratingSpec>& vecDistractors, FlashyParamVectorVector& vecFlashies)
: m_fixpt(fixpt)
, m_tMax(tMax)
, m_bUseCueCircles(bCueCircles)
, m_bUseCuePoints(bCuePoints)
, m_vecGratings(vecGratings)
, m_vecGratingsCC(vecGratings)
, m_vecParams(params)
, m_vecDistractors(vecDistractors)
, m_vecFlashies(vecFlashies)
, m_current(0)
{
	for (unsigned int i = 0; i<vecCuePairs.size(); i++)
	{
		ARContrastCircleSpec circle;
		int indGrating = i % m_vecGratings.size();

		// set up cue circle
		circle.x = m_vecGratings[indGrating].x;
		circle.y = m_vecGratings[indGrating].y;
		circle.d = m_vecGratings[indGrating].w + vecCuePairs[i].rdiff * 2;
		circle.linewidth = vecCuePairs[i].linewidth;
		circle.color = vecCuePairs[i].color;
		m_vecCues.push_back(circle);

#ifdef USE_CUE_POINTS
		// set up cue point
		ARContrastFixationPointSpec f;
		f.color = vecCuePairs[i].color;
		f.d = fixpt.d;
		f.x = m_vecGratings[indGrating].x;
		f.y = m_vecGratings[indGrating].y;
		m_vecCuePoints.push_back(f);
#else
		ARContrastRectangleSpec r(vsgSOLIDPEN, vecCuePairs[i].linewidth);
		r.color = vecCuePairs[i].color;
		r.x = m_vecGratings[indGrating].x;
		r.y = m_vecGratings[indGrating].y;
		r.w = r.h = 2*fixpt.d;
		r.orientation = 0;
		m_vecCueRects.push_back(r);
#endif

	}
};

AttentionStimSet::AttentionStimSet(ARContrastFixationPointSpec& fixpt, vector<alert::ARGratingSpec>& vecGratings, vector<AttentionCue>& vecCuePairs, bool bCueCircles, bool bCuePoints, vector<InterleavedParams>& params)
: m_fixpt(fixpt)
, m_tMax(0)
, m_bUseCueCircles(bCueCircles)
, m_bUseCuePoints(bCuePoints)
, m_vecGratings(vecGratings)
, m_vecGratingsCC(vecGratings)
, m_vecInterleaved(params)
, m_current(0)
{
	for (unsigned int i = 0; i<vecCuePairs.size(); i++)
	{
		ARContrastCircleSpec circle;
		int indGrating = i % m_vecGratings.size();

		// set up cue circle
		circle.x = m_vecGratings[indGrating].x;
		circle.y = m_vecGratings[indGrating].y;
		circle.d = m_vecGratings[indGrating].w + vecCuePairs[i].rdiff * 2;
		circle.linewidth = vecCuePairs[i].linewidth;
		circle.color = vecCuePairs[i].color;
		m_vecCues.push_back(circle);

#ifdef USE_CUE_POINTS
		// set up cue point
		ARContrastFixationPointSpec f;
		f.color = vecCuePairs[i].color;
		f.d = fixpt.d;
		f.x = m_vecGratings[indGrating].x;
		f.y = m_vecGratings[indGrating].y;
		m_vecCuePoints.push_back(f);
#else
		ARContrastRectangleSpec r(vsgSOLIDPEN, vecCuePairs[i].linewidth);
		r.color = vecCuePairs[i].color;
		r.x = m_vecGratings[indGrating].x;
		r.y = m_vecGratings[indGrating].y;
		r.w = r.h = 2*fixpt.d;
		r.orientation = 0;
		m_vecCueRects.push_back(r);
#endif


	}
};



int AttentionStimSet::init(std::vector<int> pages, int)
{
	int status = 0;
	int nlevels;
	m_pageBlank = pages[0];
	m_pageFixpt = pages[1];
	m_pageStim = pages[2];
	m_pageChg = pages[3];
	m_pageD = max(max(max(m_pageBlank, m_pageFixpt), m_pageStim), m_pageChg) + 1;

	if (m_vecInterleaved.size() > 0)
	{
		// When using interleaved, there are two additional pages
		m_pageFixptQ = pages[4];
		m_pageQStim = pages[5];
		m_pageQStimCC = pages[6];
	}

	// divvy up levels. There are only about 250 levels available but fixpt takes 2...
	m_fixpt.init(2);
	m_fixpt.setContrast(100);


	// Change level mult factor from 2 to 4 for cues to accomodate cue points. If no cue points this
	// overestimates the number of levels. 
	//nlevels = (245 - 4*m_vecCues.size())/(m_vecGratings.size()*2 + m_vecDistractors.size());

	// If using cues, take 2 levels per stim for the cues. That's because the cue point AND cue circle use the
	// same VSG object. If there are multiple sets of cues/points, they share the same set of VSG 
	// objects. Thus, if there are 3 stim, and 18 sets of cues (54 in all), then we will only need 6 levels for 
	// the cue objects.

	nlevels = (int)((245-((m_bUseCueCircles || m_bUseCuePoints) ? m_vecGratings.size()*2 : 0))/(m_vecGratings.size() * 2 + m_vecDistractors.size()));
	cerr << "Number of levels per grating " << nlevels << endl;


	// Below the cues and cue points share levels to save on the overall number of objects. 
	// The cues come in groups where each one corresponds to a stimulus grating. If there are 3 stim gratings, 
	// then the first cue corresponds to the first grating (uses its center point, and its radius is used for 
	// determining the radius offset of the cue circle), the second cue corresponds to the second grating, 
	// and the third cue corresponds to the third grating. After that, additional groups of cues correspond to 
	// the first, second, and third grating in the same way. 
	// The 'offBits' parameter for a given trial has the particular cue group encoded: The upper 8 bits are an 
	// index into the list of cues. Thus, if 'offBits' is 0x400, the cue group [4] (the fifth one, since we use
	// ordinary C indexing like we should) is used for that trial. 
	// 
	// The first group of cues is initialized (gets assigned a VSG object number, color levels) in the normal way. 
	// Subsequent groups of cues (and all the cue points) are initialized to use the same object number and levels. 

	cerr << "Initialize " << m_vecCues.size() << " cues." << endl;
	for (unsigned int i = 0; i<m_vecGratings.size(); i++)
	{
		m_vecCues[i].init(2);
	}
	for (size_t i = m_vecGratings.size(); i < m_vecCues.size(); i++)
	{
		cerr << "init cue " << i << " using cue " << (i % m_vecGratings.size()) << endl;
		m_vecCues[i].init(m_vecCues[i % m_vecGratings.size()]);
	}

#ifdef USE_CUE_POINTS
	// Initialize all cue points, even if they are not used. 
	cerr << "Initialize " << m_vecCuePoints.size() << " cue points." << endl;
	for (unsigned int i=0; i<m_vecCuePoints.size(); i++)
	{
		m_vecCuePoints[i].init(m_vecCues[i]);
	}
#else
	// Initialize all cue points, even if they are not used. 
	cerr << "Initialize " << m_vecCueRects.size() << " cue rects." << endl;
	for (unsigned int i = 0; i<m_vecCueRects.size(); i++)
	{
		m_vecCueRects[i].init(m_vecCues[i]);
	}
#endif
	cerr << "Initialize " << m_vecGratings.size() << " gratings." << endl;
	for (unsigned int i=0; i<m_vecGratings.size(); i++)
	{
		m_vecGratings[i].init(nlevels);
		m_vecGratingsCC[i].init(nlevels);
	}
	cerr << "Initialize " << m_vecDistractors.size() << " distractors." << endl;
	for (unsigned int i=0; i<m_vecDistractors.size(); i++)
	{
		m_vecDistractors[i].init(nlevels);
	}

	status = drawCurrent();

	return status;
}

int AttentionStimSet::num_pages() 
{ 
	if (m_vecInterleaved.empty())
		return (int)(4 + m_vecDistractors.size());
	else
		return 7;
};



int AttentionStimSet::drawCurrent()
{
	if (!m_vecParams.empty())
		return drawCurrentAttParams();
	else
		return drawCurrentInterleaved();
}


int AttentionStimSet::drawCurrentAttParams()
{
	int status = 0;
	int page = vsgGetZoneDisplayPage(vsgVIDEOPAGE);

	cerr << "Drawing pages for trial " << m_current << endl;

	if (m_current >= m_vecParams.size())
	{
		cerr << "Error: m_current >= m_vecParams.size()" << endl;
		return 1;
	}

	// Set color of fixpt...
	m_fixpt.color = m_vecParams[m_current].color;

	// Stim page
	cerr << "Configure page " << m_pageStim << " pre-CC stim only" << endl;
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageStim, vsgBACKGROUND);
	draw_cue_points(m_vecParams.at(m_current).iOffBits);
	draw_stim_gratings(false, m_vecParams.at(m_current).iOffBits, m_vecParams.at(m_current).dInitialPhase, m_vecParams.at(m_current).contrastPairs);
	draw_cues(m_vecParams.at(m_current).iOffBits);
	draw_fixpt();
	
	// Stim page, this one with contrast change
	cerr << "Configure page " << m_pageChg << " post-CC stim only" << endl;
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageChg, vsgBACKGROUND);
	draw_cue_points(m_vecParams.at(m_current).iOffBits);
	draw_stim_gratings(true, m_vecParams.at(m_current).iOffBits, m_vecParams.at(m_current).dInitialPhase, m_vecParams.at(m_current).contrastPairs);
	draw_cues(m_vecParams.at(m_current).iOffBits);
	draw_fixpt();

	// plain fixpt page
	cerr << "Configure page " << m_pageFixpt << " fixpt and cues, no stim" << endl;
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixpt, vsgBACKGROUND);
	draw_cue_points(m_vecParams.at(m_current).iOffBits);
	draw_cues(m_vecParams.at(m_current).iOffBits);
	draw_fixpt();

	// distractor flashy pages, if any. For each configured flashy, there will be two additional
	// pages. One will be with the pre-CC stim, and one with the post-CC stim. 
	if (m_vecFlashies.size() > 0)
	{
		if (m_current >= m_vecFlashies.size())
		{
			cerr << "ERROR: Must have flashy configuration for each trial!" << endl;
		}
		else
		{
			cerr << "Configure pages for " << m_vecFlashies[m_current].size() << " flashies." << endl;
			for (unsigned int iflashy=0; iflashy<m_vecFlashies[m_current].size(); iflashy++)
			{
				const FlashyParams& params = m_vecFlashies[m_current].at(iflashy);

				// clear page for regular (pre-CC) stim and flashy
				cerr << "Configure page " << m_pageD + 2*iflashy << " for flashy " << iflashy << endl;
				vsgSetDrawPage(vsgVIDEOPAGE, m_pageD + 2*iflashy, vsgBACKGROUND);

				// draw stuff on this page
				draw_cue_points(m_vecParams.at(m_current).iOffBits);
				draw_stim_gratings(false, m_vecParams.at(m_current).iOffBits, m_vecParams.at(m_current).dInitialPhase, m_vecParams.at(m_current).contrastPairs);
				draw_cues(m_vecParams.at(m_current).iOffBits);
				draw_flashy(params);
				draw_fixpt();

				// Now make the page with the CC and the flashy
				cerr << "Configure page " << m_pageD + 2*iflashy + 1 << " for flashy " << iflashy << endl;
				vsgSetDrawPage(vsgVIDEOPAGE, m_pageD + 2*iflashy + 1, vsgBACKGROUND);

				// and draw stuff for CC+flashy page
				draw_cue_points(m_vecParams.at(m_current).iOffBits);
				draw_stim_gratings(true, m_vecParams.at(m_current).iOffBits, m_vecParams.at(m_current).dInitialPhase, m_vecParams.at(m_current).contrastPairs);
				draw_cues(m_vecParams.at(m_current).iOffBits);
				draw_flashy(params);
				draw_fixpt();

			}  // loop over flashies for this trial
		}	// 	if (m_vecFlashies.size() > m_current)
	}

	// blank page
	cerr << "Configure page " << m_pageBlank << " background only" << endl;
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, vsgBACKGROUND);

	vsgPresent();

	// Setup page cycling
	VSGCYCLEPAGEENTRY cycle[32767];



	// When there are no flashies, setting up the animation is simple and 
	// straightforward. 

	if (m_vecFlashies.size() == 0)
	{
		cycle[0].Frames = SECONDS_TO_FRAMES(m_vecParams[m_current].dTimeToCC);
		cycle[0].Page = m_pageStim + vsgTRIGGERPAGE;   // trigger at onset of stim
		cycle[0].Xpos = cycle[0].Ypos = 0;
		cycle[0].Stop = 0;
		cycle[1].Frames = SECONDS_TO_FRAMES(m_tMax);
		cycle[1].Page = m_pageChg + vsgTRIGGERPAGE;    // Always trigger on contrast change
		cycle[1].Xpos = cycle[1].Ypos = 0;
		cycle[1].Stop = 0;
		cycle[2].Frames = 1;
		cycle[2].Page = 0 + vsgTRIGGERPAGE;            // trigger when stim ends (blank screen onset)
		cycle[2].Xpos = cycle[2].Ypos = 0;
		cycle[2].Stop = 1;
		vsgPageCyclingSetup(3, &cycle[0]);


		cerr << "Cycling: Using 3 pages" << endl;
		for (int i=0; i<3; i++)
		{
			cerr << i << ": page=" << (cycle[i].Page & vsgTRIGGERPAGE ? cycle[i].Page-vsgTRIGGERPAGE : cycle[i].Page) << " Frames=" << cycle[i].Frames << endl;
		}


	}
	else
	{

		// Once more, with flashies
		int count = 0;	    // count of steps in animation, i.e. elements of cycle[]  
		unsigned int iflashy = 0;    // index of flashy in the flashies for current trial
		//double tconfig = 0; // time of the last frame configured
		int fconfig = 0;
		const double dTimeToCC = m_vecParams[m_current].dTimeToCC;    // convenience
		const int iFramesToCC = SECONDS_TO_FRAMES(m_vecParams[m_current].dTimeToCC);
		const int iFramesToTMax = SECONDS_TO_FRAMES(m_tMax);


		// Now iterate through each flashy in this trial. 
		// The var 'tconfig' is the time (measured from the stim onset, in sec) that the 
		// animation has been configured to. There are 'count' elements of the cycle[]
		// array configured; the last one cycle[count-1] takes us to time 'tconfig'.
		//
		// For each flashy, we will configure a "gap", where there is no flashy, and then 
		// the flashy itself. The "gap" may have no frames (i.e. there is really no gap).
		// We check the "gap" and the period of the flashy to see if the contrast change happens.
		//
		// WARNING: this is where the assumption of NO OVERLAP comes into play. We do NOT look at the 
		// start time of the NEXT flashy, which we'd do if we wanted to see if there were overlap.
		// We DO check whether the CC happens during the display of this flashy.
		//
		// The var m_pageD is the first page containing the flashies and stim/cues/etc.
		// The first flashy is on m_pageD with the pre-CC stim/cues/fixpt, and m_pageD+1
		// 
		// At each step in the loop,
		// tconfig = the amount of time that has been configured (i.e. the elements of cycle[] get us this far)
		// fconfig = number of frames that have been configured this far

		for (iflashy = 0; iflashy < m_vecFlashies.at(m_current).size(); iflashy++)
		{
			const struct flashy_params& flashyParams = m_vecFlashies.at(m_current).at(iflashy);
			int page = 0;
			int frames = 0;

			cerr << "flashy " << iflashy << " frames configured " << fconfig << " CC " << iFramesToCC << " ton " << flashyParams.fon << " foff " << flashyParams.foff << endl;

			// is there a gap between fconfig and the onset of the flashy? 
			// If so, then we will create an entry in cycle[] to display the stim page without a flashy.
			// Beware, the stim page may be with or without CC.

			//if (SECONDS_TO_FRAMES(flashyParams.ton - tconfig) > 0)
			if (flashyParams.fon > fconfig)
			{
				// There is a gap between tconfig and the onset time of the flashy.
				// If the flashy starts before the CC, then the gap is m_pageStim
				// If the flashy starts after the CC, and tconfig is at or after the CC, then the gap is m_pageChg
				// Otherwise, the gap encompasses the CC.
				//if (SECONDS_TO_FRAMES(dTimeToCC - flashyParams.ton) >= 0)
				if (iFramesToCC >= flashyParams.fon)
				{
					// flashy starts at or before the CC, so page is m_pageStim for the whole gap
					cycle[count].Frames = (WORD)(flashyParams.fon - fconfig);
					cycle[count].Page = m_pageStim;
					cycle[count].Xpos = cycle[count].Ypos = 0;
					cycle[count].Stop = 0;
					count++;
				}
				//else if (SECONDS_TO_FRAMES(tconfig - dTimeToCC) >= 0)
				else if (fconfig >= iFramesToCC)
				{
					// tconfig is beyond the CC, so page is m_pageChg for the whole gap
					//cycle[count].Frames = (WORD)SECONDS_TO_FRAMES(flashyParams.ton - tconfig);
					cycle[count].Frames = (WORD)(flashyParams.fon - fconfig);
					cycle[count].Page = m_pageChg + (fconfig == iFramesToCC ? vsgTRIGGERPAGE : 0);
					cycle[count].Xpos = cycle[count].Ypos = 0;
					cycle[count].Stop = 0;
					count++;
				}
				else
				{
					// tconfig is prior to the CC so the gap encompasses the CC
					// There is a special case where fconfig is now at the CC - then the entire gap is after.
					//cycle[count].Frames = (WORD)SECONDS_TO_FRAMES(dTimeToCC - tconfig);
					if (fconfig < iFramesToCC)
					{
						cycle[count].Frames = (WORD)(iFramesToCC - fconfig);
						cycle[count].Page = m_pageStim;
						cycle[count].Xpos = cycle[count].Ypos = 0;
						cycle[count].Stop = 0;
						count++;
					}

					//cycle[count].Frames = (WORD)SECONDS_TO_FRAMES(flashyParams.ton - dTimeToCC);
					cycle[count].Frames = (WORD)(flashyParams.fon - iFramesToCC);
					cycle[count].Page = m_pageChg + vsgTRIGGERPAGE;		// trigger at CC
					cycle[count].Xpos = cycle[count].Ypos = 0;
					cycle[count].Stop = 0;
					count++;
				}
				//tconfig = flashyParams.ton;
				//tconfig = get_fconfig(cycle, count);
				fconfig = flashyParams.fon;
			}
			
			// Now we configure the cycling for the flashy itself. 
			// tconfig is at the start of the flashy.
			// Similar to the gap case above, the possibilities are
			// Entire flashy is before the CC
			// Entire flashy is at or after the CC
			// The flashy spans the CC.
			
			//if (SECONDS_TO_FRAMES(dTimeToCC - flashyParams.toff) >= 0)
			if (iFramesToCC >= flashyParams.foff)
			{
				//cycle[count].Frames = (WORD)SECONDS_TO_FRAMES(flashyParams.toff - flashyParams.ton);
				cycle[count].Frames = (WORD)(flashyParams.foff - flashyParams.fon);
				cycle[count].Page = m_pageD + 2*iflashy + vsgTRIGGERPAGE;
				cycle[count].Xpos = cycle[count].Ypos = 0;
				cycle[count].Stop = 0;
				count++;
			}
			//else if (SECONDS_TO_FRAMES(flashyParams.ton - dTimeToCC) >= 0)
			else if (flashyParams.fon >= iFramesToCC)
			{
				//cycle[count].Frames = (WORD)SECONDS_TO_FRAMES(flashyParams.toff - flashyParams.ton);
				cycle[count].Frames = (WORD)(flashyParams.foff - flashyParams.fon);
				cycle[count].Page = m_pageD + 2*iflashy + 1 + vsgTRIGGERPAGE;
				cycle[count].Xpos = cycle[count].Ypos = 0;
				cycle[count].Stop = 0;
				count++;
			}
			else
			{
				cerr << "Spanning flashy at count " << count << " flashy " << flashyParams.fon << "-" << flashyParams.foff << ", CC " << iFramesToCC << endl;
				//cycle[count].Frames = (WORD)SECONDS_TO_FRAMES(dTimeToCC - flashyParams.ton);
				cycle[count].Frames = (WORD)(iFramesToCC - flashyParams.fon);
				cycle[count].Page = m_pageD + 2*iflashy + vsgTRIGGERPAGE;
				cycle[count].Xpos = cycle[count].Ypos = 0;
				cycle[count].Stop = 0;
				count++;
				//cycle[count].Frames = (WORD)SECONDS_TO_FRAMES(flashyParams.toff - dTimeToCC);
				cycle[count].Frames = (WORD)(flashyParams.foff - iFramesToCC);
				cycle[count].Page = m_pageD + 2*iflashy + 1 + vsgTRIGGERPAGE;
				cycle[count].Xpos = cycle[count].Ypos = 0;
				cycle[count].Stop = 0;
				count++;
			}
			//tconfig = flashyParams.toff;
			//tconfig = get_fconfig(cycle, count);
			fconfig = flashyParams.foff;
		}
				
		// After the loop there may be a gap between tconfig and tMax
		// The CC may fall in this gap!			

		//if (SECONDS_TO_FRAMES(tconfig - dTimeToCC) >= 0)
		if (fconfig >= iFramesToCC)
		{
			// entire gap is after the cc. Special case when the start of
			// this gap is at dTimeToCC -- have to issue trigger
			//cycle[count].Frames = (WORD)SECONDS_TO_FRAMES(m_tMax - (tconfig - dTimeToCC));
			cycle[count].Frames = iFramesToTMax - (fconfig - iFramesToCC);
			//cycle[count].Page = m_pageChg + ((SECONDS_TO_FRAMES(tconfig - dTimeToCC) == 0) ? vsgTRIGGERPAGE : 0);
			cycle[count].Page = m_pageChg + ((fconfig == iFramesToCC) ? vsgTRIGGERPAGE : 0);
			cycle[count].Xpos = cycle[count].Ypos = 0;
			cycle[count].Stop = 0;
			count++;
		}
		else
		{
			// split the gap
			//cycle[count].Frames = (WORD)SECONDS_TO_FRAMES(dTimeToCC - tconfig);
			cycle[count].Frames = (WORD)(iFramesToCC - fconfig);
			cycle[count].Page = m_pageStim;
			cycle[count].Xpos = cycle[count].Ypos = 0;
			cycle[count].Stop = 0;
			count++;
			//cycle[count].Frames = (WORD)SECONDS_TO_FRAMES(m_tMax);
			cycle[count].Frames = (WORD)(iFramesToTMax);
			cycle[count].Page = m_pageChg + vsgTRIGGERPAGE;
			cycle[count].Xpos = cycle[count].Ypos = 0;
			cycle[count].Stop = 0;
			count++;
		}
		cycle[count].Page = 0 + vsgTRIGGERPAGE;            // trigger when stim ends (blank screen onset)
		cycle[count].Xpos = cycle[count].Ypos = 0;
		cycle[count].Stop = 1;
		cycle[count].Frames = 1;
		count++;
		
		vsgPageCyclingSetup(count, &cycle[0]);

		cerr << "Cycling: Using " << count << " pages" << endl;
		for (int i=0; i<count; i++)
		{
			cerr << i << ": page=" << (cycle[i].Page & vsgTRIGGERPAGE ? cycle[i].Page-vsgTRIGGERPAGE : cycle[i].Page) << " Frames=" << cycle[i].Frames << endl;
		}
	}



	return status;
}




int AttentionStimSet::drawCurrentInterleaved()
{
	int status = 0;
	int page = vsgGetZoneDisplayPage(vsgVIDEOPAGE);

	cerr << "Drawing pages for trial " << m_current << endl;

	if (m_current >= m_vecInterleaved.size())
	{
		cerr << "Error: m_current >= m_vecInterleaved.size()" << endl;
		return 1;
	}

	// Set color of fixpt...
	m_fixpt.color = m_vecInterleaved[m_current].color;

	// plain fixpt page
	cerr << "Configure page " << m_pageFixpt << " fixpt only" << endl;
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixpt, vsgBACKGROUND);
	draw_fixpt();

	// fixpt + cue(s) page
	cerr << "Configure page " << m_pageFixptQ << " fixpt and cues" << endl;
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixptQ, vsgBACKGROUND);
	draw_cue_points(m_vecInterleaved.at(m_current).iOffBits);
	draw_cues(m_vecInterleaved.at(m_current).iOffBits);
	draw_fixpt();

	// Stim page, with cues
	cerr << "Configure page " << m_pageQStim << " stim & cues" << endl;
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageQStim, vsgBACKGROUND);
	draw_cue_points(m_vecInterleaved.at(m_current).iOffBits);
	draw_stim_gratings(false, m_vecInterleaved.at(m_current).iOffBits, m_vecInterleaved.at(m_current).dInitialPhase, m_vecInterleaved.at(m_current).contrastPairs);
	draw_cues(m_vecInterleaved.at(m_current).iOffBits);
	draw_fixpt();

	// StimCC page, with cues
	cerr << "Configure page " << m_pageQStimCC << " stimCC & cues" << endl;
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageQStimCC, vsgBACKGROUND);
	draw_cue_points(m_vecInterleaved.at(m_current).iOffBits);
	draw_stim_gratings(true, m_vecInterleaved.at(m_current).iOffBits, m_vecInterleaved.at(m_current).dInitialPhase, m_vecInterleaved.at(m_current).contrastPairs);
	draw_cues(m_vecInterleaved.at(m_current).iOffBits);
	draw_fixpt();

	// Stim page
	cerr << "Configure page " << m_pageStim << " pre-CC stim only" << endl;
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageStim, vsgBACKGROUND);
	draw_stim_gratings(false, m_vecInterleaved.at(m_current).iOffBits, m_vecInterleaved.at(m_current).dInitialPhase, m_vecInterleaved.at(m_current).contrastPairs);
	draw_fixpt();

	// Stim page, this one with contrast change
	cerr << "Configure page " << m_pageChg << " post-CC stim only" << endl;
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageChg, vsgBACKGROUND);
	draw_stim_gratings(true, m_vecInterleaved.at(m_current).iOffBits, m_vecInterleaved.at(m_current).dInitialPhase, m_vecInterleaved.at(m_current).contrastPairs);
	draw_fixpt();

	// blank page
	cerr << "Configure page " << m_pageBlank << " background only" << endl;
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, vsgBACKGROUND);

	vsgPresent();

	// Setup page cycling
	VSGCYCLEPAGEENTRY cycle[32767];
	vector<unsigned int> pages;
	pages.push_back(m_pageBlank);
	pages.push_back(m_pageFixpt);
	pages.push_back(m_pageFixptQ);
	pages.push_back(m_pageQStim);
	pages.push_back(m_pageQStimCC);
	pages.push_back(m_pageStim);
	pages.push_back(m_pageChg);

	vector<double> times(m_vecInterleaved.at(m_current).times, m_vecInterleaved.at(m_current).times+5);
	unsigned int nCycle = qCycle(cycle, times, pages);

	vsgPageCyclingSetup(nCycle, &cycle[0]);

	cerr << "Cycling: Using " << nCycle << " pages" << endl;
	for (unsigned int i = 0; i<nCycle; i++)
	{
		cerr << i << ": page=" << (cycle[i].Page & vsgTRIGGERPAGE ? cycle[i].Page - vsgTRIGGERPAGE : cycle[i].Page) << " Frames=" << cycle[i].Frames << endl;
	}

	// 0. pgBlank
	// 1. pgFix
	// 2. pgFixQ
	// 3. pgFixQStim
	// 4. pgFixQStimCC
	// 5. pgFixStim
	// 6. pgFixStimCC


	return status;
}

void AttentionStimSet::draw_stim_gratings(bool bIsCC, int iOffBits, double dInitialPhase, const std::vector<std::pair<int, int> >& contrastPairs)
{
	vector<alert::ARGratingSpec> *pvecGratings = (bIsCC ? &m_vecGratingsCC : &m_vecGratings);
	for (unsigned int i=0; i<pvecGratings->size(); i++)
	{
		// Check if this stim has an off bit set.
		if (iOffBits & (1 << i))
		{
			(*pvecGratings)[i].setContrast(0);
//			cerr << "draw_stim_gratings: stim is off." << endl;
		}
		else
		{
			(*pvecGratings)[i].setContrast(bIsCC ? contrastPairs[i].second : contrastPairs[i].first);
//			cerr << "draw_stim_gratings: contrast is " << (*pvecGratings)[i].contrast << endl;
		}
		(*pvecGratings)[i].setSpatialPhase(dInitialPhase);
		(*pvecGratings)[i].select();
		vsgObjResetDriftPhase();
		(*pvecGratings)[i].draw();
	}
	return;
}

void AttentionStimSet::draw_cues(int iOffBits)
{
	if (!m_bUseCueCircles) return;

	// Draw cue circles.
	// One for each grating, but the set of cues used are taken from 
	// (iOffBits & 0xff00) >> 8
	int iCueBase = (iOffBits & 0xff00) >> 8;

	//cout << "iCueBase: " << ios::showbase << ios::internal << ios::hex << iCueBase << ios::dec << endl;

	//cout << "There are " << m_vecCues.size() << " cues." << endl;
	for (unsigned int i=0; i<m_vecGratings.size(); i++)
	{
		//cout << "cue " << i << " (iOffBits & (1 << i)) " << (iOffBits & (1 << i)) << endl;
		// Check if this stim has an off bit set.
		if (iOffBits & (1 << i))
		{
			//cout << "Nothing to do." << endl;
			// do nothing
		}
		else
		{
			m_vecCues[iCueBase*m_vecGratings.size() + i].draw();
		}
	}

	return;
}


void AttentionStimSet::draw_cue_points(int iOffBits)
{
	if (!m_bUseCuePoints) return;

	// Draw cue points
	// One for each grating, but the set of cues used are taken from 
	// (iOffBits & 0xff00) >> 8
	int iCueBase = (iOffBits & 0xff00) >> 8;

	for (unsigned int i=0; i<m_vecGratings.size(); i++)
	{
		//cout << "cue " << i << " (iOffBits & (1 << i)) " << (iOffBits & (1 << i)) << endl;
		// Check if this stim has an off bit set.
		if (iOffBits & (1 << i))
		{
			//cout << "Nothing to do." << endl;
			// do nothing
		}
		else
		{
#ifdef USE_CUE_POINTS
			if (m_vecCuePoints.size() > iCueBase*m_vecGratings.size() + i)
			{
				cout << "draw cue point " << i << endl;
				m_vecCuePoints[iCueBase*m_vecGratings.size() + i].draw();
			}
#else
			if (m_vecCueRects.size() > iCueBase*m_vecGratings.size() + i)
			{
				cout << "draw cue rect " << i << endl;
				m_vecCueRects[iCueBase*m_vecGratings.size() + i].draw();
			}
#endif
		}
	}

	return;
}



void AttentionStimSet::draw_fixpt()
{
	m_fixpt.draw();
}

void AttentionStimSet::draw_flashy(const FlashyParams& params)
{
	// set x,y,w,h
	m_vecDistractors[params.nk].x = params.x;
	m_vecDistractors[params.nk].y = params.y;
	m_vecDistractors[params.nk].h = params.h;
	m_vecDistractors[params.nk].w = params.w;

	// and draw
	m_vecDistractors[params.nk].draw();

	// now get outta here ya knucklehead
	return;
}


int AttentionStimSet::handle_trigger(const std::string& s, const std::string&)
{
	int status = 0;

	if (s == "F")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixpt, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "S")
	{
		double dInitialPhase;
		if (m_vecInterleaved.empty())
			dInitialPhase = m_vecParams[m_current].dInitialPhase;
		else
			dInitialPhase = m_vecInterleaved[m_current].dInitialPhase;
		for (unsigned int i=0; i<m_vecGratings.size(); i++)
		{
			m_vecGratings[i].select();
			vsgObjSetSpatialPhase(dInitialPhase);
			vsgObjResetDriftPhase();
			m_vecGratingsCC[i].select();
			vsgObjSetSpatialPhase(dInitialPhase);
			vsgObjResetDriftPhase();
		}
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "a")
	{
		m_current++;
		if (m_vecInterleaved.empty())
		{
			if (m_current == m_vecParams.size())
			{
				m_current = 0;
			}
		}
		else
		{
			if (m_current == m_vecInterleaved.size())
			{
				m_current = 0;
			}
		}
		drawCurrent();
	}
	else if (s == "X")
	{
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "A")
	{
		int ipage;
		string stmp;
		cout << "Enter page: ";
		cin >> ipage;
		vsgSetDrawPage(vsgVIDEOPAGE, ipage, vsgNOCLEAR);
		status = 1;
	}		

	return status;
}


std::string AttentionStimSet::toString() const
{
	return string("not implemented");
}
