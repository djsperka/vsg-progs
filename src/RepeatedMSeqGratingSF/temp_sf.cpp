// **************************************************************************
// Page and LUT cycling demo
// written by: A.Lucas-Dean, Cambridge Research Systems Ltd
// C++Builder conversion by Robert Shiells, April 2003
// **************************************************************************

//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"
#include <vsgex2.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormCreate(TObject *Sender)
{
  int CheckCard;
  VSGCYCLEPAGEENTRY pages[2];
  VSGLUTBUFFER Buff;
  int i;
  int Hgt;

  // Initialise the vsg card then check that it initialised O.K.
  CheckCard = vsgInit("");
  if (CheckCard < 0) exit(0);

  // Clear a drawing page in memory and display it.
  vsgSetDrawPage(vsgVIDEOPAGE, 0, 0);
  vsgSetDisplayPage(0);

  // Load a vsgLUTBUFFER for page 1
  for (i=0; i<=255; i++) {
    if (i < 128) {
      Buff[i].a = (double)i/128;
      Buff[i].b = (double)i/128;
      Buff[i].c = (double)i/128;
    } else {
      Buff[i].a = 0.0;
      Buff[i].b = 0.0;
      Buff[i].c = 0.0;
    }
  }
  vsgLUTBUFFERWrite(0, &Buff);

  // Load a vsgLUTBUFFER for page 2
  for (i=0; i<=255; i++) {
    if (i < 128) {
      Buff[i].a = 0.0;
      Buff[i].b = 0.0;
      Buff[i].c = 0.0;
    } else {
      Buff[i].a = (double)(i-128)/128;
      Buff[i].b = 0.0;
      Buff[i].c = 0.0;
    }
  }
  vsgLUTBUFFERWrite(1, &Buff);


  // Draw the text onto the video pages.
  Hgt = vsgGetScreenHeightPixels();
  vsgSetTrueTypeFont("Arial");
  vsgSetStringMode(0, Hgt/8, vsgALIGNCENTRETEXT, vsgALIGNCENTRETEXT, 0, 0);
  vsgSetDrawMode(vsgCENTREXY + vsgANTIALIASED);

  // Page 1
  vsgSetDrawPage(vsgVIDEOPAGE, 1, 0);
  vsgSetPen1(127);
  vsgSetPen2(0);
  vsgDrawString(0, -Hgt/6, "This should be White!");

  vsgSetPen1(255);
  vsgSetPen2(128);
  vsgDrawString(0, Hgt/6, "Error!");

  // Page 2
  vsgSetDrawPage(vsgVIDEOPAGE, 2, 0);
  vsgSetPen1(255);
  vsgSetPen2(128);
  vsgDrawString(0, Hgt/6, "This should be Red!");

  vsgSetPen1(127);
  vsgSetPen2(0);
  vsgDrawString(0, -Hgt/6, "Error!");

  // Initialise page cycle data array
  for (i=0; i<=1; i++) {
    pages[i].Page   = i+1;
    pages[i].Xpos   = 0;
    pages[i].Ypos   = 0;
    pages[i].ovPage = 0;
    pages[i].ovXpos = 0;
    pages[i].ovYpos = 0;
    pages[i].Frames = 1;
    pages[i].Stop   = 0;
  }

  // Simultaneously start page cycling and LUTBUFFER cycling
  vsgPageCyclingSetup(2, &pages[0]);
  vsgLUTBUFFERCyclingSetup(0, 1, 0, 1, 1, 0, 0);
  vsgSetCommand(vsgCYCLELUTENABLE + vsgCYCLEPAGEENABLE);

  // Close the program.
  exit(0);
}
//---------------------------------------------------------------------------
