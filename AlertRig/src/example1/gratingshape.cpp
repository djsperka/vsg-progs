#include <iostream>
#include <cmath>
#include "Alertlib.h"
#include "Alertutil.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif


using namespace std;

//void doOnHost();
void doOnScratch(double x, double y, double w, double h, double ori, bool bScratch);

int main(int argc, char* argv[])
{
	string s;
	double x, y, w, h, ori;
	bool bScr;
	if (argc == 7)
	{
		x = atof(argv[1]);
		y = atof(argv[2]);
		w = atof(argv[3]);
		h = atof(argv[4]);
		ori = atof(argv[5]);
		bScr = (argv[6][0] == 't' || argv[6][0] == 'T');
	}
	else
	{
		cout << "x y w h ang scr?: ";
		cin >> x >> y >> w >> h >> ori >> boolalpha >> bScr;

	}

	cout << "init vsg..." << endl;
	if (vsgInit("") < 0)
	{
		cout << "Cannot init" << endl;
		exit(0);
	}

	doOnScratch(x, y, w, h, ori, bScr);
}

// return bbox in rect[4] in order: llx, lly, urx, ury
// ori = 0 is horizontal, towards RHS. Pos angles counter-clockwise
void getBBox(double x, double y, double w, double h, double ori, double* rect)
{
	MyRot mr(ori);
	double llx = 99999999;
	double lly = 99999999;
	double urx = -99999999;
	double ury = -99999999;
	double xp, yp;
	// rotate upper right corner of box, measured relative to center pt. 
	mr.rotatePoint(w/2, h/2, xp, yp);
	if (xp > urx) urx = xp;
	if (yp > ury) ury = yp;
	if (xp < llx) llx = xp;
	if (yp < lly) lly = yp;

	// rotate lower right corner of box, measured relative to center pt. 
	mr.rotatePoint(w/2, -h/2, xp, yp);
	if (xp > urx) urx = xp;
	if (yp > ury) ury = yp;
	if (xp < llx) llx = xp;
	if (yp < lly) lly = yp;

	// rotate lower left corner of box, measured relative to center pt. 
	mr.rotatePoint(-w/2, -h/2, xp, yp);
	if (xp > urx) urx = xp;
	if (yp > ury) ury = yp;
	if (xp < llx) llx = xp;
	if (yp < lly) lly = yp;

	// rotate upper left corner of box, measured relative to center pt. 
	mr.rotatePoint(-w/2, h/2, xp, yp);
	if (xp > urx) urx = xp;
	if (yp > ury) ury = yp;
	if (xp < llx) llx = xp;
	if (yp < lly) lly = yp;

	// add back to center point to get bbox
	rect[0] = x + llx;
	rect[1] = y + lly;
	rect[2] = x + urx;
	rect[3] = y + ury;


	cout << "x,y,w,h " << x << ", " << y << ", " << w << ", " << h << " bbox " << rect[0] << ", " << rect[1] << "  " << rect[2] << ", " << rect[3] << endl;


	return;
}

void doOnScratch(double x, double y, double w, double h, double ori, bool bScratch)
{
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

	// get bbox
	double bbox[4];
	getBBox(x, y, w, h, ori, bbox);

	double bboxW = bbox[2] - bbox[0];
	double bboxH = bbox[3] - bbox[1];

	cout << "bbox w " << bboxW << " h " << bboxH << endl;

	// Need a level guaranteed to be lower than everything else, 
	// and another (not background) guaranteed higher.
	DWORD lvLow = 1;
	DWORD lvHigh = 200;

	DWORD lvDart1 = 2; // 201;
	DWORD lvDart2 = 3; // 202;
	DWORD grLow = 11;
	DWORD grNLevels = 50;
	DWORD lvAbove = 150;
	long handle;

	COLOR_TYPE dart1(1, 0, 0);
	COLOR_TYPE dart2(0, 1, 0);
	arutil_color_to_palette(dart1, lvDart1);
	arutil_color_to_palette(dart2, lvDart2);

	// prepare drawing page with a dartboard red/green
	// There are two things drawn on the page, both above and 
	// below the grating levels
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	vsgSetPen1(lvDart1);
	vsgSetPen2(lvDart2);
	vsgDrawDartboard(0, 0, 30, 30, 5, 3, 0, 0);

	// add a circle above the grating levels to see what happens
	COLOR_TYPE blueish(.5, .5, 1);
	arutil_color_to_palette(blueish, lvAbove);
	vsgSetPen1(lvAbove);
	vsgDrawOval(1, 1, 10, 2);

	if (bScratch)
	{

		// Set draw page to scratch, no clear
		vsgSetDrawPage(vsgSCRATCHPAGE, 0, vsgNOCLEAR);

	}
	else
	{
		// create host page
		handle = vsgPAGECreate(vsgHOSTPAGE, W, H, vsg8BITPALETTEMODE);
		vsgSetDrawPage(vsgHOSTPAGE, handle, vsgNOCLEAR);
	}

	// draw bbox rect.
	// This means that the extra/background that is left over, 
	// outside the edges of the grating shape we draw, will all be on 
	// this (HIGH) level.
	vsgSetPen1(lvHigh);
	vsgDrawRect(x, y, bboxW, bboxH);

	// draw oriented rectangle. This is on a lower level than the grating levels, 
	// so using MAXMODE (TRANSONLOWER) ensures that the grating shape is drawn wherever
	// the rect is drawn, but the background remains at the higher level.
	vsgSetPen1(lvLow);
	vsgDrawBar(x, y, w, h, ori);

	// draw grating

	vsgObjCreate();
	vsgObjSetDefaults();
	vsgObjSetContrast(100);
	vsgSetBackgroundColour(&back);
	vsgObjSetPixelLevels(grLow, grNLevels);
	vsgObjTableSinWave(vsgSWTABLE);
	vsgObjSetColourVector(&from, &to, vsgBIPOLAR);
	vsgObjSetDriftVelocity(.25);

	////Select the range of maximum pixel-levels to draw the grating with.
	vsgSetPen1(grLow);
	vsgSetPen2(grLow + grNLevels - 1);

	// Draw the grating
	// As noted above, this is drawn with TRANSONLOWER -- MAX mode, i.e. max of source and dest is drawn. 
	// The grating is drawn wherever the oriented rect shape was drawn, but the higher level background 
	// remains elsewhere. 
	vsgSetDrawMode(vsgCENTREXY + vsgTRANSONLOWER);
	vsgDrawGrating(x, y, bboxW, bboxH, 90, .5);

	// now copy to draw page
	// The mode vsgTRANSONSOURCE doesn't seem to work unless used with vsgTRANSONHIGHER. 
	// Presumably, combinations with vsgTRANSONDEST and vsgTRANSONLOWER work, but no idea. 
	// 
	// In this case we use vsgTRANSONHIGHER, meaning the MAX of source and dest is drawn. So, 
	// in order for this to work, anything currently drawn on the draw page should be on levels
	// below those used for the grating -- they'll be overwritten.  

	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);

	// draw another oriented box on this page at low level. This and TRANSONSOURCE ensure that vsgBACKGROUND gets overwritten.
	vsgSetDrawMode(vsgCENTREXY);
	vsgSetPen1(lvLow); //vsgSetPen1(lvHigh);
	vsgDrawBar(x, y, w, h, ori);

	// Now move the rect back to the original draw page. 
	// TRANSONLOWER (MAX mode) ensures that the grating shape will overwrite the rect that was just drawn on the low level. 
	// We need TRANSONSOURCE, however, to keep the extra "background" surrounding the shape from overwriting the stuff on the draw page.
	// The TRANSONSOURCE makes that level transparent - meaning the destination level wins out.
	vsgSetDrawMode(vsgCENTREXY + vsgTRANSONSOURCE + vsgTRANSONLOWER); //vsgSetDrawMode(vsgCENTREXY + vsgTRANSONSOURCE + vsgTRANSONHIGHER);
	vsgSetPen2(lvHigh); //vsgSetPen2(lvLow);


	if (bScratch)
	{
		vsgDrawMoveRect(vsgSCRATCHPAGE, 0, x, y, bboxW, bboxH, x, y, bboxW, bboxH);
	}
	else
	{
		vsgDrawMoveRect(vsgHOSTPAGE, handle, x, y, bboxW, bboxH, x, y, bboxW, bboxH);
	}

	//Display the object.
	vsgPresent();

	return;

}


//void doOnHost()
//{
//	vsgSetCommand(vsgPALETTECLEAR);
//	long W = vsgGetScreenWidthPixels();
//	long H = vsgGetScreenHeightPixels();
//	VSGTRIVAL from, to, back;
//	from.a = from.b = from.c = 0;
//	to.a = to.b = to.c = 1;
//	back.a = back.b = back.c = 0.5;
//	vsgSetBackgroundColour(&back);
//
//	vsgSetViewDistMM(500);
//	double WDEG, HDEG;
//	vsgUnit2Unit(vsgPIXELUNIT, W, vsgDEGREEUNIT, &WDEG);
//	vsgUnit2Unit(vsgPIXELUNIT, H, vsgDEGREEUNIT, &HDEG);
//	cerr << "Screen width in degrees: " << WDEG << endl;
//	vsgSetSpatialUnits(vsgDEGREEUNIT);
//
//	// create host page
//	long handle = vsgPAGECreate(vsgHOSTPAGE, 1024, 768, vsg8BITPALETTEMODE);
//
//	// Reserve a low level
//	//PIXEL_LEVEL myLowLevel;
//	//ARvsg::instance().request_single(myLowLevel);
//
//	// grating
//	//string sGrating = "0,0,15,15,100,.1,.25,45,b,s,e";
//	//if (parse_grating(sGrating, grating))
//	//	cerr << "Cannot parse grating " << sGrating << endl;
//	//grating.init(40);
//
//	// Reserve a HIGH level and clear host page to that level
//	//PIXEL_LEVEL myHighLevel = ARvsg::instance().request_single();
//	vsgSetDrawPage(vsgHOSTPAGE, handle, 244);
//
//	// draw shape on host page at low level
//	vsgSetPen1(1);
//	vsgDrawBar(0, 0, 12, 4, 30);
//
//	vsgObjCreate();
//	vsgObjSetDefaults();
//	vsgObjSetContrast(100);
//	vsgSetBackgroundColour(&back);
//	vsgObjSetPixelLevels(2, 100);
//	vsgObjTableSinWave(vsgSWTABLE);
//	vsgObjSetColourVector(&from, &to, vsgBIPOLAR);
//	vsgObjSetDriftVelocity(.25);
//
//	////Select the range of maximum pixel-levels to draw the grating with.
//	vsgSetPen1(2);
//	vsgSetPen2(101);
//
//	//Draw the grating
//	vsgSetDrawMode(vsgCENTREXY + vsgTRANSONLOWER);
//	vsgDrawGrating(0, 0, WDEG, HDEG, 45, 2.0);
//
//	// copy to page 0
//	//vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
//
//
//	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
//	vsgSetDrawMode(vsgCENTREXY + vsgTRANSONSOURCE);
//	vsgSetPen2(244);
//	vsgDrawMoveRect(vsgHOSTPAGE, handle, 0, 0, 15, 10, 0, 0, 15, 10);
//
//	//Display the object.
//	vsgPresent();
//
//	return;
//}



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
