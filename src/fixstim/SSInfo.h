#ifndef _SSINFO_H_
#define _SSINFO_H_

#include <vector>
#include "alertlib.h"

using namespace alert;
using namespace std;

class SSInfo
{
private:
	ARGratingSpec m_coreGrating;
	ARGratingSpec m_donutGrating;
	vector<double> m_coreXY;
	vector<double> m_donutXY;
	bool m_bCoreIsMaster;
	bool m_bDanish;
	double m_t2, m_t3;
	int m_cbase;
	vector<double> m_vecT1;
	vector<int> m_vecCUp;
	vector<int> m_vecLR;
	vector<double> m_vecOri;
	vector<double> m_vecDiam;
	vector<double> m_vecInnerDiam;
	double m_tfHC;		// temporal freq of hi-contrast grids
	double m_tHC;		// sec (total) for the grids
	bool m_bUseAnswerPoints;
	int m_iCUpAnswerPointContrast;
	int m_iCSameAnswerPointContrast;
	double m_tAnswerPoint;
public:
	SSInfo() : m_bDanish(false) {};
	SSInfo(const SSInfo& ss) : 
			m_coreGrating(ss.m_coreGrating), 
			m_donutGrating(ss.m_donutGrating), 
			m_coreXY(ss.m_coreXY), m_donutXY(ss.m_donutXY),
			m_bCoreIsMaster(ss.m_bCoreIsMaster), m_bDanish(ss.m_bDanish),
			m_t2(ss.m_t2), m_t3(ss.m_t3), m_cbase(ss.m_cbase),
			m_vecT1(ss.m_vecT1), 
			m_vecCUp(ss.m_vecCUp),
			m_vecLR(ss.m_vecLR),
			m_vecOri(ss.m_vecOri),
			m_vecDiam(ss.m_vecDiam),
			m_vecInnerDiam(ss.m_vecInnerDiam),
			m_tfHC(ss.m_tfHC),
			m_tHC(ss.m_tHC),
			m_bUseAnswerPoints(ss.m_bUseAnswerPoints),
			m_iCUpAnswerPointContrast(ss.m_iCUpAnswerPointContrast),
			m_iCSameAnswerPointContrast(ss.m_iCSameAnswerPointContrast),
			m_tAnswerPoint(ss.m_tAnswerPoint)
	{
	};
	~SSInfo() {};
	const bool getCoreIsMaster() const { return m_bCoreIsMaster; };
	const bool getIsDanish() const { return m_bDanish; };
	const ARGratingSpec& getCoreGrating() const { return m_coreGrating; };
	const ARGratingSpec& getDonutGrating() const { return m_donutGrating; };
	const double getTFHC() const { return m_tfHC; };
	const double getTHC() const { return m_tHC; };
	bool getCoreXY(int i, double& x, double& y) const
	{
		bool b = true;
		switch (i)
		{
			case 0: x = m_coreXY[0]; y = m_coreXY[1]; break;
			case 1: x = m_coreXY[2]; y = m_coreXY[3]; break;
			default: x = -9999999; y = -9999999; b = false;
		}
		return b;
	};
	bool getDonutXY(int i, double& x, double& y) const
	{
		bool b = true;
		switch (i)
		{
			case 0: x = m_donutXY[0]; y = m_donutXY[1]; break;
			case 1: x = m_donutXY[2]; y = m_donutXY[3]; break;
			default: x = -9999999; y = -9999999; b = false;
		}
		return b;
	};
	double getT2() const { return m_t2; };
	double getT3() const { return m_t3; };
	int getCBase() const { return m_cbase; };
	unsigned int getSize() const { return m_vecT1.size(); };
	bool getUseAnswerPoints() const { return m_bUseAnswerPoints; };
	void getAnswerPointParameters(int& iCUpContrast, int& iCSameContrast, double& tAnswerPoint)
	{
		iCUpContrast = m_iCUpAnswerPointContrast;
		iCSameContrast = m_iCSameAnswerPointContrast;
		tAnswerPoint = m_tAnswerPoint;
		return;
	};

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

	bool getInnerDiam(int trial, double& diam) const 
	{
		bool b = true;
		if (trial > -1 && trial < (int)m_vecInnerDiam.size()) diam = m_vecInnerDiam[trial];
		else b = false;
		return b;
	};

	static bool load(string& filename, SSInfo& ssinfo)
	{
		bool b = true;
		string s;
		vector<double>vtmp;
		vector<string>vtokens;
		unsigned int i;
		ifstream in(filename.c_str(), ifstream::in);

		if (!in.good()) 
		{
			cerr << "Cannot open input file " << filename << endl;
			return false;
		}

		// First line is "master,slave" or "slave,master". I'm just going to look at the first character.
		// Update: first line will now carry info about answer points. 
		// master,slave,answer points,100,100,2
		// That's master-core, slave-donut, use answer points, cUp contrast 100, cSame contrast 100, 2 sec max 
		// Tokenize with comma delimeter.
		// Defaults are m,s,no answer points,100,100,5
		// Can have only m, or m,s 
		// Third token should be omitted if no answer points, but can be included (anything not starting 
		// with "a" or "A") if you want to change the final page time. the contrast values will be parsed
		// in that case but not used. 
		getline(in, s);
		if (!in.good())
		{
			cerr << "Error in input at first line." << endl;
			return false;
		}
		tokenize(s, vtokens, ",");

		i = 0;
		ssinfo.m_bUseAnswerPoints = false;
		ssinfo.m_iCUpAnswerPointContrast = 100;
		ssinfo.m_iCSameAnswerPointContrast = 100;
		ssinfo.m_tAnswerPoint = 5;
		while (i < vtokens.size())
		{
			switch (i)
			{
			case 0: 
				{
					if (vtokens[i][0] == 'M' || vtokens[i][0] == 'm')
					{
						ssinfo.m_bCoreIsMaster = true;
						cerr << "Core is master screen." << endl;
					}
					else if (vtokens[i][0] == 'S' || vtokens[i][0] == 's')
					{
						ssinfo.m_bCoreIsMaster = false;
						cerr << "Core is slave screen." << endl;
					}
					else
					{
						cerr << "Error in input at first line: first line should be \"master,slave\" or \"slave,master\"." << endl;
						return false;
					}
					break;
				}
			case 1:
				{
					if (vtokens[i][0] == 'M' || vtokens[i][0] == 'm')
					{
						if (ssinfo.m_bCoreIsMaster)
						{
							cerr << "Error in input at first line: first line should start with \"master,slave\" or \"slave,master\"." << endl;
							return false;
						}
					}
					else if (vtokens[i][0] == 'S' || vtokens[i][0] == 's')
					{
						if (!ssinfo.m_bCoreIsMaster)
						{
							cerr << "Error in input at first line: first line should start with \"master,slave\" or \"slave,master\"." << endl;
							return false;
						}
					}
					else if (vtokens[i][0] == 'D' || vtokens[i][i] == 'd')
					{
						ssinfo.m_bDanish = true;
					}
					else
					{
						cerr << "Error in input at first line: first line should start with \"master,slave\" or \"slave,master\"." << endl;
						return false;
					}
					break;
				}
			case 2:
				{
					if (vtokens[i][0] == 'A' || vtokens[i][0] == 'a')
					{
						ssinfo.m_bUseAnswerPoints = true;
						cerr << "Using answer points. " << endl;
					}
					else if (vtokens[i][0] == 'N' || vtokens[i][0] == 'n')
					{
						ssinfo.m_bUseAnswerPoints = false;
					}
					else
					{
						cerr << "Error in input at first line: Third token on first line should be start with A|a|N|n for answer points (or none)" << endl;
						return false;
					}
					break;
				}
			case 3:
				{
					if (parse_integer(vtokens[i], ssinfo.m_iCUpAnswerPointContrast))
					{
						cerr << "Error in input at first line: 4th token should be integer contrast." << endl;
						return false;
					}
					break;
				}
			case 4:
				{
					if (parse_integer(vtokens[i], ssinfo.m_iCSameAnswerPointContrast))
					{
						cerr << "Error in input at first line: 5th token should be integer contrast." << endl;
						return false;
					}
					break;
				}
			case 5:
				{
					if (parse_double(vtokens[i], ssinfo.m_tAnswerPoint))
					{
						cerr << "Error in input at first line: 6th token should be answer point page max time." << endl;
						return false;
					}
					break;
				}
			default:
				{
					cerr << "Error in input: extra tokens on line 1." << endl;
					return false;
					break;
				}
			}

			// don't forget to increment i!
			i++;
		}

		// second line is core grating template
		getline(in, s);
		if (!in.good() || parse_grating(s, ssinfo.m_coreGrating))
		{
			cerr << "Error in input file: bad grating on line 2" << endl;
			return false;
		}

		// next is x y x y for offsets of core gratings
		ssinfo.m_coreXY.clear();
		getline(in, s);
		if (!in.good() || parse_number_list(s, ssinfo.m_coreXY) || ssinfo.m_coreXY.size() != 4) 
		{ 
			cerr << "Error in input: expecting 4 comma-separated numbers on line 3" << endl;
			return false;
		}

		// Lines 4 and 5 are donut grating template and offsets
		getline(in, s);
		if (!in.good() || parse_grating(s, ssinfo.m_donutGrating))
		{
			cerr << "Error in input file: bad grating on line 4" << endl;
			return false;
		}
		ssinfo.m_donutXY.clear();
		getline(in, s);
		if (!in.good() || parse_number_list(s, ssinfo.m_donutXY) || ssinfo.m_donutXY.size() != 4) 
		{ 
			cerr << "Error in input: expecting 4 comma-separated numbers on line 5" << endl;
			return false;
		}

		// line 6 has t2,t3,cbase,nHC,msHC
		getline(in, s);
		if (!in.good() || parse_number_list(s, vtmp) || vtmp.size() != 5)
		{
			cerr << "Error in input: expecting 5 comma-separated numbers (t2,t3,cbase,tfHC,msHC) on line 6" << endl;
			return false;
		}
		else
		{
			ssinfo.m_t2 = vtmp[0];
			ssinfo.m_t3 = vtmp[1];
			ssinfo.m_cbase = (int)vtmp[2];
			ssinfo.m_tfHC = vtmp[3];
			ssinfo.m_tHC = vtmp[4];
		}

		// The next 5 lines are per-trial values. Each line should have 
		// the same number of items on it!

		getline(in, s);
		if (!in.good() || parse_number_list(s, ssinfo.m_vecT1))
		{
			cerr << "Error in input: expecting comma-separated list of t1 values on line 7." << endl;
			return false;
		}
		else
		{
			cerr << "Got " << ssinfo.m_vecT1.size() << " T1 values." << endl;
		}

		getline(in, s);
		if (!in.good() || parse_int_list(s, ssinfo.m_vecCUp))
		{
			cerr << "Error in input: expecting comma-separated list of c_up values on line 8." << endl;
			return false;
		}
		else
		{
			cerr << "Got " << ssinfo.m_vecCUp.size() << " CUp values." << endl;
		}

		getline(in, s);
		if (!in.good() || parse_int_list(s, ssinfo.m_vecLR))
		{
			cerr << "Error in input: expecting comma-separated list of L/R (0 or 1) values on line 9." << endl;
			return false;
		}
		else
		{
			cerr << "Got " << ssinfo.m_vecLR.size() << " LR values." << endl;
		}

		getline(in, s);
		if (!in.good() || parse_number_list(s, ssinfo.m_vecOri))
		{
			cerr << "Error in input: expecting comma-separated list of orientation values (degrees) on line 10." << endl;
			return false;
		}
		else
		{
			cerr << "Got " << ssinfo.m_vecOri.size() << " Ori values." << endl;
		}

		getline(in, s);
		if (!in.good() || parse_number_list(s, ssinfo.m_vecDiam))
		{
			cerr << "Error in input: expecting comma-separated list of diam values on line 11." << endl;
			return false;
		}
		else
		{
			cerr << "Got " << ssinfo.m_vecDiam.size() << " Diam values." << endl;
		}

		getline(in, s);
		if (!in.good() || parse_number_list(s, ssinfo.m_vecInnerDiam))
		{
			cerr << "Error in input: expecting comma-separated list of inner diam values on line 12." << endl;
			return false;
		}
		else
		{
			cerr << "Got " << ssinfo.m_vecInnerDiam.size() << " Inner Diam values." << endl;
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
		if (ssinfo.m_vecInnerDiam.size() != n)
		{
			cerr << "Error in input: There are " << n << " values for t1, but " << ssinfo.m_vecInnerDiam.size() << " values for Inner Diam." << endl;
			return false;
		}


		return true;
	};
};


#endif