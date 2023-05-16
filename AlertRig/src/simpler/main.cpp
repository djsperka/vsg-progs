#include "VSGV8.h"
#include <iostream>
using namespace std;

#pragma comment(lib, "vsgv8.lib")

void doCycling()
{
	int i;

	VSGCYCLEPAGEENTRY cycle[100];
	cycle[0].Frames = 30;
	cycle[0].Page = 0 + vsgDUALPAGE;
	cycle[0].ovPage = 0;
	cycle[0].ovXpos = 0;
	cycle[0].ovYpos = 0;
	cycle[0].Xpos = 0;
	cycle[0].Ypos = 0;
	cycle[0].Stop = 0;

	cycle[1].Frames = 30;
	cycle[1].Page = 1 + vsgDUALPAGE;
	cycle[1].ovPage = 0;
	cycle[1].ovXpos = 0;
	cycle[1].ovYpos = 0;
	cycle[1].Xpos = 0;
	cycle[1].Ypos = 0;
	cycle[1].Stop = 0;

	cycle[2].Frames = 30;
	cycle[2].Page = 1 + vsgDUALPAGE;
	cycle[2].ovPage = 1;
	cycle[2].ovXpos = 0;
	cycle[2].ovYpos = 0;
	cycle[2].Xpos = 0;
	cycle[2].Ypos = 0;
	cycle[2].Stop = 0;

	cycle[3].Frames = 30;
	cycle[3].Page = 0 + vsgDUALPAGE;
	cycle[3].ovPage = 1;
	cycle[3].ovXpos = 0;
	cycle[3].ovYpos = 0;
	cycle[3].Xpos = 0;
	cycle[3].Ypos = 0;
	cycle[3].Stop = 0;

	i = vsgPageCyclingSetup(4, &cycle[0]);
	cerr << "vsgPageCyclingSetup " << i << endl;
	vsgSetCommand(vsgCYCLEPAGEENABLE);
}



int main(int argc, char **argv)
{
	int nOverlay;
	int Width, Height;
	double Buff[12];
	int page;
	string str;

	if (vsgInit("")<0) exit(0);


	nOverlay = vsgGetSystemAttribute(vsgNUMOVERLAYPAGES);
	cout << "Have " << nOverlay << " overlay pages." << endl;

	//Find out the horizontal and vertical resolution of the vsg screen.
	Width = vsgGetScreenWidthPixels();
	Height = vsgGetScreenHeightPixels();


	//Set lookup table 0 in palette 0 & 1 to black and white.
	// add another two colors for second checkerboard
	Buff[0] = 1.0;
	Buff[1] = 1.0;
	Buff[2] = 1.0;
	Buff[3] = 0;
	Buff[4] = 0;
	Buff[5] = 0;
	Buff[6] = 0.6;
	Buff[7] = 0.8;
	Buff[8] = 0.3;
	Buff[9] = 0.3;
	Buff[10] = 0.2;
	Buff[11] = 0.9;

	vsgPaletteWrite((VSGLUTBUFFER*)&Buff, 0, 4);

	//Draw a chess board to cover page 0.
	vsgSetDrawPage(vsgVIDEOPAGE, 0, 255);
	vsgSetDisplayPage(0);

	vsgSetPen1(1);
	vsgSetPen2(0);
	vsgSetDrawMode(vsgCENTREXY+vsgSOLIDFILL);
	vsgDrawChessboard(0, 0, Width, Height, div(Width, 8).quot, div(Height, 8).quot, 0);

	//Draw a chess board to cover page 1.
	vsgSetDrawPage(vsgVIDEOPAGE, 1, 255);
	vsgSetDisplayPage(0);

	vsgSetPen1(3);
	vsgSetPen2(2);
	vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
	vsgDrawChessboard(0, 0, Width, Height, div(Width, 8).quot, div(Height, 8).quot, 0);


//Change the drawing modes to target the Overlay.
	vsgSetCommand(vsgOVERLAYDRIFT + vsgVIDEODRIFT + vsgOVERLAYMASKMODE);

//Clear all of the Overlay memory to pixel-level(1).
	vsgSetPen2(1);
	vsgSetCommand(vsgOVERLAYCLEAR);

//Fill Overlay pixel-level(1) with red.
	Buff[1] = 0.0;
	Buff[2] = 0.0;
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, vsgNOCLEAR);
	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&Buff, 1, 1);

//Draw a window in the Overlay page (in vsgOVERLAYMASKMODE anything drawn in pixel-
//level(0) is transparent).
	vsgSetPen1(0);
	vsgDrawRect(0, 0, div(Width, 2).quot, div(Height, 4).quot);


	vsgSetDrawPage(vsgOVERLAYPAGE, 1, vsgNOCLEAR);
	vsgSetPen1(0);
	vsgDrawOval(0, 0, 100, 50);

	//vsgSetDrawPage(vsgOVERLAYPAGE, 2, vsgNOCLEAR);
	//vsgSetPen1(0);
	//vsgDrawRect(20, 20, div(Width, 2).quot, div(Height, 4).quot);
	//
	//vsgSetDrawPage(vsgOVERLAYPAGE, 3, vsgNOCLEAR);
	//vsgSetPen1(0);
	//vsgDrawRect(30, 30, div(Width, 2).quot, div(Height, 4).quot);



//Display OVERLAYPAGE 0.
	vsgSetDisplayPage(0);

	cout << "Enter page(0/1/2/3): ";
	cin >> page;
	while (page > -1 && page < 6)
	{
		if (page == 0)
			vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);
		else if (page == 1)
			vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);
		else if (page == 2)
			vsgSetDisplayPage(0);
		else if (page == 3)
			vsgSetDisplayPage(1);
		else if (page == 4)
			doCycling();
		else if (page == 5)
			vsgSetCommand(vsgCYCLEPAGEDISABLE);

		cout << "Enter page(0/1/2/3): ";
		cin >> page;
	}
	return 0;
}




#if 0
//******************************************************************************
//Demo for drifting the Overlay page over the video page.
//written by: A.Lucas-Dean
//date: 26/10/99
//C++Builder conversion by: S.Barry
//******************************************************************************
#include <vcl\vcl.h>
#include <direct.h>
#include <math.h>
#pragma hdrstop

#include "VSGV8.h"
#include "Unit1.h"
//---------------------------------------------------------------------------
#pragma resource "*.dfm"
TForm1 *Form1;
bool CloseFlag;
TaThread *aThread;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
  : TForm(Owner)
{

}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormCreate(TObject *Sender)
{
    int CheckCard;
//Initialise the vsg card then check that it initialised O.K.
    CheckCard = vsgInit("");
    if (CheckCard < 0) exit(0);

//Create the Thread that will setup and control the animation.
    aThread = new TaThread(true);
    aThread->Resume();

}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
if (aThread != NULL)
   {
     aThread->Terminate();
     aThread->WaitFor();
     aThread = NULL;
   }
}
//---------------------------------------------------------------------------
void __fastcall TaThread::Execute()
{
int Width, Height;
double Buff[6];
int Ypos, Offset;
int Shift;

//Find out the horizontal and vertical resolution of the vsg screen.
  Width = vsgGetScreenWidthPixels();
  Height = vsgGetScreenHeightPixels();

//Clear a drawing page in memory then display it.
  vsgSetDrawPage(vsgVIDEOPAGE, 0, 255);
  vsgSetDisplayPage(0);

//Set lookup table 0 in palette 0 & 1 to black and white.
  Buff[0] = 1.0;
  Buff[1] = 1.0;
  Buff[2] = 1.0;
  Buff[3] = 0;
  Buff[4] = 0;
  Buff[5] = 0;
  vsgPaletteWrite((VSGLUTBUFFER*)&Buff, 0, 2);
//Draw the chess board to cover the whole page.

  vsgSetPen1(1);
  vsgSetPen2(0);
  vsgSetDrawMode(vsgCENTREXY+vsgSOLIDFILL);
  vsgDrawChessboard(0, 0, Width, Height, div(Width, 8).quot, div(Height, 8).quot, 0);

//Change the drawing modes to target the Overlay.
  vsgSetCommand(vsgOVERLAYDRIFT+vsgOVERLAYMASKMODE);

//Clear all of the Overlay memory to pixel-level(1).
  vsgSetPen2(1);
  vsgSetCommand(vsgOVERLAYCLEAR);

//Fill Overlay pixel-level(1) with red.
  Buff[1] = 0.0;
  Buff[2] = 0.0;
  vsgSetDrawPage(vsgOVERLAYPAGE, 1, vsgNOCLEAR);
  vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&Buff, 1, 1);

//Draw a window in the Overlay page (in vsgOVERLAYMASKMODE anything drawn in pixel-
//level(0) is transparent).
  vsgSetPen1(0);
  vsgDrawRect(0, 0, div(Width, 2).quot, div(Height, 4).quot);

//Display OVERLAYPAGE 1.
  vsgSetDisplayPage(1);

//Move the Overlay page up and down over the chessboard using vsgMoveScreen and
//different values of Ypos.
  Ypos = 0;
  Shift = 1;
  Offset = div(Height, 4).quot;
  do
  {
    vsgMoveScreen(0, Ypos);
    if ((Ypos < -Offset) | (Ypos > Offset))
    {
      Shift = Shift*-1;
    }
    Ypos = Ypos+Shift;
  }
  while ( Terminated != true);
}
//---------------------------------------------------------------------------

__fastcall TaThread::TaThread(bool CreateSuspended)
  : TThread(CreateSuspended)
{

}
//---------------------------------------------------------------------------
#endif