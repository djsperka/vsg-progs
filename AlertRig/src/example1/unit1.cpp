#include "VSGV8.h"
#include <iostream>
#include <string>
using namespace std;

char buf1[2048], buf2[2048];  // buffer of signed bytes
string drawOrigin;

void vsgModes()
{
	int drawmode = vsgGetDrawMode();
	int units = vsgGetSystemAttribute(vsgSPATIALUNITS);
	string s = drawOrigin;
	if (drawmode & vsgCENTREXY) s += "CENTERXY ";
	if (units & vsgPIXELUNIT) s += "PIXELS ";
	if (units & vsgDEGREEUNIT) s += "DEGREES ";
	cout << s << " " << vsgGetScreenWidthPixels() << "x" << vsgGetScreenHeightPixels() << endl;
}


int main(int argc, char **argv)
{
	int CheckCard;
	int W, H;
	VSGLUTBUFFER Buff;
	int i, p1, p2, x, y;
	uint8_t pix[1024];

	//Initialise the vsg card then check that it initialised O.K.
	CheckCard = vsgInit("");
	if (CheckCard < 0) exit(0);

	//Find out the horizontal and vertical resolution of the vsg screen.
	W = vsgGetScreenWidthPixels();
	H = vsgGetScreenHeightPixels();

	//Clear a drawing page in memory then display it.
	vsgSetDrawPage(vsgVIDEOPAGE, 0, 0);
	vsgSetDisplayPage(0);

	//Clear another drawing page to draw onto.
	vsgSetDrawPage(vsgVIDEOPAGE, 1, 127);

	//Load a LUTBUFFER with a black-white ramp, then make 10,11,12 red, green, blue
	for (i = 0; i < 255; i++)
	{
		Buff[i].a = ((double)(i)) / 255;
		Buff[i].b = ((double)(i)) / 255;
		Buff[i].c = ((double)(i)) / 255;
	}
	Buff[10].a = 1.0; Buff[10].b = Buff[10].c = 0;
	Buff[11].a = 0.0; Buff[11].b = 1.0;  Buff[11].c = 0;
	Buff[12].a = Buff[12].b = 0; Buff[12].c = 1.0;

	//Load the contents of the LUTBUFFER into Look Up Table(LUT) 0.
	vsgLUTBUFFERWrite(0, (VSGLUTBUFFER*)&Buff);

	//Write the contents of LUT 0 to the palette.
	vsgLUTBUFFERtoPalette(0);


	// draw origin in upper left corner of screen

	vsgSetDrawOrigin(0, 0);
	//vsgSetDrawMode(0);
	drawOrigin = "(0,0) ";
	for (int jj = 0; jj < 1024; jj++)
		pix[jj] = jj%256;
	for (int ii = 0; ii < H; ii++)
	{
		pix[0] = ii % 256;
		pix[1] = ii / 256;
		pix[2] = 255;
		vsgDrawPixelLine(0, ii, pix, 1024);
	}

	//// draw red rect in upper left corner
	//vsgSetPen1(10);
	//vsgDrawRect(0, 0, W / 2, H / 2);
	//vsgSetPen1(11);
	//vsgDrawRect(W/2 , 0, W / 2, H / 2);
	//vsgSetPen1(12);
	//vsgDrawRect(W/2, H/2, W / 2, H / 2);

	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);




	string s;
	vsgModes();
	cout << "Hit key [wxdp]: ";
	cin >> s;
	while (s != "q")
	{
		vsgModes();
		if (s == "w")
		{
			int status;
			cout << "Enter pixel line: ";
			cin >> i;
			status = vsgReadPixelLine(0, i, pix, 1024);
			cout << "status " << status << endl;
			for (int j = 0; j < 1024; j++)
			{
				cout << (unsigned int)pix[j] << ",";
			}
			cout << endl;
		}
		else if (s == "x")
		{
			int x = 0;
			int status;
			cout << "Enter pixel line: ";
			cin >> i;

			// x coord to start at
			cout << "Enter x: ";
			cin >> x;

			status = vsgReadPixelLine(x, i, pix, 1024);
			cout << "status " << status << endl;
			for (int j = 0; j < 1024; j++)
			{
				cout << (unsigned int)pix[j] << ",";
			}
			cout << endl;
		}
		else if (s == "d")
		{
			cout << "set degree units" << endl;
			vsgSetViewDistMM(500);
			vsgSetSpatialUnits(vsgDEGREEUNIT);
		}
		else if (s == "p")
		{
			cout << "set pixel units" << endl;
			vsgSetSpatialUnits(vsgPIXELUNIT);
		}
		else if (s == "O")
		{
			int units = vsgGetSystemAttribute(vsgSPATIALUNITS);
			vsgSetSpatialUnits(vsgPIXELUNIT);
			cout << "set draw origin to mid-screen" << endl;
			vsgSetDrawOrigin(W / 2, H / 2);
			vsgSetSpatialUnits(units);
			drawOrigin = "(W/2,H/2) ";
		}
		else if (s == "o")
		{
			int units = vsgGetSystemAttribute(vsgSPATIALUNITS);
			vsgSetSpatialUnits(vsgPIXELUNIT);
			cout << "set draw origin to upper-left corner" << endl;
			vsgSetDrawOrigin(0, 0);
			vsgSetSpatialUnits(units);
			drawOrigin = "(0,0) ";
		}
		else if (s == "r")
		{
			cout << "assume origin at center, degrees, "
				"centerxy has no effect here, specify upper left corner of screen." << endl;
			// half-width of screen, as a positive number
			double halfwidthDeg, halfheightDeg, pixelLineDeg;
			vsgUnitToUnit(vsgPIXELUNIT, W / 2, vsgDEGREEUNIT, &halfwidthDeg);
			vsgUnitToUnit(vsgPIXELUNIT, H / 2, vsgDEGREEUNIT, &halfheightDeg);

			cout << "Enter pixel line: ";
			cin >> i;
			vsgUnitToUnit(vsgPIXELUNIT, i, vsgDEGREEUNIT, &pixelLineDeg);
			int status = vsgReadPixelLine(-halfwidthDeg, -halfheightDeg+pixelLineDeg, pix, 1024);
			cout << "status " << status << endl;
			for (int j = 0; j < 1024; j++)
			{
				cout << (unsigned int)pix[j];
				if (j % 256 != 255)
					cout << ",";
				else
					cout << endl << "-------" << endl;
			}
			cout << endl;


		}
		else if (s == "s")
		{
			cout << "assume origin at center, pixel units, "
				"centerxy has no effect here, specify left edge of line." << endl;
			// half-width of screen, as a positive number
			cout << "Enter pixel line: ";
			cin >> i;
			int status = vsgReadPixelLine(-W/2, -H/2 + i, pix, 1024);
			cout << "status " << status << endl;
			for (int j = 0; j < 1024; j++)
			{
				cout << (unsigned int)pix[j];
				if (j % 256 != 255)
					cout << ",";
				else
					cout << endl << "-------" << endl;
			}
			cout << endl;


		}

		cout << "Hit key [cpq]: ";
		cin >> s;
	}




	return 0;
}