// ActiveXSvrCtl.cpp : Implementation of the CActiveXSvrCtrl ActiveX Control class.

#include "stdafx.h"
#include "msgsvr.h"
#include "ActiveXSvr.h"
#include "ActiveXSvrCtl.h"
#include "ActiveXSvrPpg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CActiveXSvrCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CActiveXSvrCtrl, COleControl)
	//{{AFX_MSG_MAP(CActiveXSvrCtrl)
	// NOTE - ClassWizard will add and remove message map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CActiveXSvrCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CActiveXSvrCtrl)
	DISP_FUNCTION(CActiveXSvrCtrl, "getNextMessage", getNextMessage, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CActiveXSvrCtrl, "addMessage", addMessage, VT_EMPTY, VTS_BSTR)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CActiveXSvrCtrl, COleControl)
	//{{AFX_EVENT_MAP(CActiveXSvrCtrl)
	// NOTE - ClassWizard will add and remove event map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CActiveXSvrCtrl, 1)
	PROPPAGEID(CActiveXSvrPropPage::guid)
END_PROPPAGEIDS(CActiveXSvrCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CActiveXSvrCtrl, "ucdavis.msg.handler.actxcontrol",
	0xef5a57c5, 0x18dd, 0x4406, 0xa2, 0x92, 0x6b, 0xbc, 0x40, 0x40, 0xaf, 0x38)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CActiveXSvrCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DActiveXSvr =
		{ 0xe5d637cf, 0xffce, 0x4efe, { 0x86, 0xc8, 0x7d, 0x8d, 0x8f, 0x9a, 0xde, 0x4e } };
const IID BASED_CODE IID_DActiveXSvrEvents =
		{ 0xb7ca4094, 0x55be, 0x4152, { 0xb5, 0xeb, 0xb6, 0x4, 0x55, 0xd, 0xe, 0x4e } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwActiveXSvrOleMisc =
	OLEMISC_INVISIBLEATRUNTIME |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CActiveXSvrCtrl, IDS_ACTIVEXSVR, _dwActiveXSvrOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CActiveXSvrCtrl::CActiveXSvrCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CActiveXSvrCtrl

BOOL CActiveXSvrCtrl::CActiveXSvrCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegApartmentThreading to 0.

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_ACTIVEXSVR,
			IDB_ACTIVEXSVR,
			afxRegApartmentThreading,
			_dwActiveXSvrOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CActiveXSvrCtrl::CActiveXSvrCtrl - Constructor

CActiveXSvrCtrl::CActiveXSvrCtrl()
{
	InitializeIIDs(&IID_DActiveXSvr, &IID_DActiveXSvrEvents);

	// TODO: Initialize your control's instance data here.

	if (!m_handler.CreateDispatch(_T("ucdavis.msg.handler")))
	{
		CString s="Cannot create ucdavis.msg.handler";
		AfxMessageBox(s);
		AfxThrowUserException();
	}		
}


/////////////////////////////////////////////////////////////////////////////
// CActiveXSvrCtrl::~CActiveXSvrCtrl - Destructor

CActiveXSvrCtrl::~CActiveXSvrCtrl()
{
	// TODO: Cleanup your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// CActiveXSvrCtrl::OnDraw - Drawing function

void CActiveXSvrCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	// TODO: Replace the following code with your own drawing code.
	pdc->FillRect(rcBounds, CBrush::FromHandle((HBRUSH)GetStockObject(WHITE_BRUSH)));
	pdc->Ellipse(rcBounds);
}


/////////////////////////////////////////////////////////////////////////////
// CActiveXSvrCtrl::DoPropExchange - Persistence support

void CActiveXSvrCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CActiveXSvrCtrl::GetControlFlags -
// Flags to customize MFC's implementation of ActiveX controls.
//
// For information on using these flags, please see MFC technical note
// #nnn, "Optimizing an ActiveX Control".
DWORD CActiveXSvrCtrl::GetControlFlags()
{
	DWORD dwFlags = COleControl::GetControlFlags();


	// The control can activate without creating a window.
	// TODO: when writing the control's message handlers, avoid using
	//		the m_hWnd member variable without first checking that its
	//		value is non-NULL.
	dwFlags |= windowlessActivate;
	return dwFlags;
}


/////////////////////////////////////////////////////////////////////////////
// CActiveXSvrCtrl::OnResetState - Reset control to default state

void CActiveXSvrCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CActiveXSvrCtrl message handlers

BSTR CActiveXSvrCtrl::getNextMessage() 
{
	CString strResult;
	strResult = m_handler.getNextMessage();
	return strResult.AllocSysString();
}

void CActiveXSvrCtrl::addMessage(LPCTSTR strMessage) 
{
	m_handler.addMessage(strMessage);
}
