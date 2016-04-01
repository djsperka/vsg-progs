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
#if 0
	while ((c = getopt(argc, argv, "g:q:")) != -1)
	{
		switch (c) 
		{
		case 'g':
			s.assign(optarg);
			istatus = parse_grating(s, grating);
			cout << (istatus==0 ? "OK : " : "ERR: ") << grating << endl;
			break;
		case 'q':
			{
#endif
				int i;
				int iargc;
				char *iargv[100] = {0};

				s.clear();
				for (i=0; i<argc; i++)
				{
					s.append(string(argv[i]));
					s.append(" ");
				}
				cout << "Test tokenize..." << endl;
				cout << s << endl;
				make_argv(s, iargc, iargv);
				cout << "Results:" << endl;
				for (i=0; i<iargc; i++)
				{
					cout << i << ">>" << iargv[i] << "<<" << endl;
				}
#if 0
				break;
			}
		default:
			cout << "Unknown option " << c << endl;
			break;
		}
	}
#endif
	return 0;
}

