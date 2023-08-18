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
	string s;
	cout << "Enter fixpt: ";
	cin >> s;
	while (s[0] != 'q')
	{
		ARFixationPointSpec fixpt;
		stringstream ss(s);
		ss >> fixpt;
		if (!ss)
		{
			cout << "Error" << endl;
		}
		else
		{
			cout << "Success: " << fixpt << endl;
		}

		cout << "Enter fixpt: ";
		cin >> s;
	}




	return 0;
}
