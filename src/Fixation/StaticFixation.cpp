#include <iostream.h>
#include <stdio.h>
#include "VSGEX2.H"
#include <math.h>
#include <string>

using namespace std;

void main(int ArgumentCount, char *Arguments[])
{
//Declare variables that correspond to command line arguments
	int NumberOfTrials;
	double StimulusTime, BlankTime,X,Y;
	int DistractorFlag;
	string BackgroundColor,FixationPointColor;
	double FixationPointDiameter;
	int Background;
	int DistanceToScreen;
	VSGLUTBUFFER Buffer;
	VSGTRIVAL Red,Green,Blue,From, To;
	VSGOBJHANDLE Grating;

	//Parse command line arguments
	NumberOfTrials=atoi(Arguments[1]);
	StimulusTime=atof(Arguments[2]);
	BlankTime=atof(Arguments[3]);
	X=atof(Arguments[4]);
	Y=atof(Arguments[5]);
	DistractorFlag=atoi(Arguments[6]);
	BackgroundColor=Arguments[7];
	FixationPointDiameter=atof(Arguments[8]);
	FixationPointColor=Arguments[9];
	DistanceToScreen=atoi(Arguments[10]);

	//Initialize the vsg card then check that it initialized O.K.
	int CheckCard;
	CheckCard = vsgInit("");

	if (CheckCard < 0)
	{  
		printf("VSG card failed to initialize\n");
		_exit(0);
	}
	vsgSetViewDistMM(DistanceToScreen);
	vsgSetSpatialUnits(vsgDEGREEUNIT);


	//Set up the LUT for this experiment
	Red.a=1; Red.b=0; Red.c=0;
	Green.a=0; Green.b=1; Green.c=1;
	Blue.a=0; Blue.b=0; Blue.c=1;

	int index;
	for(index=0; index<256; index++)
	{
		//Make LUT entry 1 the fixation color
		if(index==1) 
		{
			if( strcmp(FixationPointColor.c_str(),"Red")==0 )
				Buffer[index]=Red;
			if( strcmp(FixationPointColor.c_str(),"Green")==0 )
				Buffer[index]=Green;
			if( strcmp(FixationPointColor.c_str(),"Blue")==0 )
				Buffer[index]=Blue;
		}
		else
		//Otherwise, make it a black to white luminance ramp
		{
			Buffer[index].a=index/255.0;
			Buffer[index].b=index/255.0;
			Buffer[index].c=index/255.0;
		}
	}
	//Copy our LUT to the VSG and make it the current Palette
	vsgLUTBUFFERWrite(0,&Buffer);
	vsgLUTBUFFERtoPalette(0);

	//Set the background color. A better way to do this would be to set
	//the rgb values explicitly as we did for the fixation color above.
	if(strcmp(BackgroundColor.c_str(),"Black")==0) Background=0;
	if(strcmp(BackgroundColor.c_str(),"Gray")==0) Background=128;
	if(strcmp(BackgroundColor.c_str(),"White")==0) Background=255;

	//Clear the video pages we're using to the background color
	vsgSetDrawPage(vsgVIDEOPAGE,0,Background);
	vsgSetDrawPage(vsgVIDEOPAGE,1,Background);
	vsgSetDrawPage(vsgVIDEOPAGE,2,Background);

	//Draw the fixation point on pages 1 and 2. We use page 1 to hold
	//the fixation point only, and page 2 to hold the fixation point with
	//a distractor. We'll use page 0 to be our blank screen.

	vsgSetDrawPage(vsgVIDEOPAGE,1,vsgNOCLEAR);
	vsgSetPen1(1);
	vsgDrawOval(X,-Y,FixationPointDiameter,FixationPointDiameter);

	vsgSetDrawPage(vsgVIDEOPAGE,2,vsgNOCLEAR);
	vsgSetPen1(1);
	vsgDrawOval(X,-Y,FixationPointDiameter,FixationPointDiameter);

	//Draw the distractor onto video page 2.
	Grating=vsgObjCreate();
	From.a=0; From.b=0; From.c=0;
	To.a=1; To.b=1; To.c=1;
	vsgObjTableSinWave(vsgSWTABLE);
	vsgObjSetDriftVelocity(1);
	vsgObjSetColourVector((VSGTRIVAL*)&From,(VSGTRIVAL*)&To,vsgBIPOLAR);
	vsgObjSetPixelLevels(129,120);
	vsgObjSetContrast(100);
	vsgDrawGrating(2,3,2,2,45,1);

	//Since vsgPresent() makes the current drawing page the display page,
	//we set our drawing page to 0 so only a blank screen appears.
	vsgSetDrawPage(vsgVIDEOPAGE,0,vsgNOCLEAR);
	vsgPresent();		

	//Now watch the state of digital input bit 1 to tell when to
	//present the stimulus and when to hide it. Keep watching it
	//until we've completed the alotted number of trials.
	long StimulusState,PreviousState;
	PreviousState=0;
	index=0;
	while (index<NumberOfTrials)
	{

		StimulusState=vsgIOReadDigitalIn();
		StimulusState=(StimulusState&vsgDIG1)/vsgDIG1;
		if (PreviousState != StimulusState)
		{
			if (StimulusState==1)
			{
				//vsgSetDisplayPage(1);
				vsgSetDrawPage(vsgVIDEOPAGE,1,vsgNOCLEAR);
				vsgObjSetTriggers(vsgTRIG_ONPRESENT+vsgTRIG_TOGGLEMODE,0,0);
				vsgPresent();
				if (DistractorFlag !=0)
				{
					Sleep(200);
					vsgSetDisplayPage(2);
				}
			}

			if (StimulusState==0)
			{
				vsgSetDrawPage(vsgVIDEOPAGE,0,vsgNOCLEAR);
				vsgObjSetTriggers(vsgTRIG_ONPRESENT+vsgTRIG_TOGGLEMODE,0,0);
				vsgPresent();
//				vsgSetDisplayPage(0);
				index = index+1;
			}
			PreviousState=StimulusState;			
		}
		else
			Sleep(2); // We put this in to reduce CPU usage from checking
					  // the state of the digital input bit.
	}


}