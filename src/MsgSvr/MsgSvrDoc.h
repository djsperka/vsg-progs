// MsgSvrDoc.h : interface of the CMsgSvrDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MSGSVRDOC_H__9215143B_AF11_47F9_8A14_F05606BAEDC2__INCLUDED_)
#define AFX_MSGSVRDOC_H__9215143B_AF11_47F9_8A14_F05606BAEDC2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CMsgSvrSrvrItem;

class CMsgSvrDoc : public COleServerDoc
{
protected: // create from serialization only
	CMsgSvrDoc();
	DECLARE_DYNCREATE(CMsgSvrDoc)

// Attributes
public:
	CMsgSvrSrvrItem* GetEmbeddedItem()
		{ return (CMsgSvrSrvrItem*)COleServerDoc::GetEmbeddedItem(); }

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMsgSvrDoc)
	protected:
	virtual COleServerItem* OnGetEmbeddedItem();
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMsgSvrDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CMsgSvrDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CMsgSvrDoc)
	afx_msg void addMessage(LPCTSTR strMessage);
	afx_msg BSTR getNextMessage();
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MSGSVRDOC_H__9215143B_AF11_47F9_8A14_F05606BAEDC2__INCLUDED_)
