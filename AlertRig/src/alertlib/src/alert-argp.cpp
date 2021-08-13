#pragma once

#include "alert-argp.h"
#include "Alertlib.h"
#include <vector>
#include <string>

error_t alert_argp_parse(const struct argp* argp, int argc, char** argv, unsigned flags, int* arg_index, void* input, int response_file_char)
{
	char* local_argv[2048];
	int local_argc = 0;
	std::string s;
	std::vector<std::string> vecArgs;
	int i;

	// Tokenize command line args.
	for (i = 0; i < argc; i++)
	{
		// Check for response_file_char -- don't put response file arg into the vector. 
		if (response_file_char &&
			argv[i][0] == '-' && argv[i][1] == response_file_char)
		{
			// the next arg [i+1] is the response filename.
			// tokenize response_file will read that file and append tokens
			// to vecArgs

			// It is an error to have nested response file args!

			if (tokenize_response_file(argv[i + 1], vecArgs) < 0)
			{
				return -1;
			}

			i += 1;
		}
		else
		{
			s.assign(argv[i]);
			vecArgs.push_back(s);
		}
	}

	// Now all tokens are in vecArgs. Make argc and argv....
	make_argv(vecArgs, local_argc, local_argv);

	return argp_parse(argp, local_argc, local_argv, flags, arg_index, input);
}

