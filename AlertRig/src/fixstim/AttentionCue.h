#pragma once

#include <vector>
#include <string>
#include <Alertlib.h>

// Struct for specifying cues (dots/rect) in attention
typedef struct attention_cue
{
	double rdiff;
	int linewidth;
	COLOR_TYPE color;
	double fixpt_d;
} AttentionCue;

int parse_attcues(const std::string& s, int nstim, std::vector<AttentionCue>& vecCues);
void dump_attcues(const std::vector<AttentionCue>& vecCues);
