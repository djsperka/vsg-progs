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
	theApp.getVSG()->update();

	// draw icon/position of stim and f.p.

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
	// save point for drawing
	m_next = point;

	// Convert 'point' to angular coords. 
	// dx and dy are the position of the cursor, as a fraction of the total view width/height, in the
	// view window. 

	CRect rect;
	GetClientRect(&rect);
	double dx = (double)point.x/(double)rect.Width();
	double dy = (double)point.y/(double)rect.Height();
	
	// Now Convert to degrees on the VSG	
	double x, y;
	x = (dx-0.5)*theApp.getVSG()->getVSGWidthDegrees();
	y = (dy-0.5)*theApp.getVSG()->getVSGHeightDegrees();
	theApp.getVSG()->setStimXY(x, y);

	Invalidate();

	CView::OnMouseMove(nFlags, point);
}

void CVSGView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
	// TODO: Add your specialized code here and/or call the base class
	std::ostringstream oss;
    CRect rect;
    GetClientRect (&rect); // rect.Width() and rect.Height() are the dimensions of the view area

	// Put origin in center of view
	pDC->SetViewportOrg(rect.Width()/2, rect.Height()/2);

	pDC->SetWindowExt(40, 30);

	CView::OnPrepareDC(pDC, pInfo);
}
