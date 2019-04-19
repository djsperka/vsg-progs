#ifndef _MSEQUENCEUSTIM_H_
#define _MSEQUENCEUSTIM_H_

#include "UStim.h"
#include "alertlib.h"
#include "AlertUtil.h"
#include <string>

#define BLANK_PAGE 0
#define STIM_PAGE 1

#define BIG_APERTURE_PAGE 2
#define NO_APERTURE_PAGE 1
#define APERTURE_PAGE 0



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


class MSequenceUStim : public UStim, public prargs_handler
{
public:
	MSequenceUStim();
	virtual ~MSequenceUStim();

	bool parse(int argc, char **argv);
	void run_stim(alert::ARvsg& vsg);
	int process_arg(int option, std::string& arg);

	// This acts as the callback function for the triggers
	int callback(int &output, const FunctorCallbackTrigger* ptrig);

private:

	// These are the args allowed and which are handled by prargs. Do not use 'F' - it is reserved for 
	// passing a command file.
	static const string m_allowedArgs;

	bool m_bSuperHack;
	bool m_bTesting;
	bool m_bTriggered;
	bool m_binaryTriggers;
	int m_nRepeats;
	int m_iapX, m_iapY;
	int m_iapXCorner, m_iapYCorner;
	string m_sFilename;
	int m_iOrder;
	int m_iDot;
	int m_iRows, m_iCols;
	bool m_verbose;
	int m_iFramesPerTerm;
	char *m_sequence;
	int m_nterms;
	int m_errflg;
	int m_pulse;
	int m_iFirst;
	int m_iLast;

	void init_triggers(TSpecificFunctor<MSequenceUStim>* pfunctor);
	int load_mseq(string& filename);
	void blank_page();
	void prepare_overlay();
	int draw_mseq(const char *seq, int order, int r, int c, int d);
	void prepare_cycling();
	void testing_loop();
	const char *get_configured_seq();

};

#endif