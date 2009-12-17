// Machine generated IDispatch wrapper class(es) created with ClassWizard
/////////////////////////////////////////////////////////////////////////////
// IMsgSvr wrapper class

class IMsgSvr : public COleDispatchDriver
{
public:
	IMsgSvr() {}		// Calls COleDispatchDriver default constructor
	IMsgSvr(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	IMsgSvr(const IMsgSvr& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	void addMessage(LPCTSTR strMessage);
	CString getNextMessage();
};
