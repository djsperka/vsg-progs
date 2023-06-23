#include <iostream>
#include "VSGV8.h"
using namespace std;

//char *f_imagefile = "c:\\Users\\djsperka\\Downloads\\bitmap.bmp";
//char *f_imagefile = "e:\\work\\img\\pepper.bmp";
char *f_imagefile = NULL;

int main(int argc, char *argv[])
{
	cout << "init vsg..." << endl;
	if (vsgInit("") < 0)
	{
		cout << "Cannot init" << endl;
		exit(0);
	}

	vsgSetCommand(vsgPALETTECLEAR);
	long W = vsgGetScreenWidthPixels();
	long H = vsgGetScreenHeightPixels();
	VSGTRIVAL from, to, back;
	from.a = from.b = from.c = 0;
	to.a = to.b = to.c = 1;
	back.a = back.b = back.c = 0.5;

	vsgSetViewDistMM(500);
	double WDEG, HDEG;
	vsgUnit2Unit(vsgPIXELUNIT, W, vsgDEGREEUNIT, &WDEG);
	vsgUnit2Unit(vsgPIXELUNIT, H, vsgDEGREEUNIT, &HDEG);
	cerr << "Screen width in degrees: " << WDEG << endl;
	vsgSetSpatialUnits(vsgDEGREEUNIT);

	// clear page
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);

	// create host page
	long handle = vsgPAGECreate(vsgHOSTPAGE, 1024, 768, vsg8BITPALETTEMODE);
	vsgSetDrawPage(vsgHOSTPAGE, handle, vsgBACKGROUND);

	// draw shape at level 1
	vsgSetPen1(1);
	vsgDrawBar(0, 0, 12, 4, 30);

	vsgObjCreate();
	vsgObjSetDefaults();
	vsgObjSetContrast(100);
	vsgSetBackgroundColour(&back);
	vsgObjSetPixelLevels(2, 100);
	vsgObjTableSinWave(vsgSWTABLE);
	vsgObjSetColourVector(&from, &to, vsgBIPOLAR);
	vsgObjSetDriftVelocity(.25);

	////Select the range of maximum pixel-levels to draw the grating with.
	vsgSetPen1(2);
	vsgSetPen2(101);

	//Draw the grating centered in the middle of the screen.
	vsgSetDrawMode(vsgCENTREXY + vsgTRANSONLOWER);
	vsgDrawGrating(0, 0, WDEG, HDEG, 45, 2.0);

	// copy to page 0
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	vsgSetDrawMode(vsgCENTREXY);
	vsgDrawMoveRect(vsgHOSTPAGE, handle, 0, 0, 15, 8, 0, 0, 15, 8);

	//Display the object.
	vsgPresent();


}



#if 0

//Clear the palette to Black so no drawing will be seen.
vsgSetCommand(vsgPALETTECLEAR);

//Clear a drawing page to the background colour.
vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);

//Set the current display page.
vsgSetDisplayPage(0);

//Find out the horizontal and vertical resolution of the vsg screen.
Width: = vsgGetScreenWidthPixels;
Height: = vsgGetScreenHeightPixels;

//Load a TRIVAL with the colour white.
From.a : = 1.0;
From.b : = 1.0;
From.c : = 1.0;

//Load a TRIVAL with the colour black.
Too.a : = 0.0;
Too.b : = 0.0;
Too.c : = 0.0;

Back.a: = (From.a + Too.a) / 2;
Back.b: = (From.b + Too.b) / 2;
Back.c: = (From.c + Too.c) / 2;
//Create a stimulus object.
vsgObjCreate;

//Load the default parameters for the stimulus object.
vsgObjSetDefaults;

//Set the background to mean grey.
vsgSetBackgroundColour(Back);

//Assign the maximum amount of pixel-levels to be used for the object.
vsgObjSetPixelLevels(1, 200);

//Load the object with a sin wave form.
vsgObjTableSinWave(vsgSWTABLE);

//Load the object with a colour vector (the black and white vsgTRIVALS that were
//loaded earlier.
vsgObjSetColourVector(From, Too, vsgBIPOLAR);

//Select the range of maximum pixel-levels to draw the grating with.
vsgSetPen1(1);
vsgSetPen2(200);

//Draw the grating centered in the middle of the screen.
vsgDrawGrating(0, 0, Width div 2, Height div 2, 45, 2.0);

//Display the object.
vsgPresent;



#endif
