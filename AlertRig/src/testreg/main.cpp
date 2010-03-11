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


bool m_getLockFile = false;


int main (int argc, char *argv[])
{

	// Check input arguments
	if (args(argc, argv))
	{
		return 1;
	}
	else
	{
		if (m_getLockFile)
		{
			string s;
			bool b;

			b = GetRegLockFile(s);
			if (b)
			{
				cerr << "Got lock file: " << s << endl;
			}
			else
			{
				cerr << "ERROR: Cannot get lock file!" << endl;
			}
		}
	}
	return 0;
}

int args(int argc, char **argv)
{	
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "l")) != -1)
	{
		switch (c) 
		{
		case 'l':
			m_getLockFile = true;
			break;
		case 'a':
			m_getLockFile = true;
			break;
		case 'h':
			errflg++;
			break;
		case '?':
            errflg++;
			break;
		default:
			errflg++;
			break;
		}
	}

	return errflg;
}

