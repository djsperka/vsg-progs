#include "Alertlib.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>
#include "vsgv8.h"
#include "Alertlib.h"
#include "AlertUtil.h"
//#include "StimSet.h"
using namespace std;
using namespace alert;


// libs

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif
#pragma comment (lib, "vsgv8.lib")



class SSInfo
{
private:
	ARGratingSpec m_masterGrating;
	ARGratingSpec m_slaveGrating;
	vector<double> m_masterXY;
	vector<double> m_slaveXY;
	double m_t2, m_t3;
	int m_cbase;
	vector<double> m_vecT1;
	vector<int> m_vecCUp;
	vector<int> m_vecLR;
	vector<double> m_vecOri;
	vector<double> m_vecDiam;

public:
	SSInfo() {};
	~SSInfo() {};
	const ARGratingSpec& getMasterGrating() const { return m_masterGrating; };
	const ARGratingSpec& getSlaveGrating() const { return m_slaveGrating; };
	bool getMasterXY(int i, double& x, double& y) const
	{
		bool b = true;
		switch (i)
		{
			case 0: x = m_masterXY[0]; y = m_masterXY[1]; break;
			case 1: x = m_masterXY[2]; y = m_masterXY[3]; break;
			default: x = -9999999; y = -9999999; b = false;
		}
		return b;
	};
	bool getSlaveXY(int i, double& x, double& y) const
	{
		bool b = true;
		switch (i)
		{
			case 0: x = m_slaveXY[0]; y = m_slaveXY[1]; break;
			case 1: x = m_slaveXY[2]; y = m_slaveXY[3]; break;
			default: x = -9999999; y = -9999999; b = false;
		}
		return b;
	};
	double getT2() const { return m_t2; };
	double getT3() const { return m_t3; };
	int getCBase() const { return m_cbase; };
	unsigned int getSize() const { return m_vecT1.size(); };
	bool getT1(int trial, double& t1) const 
	{
		bool b = true;
		if (trial > -1 && trial < (int)m_vecT1.size()) t1 = m_vecT1[trial];
		else b = false;
		return b;
	};

	bool getCUp(int trial, int& cup) const 
	{
		bool b = true;
		if (trial > -1 && trial < (int)m_vecCUp.size()) cup = m_vecCUp[trial];
		else b = false;
		return b;
	};

	bool getLR(int trial, int& lr) const 
	{
		bool b = true;
		if (trial > -1 && trial < (int)m_vecLR.size()) lr = m_vecLR[trial];
		else b = false;
		return b;
	};

	bool getOri(int trial, double& ori) const 
	{
		bool b = true;
		if (trial > -1 && trial < (int)m_vecOri.size()) ori = m_vecOri[trial];
		else b = false;
		return b;
	};

	bool getDiam(int trial, double& diam) const 
	{
		bool b = true;
		if (trial > -1 && trial < (int)m_vecDiam.size()) diam = m_vecDiam[trial];
		else b = false;
		return b;
	};

	static bool load(string& filename, SSInfo& ssinfo)
	{
		bool b = true;
		string s;
		vector<double>vtmp;
		ifstream in(filename.c_str(), ifstream::in);
		if (!in.good()) return false;

		// First line is master grating template
		getline(in, s);
		if (!in.good() || parse_grating(s, ssinfo.m_masterGrating))
		{
			cerr << "Error in input file: bad grating on line 1" << endl;
			return false;
		}

		// next is x y x y for offsets of master gratings
		ssinfo.m_masterXY.clear();
		getline(in, s);
		if (!in.good() || parse_number_list(s, ssinfo.m_masterXY) || ssinfo.m_masterXY.size() != 4) 
		{ 
			cerr << "Error in input: expecting 4 comma-separated numbers on line 2" << endl;
			return false;
		}

		// Lines 3 and 4 are slave grating template and offsets
		getline(in, s);
		if (!in.good() || parse_grating(s, ssinfo.m_slaveGrating))
		{
			cerr << "Error in input file: bad grating on line 3" << endl;
			return false;
		}
		ssinfo.m_slaveXY.clear();
		getline(in, s);
		if (!in.good() || parse_number_list(s, ssinfo.m_slaveXY) || ssinfo.m_slaveXY.size() != 4) 
		{ 
			cerr << "Error in input: expecting 4 comma-separated numbers on line 4" << endl;
			return false;
		}

		// line 5 has t2,t3,cbase
		getline(in, s);
		if (!in.good() || parse_number_list(s, vtmp) || vtmp.size() != 3) 
		{ 
			cerr << "Error in input: expecting 3 comma-separated numbers on line 5" << endl;
			return false;
		}
		else
		{
			ssinfo.m_t2 = vtmp[0];
			ssinfo.m_t3 = vtmp[1];
			ssinfo.m_cbase = (int)vtmp[2];
		}

		// The next 5 lines are per-trial values. Each line should have 
		// the same number of items on it!

		getline(in, s);
		if (!in.good() || parse_number_list(s, ssinfo.m_vecT1))
		{
			cerr << "Error in input: expecting comma-separated list of t1 values on line 6." << endl;
			return false;
		}

		getline(in, s);
		if (!in.good() || parse_int_list(s, ssinfo.m_vecCUp))
		{
			cerr << "Error in input: expecting comma-separated list of c_up values on line 7." << endl;
			return false;
		}

		getline(in, s);
		if (!in.good() || parse_int_list(s, ssinfo.m_vecLR))
		{
			cerr << "Error in input: expecting comma-separated list of L/R (0 or 1) values on line 8." << endl;
			return false;
		}

		getline(in, s);
		if (!in.good() || parse_number_list(s, ssinfo.m_vecOri))
		{
			cerr << "Error in input: expecting comma-separated list of orientation values (degrees) on line 9." << endl;
			return false;
		}

		getline(in, s);
		if (!in.good() || parse_number_list(s, ssinfo.m_vecDiam))
		{
			cerr << "Error in input: expecting comma-separated list of diam values on line 10." << endl;
			return false;
		}

		// Verify that the per-trial lines all have the same number of elements
		unsigned int n = ssinfo.m_vecT1.size();
		if (ssinfo.m_vecCUp.size() != n)
		{
			cerr << "Error in input: There are " << n << " values for t1, but " << ssinfo.m_vecCUp.size() << " values for CUp." << endl;
			return false;
		}
		if (ssinfo.m_vecLR.size() != n)
		{
			cerr << "Error in input: There are " << n << " values for t1, but " << ssinfo.m_vecLR.size() << " values for LR." << endl;
			return false;
		}
		if (ssinfo.m_vecOri.size() != n)
		{
			cerr << "Error in input: There are " << n << " values for t1, but " << ssinfo.m_vecOri.size() << " values for Ori." << endl;
			return false;
		}
		if (ssinfo.m_vecDiam.size() != n)
		{
			cerr << "Error in input: There are " << n << " values for t1, but " << ssinfo.m_vecDiam.size() << " values for Diam." << endl;
			return false;
		}


		return true;
	};
};

int main(int argc, char **argv)
{
	SSInfo ssinfo;
	if (SSInfo::load(string("ssinfo.txt"), ssinfo))
	{
		double x, y;
		cout << "Load OK." << endl;
		cout << "Master grating " << ssinfo.getMasterGrating() << endl;
		ssinfo.getMasterXY(0, x, y);
		cout << "Master offsets " << x << ", " << y << " / ";
		ssinfo.getMasterXY(1, x, y);
		cout << x << ", " << y << endl;

		cout << "Slave grating " << ssinfo.getSlaveGrating() << endl;
		ssinfo.getSlaveXY(0, x, y);
		cout << "Slave offsets " << x << ", " << y << " / ";
		ssinfo.getSlaveXY(1, x, y);
		cout << x << ", " << y << endl;

		cout << "t2=" << ssinfo.getT2() << endl;
		cout << "t3=" << ssinfo.getT3() << endl;
		cout << "cbase=" << ssinfo.getCBase() << endl;
		cout << "Got " << ssinfo.getSize() << " trials." << endl;
		for (unsigned int i=0; i<10; i++)
		{
			double t1, ori, diam;
			int CUp, LR;
			if (!ssinfo.getT1(i, t1) ||
				!ssinfo.getCUp(i, CUp) ||
				!ssinfo.getLR(i, LR) ||
				!ssinfo.getOri(i, ori) ||
				!ssinfo.getDiam(i, diam))
			{
				cout << "Cannot get params for trial " << i << endl;
				break;
			}
			else 
			{
				cout << "Trial " << i << ": " << t1 << " " << CUp << " " << LR << "  " << ori << " " << diam << endl;
			}
		}
	}
	else
	{
		cout << "Load error" << endl;
	}
	return 0;
}