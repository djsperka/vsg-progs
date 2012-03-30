// Machine generated IDispatch wrapper class(es) created with Add Class from Typelib Wizard

#import "D:\\work\\AlertRig\\src\\MsgSvr\\Release\\MsgSvr.tlb" no_namespace
// MyMsgSvr wrapper class

class MyMsgSvr : public COleDispatchDriver
{
public:
	MyMsgSvr(){} // Calls COleDispatchDriver default constructor
	MyMsgSvr(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	MyMsgSvr(const MyMsgSvr& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

	// Attributes
public:

	// Operations
public:


	// IMsgSvr methods
public:
	void addMessage(LPCTSTR strMessage)
	{
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x1, DISPATCH_METHOD, VT_EMPTY, NULL, parms, strMessage);
	}
	CString getNextMessage()
	{
		CString result;
		InvokeHelper(0x2, DISPATCH_METHOD, VT_BSTR, (void*)&result, NULL);
		return result;
	}
	void clearMessages()
	{
		InvokeHelper(0x3, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}

	// IMsgSvr properties
public:

};
