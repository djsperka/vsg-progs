/* $Id: CMouseUStim.h,v 1.2 2016-05-18 19:36:46 devel Exp $*/

#ifndef _CMOUSEUSTIM_H_
#define _CMOUSEUSTIM_H_

#include "UStim.h"
#include "alertlib.h"
#include "AlertUtil.h"

// for interacting with argp 
struct cmouse_arguments
{
	int screenDistanceMM;
	bool bHaveDistance;
	COLOR_TYPE bkgdColor;
	bool useBinaryTriggers;
	bool verbose;
	ARGratingSpec grating;
	bool bHaveGrating;
	ARContrastFixationPointSpec fixpt;
	bool bHaveFixpt;
	bool isAlert;
	bool allowQuit;
	int pulse;
	int sleepMS;
	bool useRegDump;	// use a file to save stimulus parameters
	std::string sRegDumpFile;
	int portClient;		// if mouse control is false, this is the port number to listen for a client	
	bool bMouseControl;
	bool bPromptForDistance;
	cmouse_arguments()
		: screenDistanceMM(0)
		, bHaveDistance(false)
		, bkgdColor(gray)
		, useBinaryTriggers(false)
		, verbose(false)
		, grating()
		, bHaveGrating(false)
		, fixpt()
		, bHaveFixpt(false)
		, isAlert(false)
		, allowQuit(false)
		, pulse(0x40)
		, sleepMS(0)
		, useRegDump(false)
		, sRegDumpFile("")
		, portClient(0)
		, bMouseControl(true)
		, bPromptForDistance(false)
	{};

};


// Implementation of UStim interface for the cmouse app.

class CMouseUStim: public UStim
{
public:
	CMouseUStim();
	virtual ~CMouseUStim() {};

	bool parse(int argc, char **argv);
	void run_stim(alert::ARvsg& vsg);
	//int process_arg(int option, std::string& arg);

	// This acts as the callback function for the triggers
	int callback(int &output, const FunctorCallbackTrigger* ptrig, const std::string&);

private:
	struct cmouse_arguments m_arguments;
	//int m_screenDistanceMM;
	//COLOR_TYPE m_background;
	//bool m_binaryTriggers;
	//bool m_verbose;
	//ARGratingSpec m_grating;
	//ARContrastFixationPointSpec m_fixpt;
	//bool m_bHaveFixpt;
	//bool m_alert;
	//bool m_allowq;
	//int m_pulse;
	//int m_sleepMS;
	//bool m_bUseRegDump;	// use a file to save stimulus parameters
	//std::string m_sRegDumpFile;
	//bool m_bMouseControl;	// if true (default), use mouse/keyboard to control aperture and grating parameters
	//int m_portClient;		// if mouse control is false, this is the port number to listen for a client	
	bool m_bFixationOn;
	double m_vsgWidthPixels;
	double m_vsgHeightPixels;
	double m_vsgWidthDegrees;
	double m_vsgHeightDegrees;
	double m_monWidthPixels;
	double m_monHeightPixels;
	double m_tfPrevious;
	int m_gratingPage;			// for page flipping
	int m_overlayPage;			// for page flipping the overlay



// These are the args allowed and which are handled by prargs. Do not use 'F' - it is reserved for 
// passing a command file.
	static const string m_allowedArgs;

	void joystickPositionToVSGDrawDegrees(double joyX, double joyY, double *pvsgDegX, double *pvsgDegY);
	void mousePixelsToVSGPixels(int pixMouseX, int pixMouseY, double* pvsgPixelsX, double* pvsgPixelsY);
	void vsgPixelsToVSGDrawDegrees(int vsgPixX, int vsgPixY, double *pvsgDegX, double *pvsgDegY);
	void mousePosToVSGDrawDegrees(int pixMouseX, int pixMouseY, double* pvsgDegX, double* pvsgDegY);
	void vsgDrawDegreesToMousePos(double vsgDegX, double vsgDegY, long* pixMouseX, long* pixMouseY);
	int init_screen_params();
	void init_triggers(TSpecificFunctor<CMouseUStim>* pfunctor);
	void overlay(bool bFixationOn, double fixX, double fixY, double fixD, double apertureX, double apertureY, double apertureDiameter);
	int init_pages();
	void doMouseKBLoop();
	void doJSClientLoop();
	void updateSF(double sf);
	void updateTF(double tf);
	void updateContrast(int contrast);
	void updateOrientation(double ori);
	void updateGrating();
	void flip_draw_grating();
	void flip_draw_overlay(bool bFixationOn, double fixX, double fixY, double fixD, double apertureX, double apertureY, double apertureDiameter);

};

#endif
