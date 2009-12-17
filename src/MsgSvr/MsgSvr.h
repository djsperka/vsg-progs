// MsgSvr.h : main header file for the MSGSVR application
//

#if !defined(AFX_MSGSVR_H__52A12655_3D5E_4D46_8319_438448C3D3C2__INCLUDED_)
#define AFX_MSGSVR_H__52A12655_3D5E_4D46_8319_438448C3D3C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CMsgSvrApp:
// See MsgSvr.cpp for the implementation of this class
//

class CMsgSvrApp : public CWinApp
{
public:
	CMsgSvrApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMsgSvrApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	COleTemplateServer m_server;
	CStringList m_listMessages;
		// Server object for document creation
	//{{AFX_MSG(CMsgSvrApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MSGSVR_H__52A12655_3D5E_4D46_8319_438448C3D3C2__INCLUDED_)
