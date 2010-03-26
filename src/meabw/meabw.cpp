#include "VSGV8.H"
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__
#include <iostream>
#include <cstdio>
#include <cmath>
#include <conio.h>
#include "alertlib.h"

using namespace std;
using namespace alert;

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")



VSGCYCLEPAGEENTRY mpos[2];

#define WHITE_PAGE 1
#define BLACK_PAGE 0



int main(int argc, char **argv)
{
	int istatus=0;

	// Init vsg card
	istatus = vsgInit("");
	if (istatus)
	{
		cerr << "Error in vsgInit. Another VSG program may be running!" << endl;
		return 1;
	}


	// setup vsg
	vsgSetCommand(vsgPALETTERAMP);
	vsgSetPen1(255);
	vsgSetPen2(0);
	vsgSetCommand(vsgVIDEOCLEAR);

	vsgSetDrawPage(vsgVIDEOPAGE, BLACK_PAGE, 0);
	vsgSetDrawPage(vsgVIDEOPAGE, WHITE_PAGE, 255);

	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);

	// testing loop...
	std::string s;
	bool bQuit = false;
	while (!bQuit)
	{
		// Get a new "trigger" from user
		cout << "Enter key: ";
		cin >> s;

		switch(s[0])
		{
		case 'q':
		case 'Q':
			{
				bQuit = true;
				break;
			}
		case '0':
		case '1':
			{
				int ipage = atoi(s.c_str());
				vsgSetZoneDisplayPage(vsgVIDEOPAGE, ipage);
				break;
			}
		case 'm':
			{
				mpos[0].Frames = 1;
				mpos[0].Page = BLACK_PAGE + vsgTRIGGERPAGE;
				mpos[0].Stop = 0;
				mpos[0].Xpos = 0;
				mpos[0].Ypos = 0;

				mpos[1].Frames = 1;
				mpos[1].Page = WHITE_PAGE + vsgTRIGGERPAGE;
				mpos[1].Stop = 0;
				mpos[1].Xpos = 0;
				mpos[1].Ypos = 0;

				vsgPageCyclingSetup(2, mpos);

				cout << "Start cycling ...." << endl;
				vsgSetCommand(vsgCYCLEPAGEENABLE);
				while(!_kbhit()) Sleep(1000);
				vsgSetCommand(vsgCYCLEPAGEDISABLE);
				cout << "Done cycling." << endl;
				break;
			}
		default:
			{
				cout << "Unknown entry. Try again." << endl;
				break;
			}
		}
	}




	return 0;
}




