// CoolerMouseDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CoolerMouse.h"
#include "CoolerMouseDlg.h"
#include "VSGHelper.h"
#include "alertlib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCoolerMouseDlg dialog

CCoolerMouseDlg::CCoolerMouseDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCoolerMouseDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCoolerMouseDlg)
	m_dStimX = 0.0;
	m_dStimY = 0.0;
	m_dStimWidth = 0.0;
	m_dStimHeight = 0.0;
	m_dStimOrientation = 0;
	m_dStimSF = 0.0;
	m_dStimTF = 0.0;
	m_dFixationX = 0.0;
	m_dFixationY = 0.0;
	m_dFixationDiameter = 0.0;
	m_iStimContrast = 0;
	m_nPattern = -1;
	m_nAperture = -1;
	m_nStimColorVector = -1;
	m_nFixationColor = -1;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCoolerMouseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCoolerMouseDlg)
	DDX_Text(pDX, IDC_X, m_dStimX);
	DDX_Text(pDX, IDC_Y, m_dStimY);
	DDX_Text(pDX, IDC_W, m_dStimWidth);
	DDX_Text(pDX, IDC_H, m_dStimHeight);
	DDX_Text(pDX, IDC_ORIENTATION, m_dStimOrientation);
	DDV_MinMaxLong(pDX, m_dStimOrientation, 0, 360);
	DDX_Text(pDX, IDC_SPATIAL, m_dStimSF);
	DDX_Text(pDX, IDC_TEMPORAL, m_dStimTF);
	DDX_Text(pDX, IDC_FIXATIONX, m_dFixationX);
	DDX_Text(pDX, IDC_FIXATIONY, m_dFixationY);
	DDX_Text(pDX, IDC_FIXATIONDIAMETER, m_dFixationDiameter);
	DDV_MinMaxDouble(pDX, m_dFixationDiameter, 1.e-002, 100.);
	DDX_Text(pDX, IDC_CONTRAST, m_iStimContrast);
	DDX_Radio(pDX, IDC_SINEWAVE, m_nPattern);
	DDX_Radio(pDX, IDC_CIRCLE, m_nAperture);
	DDX_CBIndex(pDX, IDC_COLORVECTOR, m_nStimColorVector);
	DDX_CBIndex(pDX, IDC_FIXATIONCOLOR, m_nFixationColor);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCoolerMouseDlg, CDialog)
	//{{AFX_MSG_MAP(CCoolerMouseDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	ON_EN_CHANGE(IDC_CONTRAST, OnChangeContrast)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCoolerMouseDlg message handlers

BOOL CCoolerMouseDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_cFrame.Create(this, CRect(25, 5, 440, 330), 0);
	m_cFrame.ShowWindow(SW_SHOW);


	// Fetch dialog values from VSG helper. 
	GetVSGParameters();


	return TRUE;  // return TRUE  unless you set the focus to a control
}



void CCoolerMouseDlg::GetVSGParameters()
{
//	VSGHelper* vsg = AfxGetApp()->getVSG();
	VSGHelper* vsg = theApp.getVSG();

	m_dStimX = vsg->stim().x;
	m_dStimY = vsg->stim().y;
	m_dStimWidth = vsg->stim().w;
	m_dStimHeight = vsg->stim().h;
	m_dStimOrientation = vsg->stim().orientation;
	m_dStimSF = vsg->stim().sf;
	m_dStimTF = vsg->stim().tf;
	m_iStimContrast = vsg->stim().contrast;

	if (vsg->stim().pattern == sinewave) m_nPattern = 0;
	else m_nPattern = 1;
	if (vsg->stim().aperture == ellipse) m_nAperture = 0;
	else m_nAperture = 1;

	switch (vsg->stim().cv)
	{
	case b_w: 
		m_nStimColorVector = 0;
		break;
	case l_cone: 
		m_nStimColorVector = 1;
		break;
	case m_cone: 
		m_nStimColorVector = 2;
		break;
	case s_cone: 
		m_nStimColorVector = 3;
		break;
	default: 
		m_nStimColorVector = 0;
		break;
	}

	m_dFixationX = vsg->fixpt().x;
	m_dFixationY = vsg->fixpt().y;
	m_dFixationDiameter = vsg->fixpt().d;
	switch (vsg->fixpt().color) 
	{
	case red:
		m_nFixationColor = 0;
		break;
	case green:
		m_nFixationColor = 1;
		break;
	case blue:
		m_nFixationColor = 2;
		break;
	default:
		m_nFixationColor = 0;
		break;
	}

	UpdateData(FALSE);

}

void CCoolerMouseDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCoolerMouseDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCoolerMouseDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CCoolerMouseDlg::OnApply() 
{
	// TODO: Add your control notification handler code here
	
}

void CCoolerMouseDlg::OnChangeContrast() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here

	update();
}



void CCoolerMouseDlg::update()
{
	VSGHelper* vsg = theApp.getVSG();

	vsg->stim().x = m_dStimX;
	vsg->stim().y = m_dStimY;
	vsg->stim().w = m_dStimWidth;
	vsg->stim().h = m_dStimHeight;
	vsg->stim().orientation = m_dStimOrientation;
	vsg->stim().sf = m_dStimSF;
	vsg->stim().tf = m_dStimTF;
	vsg->stim().contrast = m_iStimContrast;

	if (m_nPattern == 0) vsg->stim().pattern = sinewave;
	else vsg->stim().pattern = squarewave;

	if (m_nAperture == 0) vsg->stim().aperture = ellipse;
	else vsg->stim().aperture = rectangle;

	switch (m_nStimColorVector)
	{
	case 0:
		vsg->stim().cv = b_w;
		break;
	case 1:
		vsg->stim().cv = l_cone;
		break;
	case 2:
		vsg->stim().cv = m_cone;
		break;
	case 3:
		vsg->stim().cv = s_cone;
		break;
	default:
		vsg->stim().cv = b_w;
		break;
	}

	vsg->fixpt().x = m_dFixationX;
	vsg->fixpt().y = m_dFixationY;
	vsg->fixpt().d = m_dFixationDiameter;
	switch (m_nFixationColor)
	{
	case 0:
		vsg->fixpt().color = red;
		break;
	case 1:
		vsg->fixpt().color = green;
		break;
	case 2:
		vsg->fixpt().color = blue;
		break;
	default:
		vsg->fixpt().color = red;
		break;
	}

}
