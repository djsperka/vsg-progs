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
bool m_getScreenDistance = false;
bool m_getFixpt = false;
bool m_getStimulus = false;

int testLockFile();
int testScreenDistance();
int testFixpt();
int testStimulus();

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
			if (!testLockFile())
				cerr << "Lock File test OK." << endl;
			else
				cerr << "Lock File test FAILED." << endl;
		}

		if (m_getScreenDistance)
		{
			if (!testScreenDistance())
				cerr << "Screen Distance test OK." << endl;
			else
				cerr << "Screen Distance test FAILED." << endl;
		}

		if (m_getFixpt)
		{
			if (!testFixpt())
				cerr << "Fixpt test OK." << endl;
			else
				cerr << "Fixpt test FAILED." << endl;
		}

		if (m_getStimulus)
		{
			if (!testStimulus())
				cerr << "Stimulus test OK." << endl;
			else
				cerr << "Stimulus test FAILED." << endl;
		}

	}
	return 0;
}

int testLockFile()
{
	int status = 0;
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
		status = -1;
	}

	return status;
}

int testScreenDistance()
{
	int status = 0;
	int distance = 0;

	if (!GetRegScreenDistance(distance))
	{
		cerr << "ERROR: Cannot get screen distance from registy." << endl;
		status = -1;
	}
	else
		cerr << "Got screen distance = " << distance << endl;

	return status;
}

int testFixpt()
{
	int status = 0;
	double x, y, d;
	COLOR_TYPE c;
	if (!GetRegFixpt(x, y, d, c))
	{
		cerr << "ERROR: Cannot get fixpt parameters from registy." << endl;
		status = -1;
	}
	else
		cerr << "Got fixpt x,y,d,c = " << x << "," << y << "," << d << "," << c << endl;

	return status;
}

int testStimulus()
{
	int status = 0;
	ARApertureGratingSpec stimulus;

	if (!GetRegStimulus(stimulus))
	{
		cerr << "ERROR: Cannot get stimulus parameters from registy." << endl;
		status = -1;
	}
	else
		cerr << "Got stimulus = " << stimulus << endl;

	return status;
}

int args(int argc, char **argv)
{	
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "aldfs")) != -1)
	{
		switch (c) 
		{
		case 'a':
			m_getLockFile = true;
			m_getScreenDistance = true;
			m_getFixpt = true;
			m_getStimulus = true;
			break;
		case 'l':
			m_getLockFile = true;
			break;
		case 'd':
			m_getScreenDistance = true;
			break;
		case 'f':
			m_getFixpt = true;
			break;
		case 's':
			m_getStimulus = true;
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

