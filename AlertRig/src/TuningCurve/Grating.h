// Grating.h: interface for the Grating class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRATING_H__4DAA3A10_7DE5_4892_A62D_E71939732A63__INCLUDED_)
#define AFX_GRATING_H__4DAA3A10_7DE5_4892_A62D_E71939732A63__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <afxwin.h>
//#include <iostream.h>
#include "VSGEX2.H"
#include "ParameterFile.h"

class Grating  
{
public:
	double GetBlankDuration();
	double GetSignalDuration();
	void ShowGrating();
	void InitGrating();
	void DrawGrating(double FixationX,double FixationY,double FixationDiameter);
	int NextGrating();
	int ReadParameterFile(CString ParameterFileName);
	void setParameterFile(ParameterFile* pf) { p = pf; }
	Grating();
	virtual ~Grating();

private:
	VSGOBJHANDLE Handle;
	int CurrentIndex;
	ParameterFile *p;
/*
	CString ExperimentType;
	CString BackgroundColor, Pattern, ApertureType;
	double Contrast, Orientation, TemporalFrequency, SpatialFrequency;
	double ApertureWidth, ApertureHeight, ApertureX, ApertureY;
	double SignalDuration, BlankDuration;
	double TunedParameter[1000];
	int NumberOfSteps, NumberOfRepeats;
*/
};

#endif // !defined(AFX_GRATING_H__4DAA3A10_7DE5_4892_A62D_E71939732A63__INCLUDED_)
