// VSGFrameCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "CoolerMouse.h"
#include "VSGFrameCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "VSGView.h"

/////////////////////////////////////////////////////////////////////////////
// CVSGFrameCtrl

IMPLEMENT_DYNCREATE(CVSGFrameCtrl, CFrameWnd)

#define VSGFRAMECTRL_CLASSNAME _T("VSGFrameCtrl")

CVSGFrameCtrl::CVSGFrameCtrl()
{

    WNDCLASS wndcls;
    HINSTANCE hInst = AfxGetInstanceHandle();

    if (!(::GetClassInfo(hInst, VSGFRAMECTRL_CLASSNAME, &wndcls)))
    {
	    // otherwise we need to register a new class
	    wndcls.style            = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	    wndcls.lpfnWndProc      = ::DefWindowProc;
	    wndcls.cbClsExtra       = wndcls.cbWndExtra = 0;
	    wndcls.hInstance        = hInst;
	    wndcls.hIcon            = NULL;
	    wndcls.hCursor          = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
	    wndcls.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
	    wndcls.lpszMenuName     = NULL;
	    wndcls.lpszClassName    = VSGFRAMECTRL_CLASSNAME;

	    if (!AfxRegisterClass(&wndcls))
	    {
		    AfxThrowResourceException();
	    }
    }

	m_bCreated = false;
}

BOOL CVSGFrameCtrl::Create(CWnd* pParentWnd, const RECT& rect, UINT nID, DWORD dwStyle)
{
    return CFrameWnd::Create(VSGFRAMECTRL_CLASSNAME, _T(""), dwStyle, rect, pParentWnd);
}

CVSGFrameCtrl::~CVSGFrameCtrl()
{
}


BEGIN_MESSAGE_MAP(CVSGFrameCtrl, CFrameWnd)
	//{{AFX_MSG_MAP(CVSGFrameCtrl)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVSGFrameCtrl message handlers

int CVSGFrameCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (!m_bCreated)
    {
	    if (CFrameWnd::OnCreate(lpCreateStruct) == -1) return -1;
    }

    m_bCreated = true;

    CCreateContext* pContext = new CCreateContext;
    pContext->m_pNewViewClass = RUNTIME_CLASS(CVSGView);
    pContext->m_pCurrentDoc = NULL;
    pContext->m_pNewDocTemplate = NULL;
    pContext->m_pLastView = NULL;
    pContext->m_pCurrentFrame = NULL;

    CreateView(pContext);

    delete pContext;
	return 0;
}

void CVSGFrameCtrl::PostNcDestroy() 
{
}
