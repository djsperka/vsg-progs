// Machine generated IDispatch wrapper class(es) created with ClassWizard

#include "stdafx.h"
#include "msgsvr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// IMsgSvr properties

/////////////////////////////////////////////////////////////////////////////
// IMsgSvr operations

void IMsgSvr::addMessage(LPCTSTR strMessage)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x1, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 strMessage);
}

CString IMsgSvr::getNextMessage()
{
	CString result;
	InvokeHelper(0x2, DISPATCH_METHOD, VT_BSTR, (void*)&result, NULL);
	return result;
}
