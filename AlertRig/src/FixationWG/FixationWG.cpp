#include <iostream.h>
#include <stdio.h>
#include "VSGEX2.H"
#include <math.h>
#include <string>

using namespace std;

void main(int ArgumentCount, char *Arguments[])
{
//Declare variables that correspond to command line arguments
	int NumberOfTrials, i;
	double StimulusTime, BlankTime,X,Y;
	int DistractorFlag;
	string BackgroundColor,FixationPointColor;
	double FixationPointDiameter,GX,GY,SmallSize, LargeSize, PeakSize,PeakSFd,LowSFd,HighSFd, tempSF;
	double SF[10] = {.1, .25, .5, .75, 1, 1.25, 1.5, 1.75, 2, 2.25};
	int Background;
	int DistanceToScreen,CellType,Con;
	double Diameter[8] = {0, .1, .4, 1, 2, 5, 7.5, 10}, r;
	VSGLUTBUFFER Buffer;
	VSGTRIVAL Red,Green,Blue,From, To;
	VSGOBJHANDLE Grating;

	VSGTRIVAL Background2[4];
	
	
	Background2[0].a = 0;	Background2[0].b = 0;	Background2[0].c = 0;
	Background2[1].a = 0.5;	Background2[1].b = 0.5;	Background2[1].c = 0.5;
	Background2[2].a = 1;	Background2[2].b = 1;	Background2[2].c = 1;

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
	GX=atof(Arguments[11]);
	GY=atof(Arguments[12]);
	SmallSize = atof(Arguments[13]);
	PeakSize = atof(Arguments[14]);
	LargeSize = atof(Arguments[15]);
	LowSFd = atof(Arguments[16]);
	PeakSFd = atof(Arguments[17]);
	HighSFd = atof(Arguments[18]);
	CellType = atoi(Arguments[19]);
	if (CellType==0)
	{
		Con=100;
		printf("Cell Type is on\n");
	}
	else
	{
		Con=-100;
		printf("Cell Type is off\n");
	}



	i=0;
	r=0;
	for (r=SmallSize; r<=PeakSize; r=r+((PeakSize-SmallSize)/3))
	{
		Diameter[i] = r;
		printf("Size  %i = %f\n  ",i,Diameter[i]) ;
	
		i=i+1;
	}
	
	r=r+.1;

	for (i=4; i<=7; i=i+1)
	{
		
		if (r> LargeSize)
		{
			r=LargeSize;
		}
		Diameter[i] = r;
		printf("Size  %i = %f\n  ",i,Diameter[i]) ;
		r=r+((LargeSize-(PeakSize+.1))/3);
	}

	tempSF = LowSFd;
	for (i=0; i<=9; i=i+1)
	{
		SF[i] = tempSF;
		printf("SF %i = %f\n  ",i,SF[i]) ;
		tempSF = tempSF + ((HighSFd -LowSFd)/9);
	}

	//Initialize the vsg card then check that it initialized O.K.
	int CheckCard;
	CheckCard = vsgInit("");

	if (CheckCard < 0)
	{  
		printf("VSG card failed to initialize\n");
		_exit(0);
	}
	vsgSetCommand(vsgPALETTECLEAR);
	vsgSetCommand(vsgVIDEOCLEAR);
	vsgSetViewDistMM(DistanceToScreen);
	vsgSetSpatialUnits(vsgDEGREEUNIT);
	vsgSetCommand(vsgOVERLAYMASKMODE);
	vsgSetDrawMode(vsgCENTREXY);
	
    VSGLUTBUFFER OverlayBuffer;  // ???????
	OverlayBuffer[1]=Background2[0];  
	OverlayBuffer[2]=Background2[1];
	OverlayBuffer[3]=Background2[2];
	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&OverlayBuffer, 0, 3); ///????????????

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
			Buffer[2].a=.5;
			Buffer[2].b=.5;
			Buffer[2].c=.5;
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
	printf("Fixation Point = %f\t%f",X,Y);

	vsgSetDrawPage(vsgVIDEOPAGE,2,vsgNOCLEAR);
	vsgSetPen1(1);
	vsgDrawOval(X,-Y,FixationPointDiameter,FixationPointDiameter);

	

	//Draw the distractor onto video page 2.
	Grating=vsgObjCreate();
	From.a=0; From.b=0; From.c=0;
	To.a=1; To.b=1; To.c=1;
	vsgObjTableSinWave(vsgSWTABLE);
	vsgObjSetDriftVelocity(0);
	vsgObjSetColourVector((VSGTRIVAL*)&From,(VSGTRIVAL*)&To,vsgBIPOLAR);
	vsgObjSetPixelLevels(10, 100);
	vsgObjSetContrast(100);
	vsgDrawGrating(GX,GY,7,7,90,.5);
	vsgSetDrawPage(vsgOVERLAYPAGE,2,2);
	vsgSetPen1(0);
	vsgSetPen2(1);
	vsgDrawOval(GX, GY,3,3);
	vsgDrawOval(X, -Y,FixationPointDiameter,FixationPointDiameter);
	vsgSetDrawPage(vsgVIDEOPAGE,3,vsgNOCLEAR);
	vsgSetPen1(1);
	vsgDrawOval(X,-Y,FixationPointDiameter,FixationPointDiameter);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE,2);
	vsgSetZoneDisplayPage(vsgVIDEOPAGE,2);
	//Since vsgPresent() makes the current drawing page the display page,
	//we set our drawing page to 0 so only a blank screen appears.
	vsgSetDrawPage(vsgVIDEOPAGE,0,vsgNOCLEAR);

	vsgPresent();
	
/////////////////Draw SF Grating///////////////////////////////////
int Obj1, Obj2;
  Obj2 =vsgObjCreate();
  //vsgObjSetDisplayPage(1);

 // vsgSetBackgroundColour((VSGTRIVAL*)&Back);
//Load the default parameters for the stimulus object.
  vsgObjSetDefaults();

//Assign the maximum amount of pixel-levels to be used for the object.
  vsgObjSetPixelLevels(102, 100);

//Load the object with a sin wave form.
  vsgObjTableSinWave(vsgSWTABLE);

//Load the object with a colour vector (the black and white vsgTRIVALS that were
//loaded earlier.
  vsgObjSetColourVector((VSGTRIVAL*)&From,(VSGTRIVAL*)&To,vsgBIPOLAR);

//Select the range of maximum pixel-levels to draw the grating with.
  //vsgSetZoneDisplayPage(vsgOVERLAYPAGE,0);
  vsgSetPen1(102);
  vsgSetPen2(201);
  vsgSetDrawPage(vsgVIDEOPAGE,3,vsgBACKGROUND);
  vsgDrawGrating(GX,GY,30,30,90,PeakSFd);
  /////////////////End Draw SF Grating///////////////////////////////////

	//Now watch the state of digital input bit 1 to tell when to
	//present the stimulus and when to hide it. Keep watching it
	//until we've completed the alotted number of trials.
	long StimulusState,PreviousState;
	PreviousState=0;
	index=0;
	printf("X Grating Location = %f\n",GX);
	int RN[50000];
	int RN2[50000];
	int RN3[50000];
	FILE *RandFile;
	FILE *RandFileInfo;
	char RandFileName[100];
	RandFileInfo = fopen("C:\\AlertRig\\Experiments\\CurrentDasRandInfo.txt","r");
	fscanf(RandFileInfo,"%s",RandFileName); 
	fclose(RandFileInfo);
	RandFile = fopen(RandFileName,"w");
	for (i=0;i<50000;i++)
	{
		RN[i]=(8*rand()/32767);
		RN2[i]=(2*rand()/32767);
		RN3[i]=(10*rand()/32767);
		fprintf(RandFile,"%i\t",RN[i]);
		fprintf(RandFile,"%i\t",RN2[i]);
		fprintf(RandFile,"%i\n",RN3[i]);
	}
	fclose(RandFile);
	i=0;
	int f;
	f=0;
	while (index<NumberOfTrials)
	{

		StimulusState=vsgIOReadDigitalIn();
		StimulusState=(StimulusState&vsgDIG1)/vsgDIG1;
		if (PreviousState != StimulusState)
		{
			if (StimulusState==1)
			{
				
				
				if (RN2[i]==0)
				{
					//vsgSetDrawPage(vsgVIDEOPAGE,1,vsgNOCLEAR);
					vsgSetZoneDisplayPage(vsgVIDEOPAGE,2);
					vsgObjSelect(Grating);
					vsgObjSetContrast(0);
					vsgSetDrawPage(vsgOVERLAYPAGE,2,2);
					vsgSetPen1(0);	
					vsgDrawOval(X,Y,FixationPointDiameter,FixationPointDiameter);
					vsgDrawOval(GX,GY,Diameter[RN[i]],Diameter[RN[i]]);
					f=0;
					//vsgSetZoneDisplayPage(vsgVIDEOPAGE,2+vsgTRIGGERPAGE);
					
				}
				else
				{
					
					
					//Create a stimulus object.
					vsgSetDrawPage(vsgVIDEOPAGE,3,vsgBACKGROUND);
					vsgObjSelect(Obj2);
					vsgObjDestroy(Obj2);
					vsgSetDrawPage(vsgVIDEOPAGE,3,vsgNOCLEAR);
					vsgSetPen1(1);
					vsgDrawOval(X,-Y,FixationPointDiameter,FixationPointDiameter);
					vsgSetPen1(2);
					vsgDrawOval(GX, GY,10,10);
					
					Obj2 =vsgObjCreate();
					vsgObjSetDefaults();
					vsgObjSetPixelLevels(102, 100);
					vsgObjTableSinWave(vsgSWTABLE);
					vsgObjSetColourVector((VSGTRIVAL*)&From,(VSGTRIVAL*)&To,vsgBIPOLAR);
					vsgSetPen1(102);
					vsgSetPen2(201);
					vsgDrawGrating(GX,GY,11,11,90,SF[RN3[i]]);
					vsgObjSetContrast(0);
					vsgPresent();
					
					
					vsgSetDrawPage(vsgOVERLAYPAGE,2,2);
					vsgSetPen1(0);	
					vsgDrawOval(X,Y,FixationPointDiameter,FixationPointDiameter);
					vsgDrawOval(GX,GY,5,5);
					printf("Here SF = %f\n",SF[RN3[i]]);
					vsgSetZoneDisplayPage(vsgVIDEOPAGE,3);
					
					
					//
				
					f=1;
				}
				vsgObjSetTriggers(vsgTRIG_ONPRESENT+vsgTRIG_TOGGLEMODE,0,0);
				vsgObjSetContrast(Con);
				vsgPresent();
				printf("i = %i\n",i);
				i=i+1;
				if (DistractorFlag !=0)
				{
					Sleep(200);
					//vsgSetDisplayPage(2);
				}
			}

			if (StimulusState==0)
			{
				vsgSetDrawPage(vsgVIDEOPAGE,0,vsgNOCLEAR);
				vsgSetDrawPage(vsgOVERLAYPAGE,2,2);
				vsgSetPen1(2);
				vsgDrawOval(0,0,25,25);
				
				//vsgSetZoneDisplayPage(vsgVIDEOPAGE,2+vsgTRIGGERPAGE);
				vsgObjSetTriggers(vsgTRIG_ONPRESENT+vsgTRIG_TOGGLEMODE,0,0);
				vsgPresent();
	
				index = index+1;
			}
			PreviousState=StimulusState;			
		}
		else
			Sleep(2); // We put this in to reduce CPU usage from checking
					  // the state of the digital input bit.
	}


}