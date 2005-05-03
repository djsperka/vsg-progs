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
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVSGView drawing

void CVSGView::OnDraw(CDC* pDC)
{
	double x1, y1, x2, y2;
	CPoint ul, lr;
	VSGHelper *pvsg = theApp.getVSG();
	CPen pen1, pen2;
	CBrush brush1, brush2;
	CPen *pOldPen;
	CBrush *pOldBrush;

	// get bounding box for stim
	x1 = pvsg->stim().x - pvsg->stim().w/2; 
	x2 = pvsg->stim().x + pvsg->stim().w/2; 
	y1 = pvsg->stim().y + pvsg->stim().h/2; 
	y2 = pvsg->stim().y - pvsg->stim().h/2; 

	// now convert to pixels
	DegreesToCPoint(x1, y1, &ul);
	DegreesToCPoint(x2, y2, &lr);

	// draw stim. If stim is active, draw circle no fill. If not active, fill circle.
	if( !pen1.CreatePen(PS_SOLID, 1, RGB(0,0,0))) 
		return;
	pOldPen = pDC->SelectObject(&pen1);

	if (pvsg->stimFixed())
	{
		if (!brush1.CreateSolidBrush(RGB(127, 127, 127)))
		{
			return;
		}
		pOldBrush = pDC->SelectObject(&brush1);
	}
	pDC->Ellipse(ul.x, ul.y, lr.x, lr.y);
	pDC->SelectObject( pOldPen );
	if (pvsg->stimFixed())
	{
		pDC->SelectObject(pOldBrush);
	}

	// get bounding box for fixpt
	x1 = pvsg->fixpt().x - pvsg->fixpt().d/2; 
	x2 = pvsg->fixpt().x + pvsg->fixpt().d/2; 
	y1 = pvsg->fixpt().y + pvsg->fixpt().d/2; 
	y2 = pvsg->fixpt().y - pvsg->fixpt().d/2; 

	// now convert to pixels
	DegreesToCPoint(x1, y1, &ul);
	DegreesToCPoint(x2, y2, &lr);

	// draw stim. If stim is active, draw circle no fill. If not active, fill circle.
	if( !pen2.CreatePen(PS_SOLID, 1, RGB(255,0,0))) 
		return;
	pOldPen = pDC->SelectObject(&pen2);
	if (pvsg->fixptFixed())
	{
		if (!brush2.CreateSolidBrush(RGB(255, 0, 0)))
		{
			return;
		}
		pOldBrush = pDC->SelectObject(&brush2);
	}
	pDC->Ellipse(ul.x, ul.y, lr.x, lr.y);
	pDC->SelectObject( pOldPen );
	if (pvsg->fixptFixed())
	{
		pDC->SelectObject(pOldBrush);
	}
	
	return;

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



void CVSGView::CPointToDegrees(CPoint point, double *px, double *py)
{
	// Convert 'point' to angular coords. 
	// dx and dy are the position of the cursor, as a fraction of the total view width/height, in the
	// view window. 

	CRect rect;
	GetClientRect(&rect);
	double dx = (double)point.x/(double)rect.Width();
	double dy = (double)point.y/(double)rect.Height();
	
	// Now Convert to degrees on the VSG	
	*px = (dx-0.5)*theApp.getVSG()->getVSGWidthDegrees();
	*py = -1*(dy-0.5)*theApp.getVSG()->getVSGHeightDegrees();

}

void CVSGView::DegreesToCPoint(double x, double y, CPoint *point)
{
	// Convert 'x' and 'y', which are in angular coords, to window coords. 
	// dx and dy are the position of the cursor, as a fraction of the total view width/height, in the
	// view window. 

	CRect rect;
	GetClientRect(&rect);

	double dx = x/theApp.getVSG()->getVSGWidthDegrees()+0.5;
	double dy = -1*y/theApp.getVSG()->getVSGHeightDegrees()+0.5;

	point->x = dx*(double)rect.Width();
	point->y = dy*(double)rect.Height();

}


/////////////////////////////////////////////////////////////////////////////
// CVSGView message handlers

void CVSGView::OnMouseMove(UINT nFlags, CPoint point) 
{
	double x, y;
	VSGHelper *pvsg = theApp.getVSG();
	CCoolerMouseDlg *pdlg = theApp.getDlg();

	// Is there an active object?
	if (!pvsg->stimFixed())
	{
		// Convert 'point' to angular coords. 
		CPointToDegrees(point, &x, &y);
		pdlg->setStimXY(x, y);
		Invalidate();
	}
	else if (!pvsg->fixptFixed())
	{
		// Convert 'point' to angular coords. 
		CPointToDegrees(point, &x, &y);
		pdlg->setFixationXY(x, y);
		Invalidate();
	}

//	CView::OnMouseMove(nFlags, point);
}

void CVSGView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
	// TODO: Add your specialized code here and/or call the base class
    CRect rect;
    GetClientRect (&rect); // rect.Width() and rect.Height() are the dimensions of the view area

	// Put origin in center of view
//	pDC->SetViewportOrg(rect.Width()/2, rect.Height()/2);

	pDC->SetWindowExt(40, 30);

//	CView::OnPrepareDC(pDC, pInfo);
}

void CVSGView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	double x, y;
	VSGHelper *pvsg = theApp.getVSG();

	// TODO: Add your message handler code here and/or call default

	// Is stim active?
	if (!pvsg->stimFixed())
	{
		pvsg->stimFixed(true);
		CPointToDegrees(point, &x, &y);
		theApp.getDlg()->setStimXY(x, y);

		Invalidate();
	}
	else if (!pvsg->fixptFixed())
	{
		pvsg->fixptFixed(true);
		CPointToDegrees(point, &x, &y);
		theApp.getDlg()->setFixationXY(x, y);

		Invalidate();
	}
	else
	{
		double dsqSTIM, dsqFP;
		double xstim = pvsg->stim().x;
		double ystim = pvsg->stim().y;
		double xfixpt = pvsg->fixpt().x;
		double yfixpt = pvsg->fixpt().y;
		double xclick, yclick;

		CPointToDegrees(point, &xclick, &yclick);

		// There isn't an active object, so the click means we activate the NEAREST object. 
		// Simply measure the distance from 'point' to each of m_cpStim and m_cpFP, shortest
		// distance wins. 


		dsqSTIM = (xclick - xstim)*(xclick - xstim) + (yclick - ystim)*(yclick - ystim);
		dsqFP   = (xclick - xfixpt)*(xclick - xfixpt) + (yclick - yfixpt)*(yclick - yfixpt);

		if (dsqSTIM <= dsqFP) 
		{
			pvsg->stimFixed(false);
			pvsg->fixptFixed(true);
		}
		else
		{
			pvsg->fixptFixed(false);
			pvsg->stimFixed(true);
		}
	}

	CView::OnLButtonDown(nFlags, point);
}

