#if 0
//#define BOOST_REGEX_MATCH_EXTRA 
#include <boost/regex.hpp>
#include <iostream>

void print_captures(const std::string& regx, const std::string& text)
{
	boost::regex e(regx);
	boost::smatch what;
	std::cout << "Expression:  \"" << regx << "\"\n";
	std::cout << "Text:        \"" << text << "\"\n";
	if (boost::regex_match(text, what, e))
	{
		unsigned i, j;
		std::cout << "** Match found **\n   Sub-Expressions:\n";
		for (i = 0; i < what.size(); ++i)
		{
			std::cout << "      $" << i;
			if (what[i].matched)
				std::cout << " = \"" << what[i] << "\"\n";
			else
				std::cout << " NOT MATCHED\n";
		}
		//std::cout << "   Captures:\n";
		//for (i = 0; i < what.size(); ++i)
		//{
		//	std::cout << "      $" << i << " = {";
		//	for (j = 0; j < what.captures(i).size(); ++j)
		//	{
		//		if (j)
		//			std::cout << ", ";
		//		else
		//			std::cout << " ";
		//		std::cout << "\"" << what.captures(i)[j] << "\"";
		//	}
		//	std::cout << " }\n";
		//}
	}
	else
	{
		std::cout << "** No Match found **\n";
	}
}

void parsePlaid(const std::string& arg)
{
	std::string intrx("[-+]?[0-9]+");
	std::string fltrx("[-+]?[0-9]*\\.?[0-9]+");
	std::string int_single_or_list = "(" + intrx + ")|([\\[](" + intrx + "(," + intrx + ")*)[\\]])";
	std::string flt_single_or_list = "(" + fltrx + ")|([\\[](" + fltrx + "(," + fltrx + ")*)[\\]])";
	std::string plaidrx = "(" + fltrx + "),(" + fltrx + "),(" + fltrx + "),(" + fltrx + "),(" + int_single_or_list + "),(" + flt_single_or_list + "),(" + flt_single_or_list + ")";
	boost::regex e(plaidrx);
	boost::smatch what;
	bool b = boost::regex_match(arg, what, e);
	if (b)
	{
		std::cout << "$1: " << what[1] << std::endl;
		std::cout << "$2: " << what[2] << std::endl;
		std::cout << "$3: " << what[3] << std::endl;
		std::cout << "$4: " << what[4] << std::endl;
		if (what[5].matched)
			std::cout << "either/or found: " << what[5] << std::endl;
		if (what[6].matched)
			std::cout << "int_single found: " << what[6] << std::endl;
		if (what[7].matched)
			std::cout << "int_list found: with " << what[7] << " without " << what[8] << std::endl;


		if (what[10].matched)
			std::cout << "either/or found: " << what[10] << std::endl;
		if (what[11].matched)
			std::cout << "flt_single found: " << what[11] << std::endl;
		if (what[12].matched)
			std::cout << "flt_list found: with " << what[12] << " without " << what[13] << std::endl;


		//for (int i = 0; i < what.size(); i++)
		//{
		//	std::cout << i << ": " << what[i] << std::endl;
		//}

		// 5,6,7,8,9 6==>single
	}
}

int main(int, char*[])
{
	//print_captures("(([[:lower:]]+)|([[:upper:]]+))+", "aBBcccDDDDDeeeeeeee");
	//print_captures("(.*)bar|(.*)bah", "abcbar");
	//print_captures("(.*)bar|(.*)bah", "abcbah");
	//print_captures("^(?:(\\w+)|(?>\\W+))*$",
	//	"now is the time for all good men to come to the aid of the party");
#if 0
	std::string single("((\\d+)|([\\[]\\d+(,\\d+)*[\\]]))");	// subexp 1 = whole match (single number or bracketed list), 2=single, 3=bracketed list, 4= ",rest" part of 3-match.
	//print_captures("(\\d+)|([\\[]\\d+(,\\d+)*[\\]])", "1234");
	//print_captures("(\\d+)|([\\[]\\d+(,\\d+)*[\\]])", "[1234]");
	//print_captures("(\\d+)|([\\[]\\d+(,\\d+)*[\\]])", "[12,34]");
	std::string triple = single + "," + single + "," + single;
	print_captures(triple, "1,2,3");
	print_captures(triple, "1,2,[3]");
	print_captures(triple, "1,[2],3");
	print_captures(triple, "[1],[2],3");
	print_captures(triple, "1,2,[3,4,5,6]");
#endif

	std::string intrx("[-+]?[0-9]+");
	std::string fltrx("[-+]?[0-9]*\\.?[0-9]+");
	std::string int_single_or_list = "(" + intrx + ")|([\\[](" + intrx + "(," + intrx + ")*)[\\]])";
	std::string flt_single_or_list = "(" + fltrx + ")|([\\[](" + fltrx + "(," + fltrx + ")*)[\\]])";
	std::string plaidrx = "(" + fltrx + "),(" + fltrx + "),(" + fltrx + "),(" + fltrx + "),(" + int_single_or_list + "),(" + flt_single_or_list + "),(" + flt_single_or_list + ")";

	print_captures(intrx, "123");
	print_captures(int_single_or_list, "123");
	print_captures(int_single_or_list, "[123,456,789]");
	print_captures(flt_single_or_list, "12.3");
	print_captures(flt_single_or_list, "[12.3,45.6,78.9]");
	std::cout << "plaidrx " << plaidrx << std::endl;
	print_captures(plaidrx, "1,2,3,4,5,6,7");

	parsePlaid("1,2,3,4,5,6,7");
	parsePlaid("1,2,3,4,[55,66,77,88,99],[16.3,17.5,18],7");

	return 0;
}
#endif


// this prog tests a single plaid

#include "VSGV8.h"
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
using namespace std;

#pragma comment(lib, "vsgv8.lib")

double getPhase(double x, double y, double sf, double p, double x0, double y0, double alpha)
{
	double phase = 2 * M_PI * sf / p * ((x - x0) * cos(2 * M_PI*alpha / 360) + (y - y0) * sin(2 * M_PI*alpha / 360));
	return phase;
}


int main(int argc, char **argv)
{
	double sf0 = 0.3;
	double sf1 = 0.2;
	double alpha0 = 45;
	double alpha1 = 135;
	char buffer[1024];	// i know resolution
	int W, H;
	double pixelsPerDegree;

	if (vsgInit("") < 0) exit(0);
	vsgSetViewDistMM(1000);
	vsgUnit2Unit(vsgDEGREEUNIT, 1.0, vsgPIXELUNIT, &pixelsPerDegree);
	W = vsgGetScreenWidthPixels();
	H = vsgGetScreenHeightPixels();

	std::cout << "num pages " << vsgGetSystemAttribute(vsgNUMVIDEOPAGES) << " " << vsgGetSystemAttribute(vsgPAGEWIDTH) << "x" << vsgGetSystemAttribute(vsgPAGEHEIGHT) << std::endl;
	vsgSetDrawPage(vsgVIDEOPAGE, 0, 255);
	vsgSetDisplayPage(0);

	for (int j = 0; j < H; j++)
	{
		for (int i = 0; i < W; i++)
		{
			buffer[i] = (char)(250 * 0.25 * (2 + (cos(getPhase(i, j, sf0, pixelsPerDegree, 0, 0, alpha0)) + cos(getPhase(i, j, sf1, pixelsPerDegree, 0, 0, alpha1)))));
		}
		vsgDrawPixelLineFast(-W/2, -H/2+j, buffer, W);
	}

	return 0;
}
#if 0
//Set lookup table 0 in palette 0 & 1 to black and white.
	Buff[0] = 1.0;
	Buff[1] = 1.0;
	Buff[2] = 1.0;
	Buff[3] = 0;
	Buff[4] = 0;
	Buff[5] = 0;
	vsgPaletteWrite((VSGLUTBUFFER*)&Buff, 0, 2);

//Draw the chess board to cover the whole page.

	vsgSetPen1(1);
	vsgSetPen2(0);
	vsgSetDrawMode(vsgCENTREXY+vsgSOLIDFILL);
	vsgDrawChessboard(0, 0, Width, Height, div(Width, 8).quot, div(Height, 8).quot, 0);

//Change the drawing modes to target the Overlay.
	vsgSetCommand(vsgOVERLAYDRIFT+vsgOVERLAYMASKMODE);

//Clear all of the Overlay memory to pixel-level(1).
	vsgSetPen2(1);
	vsgSetCommand(vsgOVERLAYCLEAR);

//Fill Overlay pixel-level(1) with red.
	Buff[1] = 0.0;
	Buff[2] = 0.0;
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, vsgNOCLEAR);
	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&Buff, 1, 1);

//Draw a window in the Overlay page (in vsgOVERLAYMASKMODE anything drawn in pixel-
//level(0) is transparent).
	vsgSetPen1(0);
	vsgDrawRect(0, 0, div(Width, 2).quot, div(Height, 4).quot);


	vsgSetDrawPage(vsgOVERLAYPAGE, 1, vsgNOCLEAR);
	vsgSetPen1(0);
	vsgDrawRect(10, 10, div(Width, 2).quot, div(Height, 4).quot);

	vsgSetDrawPage(vsgOVERLAYPAGE, 2, vsgNOCLEAR);
	vsgSetPen1(0);
	vsgDrawRect(20, 20, div(Width, 2).quot, div(Height, 4).quot);
	
	vsgSetDrawPage(vsgOVERLAYPAGE, 3, vsgNOCLEAR);
	vsgSetPen1(0);
	vsgDrawRect(30, 30, div(Width, 2).quot, div(Height, 4).quot);



//Display OVERLAYPAGE 0.
	vsgSetDisplayPage(0);

	cout << "Enter page(0-3): ";
	cin >> page;
	while (page >= 0 && page <= 3) 
	{
		vsgSetDisplayPage(page);
		cout << "Enter page(0-3): ";
		cin >> page;
	}

	

	return 0;
}
#endif
