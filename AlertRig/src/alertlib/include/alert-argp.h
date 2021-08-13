#pragma once

#include "argp.h"


// wrapper around argp - the first 5 args are as they are passed to argp -- except that a response file may be specified with a particular character, 
// e.g. response_file_char='F' means you can pass a "-F filename" and fill the file 'filename' with arguments. Helpful if you have a lot, more than windows
// allows. 
//
// Anyways, a first pass is taken through the args looking for a response file. If one is found, the option and arg are removed from the args, and
// the contents of the file is added to the args. 
error_t alert_argp_parse(const struct argp* argp, int argc, char** argv, unsigned flags, int* arg_index, void* input, int response_file_char = 0);
