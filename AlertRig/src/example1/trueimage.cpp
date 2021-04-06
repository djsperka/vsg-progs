#include <iostream>
#include "VSGV8.h"
using namespace std;

//char *f_imagefile = "c:\\Users\\djsperka\\Downloads\\bitmap.bmp";
//char *f_imagefile = "e:\\work\\img\\pepper.bmp";
char *f_imagefile = NULL;

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cerr << "expecting image filename" << endl;
		exit(0);
	}
	else
	{
		f_imagefile = argv[1];
	}
	cout << "init vsg..." << endl;
	if (vsgInit("") < 0)
	{
		cout << "Cannot init" << endl;
		exit(0);
	}


	VSGIMAGEATTRIBUTES attr;
	attr._StructSize = sizeof(VSGIMAGEATTRIBUTES);
	vsgImageGetAttributes(0, f_imagefile, &attr);
	cout << "W: " << attr.Width << endl;
	cout << "H: " << attr.Height << endl;
	cout << "bpp: " << attr.BitsPerPixel<< endl;
	cout << "vsgTRUECOLOURMODE compatible? " << (attr.CompatibleVSGModes & vsgTRUECOLOURMODE ? "yes" : "no");
	if (!(attr.CompatibleVSGModes & vsgTRUECOLOURMODE))
		exit(0);

	// screen distance, use degrees
	vsgSetViewDistMM(500);
	vsgSetSpatialUnits(vsgDEGREEUNIT);

	// Set the VSG to True colour mode
	// why NOGAMMACORRECT?
	vsgSetVideoMode(vsgTRUECOLOURMODE + vsgNOGAMMACORRECT);

	//Disable all of the draw modes.
	vsgSetDrawMode(vsgCENTREXY);

	//Clear a drawing page in memory then display it.
	vsgSetDrawPage(vsgVIDEOPAGE, 0, 0);
	vsgSetDisplayPage(0);

	// draw image
	vsgDrawImage(vsgBMPPICTURE + vsgPALETTELOAD, 0, 0, f_imagefile);

	// now draw a red dot
	vsgSetPen1(255*256);
	vsgDrawOval(0, 0, 2, 2);



	return 0;
}

