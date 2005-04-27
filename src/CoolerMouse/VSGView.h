#if !defined(AFX_VSGVIEW_H__43F705D7_D9FF_48FC_AABB_5DB464ABF91D__INCLUDED_)
#define AFX_VSGVIEW_H__43F705D7_D9FF_48FC_AABB_5DB464ABF91D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VSGView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVSGView view

class CVSGView : public CView
{
protected:
	CVSGView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CVSGView)

// Attributes
public:

// Operations
public:

	void setStimPoint(double x, double y);
	void setFPPoint(double x, double y);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVSGView)
	public:
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CVSGView();
	void CPointToDegrees(CPoint point, double *px, double *py);
	void DegreesToCPoint(double x, double y, CPoint *ppoint);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CVSGView)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VSGVIEW_H__43F705D7_D9FF_48FC_AABB_5DB464ABF91D__INCLUDED_)
