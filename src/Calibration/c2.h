// $Header: /opt/cvstmp/AlertRig/src/Calibration/Attic/c2.h,v 1.1 2004-09-29 20:13:35 dan Exp $

int init(double i_dDist, double i_dDiam, CString *i_psBackground, CString *i_psStim, double i_dPctCoverage);
int drawPages(int i_iBackground, double i_dDiameter, double i_dPctCoverage);
int getPixelRange(double *i_pXMax, double *i_pYMax);
void setLevel(unsigned long i_ulBits, int *i_piVert, int *i_piHoriz);
int setStimulus(unsigned long i_ulState, int i_iVert, int i_iHoriz);



// Constants for triggering, communication with Spike2
#define PRES_XMASK	0x18
#define PRES_YMASK	0x6
#define XCOORD(v)	((v&PRES_XMASK)>>3)
#define YCOORD(v)	((v&PRES_YMASK)>>1)

#define PRES_TOP	0x8
#define PRES_MIDDLE 0x18
#define PRES_BOTTOM 0x10
#define PRES_LEFT	0x4
#define PRES_CENTER 0x6
#define PRES_RIGHT	0x2
#define PRES_LEVEL	(PRES_TOP|PRES_MIDDLE|PRES_BOTTOM|PRES_LEFT|PRES_CENTER|PRES_RIGHT)

// These are set by the ced -- indicators for stim to be
// turned on or off. 
#define PRES_ON		0x20
#define PRES_OFF	0x40
#define PRES_STATE	(PRES_ON|PRES_OFF)
#define PRES_STATE_UNKNOWN 0x0

// These are set by the stim -- used to indicate to the ced/spike2 that
// the stimulus has been turned on or off 
#define STIMULUS_ON	0x1
#define STIMULUS_OFF 0x0

