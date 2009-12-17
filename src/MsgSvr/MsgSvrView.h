// MsgSvrView.h : interface of the CMsgSvrView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MSGSVRVIEW_H__697A8E4F_F5AC_426B_91AB_C3592A8EB79E__INCLUDED_)
#define AFX_MSGSVRVIEW_H__697A8E4F_F5AC_426B_91AB_C3592A8EB79E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CMsgSvrView : public CView
{
protected: // create from serialization only
	CMsgSvrView();
	DECLARE_DYNCREATE(CMsgSvrView)

// Attributes
public:
	CMsgSvrDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMsgSvrView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMsgSvrView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CMsgSvrView)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	afx_msg void OnCancelEditSrvr();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in MsgSvrView.cpp
inline CMsgSvrDoc* CMsgSvrView::GetDocument()
   { return (CMsgSvrDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MSGSVRVIEW_H__697A8E4F_F5AC_426B_91AB_C3592A8EB79E__INCLUDED_)
