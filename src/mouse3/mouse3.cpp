//****************************************************************
//
//
//		Mouse Control Program
//		This program displays a drifting grating
//		whose position is determined by the position
//		of the mouse pointer.
//		
//		The grating parameters can be changed by
//		typing (o) orientaion, (s) spatial frequency
//		(t) temporal frequency, (a) aperture size.
//
//		Michael Sceniak 4/30/02
//
//*****************************************************************


#define _CRT_SECURE_NO_WARNINGS

#include "vsgv8.h"
#include "math.h"
#include "windows.h"
#include "winuser.h"
#include <stdio.h>
#include <conio.h>


#pragma comment (lib, "vsgv8.lib")

double ScrWidth, ScrHeight;
int counter;


int UpdateGrating(int objHandle, double Orientation, double SpatialFrequency, double DriftVelocity);
void UpdateAperture(float Aperture);
void UpdateDriftVelocity(double DriftVelocity);
void UpdateContrast(double Contrast);	
void UpdateBackground(double BackgroundLuminance, double Aperture);

int InitializeCard(void)
{
	int CheckCard;
	CheckCard = vsgInit("");
	if (CheckCard < 0) {printf("VSG card failed to initialize\n"); return(-1);}
	vsgSetVideoMode(vsgPANSCROLLMODE);

	vsgSetSpatialUnits(vsgDEGREEUNIT);


	return(1);
}


void main(int ArgumentCount, char *Arguments[])
{	
//	LoadIcon(NULL, IDI_ICON1);
//	DrawIcon();
	
	int	x_val, y_val, i=0;
	int DistanceToScreen;
	int ReturnValue;
	int key;
	float Aperture=1;
	int objHandle=0;
	double DriftVelocity=4, SpatialFrequency=3, Orientation=45, Contrast=99;
	double CurrentDriftVelocity=4, PreviousDriftVelocity=4;
	double WidthOffset, HeightOffset;
	double BackgroundLuminance;
	int Page;

	Page=0;
	x_val = 0;
	y_val = 0;

	printf(" ____________________________________\n");
	printf("|                                    |\n");
	printf("| Mouse Control stimulation program. |\n");
	printf("|    Type <h> for help.              |\n");
	printf("|____________________________________|\n");


	ReturnValue=InitializeCard();

	if (ReturnValue == -1) _exit(0);

	ScrWidth = vsgGetScreenWidthPixels();
	ScrHeight = vsgGetScreenHeightPixels();
	vsgUnit2Unit(vsgPIXELUNIT,ScrWidth,vsgDEGREEUNIT,&ScrWidth);
	vsgUnit2Unit(vsgPIXELUNIT,ScrHeight,vsgDEGREEUNIT,&ScrHeight);
		
	WidthOffset = ScrWidth/2;
	HeightOffset = ScrHeight/2;

	VSGLUTBUFFER  Buffer;

	objHandle = UpdateGrating(objHandle, Orientation, SpatialFrequency, DriftVelocity);

	Buffer[1].a = 0.5;
	Buffer[1].b = 0.5;
	Buffer[1].c = 0.5;

	Buffer[2].a = 0;
	Buffer[2].b = 0;
	Buffer[2].c = 0;

	Buffer[3].a = 1;
	Buffer[3].b = 0;
	Buffer[3].c = 0;

	vsgSetCommand(vsgOVERLAYMASKMODE);
	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&Buffer, 0, 4);
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1);   // <--------------- Last value gives the overlay page color from Buffer
	vsgSetPen1(1);
	vsgSetDrawPage(vsgOVERLAYPAGE,1,1);
	vsgSetDrawPage(vsgOVERLAYPAGE,2,1);
	vsgSetDrawPage(vsgOVERLAYPAGE,3,1);
	vsgPresent();
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1);
	vsgSetPen1(0);	
	vsgDrawOval(0, 0, Aperture, Aperture); // <----------------- Surround Overlay
	
	double FixationX,FixationY,FixationDiameter;
	long StimulusState, PreviousState;

	FixationX=atof(Arguments[1]);
	FixationY=atof(Arguments[2]);
	FixationDiameter=atof(Arguments[3]);
	DistanceToScreen=atoi(Arguments[4]);
	vsgSetViewDistMM(DistanceToScreen);

	printf("Distance to Screen = %i\n",DistanceToScreen);
	

//	printf("%f %f %f \n", FixationX, FixationY, FixationDiameter);

	PreviousState=0;
//	Diameter=.1;
	double MouseX,MouseY;
	POINT Pos;
	while(i < 1)
	{
		Page=1-Page; // Toggles page between 1 and 0.
		vsgSetDrawPage(vsgOVERLAYPAGE,Page,1);
		GetCursorPos(&Pos);
		vsgUnit2Unit(vsgPIXELUNIT,Pos.x,vsgDEGREEUNIT,&MouseX);
		vsgUnit2Unit(vsgPIXELUNIT,Pos.y,vsgDEGREEUNIT,&MouseY);
		vsgSetPen1(0);
		vsgDrawOval(MouseX-ScrWidth/2, MouseY-ScrHeight/2, Aperture, Aperture);
	
		StimulusState=vsgIOReadDigitalIn();
//		printf ("%x\n", StimulusState);

		StimulusState=(StimulusState&vsgDIG1)/vsgDIG1;	
		if (StimulusState==1) 
		{
			vsgSetPen1(3);
			vsgDrawOval(FixationX,FixationY,FixationDiameter,FixationDiameter);
		}
		PreviousState=StimulusState;
		vsgSetZoneDisplayPage(vsgOVERLAYPAGE,Page);
//		if (StimulusState != PreviousState) printf("%d\n",StimulusState);


		vsgSetDrawPage(vsgOVERLAYPAGE,0,vsgNOCLEAR);		
	
		while(_kbhit())
		{

			key = _getch();
			//printf("The %d key was pressed.\n", key);
			
			
			if(key==97)	// If m(menu) key is pressed look for next key press
			{  
				printf("Aperture Size = ");
				scanf("%f", &Aperture);
//				UpdateAperture(Aperture);
			}

			if(key==98)
			{
				printf("Background = ");
				int ReturnValue;
				ReturnValue=scanf("%lf", &BackgroundLuminance);
				//printf("\n%d\n%f\n\n",ReturnValue,BackgroundLuminance);
				UpdateBackground(BackgroundLuminance, Aperture);
				
			}

			if(key==104)
			{
				printf("Help Menu\n\n");
				printf("<a> aperture size\n");
				printf("<o> orientation\n");
				printf("<s> spatial frequency\n");
				printf("<t> temporal frequency\n");
				printf("<c> contrast [0-100]\n");
				printf("<b> background luminance [0-1]\n");
				printf("<p> position information\n");
				printf("<v> current spatiotemporal values\n");
				printf("<space bar> toggles between drifting and static\n");
				printf("<esc> end program\n");
				printf("\n\n");
			}

			if(key==115)
			{
				printf("Spatial Frequency = ");
				scanf("%lf", &SpatialFrequency);
				objHandle = UpdateGrating(objHandle, Orientation, SpatialFrequency, DriftVelocity);
			}

			if(key==118)
			{
				printf("Orientation = %3.0lf, Spatial Frequency = %3.1lf, Temporal Frequency = %3.1lf.\n", Orientation, SpatialFrequency, DriftVelocity);
			}
	
			if(key==116)
			{
				printf("Temporal Frequency = ");
				scanf("%lf", &DriftVelocity);
				UpdateDriftVelocity(DriftVelocity);
				CurrentDriftVelocity = DriftVelocity;
				PreviousDriftVelocity = DriftVelocity;
			}

			if(key==111)
			{
				printf("Orientation = ");
				scanf("%lf", &Orientation);
				objHandle = UpdateGrating(objHandle, Orientation, SpatialFrequency, DriftVelocity);
			}

			if(key==32)
			{
				//printf("PreviousDriftVelocity = %f\n",PreviousDriftVelocity);
				//printf("CurrentDriftVelocity = %f\n", CurrentDriftVelocity);
				if(CurrentDriftVelocity!=0)
				{
					PreviousDriftVelocity = DriftVelocity;
					UpdateDriftVelocity(0);
					CurrentDriftVelocity = 0;
				}
				else
				{
					UpdateDriftVelocity(PreviousDriftVelocity);
					CurrentDriftVelocity = PreviousDriftVelocity;
				}
			}
				
			if(key==99)
			{
				printf("Contrast = ");
				scanf("%lf", &Contrast);
				UpdateContrast(Contrast);
			}

			if(key==112)
			{
				printf("pixels:X position = %d, Y position = %d .\n", Pos.x-400, -(Pos.y-300));
				printf("degrees:X position = %f, Y position = %f .\n\n", MouseX-WidthOffset, -(MouseY-HeightOffset));
				//printf("WidthOffset = %f, HeightOffset = %f.\n", WidthOffset, HeightOffset);

			}

			if(key==27)
			{
				vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1);
				exit(0);
			}

		}
	}
}




void UpdateAperture(float Aperture)
{
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1);
	vsgSetPen1(0);	
	vsgDrawOval(0, 0, Aperture, Aperture);
}

void UpdateBackground(double BackgroundLuminance, double Aperture)
{
	VSGLUTBUFFER Buffer;

	Buffer[1].a = BackgroundLuminance;
	Buffer[1].b = BackgroundLuminance;
	Buffer[1].c = BackgroundLuminance;

	Buffer[2].a = 0;
	Buffer[2].b = 0;
	Buffer[2].c = 0;

	Buffer[3].a = 1;
	Buffer[3].b = 1;
	Buffer[3].c = 1;

	vsgSetCommand(vsgOVERLAYMASKMODE);
	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&Buffer, 0, 4);
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1);   // <--------------- Last value gives the overlay page color from Buffer
	vsgSetPen1(1);
	vsgSetDrawPage(vsgOVERLAYPAGE,1,1);
	vsgSetDrawPage(vsgOVERLAYPAGE,2,1);
	vsgSetDrawPage(vsgOVERLAYPAGE,3,1);
	//vsgPresent();
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1);
	vsgSetPen1(0);	
	vsgDrawOval(0, 0, Aperture, Aperture);// <----------------- Surround Overlay
	vsgPresent();

}



int UpdateGrating(int objHandle, double Orientation, double SpatialFrequency, double DriftVelocity)
{


	VSGTRIVAL From, Too, Back;
	From.a = 0.0;
	From.b = 0.0;
	From.c = 0.0;

	Too.a = 1.0;
	Too.b = 1.0;
	Too.c = 1.0;

	Back.a = (From.a+Too.a)/2;
	Back.b = (From.b+Too.b)/2;
	Back.c = (From.c+Too.c)/2;

	vsgObjDestroy(objHandle);
	vsgSetCommand(vsgPALETTECLEAR);

	vsgSetDrawPage(vsgVIDEOPAGE,0,vsgBACKGROUND);
	objHandle = vsgObjCreate();

	ScrWidth = vsgGetScreenWidthPixels();
	ScrHeight = vsgGetScreenHeightPixels();
	vsgUnit2Unit(vsgPIXELUNIT,ScrWidth,vsgDEGREEUNIT,&ScrWidth);
	vsgUnit2Unit(vsgPIXELUNIT,ScrHeight,vsgDEGREEUNIT,&ScrHeight);

	vsgSetBackgroundColour((VSGTRIVAL*)&Back);
	vsgObjSetDefaults();
	vsgObjSetPixelLevels(1, 120);
	vsgObjTableSinWave(vsgSWTABLE);
	vsgObjSetColourVector((VSGTRIVAL*)&From, (VSGTRIVAL*)&Too, vsgBIPOLAR);

	
	vsgSetPen1(1);
	vsgSetPen2(120);
	vsgObjSetDriftVelocity(DriftVelocity);
	vsgDrawGrating(0, 0, ScrWidth, ScrHeight, Orientation, SpatialFrequency);
	vsgPresent();
	return(objHandle);

}


void UpdateDriftVelocity(double DriftVelocity)
{
	vsgObjSetDriftVelocity(DriftVelocity);
	vsgPresent();
}


void UpdateContrast(double Contrast)
{
	vsgObjSetContrast(Contrast);
	vsgPresent();
}
