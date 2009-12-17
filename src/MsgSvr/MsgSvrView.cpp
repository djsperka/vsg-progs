// MsgSvrView.cpp : implementation of the CMsgSvrView class
//

#include "stdafx.h"
#include "MsgSvr.h"

#include "MsgSvrDoc.h"
#include "MsgSvrView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMsgSvrView

IMPLEMENT_DYNCREATE(CMsgSvrView, CView)

BEGIN_MESSAGE_MAP(CMsgSvrView, CView)
	//{{AFX_MSG_MAP(CMsgSvrView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	ON_COMMAND(ID_CANCEL_EDIT_SRVR, OnCancelEditSrvr)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMsgSvrView construction/destruction

CMsgSvrView::CMsgSvrView()
{
	// TODO: add construction code here

}

CMsgSvrView::~CMsgSvrView()
{
}

BOOL CMsgSvrView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMsgSvrView drawing

void CMsgSvrView::OnDraw(CDC* pDC)
{
	CMsgSvrDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// OLE Server support

// The following command handler provides the standard keyboard
//  user interface to cancel an in-place editing session.  Here,
//  the server (not the container) causes the deactivation.
void CMsgSvrView::OnCancelEditSrvr()
{
	GetDocument()->OnDeactivateUI(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CMsgSvrView diagnostics

#ifdef _DEBUG
void CMsgSvrView::AssertValid() const
{
	CView::AssertValid();
}

void CMsgSvrView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMsgSvrDoc* CMsgSvrView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMsgSvrDoc)));
	return (CMsgSvrDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMsgSvrView message handlers
