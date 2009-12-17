// SrvrItem.h : interface of the CMsgSvrSrvrItem class
//

#if !defined(AFX_SRVRITEM_H__65EB43E2_41D0_49AF_B654_EE7B6B9C419F__INCLUDED_)
#define AFX_SRVRITEM_H__65EB43E2_41D0_49AF_B654_EE7B6B9C419F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMsgSvrSrvrItem : public COleServerItem
{
	DECLARE_DYNAMIC(CMsgSvrSrvrItem)

// Constructors
public:
	CMsgSvrSrvrItem(CMsgSvrDoc* pContainerDoc);

// Attributes
	CMsgSvrDoc* GetDocument() const
		{ return (CMsgSvrDoc*)COleServerItem::GetDocument(); }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMsgSvrSrvrItem)
	public:
	virtual BOOL OnDraw(CDC* pDC, CSize& rSize);
	virtual BOOL OnGetExtent(DVASPECT dwDrawAspect, CSize& rSize);
	//}}AFX_VIRTUAL

// Implementation
public:
	~CMsgSvrSrvrItem();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SRVRITEM_H__65EB43E2_41D0_49AF_B654_EE7B6B9C419F__INCLUDED_)
