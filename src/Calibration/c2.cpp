// $Header: /opt/cvstmp/AlertRig/src/Calibration/Attic/c2.cpp,v 1.1 2004-09-29 20:13:35 dan Exp $
//
// Calibration program. This program places a fixation point at different locations
// to facilitate calibrating the manual gains on the dni eye coil driver. 

#include <afxwin.h>
#include <iostream.h>
#include <stdio.h>
#include <limits.h>
#include "C:\Program Files\Cambridge Research Systems\VSGV6\Windows\Win32\Msc\INCLUDE\VSGEX2.H"
#include <math.h>
#include "c2.h"




void main(int argc, char *argv[])
{

	//Declare variables that correspond to command line arguments
	CString sBackgroundColor;
	CString sStimulusColor;
	double dStimulusDegrees=0;
	double dDistanceToScreen=0;
	double dPctCoverage=0;

	int iHorizontal=1;
	int iVertical=1;
	int iStimulusState = PRES_STATE_UNKNOWN;

	//Parse command line arguments

	if (argc<6)
	{
//		usage();
		_exit(1);
	}
	else 
	{
		// Arguments should be 
		// [1] - Background color "Black", "Gray", "White" (case-insensitive)
		// [2] - Stimulus color "Red", "Green", "Blue" (case-insensitive)
		// [3] - Stimulus diameter (degrees)
		// [4] - Distance to Screen (MM)
		// [5] - % of screen width to cover
		
		sBackgroundColor = argv[1];
		sStimulusColor = argv[2];
		dStimulusDegrees = atof(argv[3]);
		dDistanceToScreen = atof(argv[4]);
		dPctCoverage = atof(argv[5]);


		cout << sStimulusColor << endl;

		// initialize card. This will also prepare the frames and set the initial 
		// background.

		if (init(dDistanceToScreen, dStimulusDegrees, &sBackgroundColor, &sStimulusColor, dPctCoverage))
		{
			cerr << "Error in init()!" << endl;
			_exit(1);
		}
		else 
		{

			// Now we start looping - forever. 
			unsigned long ulInputBits=0;

			while (TRUE)
			{
				int istate=PRES_STATE_UNKNOWN;

				ulInputBits = vsgIOReadDigitalIn();

				// grab state bits and check for change...
				istate = ulInputBits & PRES_STATE;

				if (istate != iStimulusState)
				{
					// Detect level...
					setLevel(ulInputBits, &iHorizontal, &iVertical);

					printf("State change %x %d %d %d\n", ulInputBits, istate, iHorizontal, iVertical);
					if (setStimulus(istate, iHorizontal, iVertical))
					{
						// Should fail and exit here
						cout << "Error in setStimulus!" << endl;
					}
				}
				iStimulusState = istate;
				Sleep(2);
			}
		}
	}
}


int setStimulus(unsigned long i_ulState, int i_iVert, int i_iHoriz)
{
	int istatus=0;

	if ((i_ulState&PRES_ON) == i_ulState)
	{
		// Turn stimulus on
		int iPage=0;
		iPage = 1 + (i_iVert-1)*3 + (i_iHoriz-1);
		vsgSetDrawPage(vsgVIDEOPAGE,iPage,vsgNOCLEAR);
		vsgObjSetTriggers(vsgTRIG_ONPRESENT|vsgTRIG_OUTPUTMARKER,STIMULUS_ON,0);
		vsgPresent();		
	}
	else if ((i_ulState&PRES_OFF) == i_ulState)
	{
		// Turn stimulus off
		int iPage=0;
		vsgSetDrawPage(vsgVIDEOPAGE,iPage,vsgNOCLEAR);
		vsgObjSetTriggers(vsgTRIG_ONPRESENT|vsgTRIG_OUTPUTMARKER,STIMULUS_OFF,0);
		vsgPresent();		
	}
	else
	{
		// Error - both or neither stimulus bits are set!
		printf("Error in setStimulus - %x\n",i_ulState);
		istatus=1;
	}
	return istatus;
}


void setLevel(unsigned long i_ulbits, int *i_piVert, int *i_piHoriz)
{
	*i_piHoriz	= XCOORD(i_ulbits);
	*i_piVert	= YCOORD(i_ulbits); 
}

int init(double i_dDist, double i_dDiam, CString *i_psBackground, CString *i_psStim, double i_dPctCoverage)
{
	int istatus=0;
	VSGLUTBUFFER buffer;
	VSGTRIVAL vsgtrivialRed,vsgtrivialGreen,vsgtrivialBlue;
	VSGOBJHANDLE vsgobjDummy;
	int iBackground;

	//Initialize the vsg card then check that it initialized O.K.
	if (vsgInit(""))
	{
		istatus=1;
	}
	else 
	{
		Sleep(5000);
		vsgSetViewDistMM((unsigned long)i_dDist);
		vsgSetSpatialUnits(vsgDEGREEUNIT);


		//Set up the LUT for this experiment
		vsgtrivialRed.a=1; 
		vsgtrivialRed.b=0; 
		vsgtrivialRed.c=0;
		vsgtrivialGreen.a=0; 
		vsgtrivialGreen.b=1; 
		vsgtrivialGreen.c=0;
		vsgtrivialBlue.a=0; 
		vsgtrivialBlue.b=0; 
		vsgtrivialBlue.c=1;

		//Make LUT entry 0 the fixation color
		if( strcmpi(*i_psStim,"red")==0 )
		{
			cout << "init - red\n";
			buffer[0].a=1;
			buffer[0].b = buffer[0].c = 0;
		}
		else if( strcmpi(*i_psStim,"green")==0 )
		{
			buffer[0].b=1;
			buffer[0].a = buffer[0].c = 0;
		}
		else if( strcmpi(*i_psStim,"blue")==0 )
		{
			buffer[0].c=1;
			buffer[0].a = buffer[0].b = 0;
		}
		else 
		{
			cout << "Color? " << *i_psStim << endl;
		}

		
		for(int i=1; i<256; i++)
		{
			buffer[i].a=i/255.0;
			buffer[i].b=i/255.0;
			buffer[i].c=i/255.0;
		}
	}
	//Copy our LUT to the VSG and make it the current Palette
	vsgLUTBUFFERWrite(0,&buffer);
	vsgLUTBUFFERtoPalette(0);

	//Set the background color. A better way to do this would be to set
	//the rgb values explicitly as we did for the fixation color above.
	if(strcmpi(*i_psBackground,"black")==0) iBackground=1;
	if(strcmpi(*i_psBackground,"gray")==0) iBackground=128;
	if(strcmpi(*i_psBackground,"white")==0) iBackground=255;

	// Draw the pages.
	drawPages(iBackground, i_dDiam, i_dPctCoverage);
	
	// Create any old vsg object to enable the triggering system
	vsgobjDummy = vsgObjCreate();
	vsgObjSetPixelLevels(1,1);


	//Since vsgPresent() makes the current drawing page the display page,
	//we set our drawing page to 0 so only a blank screen appears.
	vsgSetDrawPage(vsgVIDEOPAGE,0,vsgNOCLEAR);
	vsgObjSetTriggers(vsgTRIG_ONPRESENT|vsgTRIG_OUTPUTMARKER,STIMULUS_OFF,0);
	vsgPresent();


	return istatus;
}



int drawPages(int i_iBackground, double i_dDiameter, double i_dPctCoverage)
{
	int iStatus=0;
	int iPage=0;
	double dXMax=0;
	double dYMax=0;
	double dXLocation=0;
	double dYLocation=0;
	VSGLUTBUFFER buffer;

	vsgLUTBUFFERRead(0,&buffer);
	cout << buffer[1].a << " " << buffer[1].b << " " << buffer[1].c << endl;

	if (!getPixelRange(&dXMax,&dYMax))
	{


		// Draw page 0 as a blank. 
		vsgSetDrawPage(vsgVIDEOPAGE,iPage,i_iBackground);


		// Draw pages 1-9
		for (int i=0;i<3;i++)
		{
			for (int j=0; j<3; j++)
			{
				iPage = 1 + i*3 + j;

				dXLocation = (i-1)*dXMax*i_dPctCoverage;
				dYLocation = (j-1)*dYMax*i_dPctCoverage;

				//Clear the video page we're using to the background color
				vsgSetDrawPage(vsgVIDEOPAGE,iPage,i_iBackground);

//				vsgSetDrawPage(vsgVIDEOPAGE,iPage,vsgNOCLEAR);
				vsgSetPen1(0);
				vsgDrawOval(dXLocation, dYLocation , i_dDiameter, i_dDiameter);
			}
		}
	}
	else 
	{

		iStatus=1;

	}
	return iStatus;
}


// Get conversion to pixels (degrees per pixel)
// Assumes card has been initialized and vsgSetViewDistMM has been called with dist to 
// screen. 

int getPixelRange(double *i_pXMax, double *i_pYMax)
{
	int iStatus=0;
	double dPixelsPerDegree=0;
	if (!vsgUnit2Unit(vsgDEGREEUNIT, 1, vsgPIXELUNIT, &dPixelsPerDegree))
	{
		// Convert pixels per degree to screen max. Lets allow a little room at the edge for the
		// radius of the spot. 
		*i_pXMax = 400/dPixelsPerDegree;
		*i_pYMax = 300/dPixelsPerDegree;

//		printf("There are %f pixels per degree\n", dPixelsPerDegree);
//		printf("XMax (375 pixels) is at %f degrees\n", *i_pXMax);
//		printf("YMax (275 pixels) is at %f degrees\n", *i_pYMax);

	}
	else 
	{
		printf("Conversion routine failed.\n");
		iStatus=1;
	}
	return iStatus;
}






























/*
	long StimulusState, NonJuicePins, PreviousState;
	int Fixation=0,TargetContrastChange=0,ConfounderContrastChange=0,ContrastChangeValue=0,Stim=0,AnswerPoint=0;
	int PreviousFixation=0,PreviousTargetContrastChange=0,PreviousConfounderContrastChange=0,PreviousContrastState=0,PreviousStim=0,PreviousAnswerPoint=0;
	//Determine the initial state of the VSG digital input pins.
	StimulusState=vsgIOReadDigitalIn();
	//Digital input 0 is used by the juicer, so we ignore it
	NonJuicePins=floor(StimulusState/2);
	PreviousState=NonJuicePins;
	
	DWORD TriggerState=0; //We use this to keep track of our output trigger states.
	while (TRUE)
	{
*/