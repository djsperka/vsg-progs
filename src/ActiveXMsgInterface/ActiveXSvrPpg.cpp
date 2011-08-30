// ActiveXSvrPpg.cpp : Implementation of the CActiveXSvrPropPage property page class.

#include "stdafx.h"
#include "ActiveXSvr.h"
#include "ActiveXSvrPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CActiveXSvrPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CActiveXSvrPropPage, COlePropertyPage)
	//{{AFX_MSG_MAP(CActiveXSvrPropPage)
	// NOTE - ClassWizard will add and remove message map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CActiveXSvrPropPage, "ACTIVEXSVR.ActiveXSvrPropPage.1",
	0xd02cbc8b, 0xa426, 0x43ad, 0xb4, 0xec, 0x94, 0xc9, 0x32, 0xdc, 0xde, 0x2e)


/////////////////////////////////////////////////////////////////////////////
// CActiveXSvrPropPage::CActiveXSvrPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CActiveXSvrPropPage

BOOL CActiveXSvrPropPage::CActiveXSvrPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_ACTIVEXSVR_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CActiveXSvrPropPage::CActiveXSvrPropPage - Constructor

CActiveXSvrPropPage::CActiveXSvrPropPage() :
	COlePropertyPage(IDD, IDS_ACTIVEXSVR_PPG_CAPTION)
{
	//{{AFX_DATA_INIT(CActiveXSvrPropPage)
	// NOTE: ClassWizard will add member initialization here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// CActiveXSvrPropPage::DoDataExchange - Moves data between page and properties

void CActiveXSvrPropPage::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CActiveXSvrPropPage)
	// NOTE: ClassWizard will add DDP, DDX, and DDV calls here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA_MAP
	DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// CActiveXSvrPropPage message handlers
