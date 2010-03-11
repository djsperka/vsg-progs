#include "alertlib.h"
#include "VSGV8.H"
#include <stdio.h>
#include <math.h>

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")

int f_screenDistanceMM = 0;
COLOR_TYPE f_background = { gray, {} };
int f_dotsize = 8;

using namespace alert;
using namespace std;

int main(int argc, const char **argv)
{
	string s;
	bool bQuit = false;

	// INit vsg
	if (ARvsg::instance().init(f_screenDistanceMM, f_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}


	// draw a white square on a gray page
	vsgSetVideoMode(vsgPANSCROLLMODE);
	vsgSetCommand(vsgPALETTERAMP);
	vsgSetDrawPage(vsgVIDEOPAGE, 0, 127);
	vsgSetDrawPage(vsgVIDEOPAGE, 1, 127);
	vsgSetDrawPage(vsgVIDEOPAGE, 2, 127);
	vsgSetDrawPage(vsgVIDEOPAGE, 3, 127);
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	vsgSetPen1(255);
	vsgDrawRect(624, 724, f_dotsize, f_dotsize);
	vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgNOCLEAR);
	vsgSetPen1(0);
	vsgDrawRect(624, 724, f_dotsize, f_dotsize);

	cout << "Enter command(piqmc): ";
	cin >> s;
	while (!bQuit)
	{
		if (s=="q")
		{
			bQuit = true;
		}
		else if (s=="p")
		{
			int page;
			cout << "Enter page: ";
			cin >> s;
			if (!parse_integer(s, page))
			{
				vsgSetZoneDisplayPage(vsgVIDEOPAGE, page);
			}
		}
		else if (s=="m")
		{
			int ix, iy, i;
			cout << "Enter coords for move: ";
			cin >> s;
			if (!parse_int_pair(s, ix, iy))
			{
				i = vsgMoveScreen((double)ix, (double)iy);
				printf("MoveScreen returned %d\n", i);
			}
			else printf("Error in input\n");
		}
		else if (s=="i")
		{
			int h, w;
			h = vsgGetScreenHeightPixels();
			w = vsgGetScreenWidthPixels();
			printf("screen h,w = %d, %d\n", h, w);
			h = vsgGetSystemAttribute(vsgPAGEHEIGHT);
			w = vsgGetSystemAttribute(vsgPAGEWIDTH);
			printf("page h,w = %d, %d\n", h, w);
			break;
		}
		else if (s=="c")
		{
			int i, j;
			int x, y;
			int count=0;
			int F = 10;
			VSGCYCLEPAGEENTRY cycle[32768];
			string stmp;

			for (i=0; i<100; i++)
			{
				x = 1019 - i*f_dotsize;
				for (j=0; j<75; j++)
				{
					y = 1019 - j*f_dotsize;
					cycle[count].Frames = F;
					cycle[count].Page = 0;
					cycle[count].Xpos = x;
					cycle[count].Ypos = y;
					count++;
				}
			}
			for (i=0; i<100; i++)
			{
				x = 1019 - i*f_dotsize;
				for (j=0; j<75; j++)
				{
					y = 1019 - j*f_dotsize;
					cycle[count].Frames = F;
					cycle[count].Page = 2;
					cycle[count].Xpos = x;
					cycle[count].Ypos = y;
					count++;
				}
			}
			vsgPageCyclingSetup(count, &cycle[0]);
			cout << "Hit any key to start." << endl;
			cin >> stmp;
			vsgSetCommand(vsgCYCLEPAGEENABLE);
			cout << "Hit any key to stop." << endl;
			cin >> stmp;
			vsgSetCommand(vsgCYCLEPAGEDISABLE);
		}
		else
		{
			printf("Unknown input.\n");
			break;
		}
		if (bQuit) break;
		cout << "Enter command(piqmc): ";
		cin >> s;
	}





	ARvsg::instance().release_lock();
	return 0;
}
