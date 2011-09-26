#include <iostream>
#include <string>
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__

#include "vsgv8.h"
#include "Alertlib.h"
#include "RegHelper.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment (lib, "vsgv8.lib")

int args(int argc, char **argv);
static void usage();

using namespace std;
using namespace alert;

int main (int argc, char *argv[])
{
	int c;
	string s;
	int istatus;
	ARGratingSpec grating;
	while ((c = getopt(argc, argv, "g:")) != -1)
	{
		switch (c) 
		{
		case 'g':
			s.assign(optarg);
			istatus = parse_grating(s, grating);
			cout << "Parse grating status " << istatus << ": " << grating << endl;
			break;
		default:
			cout << "Unknown option " << c << endl;
			break;
		}
	}
	return 0;
}

