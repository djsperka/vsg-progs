#pragma comment(lib, "vsgv8.lib")
#include "TuningCurve.h"
#include "ParameterFile.h"
using namespace std;

ParameterFile *pf=NULL;

void main(int ArgumentCount, char *Arguments[])
{
	double FixationX, FixationY, FixationDiameter;
	CString ParameterFileName;
	Grating Stimulus;
	FixationX=atof(Arguments[1]);
	FixationY=-atof(Arguments[2]);
	FixationDiameter=atof(Arguments[3]);

	int DigitalInputState;
	int index=0,PreviousStimulusState=0,StimulusState,NextStimulus,PreviousNextStimulus=0;
	int FixationState=0,PreviousFixationState=0;
	//Determine current ParameterFile name
	ParameterFileName=GetParameterFileName();
	cout << ParameterFileName << endl;
	if( strcmp(ParameterFileName,"")==0 )
	{
		cout << "Could not determine current parameter file." << "\n";
		_exit(0);
	}


	// djs Try to open and read parameter file. 
	try {
		pf = new ParameterFile(ParameterFileName);
	}
	catch (char *cp) {
		printf("Cannot load parameter file: %s\n",cp);
		_exit(1);
	}

	Stimulus.setParameterFile(pf);

	
	//Initialize the VSG and set appropriate draw mode parameters
	if( InitializeVSG() < 0)
		cout << "Error initializing the VSG" << "\n";

	//Setup the background via the VSGOverlay page.
	SetupOverlay(FixationX,FixationY,FixationDiameter, pf->FixationColor);
	
	//Set the initial values of the grating.
	Stimulus.InitGrating();

	//DrawGrating does NOT display the grating. Use Grating::ShowGrating for that.
	Stimulus.DrawGrating(FixationX,FixationY,FixationDiameter);
	index=0;
	while(index>=0)
	{
	//Read the state of the VSG's digital input bits
		DigitalInputState=vsgIOReadDigitalIn();


	//Find the state of VSG digital input bit 1, and whether it has changed.
	//This tells us when to present the fixation point, and when to clear the screen.
		FixationState=(DigitalInputState&vsgDIG1)/vsgDIG1;
		if (FixationState!=PreviousFixationState)
		{
			if( FixationState==1 )
				ShowFixation();
			if( FixationState==0 )
				ClearFixation();	
		}	PreviousFixationState=FixationState;

	//Find the state of the VSG digital input bit 2, and whether it has changed.
	//This tells us when to show the stimulus in our tuning curve.
		StimulusState=(DigitalInputState&vsgDIG2)/vsgDIG2;
		if(StimulusState != PreviousStimulusState)
		{
			if(StimulusState==1)
				Stimulus.ShowGrating();

			PreviousStimulusState=StimulusState;
		}

	//Find the state of the VSG digital input bit 3, and if it has changed.
	//This tells us when to draw the next stimulus in the tuning curve progression into memory.
		NextStimulus=(DigitalInputState&vsgDIG3)/vsgDIG3;		
		if(NextStimulus!=PreviousNextStimulus)
		{
			index=Stimulus.NextGrating();
			Stimulus.DrawGrating(FixationX,FixationY,FixationDiameter);
			PreviousNextStimulus=NextStimulus;
		}
	//Waiting 5 ms between each check of the digital input state reduces machine
	//load, and introduces perfectly acceptable delays into our system.
		::Sleep(5);
	}

}
void ShowFixation(void)
{
	vsgIOWriteDigitalOut(0xFFFF, vsgDIG0);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE,1+vsgTRIGGERPAGE);
//	cout << "Fixation point up" << endl;
}

void ClearFixation(void)
{
	vsgIOWriteDigitalOut(0,vsgDIG0+vsgDIG1);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE,0+vsgTRIGGERPAGE);
//	cout << "Screen cleared" << endl;
}


  