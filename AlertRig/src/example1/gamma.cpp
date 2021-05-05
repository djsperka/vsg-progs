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

	long status = 0;
	long length = 16384;
	short r[16384], g[16384], b[16384];

	// screen distance, use degrees
	vsgSetViewDistMM(500);
	vsgSetSpatialUnits(vsgDEGREEUNIT);

	//Disable all of the draw modes.
	vsgSetDrawMode(vsgCENTREXY);

	//Clear a drawing page in memory then display it.
	vsgSetDrawPage(vsgVIDEOPAGE, 0, 0);
	vsgSetDisplayPage(0);

	// now draw a  dot
	vsgSetPen1(255 * 256);
	vsgDrawOval(0, 0, 2, 2);

	cout << "Color resolution " << vsgGetSystemAttribute(vsgCOLOURRESOLUTION) << endl;
	//status = vsgGAMMAReadProfile(length, r, g, b);
	//for (int i = 0; i < 16384; i++)
	//	cout << i << "," << r[i] << "," << g[i] << "," << b[i] << endl;

	return 0;
}

