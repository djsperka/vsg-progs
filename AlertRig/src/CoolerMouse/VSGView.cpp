// VSGView.cpp : implementation file
//

#include "stdafx.h"
#include "CoolerMouse.h"
#include "VSGView.h"
#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVSGView

IMPLEMENT_DYNCREATE(CVSGView, CView)

CVSGView::CVSGView()
{
}

CVSGView::~CVSGView()
{
}


BEGIN_MESSAGE_MAP(CVSGView, CView)
	//{{AFX_MSG_MAP(CVSGView)
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVSGView drawing

void CVSGView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CVSGView diagnostics

#ifdef _DEBUG
void CVSGView::AssertValid() const
{
	CView::AssertValid();
}

void CVSGView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CVSGView message handlers

void CVSGView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	theApp.getVSG()->stim().x = point.x;
	theApp.getVSG()->stim().y = point.y;
	theApp.getDlg()->GetVSGParameters();

	CView::OnMouseMove(nFlags, point);
}

void CVSGView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
	// TODO: Add your specialized code here and/or call the base class
	std::ostringstream oss;
    CRect rect;
    GetClientRect (&rect); // rect.Width() and rect.Height() are the dimensions of the view area

	oss << rect.Width() << "x" << rect.Height();
	::MessageBox(NULL, oss.str().c_str(), "hello", MB_OK | MB_ICONHAND);
	CView::OnPrepareDC(pDC, pInfo);
}
