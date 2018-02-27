#include "QCycleUtil.h"
#include <iostream>


// 0. tQ0 - onset of pgFixQ
// 1. tQ1 - offset of queue. Target page depends on value of tS.
// 2. tS - onset of stim (target is either pgFixStim or pgFixQStim)
// 3. tCC - time of stim transition to stimCC
// 4. tE - time to end, target pgBlank
//
// pages should be in this order. Some may not be used. It'd be OK for those pages to 
// be uninitialized in that case, if you have the guts to do that. 
// 
// 0. pgBlank
// 1. pgFix
// 2. pgFixQ
// 3. pgFixQStim
// 4. pgFixQStimCC
// 5. pgFixStim
// 6. pgFixStimCC

#define SECONDS_TO_FRAMES(t) (unsigned int)(t * 1000000.0 / vsgGetSystemAttribute(vsgFRAMETIME))

unsigned int qCycle(VSGCYCLEPAGEENTRY * cycle, std::vector<double>& times, std::vector<unsigned int>& pages)
{
	// check input
	if (times.size() != 5)
	{
		std::cerr << "Incorrect number of times passed to qCycle() - expected 5, got " << times.size() << std::endl;
		return 0;
	}

	if (pages.size() != 7)
	{
		std::cerr << "Incorrect number of pages passed to qCycle() - expect 7, got " << pages.size() << std::endl;
		return 0;
	}

	unsigned int nQ0, nQ1, nS, nCC, nE;

	nQ0 = SECONDS_TO_FRAMES(times[0]);
	if (times[0] < 0)
	{
		std::cerr << "tQ0 is negative." << std::endl;
		return 0;
	}

	nQ1 = SECONDS_TO_FRAMES(times[1]);
	if (nQ1 <= nQ0)
	{
		std::cerr << "tQ1 must be greater than tQ0." << std::endl;
		return 0;
	}

	nS = SECONDS_TO_FRAMES(times[2]);
	if (nS <= nQ0)
	{
		std::cerr << "nS must be greater than nQ0" << std::endl;
		return 0;
	}

	nCC = SECONDS_TO_FRAMES(times[3]);
	if (nCC <= nS)
	{
		std::cerr << "nCC must be greater than nS" << std::endl;
		return 0;
	}

	nE = SECONDS_TO_FRAMES(times[4]);
	if (nE <= nCC)
	{
		std::cerr << "nE must be greater than nCC" << std::endl;
		return 0;
	}

	unsigned int iCycle = 0;


	// 0. pgBlank
	// 1. pgFix
	// 2. pgFixQ
	// 3. pgFixQStim
	// 4. pgFixQStimCC
	// 5. pgFixStim
	// 6. pgFixStimCC


	// Now which of the three scenarios are we in?
	if (nQ1 < nS)
	{
		// scenario 1, cue on and off prior to stim

		// fixpt only until onset of cue
		cycle[iCycle].Frames = nQ0;
		cycle[iCycle].Page = pages[1] + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

		// fixpt+cue
		cycle[iCycle].Frames = nQ1 - nQ0;
		cycle[iCycle].Page = pages[2] + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

		// cue off, fixpt only
		cycle[iCycle].Frames = nS - nQ1;
		cycle[iCycle].Page = pages[1] + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

		// stim on 
		cycle[iCycle].Frames = nCC - nS;
		cycle[iCycle].Page = pages[5] + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

		// CC
		cycle[iCycle].Frames = nE - nCC;
		cycle[iCycle].Page = pages[6] + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

		// all off
		cycle[iCycle].Frames = 1;
		cycle[iCycle].Page = pages[0] + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 1;
		iCycle++;
	}
	else if (nQ1 == nS)
	{
		// scenario 2, cue off at same time stim appears

		// fixpt only until onset of cue
		cycle[iCycle].Frames = nQ0;
		cycle[iCycle].Page = pages[1] + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

		// fixpt+cue
		cycle[iCycle].Frames = nQ1 - nQ0;
		cycle[iCycle].Page = pages[2] + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

		// stim on - note nQ1 == nS in this case. nCC-nS is the same as nCC-nQ1
		cycle[iCycle].Frames = nCC - nS;
		cycle[iCycle].Page = pages[5] + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

		// CC
		cycle[iCycle].Frames = nE - nCC;
		cycle[iCycle].Page = pages[6] + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

		// all off
		cycle[iCycle].Frames = 1;
		cycle[iCycle].Page = pages[0] + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 1;
		iCycle++;
	}
	else
	{
		// scenario 3, cue straddles onset of stim. 

		// fixpt only until onset of cue
		cycle[iCycle].Frames = nQ0;
		cycle[iCycle].Page = pages[1] + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

		// fixpt+cue
		cycle[iCycle].Frames = nS - nQ0;
		cycle[iCycle].Page = pages[2] + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

		// fixpt+cue+stim
		cycle[iCycle].Frames = nQ1 - nS;
		cycle[iCycle].Page = pages[3] + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

		// cue off, stim still on 
		cycle[iCycle].Frames = nCC - nQ1;
		cycle[iCycle].Page = pages[5] + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

		// CC
		cycle[iCycle].Frames = nE - nCC;
		cycle[iCycle].Page = pages[6] + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 0;
		iCycle++;

		// all off
		cycle[iCycle].Frames = 1;
		cycle[iCycle].Page = pages[0] + vsgTRIGGERPAGE;
		cycle[iCycle].Xpos = cycle[iCycle].Ypos = 0;
		cycle[iCycle].Stop = 1;
		iCycle++;
	}
	return iCycle;
}
