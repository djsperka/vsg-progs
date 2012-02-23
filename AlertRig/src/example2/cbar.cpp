#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include "Alertlib.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")

using namespace std;
using namespace alert;



int main (int argc, char *argv[])
{
	double W = 800;
	double H = 600;
	double w=5, h=1;	// bar width, height degrees
	double wpix, hpix;	// bar width, height, pixels
	double thetadeg = 10;	// ori angle, degrees
	double theta;		// ori angle, radians
	long d = 1000;		// view distance, mm
	vsgSetViewDistMM(d);


	// TODO: input w, h, theta

	// Conversions
	vsgUnitToUnit(vsgDEGREEUNIT, w, vsgPIXELUNIT, &wpix);
	vsgUnitToUnit(vsgDEGREEUNIT, h, vsgPIXELUNIT, &hpix);
	theta = thetadeg * M_PI/180.0;

	// (u,v) is orientation unit vector. 
	// Zero orientation is horizontal, direction of motion is "up" on screen. 
	// Since VSG pixel coords are positive-down, fix things to get that right. 

	double u[2];	
	u[0] = -sin(theta);
	u[1] = -cos(theta);

	// lv is left vector; vector from left hand side towards middle
	double lv[2];

	return 0;
}
