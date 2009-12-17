// SendMessageDlg.h : header file
//

#if !defined(AFX_SENDMESSAGEDLG_H__61B57B67_DE5D_435A_9B2F_C1E201AD3534__INCLUDED_)
#define AFX_SENDMESSAGEDLG_H__61B57B67_DE5D_435A_9B2F_C1E201AD3534__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CSendMessageDlg dialog

class CSendMessageDlg : public CDialog
{
// Construction
public:
	CSendMessageDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSendMessageDlg)
	enum { IDD = IDD_SENDMESSAGE_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSendMessageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CSendMessageDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENDMESSAGEDLG_H__61B57B67_DE5D_435A_9B2F_C1E201AD3534__INCLUDED_)
