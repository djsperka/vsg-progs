#include <stdio.h>
#include "C:\Program Files\Cambridge Research Systems\VSGV6\Windows\Win32\Msc\INCLUDE\VSGEX2.H"
#include <math.h>
#include <STDLIB.H>
#include <direct.h>
#include <time.h>
#include <winbase.h>
#include <string.h>
#include <process.h>
#define PixelSize 10
#define NumberOfTerms 32767
//#define Rand_Max 4
//#undef RAND_MAX




//////////////////////////////////////

void main(int ArgumentCount, char *Arguments[])
{
	int CheckCard, FPT, i, DistanceToScreen;
	double Height,Width,SFd, Ori, Con, SmallSize, LargeSize, Signal, Blank, Repeats,X,Y;
	int DrawGrating(int FPT, double SFd, double Ori, double Con, double SmallSize, double LargeSize, double X, double Y, double Signal, double Blank, int DistanceToScreen,int Repeats);

	VSGTRIVAL Background;
	
	Background.a = 0.5;	Background.b = 0.5;	Background.c = 0.5;

	LargeSize = atof(Arguments[1]);
	SmallSize = atof(Arguments[2]);
	Con = atof(Arguments[3]);
	SFd = atof(Arguments[4]);
	Ori = atof(Arguments[5]);
	DistanceToScreen = atoi(Arguments[6]);
	FPT=atoi(Arguments[7]);
	X = atof(Arguments[8]);
	Y = atof(Arguments[9]);
	
	Repeats = atoi(Arguments[10]);
	Signal = atof(Arguments[11]);
	Blank = atof(Arguments[12]);
	
	
	//Sleep(50000);
	
	
	
	i=0;
	CheckCard = vsgInit("");
	if (CheckCard < 0)
	{
		printf("VSG card failed to initialize\n");
		_exit(0);
	}
	//Clear the palette to black so no drawing will be seen.
	vsgSetCommand(vsgPALETTECLEAR);

	// this makes all onjects centered on given coordinates.
	vsgSetDrawMode(vsgCENTREXY);
	VSGLUTBUFFER OverlayBuffer;  // ???????
	OverlayBuffer[0]=Background;  //// ????????????????????
	vsgSetCommand(vsgOVERLAYMASKMODE);  // activates the ovrlay masking mode  ????????
	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&OverlayBuffer, 0, 4); ///????????????
	Width = vsgGetScreenWidthPixels();
	Height = vsgGetScreenHeightPixels();
	vsgSetViewDistMM(DistanceToScreen);
	vsgSetSpatialUnits(vsgPIXELUNIT);
	vsgSetDrawMode(vsgCENTREXY);
	//Convert the resolution parameters to visual degrees
  	vsgUnit2Unit(vsgPIXELUNIT,Width,vsgDEGREEUNIT,&Width);
	vsgUnit2Unit(vsgPIXELUNIT,Height,vsgDEGREEUNIT,&Height);
	DrawGrating(FPT,SFd,Ori,Con,SmallSize,LargeSize,X,Y,Signal,Blank,DistanceToScreen,Repeats);
}


int DrawGrating(int FPT, double SFd, double Ori, double Con, double SmallSize, double LargeSize, double X, double Y, double Signal, double Blank, int DistancetoMonitor,int Repeats)
{
	int i, j,k,z, dx,dy, OD, OD2,index2, index4;
	double temp_array[8]= {1, 2, 3, 4, 5, 10, 15, 20}, PhaseSet[8] = {0, 0.7854, 1.5708, 2.3562, 3.1416, 3.9270, 4.7124, 5.4978};
	double Diameter[12], NumTerms,BlankTerms,x,y,RN[200];
	double phase, index, xcor, ycor, phase1, phase2, Size[12], temp, NewBuffer[8][256];
	double SFr,SquareSize, MaxVal, Distance, r, r2, r3,r4;
	double temp_degrees, temp_radians, MMpCYCLE,PIXELSpCYCLE,PIXELSpDegree;
	const double Pi = 3.1416;
	const double MMpPIXEL = .445;
			
	Y=-Y;
			
	X=X+5;
	DistancetoMonitor=1600;

	VSGTRIVAL back;
	back.a = 0.5;	back.b = 0.5;	back.c = 0.5;
	vsgSetBackgroundColour((VSGTRIVAL*)&back);

	vsgSetDrawPage(vsgOVERLAYPAGE,0,0);
	vsgSetPen1(0);
	vsgDrawOval(0,0,600,600);
	vsgPresent();

	NumTerms=Signal/(.00713*FPT);
	BlankTerms=Blank/(.00713*FPT);
	printf("Total number of terms = %f\n%f\n%f\n",NumTerms+BlankTerms,NumTerms,BlankTerms);
	Sleep(2000);
	// SFd is given in degrees.  We need to use the cycles/deg to figure out cycles/rad.  We will use cycle/rad to
	//calculate the number of pixels/cycle
	//First we have SF in degrees.  we chagnge that to radians (radians  = degrees*2*pi / 360)
	// next we calculate the distance across the length of the monitor that represents 1 degree (.0175 radians).
	// for a Distance to monitor of 1200 mm this is tan(.0175) = x/1200.  x= 21.00 mm
	SFr = 57.1429*SFd; /// cycles/degree change to cycles/.0175 radians (same size) 1/.0175 = 57.1429
	temp_radians=(1/SFr);  
	MMpCYCLE = tan(1/SFr) * DistancetoMonitor;
	
	PIXELSpCYCLE = floor(MMpCYCLE/ MMpPIXEL);
	PIXELSpDegree = PIXELSpCYCLE * (SFd);  // the number of pixles for 1 degree
	Diameter[0] = ceil(SmallSize*PIXELSpDegree);  // the number of pixels across for each grating size
	Diameter[1] = ceil(LargeSize*PIXELSpDegree);
	SquareSize=1;
	temp= PIXELSpCYCLE;
    
	struct MyBuffer
	{
		//struct InsideBuffer Indexer[256];
		VSGLUTBUFFER Buffer;
	};
	struct MyBuffer TempBuffer2[64];
	struct MyBufer *TempPointer;
	//VSGLUTBUFFER GrayBuffer;
	for(i=0; i<8; i++)
	{
		if (temp > 256)  /// there are 256 vlaues in the lut, if there are more than 256 pixels /cycle we need
			// to reduce the number by some interger value fraction (i.e., 256/2 =128
		{ 
			SquareSize =1;
			SquareSize = ceil(PIXELSpCYCLE/256);
			PIXELSpCYCLE = floor((PIXELSpCYCLE / SquareSize));
		}
	}
	index2=0;
	index4=0;
	index=0;
    index=0;
	printf("Pixels per Cycles %f",PIXELSpCYCLE);  
	for (i= 0; i<PIXELSpCYCLE; i++)
	{
		index=index+1;
		TempBuffer2[0].Buffer[i].a =(sin(2*Pi*(index/PIXELSpCYCLE)+0)/2)+.5;
		TempBuffer2[0].Buffer[i].b =(sin(2*Pi*(index/PIXELSpCYCLE)+0)/2)+.5;
		TempBuffer2[0].Buffer[i].c =(sin(2*Pi*(index/PIXELSpCYCLE)+0)/2)+.5;
		TempBuffer2[1].Buffer[i].a =(sin(2*Pi*(index/PIXELSpCYCLE)+Pi)/2)+.5;
		TempBuffer2[1].Buffer[i].b =(sin(2*Pi*(index/PIXELSpCYCLE)+Pi)/2)+.5;
		TempBuffer2[1].Buffer[i].c =(sin(2*Pi*(index/PIXELSpCYCLE)+Pi)/2)+.5;
		TempBuffer2[2].Buffer[i].a =.5;
		TempBuffer2[2].Buffer[i].b =.5;
		TempBuffer2[2].Buffer[i].c =.5;	
	}	
	vsgLUTBUFFERWrite(i+1, (VSGLUTBUFFER*)&TempBuffer2[0].Buffer);
	vsgLUTBUFFERWrite(i+1, (VSGLUTBUFFER*)&TempBuffer2[1].Buffer);
	vsgLUTBUFFERWrite(i+1, (VSGLUTBUFFER*)&TempBuffer2[2]);
	index=0;
	x=0;
	OD = 700;
	OD2 = OD/2;
	double NumCycles;
	NumCycles = (OD/PIXELSpCYCLE);
	srand( (unsigned)time( NULL ) );
	//draw each sinwave grating that we need (2 diameters)
	for (z = 0; z<2; z++)
	{
		
		MaxVal = (cos(Ori)*(OD)) + (sin(Ori)*(OD));
		printf("Diameter %i\t  = %f\n",z,Diameter[z]);
		vsgSetDrawPage(vsgVIDEOPAGE,z,vsgBACKGROUND);
		
		for (x= -OD2; x<=OD2; x=x+1)
		{
		
			for (y= -OD2; y<=OD2; y=y+1)
			{			
				xcor = (cos(Ori)*(x+(OD2)));
				ycor = sin(Ori)*(y+OD2);
				phase = NumCycles*2*Pi*((((xcor +ycor)))/MaxVal); 
				//	printf("here %f %f %f %i",PIXELSpCYCLE,MMpPIXEL,MMpCYCLE,DistancetoMonitor);
				//	Sleep(10000);
				while (phase> 2*Pi)
				{
					phase = phase - (2*Pi);
				}
				phase=ceil(((phase*((PIXELSpCYCLE-1)/SquareSize))/(2*Pi)));
			
				dx = abs(x);
				dy = abs(y);
				Distance = sqrt( pow(dx,2) +  pow(dy,2) );	
				if (Distance<= (.5*Diameter[z]))
				{
					vsgSetPen1(phase);
					vsgDrawRect(x+X,y+Y,1,1);
				//	printf("%f\n%f\n%f\n %f\n ",x+X,y+Y,phase,(PIXELSpCYCLE/SquareSize)+1);
				//	Sleep(1000);
				}
				else
				{
					vsgSetPen1(0);
				}
			}
		}
	}
r3=0;
FILE *RandFile;

char FileName[46];
//FileName = "C:\AcuteRig\Area White Noise\CurrentRandData.txt"

RandFile = fopen("C:\AcuteRig\Area White Noise\CurrentRandData.txt","w");

for (i=0; i<=NumTerms; i++)
{
	RN[i]=rand();
	RN[i]=RN[i]/32767;
	if (RN[i]<.5)
	{
		RN[i] = 1;
		fprintf(RandFile,"%f\n",RN[i]);
					
	}
	else
	{
		RN[i] = 0;
		fprintf(RandFile,"%f\n",RN[i]);
	}
}
r3=0;
fclose(RandFile);
for (k=1; k<=Repeats*2; k++)
{
	for (i= 1; i<=NumTerms+BlankTerms; i++)
	{
		for (j = 1; j<=FPT; j++)
		{
			if (i<=NumTerms)
			{
				if (j==1)
				{	
					if (RN[i]==1)
					{
					
						vsgPaletteWrite((VSGLUTBUFFER*)&TempBuffer2[0].Buffer,0,ceil(PIXELSpCYCLE/SquareSize)+1);
						vsgSetDisplayPage(r3+vsgTRIGGERPAGE);
					
						
					
					}
					else
					{
					
						vsgPaletteWrite((VSGLUTBUFFER*)&TempBuffer2[1].Buffer,0,ceil(PIXELSpCYCLE/SquareSize)+1);		
						vsgSetDisplayPage(r3+vsgTRIGGERPAGE);
					
					}
				}	
				else
				{
					vsgSetZoneDisplayPage(vsgVIDEOPAGE,r3);
					
				
				}
			}
			else
			{
				if (j==1)
				{	
						vsgPaletteWrite((VSGLUTBUFFER*)&TempBuffer2[2].Buffer,0,ceil(PIXELSpCYCLE/SquareSize)+1);
						vsgSetDisplayPage(r3+vsgTRIGGERPAGE);
					
				
				}	
				else
				{
					vsgSetZoneDisplayPage(vsgVIDEOPAGE,r3);
				
				
				}
			}
		}
	}
	if (r3==1)
	{	
		r3=0;
	}
	else
	{
		r3=1;
	}
}

	return (1);
}

