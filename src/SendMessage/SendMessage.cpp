// SendMessage.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "SendMessage.h"
#include "msgsvr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSendMessageApp

BEGIN_MESSAGE_MAP(CSendMessageApp, CWinApp)
	//{{AFX_MSG_MAP(CSendMessageApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSendMessageApp construction

CSendMessageApp::CSendMessageApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSendMessageApp object

CSendMessageApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CSendMessageApp initialization

BOOL CSendMessageApp::InitInstance()
{
	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox("Could not initialize OLE libraries.");
		return FALSE;
	}


	IMsgSvr m_handler;
	if (!m_handler.CreateDispatch(_T("ucdavis.msg.handler")))
	{
		AfxMessageBox("Unable to attach to message server.");
		AfxThrowUserException();
	}

	CString strTemp = CString(m_lpCmdLine);
	m_handler.addMessage(strTemp);

	return TRUE;

}
