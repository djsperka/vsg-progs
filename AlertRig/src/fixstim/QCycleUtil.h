#pragma once

#include <VSGV8.h>
#include <vector>
#include <algorithm>


// times (seconds) should be in this order.
// Prior to tQ0, pgFixpt is displayed.
//
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


unsigned int qCycle(VSGCYCLEPAGEENTRY* cycle, std::vector<double>& times, std::vector<unsigned int>& pages);