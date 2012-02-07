#include <windows.h>
#include <process.h>
#include <stddef.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <string>
#include <cmath>
#include "Alertlib.h"
#include "alertutil.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")

using namespace std;
using namespace alert;

void do_dots(void *whatever);
bool f_quit = false;
int f_page;
PIXEL_LEVEL f_dotlevel;
double f_dots[1000][2];

int main (int argc, char *argv[])
{
	string s;
	COLOR_TYPE background = { gray, {0.5, 0.5, 0.5}};
	COLOR_TYPE r = { red, { 1.0, 0.0, 0.0 }};
	int dist = 555;

	ARvsg::instance().init(dist, background);
	ARvsg::instance().request_single(f_dotlevel);
	arutil_color_to_palette(r, f_dotlevel);

	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);

	f_page = 1;
	_beginthread(do_dots, 0, NULL);

	_getch();	// keystroke will end prog
	f_quit = true;

	ARvsg::instance().clear(0);
	
	return 0;
}


void init_dots()
{
	int i;
	srand( (unsigned)time( NULL ) );

	for (i=0; i<1000; i++)
	{
		f_dots[i][0] = (double)rand() * 800.0/RAND_MAX - 400.0; 
		f_dots[i][1] = (double)rand() * 600.0/RAND_MAX - 300.0; 
	}
}

void shift_dots()
{
	int i;
	// shift all dots right by 2 pixels, wrap if off screen
	for (i=0; i<1000; i++)
	{
		f_dots[i][0] += 2;
		if (f_dots[i][0] > 400) f_dots[i][0] -= 800;
	}
}

void do_dots(void *whatever)
{
	int i;
	init_dots();
	vsgDOTInitialise(1000, 1, vsgDOTAUTORESERVE);
	while (!f_quit)
	{
		f_page = 1 - f_page;
		vsgSetDrawPage(vsgVIDEOPAGE, f_page, vsgBACKGROUND);
		shift_dots();
		vsgDOTInitLoadGroup(vsgDOTXY, 2, 2, f_dotlevel);
		for (i=0; i<1000; i++)
		{
			vsgDOTGenerateNormalDot(f_dots[i][0], f_dots[i][1], 0);
		}
		vsgDOTLoadGroup();
		vsgDrawDots(vsgDOTDRAWNORMAL + vsgTRIGGERPAGE);
	}
	return;
}