#if !defined(AFX_ACTIVEXSVRPPG_H__7C59DFE7_1717_4E4B_94B4_0DE78545CEB9__INCLUDED_)
#define AFX_ACTIVEXSVRPPG_H__7C59DFE7_1717_4E4B_94B4_0DE78545CEB9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ActiveXSvrPpg.h : Declaration of the CActiveXSvrPropPage property page class.

////////////////////////////////////////////////////////////////////////////
// CActiveXSvrPropPage : See ActiveXSvrPpg.cpp.cpp for implementation.

class CActiveXSvrPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CActiveXSvrPropPage)
	DECLARE_OLECREATE_EX(CActiveXSvrPropPage)

// Constructor
public:
	CActiveXSvrPropPage();

// Dialog Data
	//{{AFX_DATA(CActiveXSvrPropPage)
	enum { IDD = IDD_PROPPAGE_ACTIVEXSVR };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
	//{{AFX_MSG(CActiveXSvrPropPage)
		// NOTE - ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ACTIVEXSVRPPG_H__7C59DFE7_1717_4E4B_94B4_0DE78545CEB9__INCLUDED)
