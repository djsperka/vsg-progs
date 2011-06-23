#include <string>

class RepeatingSpec
{
	int _uspf;			// microseconds per frame
	int _iRepeats;		// number of repetitions
	int _iFirst;		// first frame
	int _iLast;			// last frame
	int _iFramesPerTerm;// frames per term. Duh.

public:
	RepeatingSpec(long uspf) : _uspf(uspf) {};
	~RepeatingSpec() {};
	int getFirst() { return _iFirst; };
	int getLast() { return _iLast; };
	int getRepeats() { return _iRepeats; };
	int getFramesPerTerm() { return _iFramesPerTerm; };

	// total ms for all repeats
	long getTimeUS() { return ((_iLast - _iFirst + 1) * _iRepeats * _iFramesPerTerm * _uspf); };


	friend int parse_repeating(std::string &s, RepeatingSpec &r);
};

int args(int argc, char **argv);
int parse_repeating(std::string &s, RepeatingSpec &r);
void usage();
int load_mseq(std::string& filename);
int getZoomFactor(int idot, int& zoom);

