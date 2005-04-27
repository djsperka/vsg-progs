// CoolerMouse.h : main header file for the COOLERMOUSE application
//

#if !defined(AFX_COOLERMOUSE_H__AD0E7A46_C6FD_40F5_9CE5_5C265506A9E0__INCLUDED_)
#define AFX_COOLERMOUSE_H__AD0E7A46_C6FD_40F5_9CE5_5C265506A9E0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "VSGHelper.h"
#include "CoolerMouseDlg.h"


/////////////////////////////////////////////////////////////////////////////
// CCoolerMouseApp:
// See CoolerMouse.cpp for the implementation of this class
//

class CCoolerMouseApp : public CWinApp
{
public:
	CCoolerMouseApp();

	// Return pointer to VSG helper object. 
	VSGHelper *getVSG() { return m_pvsg; };
	CCoolerMouseDlg *getDlg() { return m_pdlg; };

	bool GetRegConfiguration();
	bool GetRegScreenDistance(int& dist);
	bool GetRegFixpt(std::string& s);
	bool SaveRegFixpt();
	bool GetRegStimulus(std::string& s);
	bool SaveRegStimulus(std::string s);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCoolerMouseApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CCoolerMouseApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CCoolerMouseDlg *m_pdlg;
	VSGHelper *m_pvsg;
	std::string m_szConfig;
	bool m_bHaveConfig;


};



extern CCoolerMouseApp theApp;


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COOLERMOUSE_H__AD0E7A46_C6FD_40F5_9CE5_5C265506A9E0__INCLUDED_)
