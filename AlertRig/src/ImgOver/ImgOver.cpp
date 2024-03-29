// ImgOver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "vsgv8.h"
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char **argv)
{
	// print args
	for (int i = 0; i < argc; i++)
		cerr << i << " " << argv[i] << endl;

	if (argc != 3)
	{
		cerr << "expecting 2 args: file1 file_over" << endl;
		return 0;
	}

	// init vsg
	char s[1] = { 0 };
	if (vsgInit(s) < 0)
	{
		cerr << "vsg init failed" << endl;
	}

	// when init'd, draw page is 1, cleared to level 0
	// draw first image to screen 
	vsgDrawImage(vsgPALETTELOAD, 0, 0, argv[1]);


	cout << "How to draw second picture?" << endl <<
		"a: vsgDrawImage, vsgTRANSONSOURCE, no palette load" << endl <<
		"A: vsgDrawImage, vsgTRANSONSOURCE, load palette" << endl <<
		"b: vsgDrawImage to SCRATCHPAGE, vsgDrawMoveRect with vsgTRANSONSOURCE" << endl <<
		"L: vsgDrawPixelLine vsgTRANSONSOURCE" << endl <<
		"Choice: ";
	string input;
	cin >> input;
	if (input == "a")
	{
		// draw second on top of first
		vsgSetDrawMode(vsgCENTREXY | vsgTRANSONSOURCE);
		vsgSetPen2(255);
		vsgDrawImage(0, 0, 0, argv[2]);
	}
	else if (input == "A")
	{
		// draw second on top of first
		vsgSetDrawMode(vsgCENTREXY | vsgTRANSONSOURCE);
		vsgSetPen2(255);
		vsgDrawImage(vsgPALETTELOAD, 0, 0, argv[2]);
	}
	else if (input == "b")
	{
		// set draw page to SCRATCH PAGE
		vsgSetDrawPage(vsgSCRATCHPAGE, 0, 0);

		// get size of image
		double w, h;
		vsgImageGetSize(0, argv[2], &w, &h);
		vsgDrawImage(0, 0, 0, argv[2]);

		// move image
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		vsgSetDrawMode(vsgCENTREXY | vsgTRANSONSOURCE);
		vsgSetPen2(255);
		vsgDrawMoveRect(vsgSCRATCHPAGE, 0, 0, 0, w, h, 0, 0, w, h);

	}
	else if (input == "L")
	{
		int level = -1;

		// try a level?
		while (level < 0 || level > 255)
		{
			cerr << "Enter level 0-255: ";
			cin >> level;
		}

		vsgSetDrawMode(vsgCENTREXY | vsgTRANSONSOURCE);
		vsgSetPen2(level);
		char buffer[512];
		for (int j = 0; j < 512; j++)
			buffer[j] = level;

		for (int j = 0; j < 256; j++)
		{
			vsgDrawPixelLine(0, j, buffer, 512);
		}
	}
	else
		cerr << "bad input: " << input << endl;





	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);

	return 0;
}