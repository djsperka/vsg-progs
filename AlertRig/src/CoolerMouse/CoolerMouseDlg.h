// CoolerMouseDlg.h : header file
//

#if !defined(AFX_COOLERMOUSEDLG_H__2C3E5B03_D0C2_48E9_B57D_96235416EF65__INCLUDED_)
#define AFX_COOLERMOUSEDLG_H__2C3E5B03_D0C2_48E9_B57D_96235416EF65__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VSGFrameCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CCoolerMouseDlg dialog

class CCoolerMouseDlg : public CDialog
{
// Construction
public:
	CCoolerMouseDlg(CWnd* pParent = NULL);	// standard constructor
	void GetVSGParameters();
	void update();
	void updateApertureProperties();
	void updateStimProperties();
	void setStimXY(double x, double y);	// new X,Y from view. Update dlg values and then update vsg. 
	void setFixationXY(double x, double y);	// new X,Y from view. Update dlg values and then update vsg. 
	COLOR_VECTOR_TYPE CCoolerMouseDlg::getDialogCV();
	APERTURE_TYPE getDialogApertureType();
	PATTERN_TYPE getDialogPatternType();
	COLOR_TYPE getDialogFixationColor();

	
	
	// Dialog Data
	//{{AFX_DATA(CCoolerMouseDlg)
	enum { IDD = IDD_COOLERMOUSE_DIALOG };
	double	m_dStimX;
	double	m_dStimY;
	long	m_dStimOrientation;
	double	m_dStimSF;
	double	m_dStimTF;
	double	m_dFixationX;
	double	m_dFixationY;
	double	m_dFixationDiameter;
	int		m_iStimContrast;
	int		m_nPattern;
	int		m_nAperture;
	int		m_nStimColorVector;
	int		m_nFixationColor;
	double	m_dStimDiameter;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCoolerMouseDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

	CVSGFrameCtrl m_cFrame;	


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CCoolerMouseDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnApply();
	afx_msg void OnKillfocusContrast();
	afx_msg void OnKillfocusW();
	afx_msg void OnKillfocusX();
	afx_msg void OnKillfocusY();
	afx_msg void OnKillfocusSpatial();
	afx_msg void OnKillfocusTemporal();
	afx_msg void OnKillfocusOrientation();
	afx_msg void OnSelchangeColorvector();
	afx_msg void OnSinewave();
	afx_msg void OnSquarewave();
	afx_msg void OnCircle();
	afx_msg void OnSquare();
	afx_msg void OnSelchangeFixationcolor();
	afx_msg void OnKillfocusFixationdiameter();
	afx_msg void OnKillfocusFixationx();
	afx_msg void OnKillfocusFixationy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COOLERMOUSEDLG_H__2C3E5B03_D0C2_48E9_B57D_96235416EF65__INCLUDED_)
