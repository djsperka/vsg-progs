#include "VSGV8.H"
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__
#include "alertlib.h"
#include <string>
using namespace std;
using namespace alert;

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")


int main(int argc, char **argv)
{
	PIXEL_LEVEL a, b, c, d, lf1, lf2;
	ARContrastFixationPointSpec f1, f2;
	ARvsg::master().request_range(2, a);
	ARvsg::master().request_range(2, b);
	ARvsg::master().request_range(2, c);
	ARvsg::master().request_range(2, d);

	cout << a << " " << b << " " << c << " " << d << endl;


	f1.init(ARvsg::master(), 2);
	f2.init(ARvsg::master(), 2);
	return 0;
}