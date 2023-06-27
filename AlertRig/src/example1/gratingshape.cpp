#include <iostream>
#include "Alertlib.h"
#include "Alertutil.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif


using namespace std;

int main(int argc, char *argv[])
{
	alert::ARGratingSpec grating;

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
	vsgSetBackgroundColour(&back);

	vsgSetViewDistMM(500);
	double WDEG, HDEG;
	vsgUnit2Unit(vsgPIXELUNIT, W, vsgDEGREEUNIT, &WDEG);
	vsgUnit2Unit(vsgPIXELUNIT, H, vsgDEGREEUNIT, &HDEG);
	cerr << "Screen width in degrees: " << WDEG << endl;
	vsgSetSpatialUnits(vsgDEGREEUNIT);

	// create host page
	long handle = vsgPAGECreate(vsgHOSTPAGE, 1024, 768, vsg8BITPALETTEMODE);

	// Reserve a low level
	PIXEL_LEVEL myLowLevel = ARvsg::instance().request_single();

	// grating
	string sGrating = "0,0,15,15,100,.1,.25,45,b,s,e";
	if (parse_grating(sGrating, grating))
		cerr << "Cannot parse grating " << sGrating << endl;
	grating.init(40);

	// Reserve a HIGH level and clear host page to that level
	PIXEL_LEVEL myHighLevel = ARvsg::instance().request_single();
	vsgSetDrawPage(vsgHOSTPAGE, handle, myHighLevel);

	// draw shape on host page at low level
	vsgSetPen1(myLowLevel);
	vsgDrawBar(0, 0, 12, 4, 30);

	//Draw the grating
	//vsgSetDrawMode(vsgCENTREXY + vsgTRANSONLOWER);
	grating.draw()
	vsgDrawGrating(0, 0, WDEG, HDEG, 45, 2.0);

	// copy to page 0
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);


	vsgSetDrawPage(vsgVIDEOPAGE, 0, 0);
	vsgSetDrawMode(vsgCENTREXY + vsgTRANSONSOURCE);
	vsgSetPen2(244);
	vsgDrawMoveRect(vsgHOSTPAGE, handle, 0, 0, 15, 10, 0, 0, 15, 10);

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
