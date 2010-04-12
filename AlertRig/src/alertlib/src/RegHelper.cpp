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
string f_szRigName;
string f_szRegistryKey;

bool GetRegConfiguration()
{
	bool b = false;
	size_t iGetenvReturnValue;
	char buffer[256];
	string key;

	if (f_bHaveConfig)
	{
		b = true;
	}
	else
	{

		CRegistry regMyReg( NULL );  // No special flags

		/* Get rig name. Following the convention of the scripts, if an environment var 
		 * RIG is defined, it is used in the registry path to config vars. If RIG is not
		 * found then "AlertRig" is used. For example, if RIG is "AcuteRig", then the 
		 * following path is used: Software\\CED\\Spike2\\AcuteRig
		 */

		if (!getenv_s(&iGetenvReturnValue, buffer, 256, "RIG"))
		{
			f_szRigName.assign(buffer);
			cout << "Env var RIG found: rig name is " << f_szRigName << endl;
		}
		else
		{
			f_szRigName.assign("AlertRig");
			cout << "Env var RIG not found. using \"AlertRig\" as rig name." << endl;
		}


	    /* Now attempt to open the key Software/CED/Spike2/AlertRig/. */

		key = "Software\\CED\\Spike2\\" + f_szRigName;
		if (!CRegistry::KeyExists(key.c_str(), HKEY_CURRENT_USER)) 
		{
			cout << "Registry key \"" + key + "\" not found!!!!" << endl;
		}
		else
		{
			cout << "Opening key " << key << endl;
			regMyReg.Open(key.c_str(), HKEY_CURRENT_USER);
			if (!regMyReg["CurrentConfiguration"].Exists())
			{
				cout << "Registry value for CurrentConfiguration not found!!!!" << endl;
			}
			else
			{
				// Get current configuration
				f_szConfig = (std::string)regMyReg["CurrentConfiguration"];
				f_szRegistryKey.assign("Software\\CED\\Spike2\\");
				f_szRegistryKey.append(f_szRigName);
				f_szRegistryKey.append("\\");
				f_szRegistryKey.append(f_szConfig);
				cout << "Registry key is " << f_szRegistryKey << endl;
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

		/* On the AcuteRig there is no DAQ key - the screen distance is stored with Data parameters. */
		if (f_szRigName == "AcuteRig")
		{
			szKey.assign("Software\\CED\\Spike2\\AcuteRig\\Data");
		}
		else
		{
			szKey = f_szRegistryKey + "\\DAQ";
		}

		if (!CRegistry::KeyExists(szKey.c_str(), HKEY_CURRENT_USER)) 
		{
			std::string sztemp = "Registry key " + szKey + " not found!!!!";
			std::cout << sztemp << std::endl;
		}
		else
		{
			regMyReg.Open(szKey.c_str(), HKEY_CURRENT_USER);
			if (!regMyReg["DistanceToScreenMM"].Exists())
			{
				std::string sztemp = "Registry value DistanceToScreenMM for key " + szKey + " not found!!!!";
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
		szKey = f_szRegistryKey + "\\FixationPoint";
		if (!CRegistry::KeyExists(szKey.c_str(), HKEY_CURRENT_USER)) 
		{
			std::string sztemp = "Registry key " + szKey + " not found!!!!";
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
					std::string sztemp = "Registry value FixationX for key " + szKey + " not found!!!!";
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
					std::string sztemp = "Registry value FixationY for key " + szKey + " not found!!!!";
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
					std::string sztemp = "Registry value FixationDiameter for key " + szKey + " not found!!!!";
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
					std::string sztemp = "Registry value FixationColor for key " + szKey + " not found!!!!";
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
		szKey = f_szRegistryKey + "\\Gratings";
		if (!CRegistry::KeyExists(szKey.c_str(), HKEY_CURRENT_USER)) 
		{
			std::string sztemp = "Registry key " + szKey + " not found!!!!";
			cout << sztemp << endl;
		}
		else
		{
			regMyReg.Open(szKey.c_str(), HKEY_CURRENT_USER);
			if (!regMyReg["Stimulus"].Exists())
			{
				std::string sztemp = "Registry value Stimulus for key " + szKey + " not found!!!!";
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
		szKey = f_szRegistryKey + "\\Gratings";
		if (!CRegistry::KeyExists(szKey.c_str(), HKEY_CURRENT_USER)) 
		{
			std::string sztemp = "Registry key " + szKey + " not found!!!!";
			cout << sztemp << endl;
		}
		else
		{
			regMyReg.Open(szKey.c_str(), HKEY_CURRENT_USER);
			if (!regMyReg["Stimulus"].Exists())
			{
				std::string sztemp = "Registry value Stimulus for key " + szKey + " not found!!!!";
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
