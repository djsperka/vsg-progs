#if !defined(AFX_VSGFRAMECTRL_H__0046AD76_1D42_400F_9505_25503C9FC62B__INCLUDED_)
#define AFX_VSGFRAMECTRL_H__0046AD76_1D42_400F_9505_25503C9FC62B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VSGFrameCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVSGFrameCtrl frame

class CVSGFrameCtrl : public CFrameWnd
{
public:
	DECLARE_DYNCREATE(CVSGFrameCtrl)
	CVSGFrameCtrl();           // protected constructor used by dynamic creation
	virtual ~CVSGFrameCtrl();
	virtual BOOL Create(CWnd* pParentWnd, const RECT& rect, UINT nID, DWORD dwStyle = WS_VISIBLE | WS_CHILD);
	BOOL m_bCreated;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVSGFrameCtrl)
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CVSGFrameCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VSGFRAMECTRL_H__0046AD76_1D42_400F_9505_25503C9FC62B__INCLUDED_)
