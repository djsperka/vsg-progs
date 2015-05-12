/* $Id: CalibrationUStim.h,v 1.1 2015-05-12 17:26:59 devel Exp $*/

#ifndef _CALIBRATIONUSTIM_H_
#define _CALIBRATIONUSTIM_H_

#include "UStim.h"
#include "alertlib.h"
#include "AlertUtil.h"
#include <string>

using namespace alert;
using namespace std;

class CalibrationUStim: public UStim, public prargs_handler
{
public:
	CalibrationUStim();
	virtual ~CalibrationUStim() {};

	bool parse(int argc, char **argv);
	void run_stim(alert::ARvsg& vsg);
	int process_arg(int option, std::string& arg);

	// This acts as the callback function for the triggers
	int callback(int &output, const FunctorCallbackTrigger* ptrig);

private:
	bool m_verbose;
	int m_pulse;
	ARContrastFixationPointSpec m_fixpt;
	ARContrastFixationPointSpec m_fixpts[9];
	COLOR_TYPE m_background;
	int m_screenDistanceMM;
	long m_lComPort;
	double m_dCalibrationOffset;
	string m_szConfigFile;

	static const string m_allowedArgs;


	int init_calibration();
	void init_triggers(TSpecificFunctor<CalibrationUStim>* pfunctor);
};

#endif
