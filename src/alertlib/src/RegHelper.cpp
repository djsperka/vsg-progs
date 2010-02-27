/*#include "stdafx.h"*/

#define _CRT_SECURE_NO_WARNINGS
#include "reghelper.h"
#include "Registry.h"
#include "alertlib.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
using namespace alert;

string f_szConfig;
bool f_bHaveConfig = false;


bool GetRegConfiguration()
{
	bool b = false;
	if (f_bHaveConfig)
	{
		b = true;
	}
	else
	{

		CRegistry regMyReg( NULL );  // No special flags

	    /* Now attempt to open the key Software/CED/Spike2/AlertRig/. */

		if (!CRegistry::KeyExists("Software\\CED\\Spike2\\AlertRig", HKEY_CURRENT_USER)) 
		{
			cout << "AlertRig registry key not found!!!!" << endl;
		}
		else
		{
			regMyReg.Open("Software\\CED\\Spike2\\AlertRig", HKEY_CURRENT_USER);
			if (!regMyReg["CurrentConfiguration"].Exists())
			{
				cout << "AlertRig registry value for CurrentConfiguration not found!!!!" << endl;
			}
			else
			{
				// Get current configuration
				f_szConfig = (std::string)regMyReg["CurrentConfiguration"];
				f_bHaveConfig = true;
				b = true;
			}
			// Close the open key
			regMyReg.Close();

		}
	
	}

	return b;

}


bool GetRegScreenDistance(int& dist)
{
	bool b = false;
	std::string szKey;

	if (GetRegConfiguration())
	{
		CRegistry regMyReg( NULL );  // No special flags

	    /* Now attempt to open the key Software/CED/Spike2/AlertRig/. */
		szKey = "Software\\CED\\Spike2\\AlertRig\\" + f_szConfig + "\\DAQ";
		if (!CRegistry::KeyExists(szKey.c_str(), HKEY_CURRENT_USER)) 
		{
			std::string sztemp = "AlertRig registry key " + szKey + " not found!!!!";
			std::cout << sztemp << std::endl;
		}
		else
		{
			regMyReg.Open(szKey.c_str(), HKEY_CURRENT_USER);
			if (!regMyReg["DistanceToScreenMM"].Exists())
			{
				std::string sztemp = "AlertRig registry value DistanceToScreenMM for key " + szKey + " not found!!!!";
				std::cout << sztemp << std::endl;
			}
			else
			{
				// Get current configuration
				dist = regMyReg["DistanceToScreenMM"];
				b = true;
			}
			// Close the open key
			regMyReg.Close();
		}
	}
	else 
	{
		std::cout << "GetRegScreenDistance - GetRegConfig failed!" << std::endl;
	}

	return b;
}

bool GetRegFixpt(double& fixX, double& fixY, double &fixD, COLOR_TYPE& fixC)
{
	bool b = true;			// turn false on error --diff't logic than other GetReg* funcs
	std::string szKey;
	std::string szX, szY, szD;
	int icolor;

	if (GetRegConfiguration())
	{
		CRegistry regMyReg( NULL );  // No special flags

	    /* Now attempt to open the key Software/CED/Spike2/AlertRig/. */
		szKey = "Software\\CED\\Spike2\\AlertRig\\" + f_szConfig + "\\FixationPoint";
		if (!CRegistry::KeyExists(szKey.c_str(), HKEY_CURRENT_USER)) 
		{
			std::string sztemp = "AlertRig registry key " + szKey + " not found!!!!";
			std::cout << sztemp << std::endl;
			b = false;
		}
		else
		{
			// There are 4 values to get: FixationX,FixationY,FixationDiameter,color
			regMyReg.Open(szKey.c_str(), HKEY_CURRENT_USER);
			if (b)
			{
				if (!regMyReg["FixationX"].Exists())
				{
					std::string sztemp = "AlertRig registry value FixationX for key " + szKey + " not found!!!!";
					std::cout << sztemp << std::endl;
					b = false;
				}
				else
				{
					// Get current configuration
					szX = (std::string)regMyReg["FixationX"];
					fixX = atof(szX.c_str());
				}
			}

			if (b)
			{
				if (!regMyReg["FixationY"].Exists())
				{
					std::string sztemp = "AlertRig registry value FixationY for key " + szKey + " not found!!!!";
					std::cout << sztemp << std::endl;
					b = false;
				}
				else
				{
					// Get current configuration
					szY = (std::string)regMyReg["FixationY"];
					fixY = atof(szY.c_str());
				}
			}
			
			if (b)
			{
				if (!regMyReg["FixationDiameter"].Exists())
				{
					std::string sztemp = "AlertRig registry value FixationDiameter for key " + szKey + " not found!!!!";
					std::cout << sztemp << std::endl;
					b = false;
				}
				else
				{
					// Get current configuration
					szD = (std::string)regMyReg["FixationDiameter"];
					fixD = atof(szD.c_str());
				}
			}

			if (b)
			{
				if (!regMyReg["FixationColor"].Exists())
				{
					std::string sztemp = "AlertRig registry value FixationColor for key " + szKey + " not found!!!!";
					std::cout << sztemp << std::endl;
					b = false;
				}
				else
				{
					// Get current configuration
					icolor = regMyReg["FixationColor"];
				}
			}
			

			if (b)
			{
				// Note that the color index stored refers to that used
				// in spike2. We add 3 to it to get the alertlib's index. 
				fixC.type = (COLOR_ENUM)(icolor+3);
			}

			// Close the open key
			regMyReg.Close();
		}
	}
	else 
	{
		std::cout << "GetRegFixpt - GetRegConfig failed!" << std::endl;
		b = false;
	}
	return b;
}

bool GetRegStimulus(ARApertureGratingSpec &stim)
{
	bool b = false;
	std::string szKey;
	if (GetRegConfiguration())
	{
		CRegistry regMyReg( NULL );  // No special flags

	    /* Now attempt to open the key Software/CED/Spike2/AlertRig/. */
		szKey = "Software\\CED\\Spike2\\AlertRig\\" + f_szConfig + "\\Gratings";
		if (!CRegistry::KeyExists(szKey.c_str(), HKEY_CURRENT_USER)) 
		{
			std::string sztemp = "AlertRig registry key " + szKey + " not found!!!!";
			cout << sztemp << endl;
		}
		else
		{
			regMyReg.Open(szKey.c_str(), HKEY_CURRENT_USER);
			if (!regMyReg["Stimulus"].Exists())
			{
				std::string sztemp = "AlertRig registry value Stimulus for key " + szKey + " not found!!!!";
				cout << sztemp << endl;
			}
			else
			{
				// Get current configuration
				string s = (std::string)regMyReg["Stimulus"];
				parse_grating(s, stim);
				b = true;
			}
			// Close the open key
			regMyReg.Close();
		}
	}
	else 
	{
		cout << "GetRegStimulus - GetRegConfig failed!" << endl;
	}

	return b;
}

bool SaveRegStimulus(std::string s)
{
	bool b = false;
	std::string szKey;
	if (GetRegConfiguration())
	{
		CRegistry regMyReg( NULL );  // No special flags

	    /* Now attempt to open the key Software/CED/Spike2/AlertRig/. */
		szKey = "Software\\CED\\Spike2\\AlertRig\\" + f_szConfig + "\\Gratings";
		if (!CRegistry::KeyExists(szKey.c_str(), HKEY_CURRENT_USER)) 
		{
			std::string sztemp = "AlertRig registry key " + szKey + " not found!!!!";
			cout << sztemp << endl;
		}
		else
		{
			regMyReg.Open(szKey.c_str(), HKEY_CURRENT_USER);
			if (!regMyReg["Stimulus"].Exists())
			{
				std::string sztemp = "AlertRig registry value Stimulus for key " + szKey + " not found!!!!";
				cout << sztemp << endl;
			}
			else
			{
				// Get current configuration
				regMyReg["Stimulus"] = s;
				b = true;
			}
			// Close the open key
			regMyReg.Close();
		}
	}
	else 
	{
		cout << "GetRegStimulus - GetRegConfig failed!" << endl;
	}

	return b;
}


bool GetRegLockFile(std::string &s)
{
	bool b = false;
	CRegistry regMyReg( NULL );  // No special flags

    /* Attempt to open the key Software/CED/Spike2/. */

	if (!CRegistry::KeyExists("Software\\CED\\Spike2", HKEY_CURRENT_USER)) 
	{
		cout << "Spike2 registry key not found in HKEY_CURRENT_USER (Is spike2 installed on this machine?" << endl;
	}
	else
	{
		regMyReg.Open("Software\\CED\\Spike2", HKEY_CURRENT_USER);
		if (!regMyReg["LockFile"].Exists())
		{
			cout << "Spike2 registry value for LockFile not found!!!!" << endl;
		}
		else
		{
			// Get the value
			s = (std::string)regMyReg["LockFile"];
			b = true;
		}
		// Close the open key
		regMyReg.Close();

	}

	return b;

}
