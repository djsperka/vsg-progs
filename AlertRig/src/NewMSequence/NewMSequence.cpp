// NewMSequence.cpp : Defines the entry point for the console application.
//

#include "NewMSequence.h"
//#include "VSGEX2.H"
#include "vsgv8.h"
#include <math.h>
#include <iostream>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#define NumberOfTerms 32767

#pragma comment(lib, "vsgv8.lib")

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

//CWinApp theApp;

using namespace std;

int main(int argc, char* argv[], char* envp[])
{
	int nRetCode = 0;

	int NumberOfRepeats;
	int NumberOfPixels=16,FramesPerTerm,DotSize;
	short PixelWidth=12, PixelHeight=3;
	short Width,Height;
	int X, Y;
	short ApertureX,ApertureY;
	double ScreenHeight,ScreenWidth;

	FramesPerTerm=atoi(argv[1]);
	DotSize=atoi(argv[2]);
	X=atoi(argv[3]);
	Y=atoi(argv[4]);
	NumberOfRepeats=atoi(argv[5]);
	PixelWidth=DotSize;
	PixelHeight=DotSize/4;
	Width=NumberOfPixels*PixelWidth;
	Height=NumberOfPixels*PixelHeight;

	cout << "pix width " << PixelWidth << " pix height " << PixelHeight << endl;
//Initialize the VSG
	nRetCode=vsgInit("");
	ScreenHeight=vsgGetSystemAttribute(vsgSCREENHEIGHT);
	ScreenWidth=vsgGetSystemAttribute(vsgSCREENWIDTH);

	ApertureX=(short)ScreenWidth/2  +(short)X;
	ApertureY=(short)ScreenHeight/2 +(short)Y;
	vsgSetDrawOrigin(0,0);
 	vsgSetVideoMode(vsgPANSCROLLMODE);
	vsgSetVideoMode(vsgZOOM4);
//	vsgSetSpatialUnits(vsgDEGREEUNIT);


//Setup the overlay page
	VSGLUTBUFFER Overlay;
	Overlay[1].a=.5; Overlay[1].b=.5; Overlay[1].c=.5;
	vsgSetCommand(vsgOVERLAYMASKMODE);

	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&Overlay, 0, 2);

	vsgSetDrawPage(vsgOVERLAYPAGE, 1, 1);
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1 );
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE,1);
	vsgSetPen1(0);
	vsgDrawRect(ApertureX,ApertureY,Width,Height*4);

//Read the M sequence terms into memory
	FILE *stream=NULL;
	int MSequence[NumberOfTerms],term;
	int index;
	fopen_s(&stream, "c:\\AcuteRig\\MSequenceTerms", "r");
	if (stream== NULL)
	{
		printf("Could not find M-sequenceFile\n");
		_exit(0);
	}
	for (index=0; index <= NumberOfTerms; index++)
		fscanf_s(stream,"%1d",&MSequence[index]);
	fclose(stream);
	//vsgMoveScreen(-ApertureX+Width/2,-ApertureY+Width/2);
//Draw the M msequence terms into video memory
	vsgSetPen2(0);
	vsgSetCommand(vsgVIDEOCLEAR);
	vsgSetDrawPage(vsgVIDEOPAGE,0,vsgNOCLEAR);
	
//	vsgSetDisplayPage(0);
	vsgSetPen1(255);

	int rindex, cindex;


	cout << "preparing video memory...." << endl;
	for (rindex=0; rindex<128*16+15; rindex++)
	{
		for (cindex=0; cindex<31; cindex++)
		{
			term=(16*128*rindex+128*cindex) % NumberOfTerms;
			if (MSequence[term]==1) 
			{
				vsgDrawRect(PixelWidth*(cindex+.5),PixelHeight*(rindex+.5),PixelWidth,PixelHeight);
			}
			
		}		
	}

//Create a VSGPAGECYCLE buffer to hold the different positions in video memory
//corresponding to our stimulus. We can only display about 2000 of them at a time,
//due to VSG limitations.

	cout << "preparing page cycling buffer......" << endl;
	VSGCYCLEPAGEENTRY MPositions[32768];
	short row, column;
	for (index=0; index < 32767; index++)
	{
		row=(index%128)*Height+PixelHeight*(index/(128*16));
		column=(short)((index/128)%16);
		MPositions[index].Page=0+vsgDUALPAGE+vsgTRIGGERPAGE ;
		MPositions[index].Xpos=-ApertureX+Width/2 +column*PixelWidth ;
		MPositions[index].Ypos=-ApertureY/4+Height/2+row;
		MPositions[index].Frames=FramesPerTerm;
		MPositions[index].Stop=0;
		MPositions[index].ovPage=0;
		MPositions[index].ovXpos=0;
		MPositions[index].ovYpos=0;
	}
	MPositions[32767].Page=0+vsgDUALPAGE;
	MPositions[32767].Xpos=0;
	MPositions[32767].Ypos=0;
	MPositions[32767].Frames=1;
	MPositions[32767].ovPage=1;
	MPositions[32767].ovXpos=0;
	MPositions[32767].ovYpos=0;
	MPositions[32767].Stop=1;

	long endcycle=0;

	cout << "start cycling: " << NumberOfRepeats << " repeats, " << FramesPerTerm << " frames per term" << endl;
	for (index=0; index<NumberOfRepeats; index++)
	{
		vsgSetCommand(vsgVIDEODRIFT+vsgOVERLAYDRIFT);
		vsgPageCyclingSetup(32768,&MPositions[0]);
		Sleep(1000);
		cout << "cycle " << index << ", sleep for " << (32766) * ( (double)(7.14) * (double)FramesPerTerm)/1000 << " sec " << endl;
		vsgSetCommand(vsgCYCLEPAGEENABLE);
		Sleep((DWORD)((double)(32766) * ( (double)(7.14) * (double)FramesPerTerm)));
		cout << "Done sleeping" << endl;
		endcycle=vsgGetSystemAttribute(vsgPAGECYCLINGSTATE);
		while(endcycle>-1)
		{
			endcycle=vsgGetSystemAttribute(vsgPAGECYCLINGSTATE);
		//	cout << endcycle <<endl;
		}
	}

	return 0;
}


