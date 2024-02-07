#pragma once

#include "ConteUStim.h"
#include "alert-argp.h"

// callback for arg parser
error_t parse_conte_opt(int key, char* carg, struct argp_state* state);

// parse trials file
bool parse_trials_file(const std::string& filename, conte_trial_list_t& trials);

// parse xy file
bool parse_dot_supply_file(const std::string& filename, ConteCueDotSupply& dotsupply);

// input stuff
std::istream& operator>>(std::istream& in, conte_stim_params_t& stim);
bool parse_conte_stim_params(const std::vector<string>& tokens, unsigned int first, conte_stim_params_t& stim);
