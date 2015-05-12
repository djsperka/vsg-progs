/* $Id: CMouseUStim.h,v 1.1 2015-05-12 17:26:59 devel Exp $*/

#ifndef _CMOUSEUSTIM_H_
#define _CMOUSEUSTIM_H_

#include "UStim.h"
#include "alertlib.h"
#include "AlertUtil.h"

// Implementation of UStim interface for the cmouse app.

class CMouseUStim: public UStim, public prargs_handler
{
public:
	CMouseUStim();
	virtual ~CMouseUStim() {};

	bool parse(int argc, char **argv);
	void run_stim(alert::ARvsg& vsg);
	int process_arg(int option, std::string& arg);

	// This acts as the callback function for the triggers
	int callback(int &output, const FunctorCallbackTrigger* ptrig);

private:
	int m_screenDistanceMM;
	COLOR_TYPE m_background;
	bool m_binaryTriggers;
	bool m_verbose;
	ARGratingSpec m_grating;
	ARContrastFixationPointSpec m_fixpt;
	bool m_alert;
	int m_pulse;
	int m_sleepMS;
	bool m_bFixationOn;
	bool m_bUseRegDump;	// use a file to save stimulus parameters
	std::string m_sRegDumpFile;

	double m_vsgWidthPixels;
	double m_vsgHeightPixels;
	double m_vsgWidthDegrees;
	double m_vsgHeightDegrees;
	double m_monWidthPixels;
	double m_monHeightPixels;
	double m_tfPrevious;



// These are the args allowed and which are handled by prargs. Do not use 'F' - it is reserved for 
// passing a command file.
	static const string m_allowedArgs;

	void mousePixelsToVSGPixels(int pixMouseX, int pixMouseY, double* pvsgPixelsX, double* pvsgPixelsY);
	void vsgPixelsToVSGDrawDegrees(int vsgPixX, int vsgPixY, double *pvsgDegX, double *pvsgDegY);
	void mousePosToVSGDrawDegrees(int pixMouseX, int pixMouseY, double* pvsgDegX, double* pvsgDegY);
	void vsgDrawDegreesToMousePos(double vsgDegX, double vsgDegY, long* pixMouseX, long* pixMouseY);
	int init_screen_params();
	void init_triggers(TSpecificFunctor<CMouseUStim>* pfunctor);
	void overlay(bool bFixationOn, double fixX, double fixY, double fixD, double apertureX, double apertureY, double apertureDiameter);
	int init_pages();


};

#endif
