#if !defined(__PARAMETERFILE_H_)
#define __PARAMETERFILE_H_

#include <afx.h>

class ParameterFile
{
public:
	ParameterFile(CString ParameterFileName);

	int CurrentIndex;
	CString ExperimentType;		// Current experiment type
	CString BackgroundColor;	// background color
	CString Pattern;			// 
	CString ApertureType;		// 
	CString GratingColor;		//
	CString FixationColor;		// red, green or blue
	double Contrast;
	double Orientation;
	double TemporalFrequency;
	double SpatialFrequency;
	double ApertureWidth;
	double ApertureHeight;
	double ApertureX;
	double ApertureY;
	double SignalDuration;
	double BlankDuration;
	double TunedParameter[1000];
	int NumberOfSteps;
	int NumberOfRepeats;
};


#endif