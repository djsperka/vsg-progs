// CoolerMouse.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "CoolerMouse.h"
#include "CoolerMouseDlg.h"

#include "Registry.h"
#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCoolerMouseApp

BEGIN_MESSAGE_MAP(CCoolerMouseApp, CWinApp)
	//{{AFX_MSG_MAP(CCoolerMouseApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCoolerMouseApp construction

CCoolerMouseApp::CCoolerMouseApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CCoolerMouseApp object

CCoolerMouseApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CCoolerMouseApp initialization

BOOL CCoolerMouseApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Fetch stuff from registry and initialize vsg helper class. 
	// Initialize vsg helper. First, get screen distance, background, etc. 
	int dist=0;
	if (!GetRegConfiguration())
	{
		return FALSE;
	}

	if (!GetRegScreenDistance(dist))
	{
		return FALSE;
	}
	else
	{
		m_pvsg = new VSGHelper(true, dist);
	}

	// Now get init string for stimulus and fixpt. 
	std::string s;
	if (!GetRegStimulus(s))
	{
		return FALSE;
	}
	else
	{
		m_pvsg->setStimString(s);
	}

	if (!GetRegFixpt(s))
	{
		return FALSE;
	}
	else
	{
		m_pvsg->setFixationPointString(s);
	}

	// OK, all is well. If we made it this far then we can start up the vsg......
	m_pvsg->start();




	CCoolerMouseDlg dlg;
	m_pMainWnd = &dlg;
	m_pdlg = &dlg;			// for access from view class. 
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


bool CCoolerMouseApp::GetRegConfiguration()
{
	bool b = false;
	if (m_bHaveConfig)
	{
		b = true;
	}
	else
	{

		CRegistry regMyReg( NULL );  // No special flags

	    /* Now attempt to open the key Software/CED/Spike2/AlertRig/. */

		if (!CRegistry::KeyExists("Software\\CED\\Spike2\\AlertRig", HKEY_CURRENT_USER)) 
		{
			::MessageBox(NULL, "AlertRig registry key not found!!!!", "Error", MB_OK | MB_ICONHAND);
		}
		else
		{
			regMyReg.Open("Software\\CED\\Spike2\\AlertRig", HKEY_CURRENT_USER);
			if (!regMyReg["CurrentConfiguration"].Exists())
			{
				::MessageBox(NULL, "AlertRig registry value for CurrentConfiguration not found!!!!", "Error", MB_OK | MB_ICONHAND);
			}
			else
			{
				// Get current configuration
				m_szConfig = (std::string)regMyReg["CurrentConfiguration"];
				m_bHaveConfig = true;

				// Display the current start page value
				::MessageBox(NULL, m_szConfig.c_str(), "CurrentConfiguration", MB_OK);

				b = true;
			}
			// Close the open key
			regMyReg.Close();

		}
	
	}

	return b;

}


bool CCoolerMouseApp::GetRegScreenDistance(int& dist)
{
	bool b = false;
	std::string szKey;

	if (GetRegConfiguration())
	{
		CRegistry regMyReg( NULL );  // No special flags

	    /* Now attempt to open the key Software/CED/Spike2/AlertRig/. */
		szKey = "Software\\CED\\Spike2\\AlertRig\\" + m_szConfig + "\\DAQ";
		if (!CRegistry::KeyExists(szKey.c_str(), HKEY_CURRENT_USER)) 
		{
			std::string sztemp = "AlertRig registry key " + szKey + " not found!!!!";
			::MessageBox(NULL, sztemp.c_str(), "Error", MB_OK | MB_ICONHAND);
		}
		else
		{
			regMyReg.Open(szKey.c_str(), HKEY_CURRENT_USER);
			if (!regMyReg["DistanceToScreenMM"].Exists())
			{
				std::string sztemp = "AlertRig registry value DistanceToScreenMM for key " + szKey + " not found!!!!";
				::MessageBox(NULL, sztemp.c_str(), "Error", MB_OK | MB_ICONHAND);
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
		::MessageBox(NULL, "GetRegScreenDistance - must call GetRegConfig first!", "Error", MB_OK | MB_ICONHAND);
	}

	return b;
}

bool CCoolerMouseApp::GetRegFixpt(std::string& s)
{
	bool b = true;			// turn false on error --diff't logic than other GetReg* funcs
	std::string szKey;
	std::string szX, szY, szD;
	int icolor;

	if (GetRegConfiguration())
	{
		CRegistry regMyReg( NULL );  // No special flags

	    /* Now attempt to open the key Software/CED/Spike2/AlertRig/. */
		szKey = "Software\\CED\\Spike2\\AlertRig\\" + m_szConfig + "\\FixationPoint";
		if (!CRegistry::KeyExists(szKey.c_str(), HKEY_CURRENT_USER)) 
		{
			std::string sztemp = "AlertRig registry key " + szKey + " not found!!!!";
			::MessageBox(NULL, sztemp.c_str(), "Error", MB_OK | MB_ICONHAND);
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
					::MessageBox(NULL, sztemp.c_str(), "Error", MB_OK | MB_ICONHAND);
					b = false;
				}
				else
				{
					// Get current configuration
					szX = (std::string)regMyReg["FixationX"];
				}
			}

			if (b)
			{
				if (!regMyReg["FixationY"].Exists())
				{
					std::string sztemp = "AlertRig registry value FixationY for key " + szKey + " not found!!!!";
					::MessageBox(NULL, sztemp.c_str(), "Error", MB_OK | MB_ICONHAND);
					b = false;
				}
				else
				{
					// Get current configuration
					szY = (std::string)regMyReg["FixationY"];
				}
			}
			
			if (b)
			{
				if (!regMyReg["FixationDiameter"].Exists())
				{
					std::string sztemp = "AlertRig registry value FixationDiameter for key " + szKey + " not found!!!!";
					::MessageBox(NULL, sztemp.c_str(), "Error", MB_OK | MB_ICONHAND);
					b = false;
				}
				else
				{
					// Get current configuration
					szD = (std::string)regMyReg["FixationDiameter"];
				}
			}

			if (b)
			{
				if (!regMyReg["FixationColor"].Exists())
				{
					std::string sztemp = "AlertRig registry value FixationColor for key " + szKey + " not found!!!!";
					::MessageBox(NULL, sztemp.c_str(), "Error", MB_OK | MB_ICONHAND);
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
				// put it all together.
				std::ostringstream oss;
				oss << szX << "," << szY << "," << szD << "," << (COLOR_TYPE)icolor;
				s = oss.str();
			}

			// Close the open key
			regMyReg.Close();
		}
	}
	else 
	{
		::MessageBox(NULL, "GetRegFixpt - must call GetRegConfig first!", "Error", MB_OK | MB_ICONHAND);
		b = false;
	}
	return b;
}

bool CCoolerMouseApp::GetRegStimulus(std::string& s)
{
	bool b = false;
	std::string szKey;
	if (GetRegConfiguration())
	{
		CRegistry regMyReg( NULL );  // No special flags

	    /* Now attempt to open the key Software/CED/Spike2/AlertRig/. */
		szKey = "Software\\CED\\Spike2\\AlertRig\\" + m_szConfig + "\\Gratings";
		if (!CRegistry::KeyExists(szKey.c_str(), HKEY_CURRENT_USER)) 
		{
			std::string sztemp = "AlertRig registry key " + szKey + " not found!!!!";
			::MessageBox(NULL, sztemp.c_str(), "Error", MB_OK | MB_ICONHAND);
		}
		else
		{
			regMyReg.Open(szKey.c_str(), HKEY_CURRENT_USER);
			if (!regMyReg["Stimulus"].Exists())
			{
				std::string sztemp = "AlertRig registry value Stimulus for key " + szKey + " not found!!!!";
				::MessageBox(NULL, sztemp.c_str(), "Error", MB_OK | MB_ICONHAND);
			}
			else
			{
				// Get current configuration
				s = (std::string)regMyReg["Stimulus"];
				b = true;
			}
			// Close the open key
			regMyReg.Close();
		}
	}
	else 
	{
		::MessageBox(NULL, "GetRegStimulus - must call GetRegConfig first!", "Error", MB_OK | MB_ICONHAND);
	}

	return b;
}
