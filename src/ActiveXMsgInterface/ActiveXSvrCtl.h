#if !defined(AFX_ACTIVEXSVRCTL_H__52254868_D7A5_4DB7_9323_D3A815516646__INCLUDED_)
#define AFX_ACTIVEXSVRCTL_H__52254868_D7A5_4DB7_9323_D3A815516646__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ActiveXSvrCtl.h : Declaration of the CActiveXSvrCtrl ActiveX Control class.

/////////////////////////////////////////////////////////////////////////////
// CActiveXSvrCtrl : See ActiveXSvrCtl.cpp for implementation.

class CActiveXSvrCtrl : public COleControl
{
	DECLARE_DYNCREATE(CActiveXSvrCtrl)

// Constructor
public:
	CActiveXSvrCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CActiveXSvrCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	virtual DWORD GetControlFlags();
	//}}AFX_VIRTUAL

// Implementation
protected:
	~CActiveXSvrCtrl();

	IMsgSvr m_handler;

	DECLARE_OLECREATE_EX(CActiveXSvrCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CActiveXSvrCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CActiveXSvrCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CActiveXSvrCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CActiveXSvrCtrl)
		// NOTE - ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CActiveXSvrCtrl)
	afx_msg BSTR getNextMessage();
	afx_msg void addMessage(LPCTSTR strMessage);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

// Event maps
	//{{AFX_EVENT(CActiveXSvrCtrl)
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CActiveXSvrCtrl)
	dispidGetNextMessage = 1L,
	dispidAddMessage = 2L,
	//}}AFX_DISP_ID
	};
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ACTIVEXSVRCTL_H__52254868_D7A5_4DB7_9323_D3A815516646__INCLUDED)
