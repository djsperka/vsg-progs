#include "AttentionCue.h"

using namespace std;

int parse_attcues(const string& s, int nstim, vector<AttentionCue>& vecCues)
{
	COLOR_TYPE color;
	double rdiff;
	int linewidth;
	unsigned int i;
	vector<string> tokens;
	istringstream iss;
	AttentionCue acue;
	tokenize(s, tokens, ",");
	if (tokens.size() % (nstim * 3) == 0)
	{
		for (i = 0; i<tokens.size() / 3; i++)
		{

			// expect rdiff,linewidth,color
			// rdiff is difference between radius of circle and corresponding grating
			// linewidth is width of circle outline, in pixels (must be integer)

			if (parse_double(tokens[i * 3], rdiff) || parse_integer(tokens[i * 3 + 1], linewidth) || parse_color(tokens[i * 3 + 2], color))
			{
				cerr << "Error reading attention cues (" << s << ")" << endl;
				return 1;
			}
			acue.rdiff = rdiff;
			acue.linewidth = linewidth;
			acue.color = color;
			vecCues.push_back(acue);

		}
	}
	else
	{
		cerr << "Error reading attention cues (" << s << ") Expecting a multiple of " << nstim * 3 << " args, 3 for each stim." << endl;
		return 1;
	}
	return 0;
}


void my_dump_attcues(const AttentionCue& acue)
{
	cerr << "Q: rdiff=" << acue.rdiff << ", linewidth=" << acue.linewidth << ", color=" << acue.color << endl;
}

void dump_attcues(const vector<AttentionCue>& vecCues)
{
	for_each(vecCues.begin(), vecCues.end(), my_dump_attcues);
}
