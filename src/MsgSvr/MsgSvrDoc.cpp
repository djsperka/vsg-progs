// MsgSvrDoc.cpp : implementation of the CMsgSvrDoc class
//

#include "stdafx.h"
#include "MsgSvr.h"

#include "MsgSvrDoc.h"
#include "SrvrItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMsgSvrDoc

IMPLEMENT_DYNCREATE(CMsgSvrDoc, COleServerDoc)

BEGIN_MESSAGE_MAP(CMsgSvrDoc, COleServerDoc)
	//{{AFX_MSG_MAP(CMsgSvrDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CMsgSvrDoc, COleServerDoc)
	//{{AFX_DISPATCH_MAP(CMsgSvrDoc)
	DISP_FUNCTION(CMsgSvrDoc, "addMessage", addMessage, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(CMsgSvrDoc, "getNextMessage", getNextMessage, VT_BSTR, VTS_NONE)
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CMsgSvrDoc, "clearMessages", dispidclearMessages, clearMessages, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()

// Note: we add support for IID_IMsgSvr to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {CF8D4371-359D-424B-A37C-FC7E20D404E1}
static const IID IID_IMsgSvr =
{ 0xcf8d4371, 0x359d, 0x424b, { 0xa3, 0x7c, 0xfc, 0x7e, 0x20, 0xd4, 0x4, 0xe1 } };

BEGIN_INTERFACE_MAP(CMsgSvrDoc, COleServerDoc)
	INTERFACE_PART(CMsgSvrDoc, IID_IMsgSvr, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMsgSvrDoc construction/destruction

CMsgSvrDoc::CMsgSvrDoc()
{
	// Use OLE compound files
//	EnableCompoundFile();

	// TODO: add one-time construction code here

	EnableAutomation();

	AfxOleLockApp();
}

CMsgSvrDoc::~CMsgSvrDoc()
{
	AfxOleUnlockApp();
}

BOOL CMsgSvrDoc::OnNewDocument()
{
	if (!COleServerDoc::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMsgSvrDoc server implementation

COleServerItem* CMsgSvrDoc::OnGetEmbeddedItem()
{
	// OnGetEmbeddedItem is called by the framework to get the COleServerItem
	//  that is associated with the document.  It is only called when necessary.

	CMsgSvrSrvrItem* pItem = new CMsgSvrSrvrItem(this);
	ASSERT_VALID(pItem);
	return pItem;
}



/////////////////////////////////////////////////////////////////////////////
// CMsgSvrDoc serialization

void CMsgSvrDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMsgSvrDoc diagnostics

#ifdef _DEBUG
void CMsgSvrDoc::AssertValid() const
{
	COleServerDoc::AssertValid();
}

void CMsgSvrDoc::Dump(CDumpContext& dc) const
{
	COleServerDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMsgSvrDoc commands

void CMsgSvrDoc::addMessage(LPCTSTR strMessage) 
{
	((CMsgSvrApp *)AfxGetApp())->m_listMessages.AddTail(strMessage);
}

BSTR CMsgSvrDoc::getNextMessage() 
{
	CString nextString;
	if ( ((CMsgSvrApp *)AfxGetApp())->m_listMessages.IsEmpty() )
		return NULL;
	else {
		nextString = ((CMsgSvrApp *)AfxGetApp())->m_listMessages.GetHead();
		((CMsgSvrApp *)AfxGetApp())->m_listMessages.RemoveHead();
		return nextString.AllocSysString();
	}
}


void CMsgSvrDoc::clearMessages(void)
{
	AFX_MANAGE_STATE(AfxGetAppModuleState());

	((CMsgSvrApp *)AfxGetApp())->m_listMessages.RemoveAll();
}
