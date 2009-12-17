// SendMessage.h : main header file for the SENDMESSAGE application
//

#if !defined(AFX_SENDMESSAGE_H__212471BF_D333_4D51_B19D_1E13D5A226FF__INCLUDED_)
#define AFX_SENDMESSAGE_H__212471BF_D333_4D51_B19D_1E13D5A226FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSendMessageApp:
// See SendMessage.cpp for the implementation of this class
//

class CSendMessageApp : public CWinApp
{
public:
	CSendMessageApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSendMessageApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSendMessageApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENDMESSAGE_H__212471BF_D333_4D51_B19D_1E13D5A226FF__INCLUDED_)
