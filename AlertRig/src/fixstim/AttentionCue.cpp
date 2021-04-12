#include "AttentionCue.h"

using namespace std;

// djs - number of stim is ignored because its hard with images. Beware!

int parse_attcues(const string& s, int, vector<AttentionCue>& vecCues)
{
	COLOR_TYPE color;
	double rdiff;
	int linewidth;
	unsigned int i;
	vector<string> tokens;
	istringstream iss;
	AttentionCue acue;
	tokenize(s, tokens, ",");
	if (tokens.size() % 3 != 0)
	{
		cerr << "Warning! Expect attention cue parameters to come in groups of 3. Found " << tokens.size() << " parameters." << endl;
	}

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
