#include <stdio.h>
#include "VSGEX2.H"
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
	int CheckCard, FPT, i, DistanceToScreen,Repeats;
	double Height,Width,SFd[2], Ori, Con, Size, Signal, Blank,X,Y;
	int DrawGrating(int FPT, double SFd[2], double Ori, double Con, double Size, double X, double Y, double Signal, double Blank, int DistanceToScreen,int Repeats);

	VSGTRIVAL Background;
	
	Background.a = 0.5;	Background.b = 0.5;	Background.c = 0.5;

	Size = atof(Arguments[1]);
	Con = atof(Arguments[2]);
	SFd[0] = atof(Arguments[3]);
	SFd[1] = atof(Arguments[4]);
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
	DrawGrating(FPT,SFd,Ori,Con,Size,X,Y,Signal,Blank,DistanceToScreen,Repeats);
}


int DrawGrating(int FPT, double SFd[2], double Ori, double Con, double Size, double X, double Y, double Signal, double Blank, int DistancetoMonitor,int Repeats)
{
	int i, j,k,z, dx,dy, OD, OD2,index2, index4;
	double Diameter[12], NumTerms,BlankTerms,x,y,RN[200];
	double phase,xphase,yphase, index, xcor, ycor, phase1, phase2, temp,tempPC[2], NewBuffer[8][256], SX[2],SY[2];
	double SFr[2],SquareSize[2], MaxVal, Distance, r, r2, r3,r4[2];
	double temp_degrees, MMpCYCLE[2],PIXELSpCYCLE[2],PIXELSpDegree[2],step;
	const double Pi = 3.1416;
	const double MMpPIXEL = .445;
	int temp_int;
	temp_int=1.23;
	printf("Int is %i\n",temp_int);
			
	Y=-Y;
			
	X=X;
	//DistancetoMonitor=1600;

	VSGTRIVAL back;
	back.a = 0.5;	back.b = 0.5;	back.c = 0.5;
	vsgSetBackgroundColour((VSGTRIVAL*)&back);

	vsgSetDrawPage(vsgOVERLAYPAGE,0,0);
	vsgSetPen1(0);
	vsgDrawOval(900,900,600,600);
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
	SFr[0] = 57.1429*SFd[0]; /// cycles/degree change to cycles/.0175 radians (same size) 1/.0175 = 57.1429
	SFr[1] = 57.1429*SFd[1]; 
 
	MMpCYCLE[0] = tan(1/SFr[0]) * DistancetoMonitor;
	MMpCYCLE[1] = tan(1/SFr[1]) * DistancetoMonitor;
	
	PIXELSpCYCLE[0] = floor(MMpCYCLE[0]/ MMpPIXEL);
	PIXELSpCYCLE[1] = PIXELSpCYCLE[0] / (SFd[1]/SFd[0]);//floor(MMpCYCLE[1]/ MMpPIXEL);
	PIXELSpDegree[0] = PIXELSpCYCLE[0] * (SFd[0]);  // the number of pixles for 1 degree
	PIXELSpDegree[1] = PIXELSpCYCLE[1] * (SFd[0]);
	Diameter[0] = ceil(Size*PIXELSpDegree[0]);  // the number of pixels across for each grating size


    
	struct MyBuffer
	{
		//struct InsideBuffer Indexer[256];
		VSGLUTBUFFER Buffer;
	};
	struct MyBuffer TempBuffer2[64];
	struct MyBufer *TempPointer;

	TempBuffer2[0].Buffer[0].a =.5;
	TempBuffer2[0].Buffer[0].b =.5;
	TempBuffer2[0].Buffer[0].c =.5;	
	TempBuffer2[1].Buffer[0].a =.5;
	TempBuffer2[1].Buffer[0].b =.5;
	TempBuffer2[1].Buffer[0].c =.5;	
	TempBuffer2[2].Buffer[0].a =.5;
	TempBuffer2[2].Buffer[0].b =.5;
	TempBuffer2[2].Buffer[0].c =.5;	
	TempBuffer2[3].Buffer[0].a =.5;
	TempBuffer2[3].Buffer[0].b =.5;
	TempBuffer2[3].Buffer[0].c =.5;	
	TempBuffer2[4].Buffer[0].a =.5;
	TempBuffer2[4].Buffer[0].b =.5;
	TempBuffer2[4].Buffer[0].c =.5;	
//Both buffers have the same number of pixels,so the two frequencies should be multiples
//each other
	index=1;
	FILE *SinFile;
	SinFile = fopen("C:\\SinFile.txt","w");
	for (i= 1; i<=200; i++)
	{
		fprintf(SinFile,"%f\n",(cos(2*Pi*(index/200)+0)/2)+.5);
		TempBuffer2[0].Buffer[i].a =(cos(2*Pi*(index/200)+0)/2)+.5;
		TempBuffer2[0].Buffer[i].b =(cos(2*Pi*(index/200)+0)/2)+.5;
		TempBuffer2[0].Buffer[i].c =(cos(2*Pi*(index/200)+0)/2)+.5;
		TempBuffer2[1].Buffer[i].a =(cos(2*Pi*(index/200)+Pi)/2)+.5;
		TempBuffer2[1].Buffer[i].b =(cos(2*Pi*(index/200)+Pi)/2)+.5;
		TempBuffer2[1].Buffer[i].c =(cos(2*Pi*(index/200)+Pi)/2)+.5;
		TempBuffer2[4].Buffer[i].a =.5;
		TempBuffer2[4].Buffer[i].b =.5;
		TempBuffer2[4].Buffer[i].c =.5;	
		TempBuffer2[2].Buffer[i].a =(cos(2*Pi*(index/200)+0)/2)+.5;
		TempBuffer2[2].Buffer[i].b =(cos(2*Pi*(index/200)+0)/2)+.5;
		TempBuffer2[2].Buffer[i].c =(cos(2*Pi*(index/200)+0)/2)+.5;
		TempBuffer2[3].Buffer[i].a =(cos(2*Pi*(index/200)+Pi)/2)+.5;
		TempBuffer2[3].Buffer[i].b =(cos(2*Pi*(index/200)+Pi)/2)+.5;
		TempBuffer2[3].Buffer[i].c =(cos(2*Pi*(index/200)+Pi)/2)+.5;
		index=index+1;
	}	
		TempBuffer2[4].Buffer[255].a =1;
		TempBuffer2[4].Buffer[255].b =0;
		TempBuffer2[4].Buffer[255].c =0;	
	fclose(SinFile);
	vsgLUTBUFFERWrite(0, (VSGLUTBUFFER*)&TempBuffer2[0].Buffer);
	vsgLUTBUFFERWrite(1, (VSGLUTBUFFER*)&TempBuffer2[1].Buffer);
	vsgLUTBUFFERWrite(2, (VSGLUTBUFFER*)&TempBuffer2[2].Buffer);
	vsgLUTBUFFERWrite(3, (VSGLUTBUFFER*)&TempBuffer2[3].Buffer);
	vsgLUTBUFFERWrite(4, (VSGLUTBUFFER*)&TempBuffer2[4].Buffer);
	index=0;
	x=0;



	srand( (unsigned)time( NULL ) );
	//draw each sinwave grating that we need (2 diameters)
	vsgSetDrawPage(vsgVIDEOPAGE,4,vsgBACKGROUND);
	vsgLUTBUFFERtoPalette(4);
	printf("Here 1");
	vsgSetPen1(255);
	printf("Here");
	
	vsgDrawRect(0,0,900,900);
	SX[0] = (X-450);
	SX[1] = (X+450);
	SY[0] = (Y-450);
	SY[1] = (Y+450);
	

	for (z = 0; z<=1; z++)
	{
		temp_int=101;
		vsgSetDrawPage(vsgVIDEOPAGE,z,vsgBACKGROUND);
		vsgSetPen1(0);
		vsgDrawRect(0,0,900,900);
		step= (1/PIXELSpCYCLE[z]);
		xphase=0;
		yphase=0;
		phase=0;
		for (x= SX[0]; x<=SX[1]; x=x+1)
		{
			xphase=xphase+step;
			yphase=0;
		//	printf("phase %f\n",phase);
		//	Sleep(100);
			for (y= SY[0]; y<=SY[1]; y=y+1)
			{	
				yphase = yphase+step;
				xcor = cos(Ori)*(xphase);
				ycor = sin(Ori)*(yphase);
				phase = xcor+ycor; 
				
				
			while (phase>= 1)
				{
					phase = phase - 1;
				}
			//	printf("phase %f\t%f\n ",phase,phase%1);
			//	phase=phase%1;
			//	printf("phase %f\t%f\t ",phase/(2*Pi),256*phase/(2*Pi));

				phase = ceil(200*phase);
				if (phase==0)
				{
				printf("phase %f\n",phase);	
				}
			//	
			//	Sleep(100);
				dx = abs(x);
				dy = abs(y);
				Distance = sqrt( pow(dx,2) +  pow(dy,2) );
				
			//	printf("phase %f\n",phase);
			//	Sleep(100);
				temp_int=temp_int+1;
			
				if (Distance<= (.5*Diameter[0]))
				{
					vsgSetPen1(phase);
					vsgDrawRect(x+X,y+Y,1,1);
				//	printf("%f\n%f\n%f\n %f\n ",x+X,y+Y,phase,(PIXELSpCYCLE/SquareSize)+1);
				//	Sleep(1000);
				}
				else
				{
				//	vsgDrawRect(x+X,y+Y,1,1);
				//	vsgSetPen1(0);
				}
			}
		}
	}
r3=0;
r4[0]=0;
r4[1]=1;
FILE *RandFile;

char FileName[46];
//FileName = "C:\AcuteRig\Area White Noise\CurrentRandData.txt"
printf("hgere");
RandFile = fopen("C:\\CurrentRandDataSF.txt","w");

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
						vsgLUTBUFFERtoPalette(r4[0]);
						//vsgPaletteWrite((VSGLUTBUFFER*)&TempBuffer2[0].Buffer,0,ceil(PIXELSpCYCLE/SquareSize)+1);
						vsgSetDisplayPage(r3+vsgTRIGGERPAGE);
					
						
					
					}
					else
					{
						vsgLUTBUFFERtoPalette(r4[1]);
					//	vsgPaletteWrite((VSGLUTBUFFER*)&TempBuffer2[1].Buffer,0,ceil(PIXELSpCYCLE/SquareSize)+1);		
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
						vsgLUTBUFFERtoPalette(4);
						//vsgPaletteWrite((VSGLUTBUFFER*)&TempBuffer2[2].Buffer,0,ceil(PIXELSpCYCLE/SquareSize)+1);
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
		r4[0]=0;
		r4[1]=1;
	}
	else
	{
		r3=1;
		r4[0]=2;
		r4[1]=3;
	}
}

	return (1);
}

