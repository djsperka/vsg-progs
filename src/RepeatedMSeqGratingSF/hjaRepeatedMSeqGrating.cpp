#include <stdio.h>
#include "C:\Program Files\Cambridge Research Systems\VSGV6\Windows\Win32\Msc\INCLUDE\VSGEX2.H"
#include <math.h>
#include <direct.h>
#include <time.h>
#include <winbase.h>
#include <string.h>
#include <process.h>
#define PixelSize 10
#define NumberOfTerms 32767

int MSequence[NumberOfTerms];
int DisplayedTerms;
VSGTRIVAL On, Off, Background;
VSGLUTBUFFER Buffer;
VSGLUTBUFFER OverlayBuffer;

/////////////////////////////////////////////////////////////////////////////////////
int InitializeCard(double DistanceToScreen, double X, double Y)
{
	int CheckCard;
//Initialize the vsg card then check that it initialized O.K.
	CheckCard = vsgInit("");
	if (CheckCard < 0) {printf("VSG card failed to initialize\n"); return(-1);}
//Configure drawing modes
//	vsgSetVideoMode(vsgPANSCROLLMODE);
	vsgSetDrawMode(vsgCENTREXY);
	vsgSetDrawOrigin(400+X,300-Y);
	vsgSetViewDistMM(DistanceToScreen);
	vsgSetSpatialUnits(vsgDEGREEUNIT);
	return(1);
}
/////////////////////////////////////////////////////////////////////////////////////
int ReadMSequence()
{
	FILE *stream;
	int MSequenceIndex;
//Read the M-sequence values from a file into an array.
	stream = fopen("c:\\AcuteRig\\MSequenceTerms","r");
	if (stream== NULL)
	{
		printf("Could not find M-sequenceFile\n");
		getchar();
		return(-1);
	}
	for (MSequenceIndex=0; MSequenceIndex <= NumberOfTerms; MSequenceIndex++)
		fscanf(stream,"%1d",&MSequence[MSequenceIndex]);
	_fcloseall();
	return(1);
}
/////////////////////////////////////////////////////////////////////////////////////
int DrawGrating(double SpatialFrequency, double Contrast, double Orientation, double X, double Y)
{

  double Width, Height;
  VSGTRIVAL From, Too, Back;
//Clear the palette to black so no drawing will be seen.
  vsgSetCommand(vsgPALETTECLEAR);
  

//Set the current drawing page.
  vsgSetDrawPage(vsgVIDEOPAGE,0,vsgBACKGROUND);

//Find out the horizontal and vertical resolution of the vsg screen.
  Width = vsgGetScreenWidthPixels();
  Height = vsgGetScreenHeightPixels();
//Convert the resolution parameters to visual degrees
  	vsgUnit2Unit(vsgPIXELUNIT,Width,vsgDEGREEUNIT,&Width);
	vsgUnit2Unit(vsgPIXELUNIT,Height,vsgDEGREEUNIT,&Height);

//Load a TRIVAL with the colour white.
  From.a = 1.0;
  From.b = 1.0;
  From.c = 1.0;

//Load a TRIVAL with the colour black.
  Too.a = 0.0;
  Too.b = 0.0;
  Too.c = 0.0;

  Back.a = (From.a+Too.a)/2;
  Back.b = (From.b+Too.b)/2;
  Back.c = (From.c+Too.c)/2;

//Create a stimulus object.
  vsgObjCreate();

  vsgSetBackgroundColour((VSGTRIVAL*)&Back);
//Load the default parameters for the stimulus object.
  vsgObjSetDefaults();

//Assign the maximum amount of pixel-levels to be used for the object.
  vsgObjSetPixelLevels(1, 200);

//Load the object with a sin wave form.  vsgSWTable specifeis a spatial sine wave.
  vsgObjTableSinWave(vsgSWTABLE);

//Load the object with a colour vector (the black and white vsgTRIVALS that were
//loaded earlier.
  vsgObjSetColourVector((VSGTRIVAL*)&From, (VSGTRIVAL*)&Too, vsgBIPOLAR);

//Select the range of maximum pixel-levels to draw the grating with.
  vsgSetPen1(1);
  vsgSetPen2(200);

//Draw the grating centered in the middle of the screen.
//  vsgSetDrawMode(vsgCENTREXY);  This should already be turned on
  vsgDrawGrating(X, Y,Width, Height, Orientation, SpatialFrequency);
  vsgObjSetDriftVelocity(0);
  vsgObjSetContrast(Contrast*100);;
//Display the object.
  vsgPresent();

  return 1;
}
///////////////////////////////////////////////////////////////////////////////////

int SetupOverlay ( double GratingDiameter, double X, double Y)
{
    int DotContrast;
		DotContrast =1;
	vsgSetCommand(vsgOVERLAYMASKMODE+vsgOVERLAYDRIFT);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE,3);
	Background.a = 0.5;	Background.b = 0.5;	Background.c = 0.5;
	On.a=.5+DotContrast/2;		On.b=.5+DotContrast/2;		On.c=.5+DotContrast/2;
	Off.a=.5-DotContrast/2;	Off.b=.5-DotContrast/2;	Off.c=.5-DotContrast/2;

	Buffer[1]=Background;
	Buffer[2]=On;
	Buffer[3]=Off;
	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&Buffer, 0, 4);
	
	

	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1);
	vsgSetPen1(0);
	vsgDrawOval(X,Y,GratingDiameter,GratingDiameter);
	vsgSetPen1(1);
	//vsgDrawOval(0,0,DotSurround,DotSurround);
//	vsgSetPen1(2);
//	vsgDrawOval(0,0,DotDiameter,DotDiameter);
	

	return 1;
}
////////////////////////////////////////////////////////////////////////////////////

unsigned __stdcall DisplayMSequence(void *FramesPerTerm)
{
int index=0,RepeatIndex,NumberOfRepeats;
	NumberOfRepeats = *(int*)FramesPerTerm;
	Sleep(2);
	vsgSetDisplayPage(0+vsgTRIGGERPAGE);
	vsgResetTimer();
	FILE *fp;
	int fo;
	float OptPhase, OptPhase2;
    OptPhase2 = OptPhase+180;
    fp = fopen("c:\\AcuteRig\\Experiments\\Phase3.txt","w");

   FILE *RanStream;
   int RanSeq[32767];
   int P;
   int Con[2], Con2[2];

   	FILE *CF;
	CF = fopen("c:\\AcuteRig\\Experiments\\ContrastFile.txt","r");
	fscanf(CF,"%d",&Con[1]);
	fscanf(CF,"%d",&Con[2]);
    printf("%d %d", Con[1], Con[2]);
  // Con[1]=100;
  // Con[2]=25;
   Con2[1]= (-1 * Con[1]);
   Con2[2]= (-1 * Con[2]);
   P=1;

 
   RanStream = fopen("C:\\Henry\\CR withRandom Contrast\\RandNumbers.txt", "r");
   //RanStream = fopen("c:\\AcuteRig\\MSequenceTerms","r");
   
   for (index=0; index <= DisplayedTerms; index++)
   {
	fscanf(RanStream,"%3d",&RanSeq[index]);
   }



	

	vsgObjSetSpatialPhase(OptPhase);

	for (index=0; index <= DisplayedTerms; index++)	
	{
		
		for (RepeatIndex=0; RepeatIndex <NumberOfRepeats; RepeatIndex++)
		{
			if (MSequence[index] == 0)
			{
				printf( "%1d\n", Con2[2]);
				vsgObjSetContrast(Con[P]);
				vsgPresent();
				
			//	printf( "%1d\n", P );
				if (RanSeq[index] >90)
				{
					if (RepeatIndex ==0)
					{
						if (P==1)
						{
							P=2;
							
						}
						else
						{
							P=1;
						}
					}

				}
				

			}
			else
			{
				printf( "%1d\n", Con2[1]);
				vsgObjSetContrast(Con2[P]);
				vsgPresent();
				
			//	printf( "%1d\n", P );
				if (index == 500)
				{
					fclose(fp);
				}
				if (RanSeq[index] >90)
				{
					if (RepeatIndex==0)
					{
						if (P==1)
						{
							P=2;
							fprintf(fp, "%1d\n", index );
						}
						else 
						{
							P=1;
						}
					
					}
				}

			
			}
			if (index == DisplayedTerms)
			{
				//vsgObjSetContrast(0);
			//	vsgPresent();
			}

		}
	}
	
	printf("%d\n",vsgGetTimer());
	return 1;
}
//////////////////////////////////////////////////////////////////////////////////////

UINT main(int ArgumentCount, char *Arguments[])
{


	int Page=0,ReturnValue;
	double GratingDiameter, GratingContrast,LowGratingContrast, SpatialFrequency, Orientation;
	double DistanceToScreen,X,Y;
	int FramesPerTerm,Repeats,RepeatIndex;
	double RunTime,BlankTime;
	GratingDiameter=atof(Arguments[1]);
	GratingContrast=atof(Arguments[2]);
	LowGratingContrast=atof(Arguments[12]);
	SpatialFrequency=atof(Arguments[3]);
	Orientation=atof(Arguments[4]);
	DistanceToScreen=atof(Arguments[5]);
	FramesPerTerm=atoi(Arguments[6]);
	X=atof(Arguments[7]);
	Y=-1*atof(Arguments[8]);
	Repeats=atoi(Arguments[9]);
	RunTime=atof(Arguments[10]);
	BlankTime=atof(Arguments[11]);
	printf("%d\n",RunTime);
	DisplayedTerms=RunTime/(FramesPerTerm*.007140);
	//Initialize card and specify settings
	ReturnValue=InitializeCard(DistanceToScreen,X,Y);
	FILE *CF;
	CF = fopen("c:\\AcuteRig\\Experiments\\ContrastFile.txt","w");
	fprintf(CF,"%3.0f\n %3.0f\n", GratingContrast,LowGratingContrast);
	fclose(CF);

	if (ReturnValue == -1) _exit(0);

	//Read the MSequence into memory	

	ReturnValue=ReadMSequence();

	if (ReturnValue == -1) _exit(0);
	
	DrawGrating(SpatialFrequency,GratingContrast, Orientation,X,Y);
	SetupOverlay(GratingDiameter,X,Y);
	HANDLE hThread;
    unsigned threadID ;
	Sleep(2000);
	vsgSetZoneDisplayPage(vsgVIDEOPAGE,0+vsgTRIGGERPAGE);
	for (RepeatIndex=1; RepeatIndex<=Repeats; RepeatIndex++)
	{
		vsgSetZoneDisplayPage(vsgOVERLAYPAGE,1);
		hThread = (HANDLE)_beginthreadex( NULL, 0,DisplayMSequence,&FramesPerTerm, 0,
			&threadID );
		ReturnValue=SetThreadPriority(hThread,THREAD_PRIORITY_TIME_CRITICAL);
		if (ReturnValue !=1) printf("Unable to set thread priority. Re-start M-sequence.");
		WaitForSingleObject(hThread,INFINITE);
		CloseHandle( hThread );
		vsgSetZoneDisplayPage(vsgOVERLAYPAGE,1);
		Sleep(BlankTime*1000);
		printf("%f\n", BlankTime);
		vsgSetZoneDisplayPage(vsgVIDEOPAGE,0+vsgTRIGGERPAGE);
	}
//	vsgSetZoneDisplayPage(vsgVIDEOPAGE,0+vsgTRIGGERPAGE);

}


