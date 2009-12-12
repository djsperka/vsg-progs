#define _CRT_SECURE_NO_WARNINGS
#include "AustinUtilities.h"

using namespace std;

CString TrimLine(char input[100])
{
	CString output;
	int pos;

	output=input;
	pos=output.Find("\n");
	if(pos>0) output=output.Left(pos);
	output.TrimRight();

	return output;
}


CString GetParameterFileName(void)
{
	FILE *CurrentExperiment, *LastFile;
	char raw[100];
	CString path,LastFileName,ParameterFileName;

	//Determine the current experiment, return an error if unable to do so.
	if( (CurrentExperiment  = fopen( "c:\\AlertRig\\Experiments\\CurrentExperiment.par", "r" )) == NULL )
	{
		cout << "Could not determine current experiment" << "\n";
		return "";
	}
	else
	{
	//read the CurrentExperiment.par file, return an error if its less than 2 characters.
	//Less than 2 is a fairly arbitrary error condition, but it's certainly impossible
	//to have a valid CurrentExperiment.par file with less than 2 characters in the first
	//line
		fgets(raw,100,CurrentExperiment);
		fclose(CurrentExperiment);
		path=TrimLine(raw);
		if (path.GetLength()<2)
		{
			cout << "Invalid CurrentExperiment.par file" << "\n";
			return "";
		}

		LastFileName=path+"\\LastFile";
		if( (LastFile = fopen(LastFileName,"r")) == NULL )
		{
			cout << "Could not open LastFile" << "\n";
			return "";
		}
		fgets(raw,100,LastFile);
		fclose(LastFile);
		ParameterFileName=TrimLine(raw);
		if (ParameterFileName.GetLength()<2)
		{
			cout << "Invalid LastFile entry" << "\n";
			return "";
		}
		else
		{
			ParameterFileName=path+"\\DataFiles\\"+ParameterFileName+".par";
			return ParameterFileName;
		}
	}
}

int GetDistanceToScreen(void)
{	
	FILE *CurrentExperiment, *ExperimentInfo;
	char raw[100];
	CString path,ExperimentInfoName,line;
	int DistanceToScreen;

	//Determine the current experiment, return an error if unable to do so.
	if( (CurrentExperiment  = fopen( "c:\\AlertRig\\Experiments\\CurrentExperiment.par", "r" )) == NULL )
	{
		cout << "Could not determine current experiment" << "\n";
		return -1;
	}
	else
	{
	//read the CurrentExperiment.par file, return an error if its less than 2 characters.
	//Less than 2 is a fairly arbitrary error condition, but it's certainly impossible
	//to have a valid CurrentExperiment.par file with less than 2 characters in the first
	//line
		fgets(raw,100,CurrentExperiment);
		fclose(CurrentExperiment);
		path=TrimLine(raw);
		if (path.GetLength()<2)
		{
			cout << "Invalid CurrentExperiment.par file" << "\n";
			return -1;
		}

		ExperimentInfoName=path+"\\Experiment.par";
		if( (ExperimentInfo = fopen(ExperimentInfoName,"r")) == NULL )
		{
			cout << "Could not open Experiment.par" << "\n";
			return -1;
		}
		fgets(raw,100,ExperimentInfo);
		line=TrimLine(raw);
		DistanceToScreen=atoi(line);
		if(DistanceToScreen < 2) 
		{
			cout << "Invalid Experiment.par file" << "\n";
			return -1;
		}
		return DistanceToScreen;
	}
}


int InitializeVSG(void)
{
	int DistanceToScreen;
	if( (DistanceToScreen=GetDistanceToScreen()) < 1) 
	{
		cout << "Could not determine distance to screen" << "\n";
		return -1;
	}

	if( vsgInit("") < 0) return -1;
	vsgSetViewDistMM(DistanceToScreen);
	vsgSetSpatialUnits(vsgDEGREEUNIT);
	vsgSetColourSpace(vsgCS_RGB);
	return 1;
}


void SetupOverlay(double FixationX,double FixationY,double FixationDiameter, CString &FixationColor)
{
	VSGLUTBUFFER OverlayBuffer;
	VSGTRIVAL Background,Fixation;

	Background.a=.5; Background.b=.5; Background.c=.5;

	// Default fixation color is red. 
	if (FixationColor == "Green") {
		Fixation.a=0; Fixation.b=1; Fixation.c=0;
	}
	else if (FixationColor == "Blue") {
		Fixation.a=0; Fixation.b=0; Fixation.c=1;
	}
	else {
		Fixation.a=1; Fixation.b=0; Fixation.c=0;
	}

	vsgSetCommand(vsgOVERLAYMASKMODE);
	OverlayBuffer[1]=Background;
	OverlayBuffer[2]=Fixation;
	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&OverlayBuffer, 0, 4);
	vsgSetDrawPage(vsgOVERLAYPAGE,0,1);
	vsgSetDrawPage(vsgOVERLAYPAGE,1,1);
	vsgSetPen1(2);
	vsgDrawOval(FixationX,FixationY,FixationDiameter,FixationDiameter);
}

 




