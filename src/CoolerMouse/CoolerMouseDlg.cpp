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
	m_dStimDiameter = 0.0;
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
	DDX_Text(pDX, IDC_W, m_dStimDiameter);
	DDV_MinMaxDouble(pDX, m_dStimDiameter, 0.2, 50.);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCoolerMouseDlg, CDialog)
	//{{AFX_MSG_MAP(CCoolerMouseDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	ON_EN_KILLFOCUS(IDC_CONTRAST, OnKillfocusContrast)
	ON_EN_KILLFOCUS(IDC_W, OnKillfocusW)
	ON_EN_KILLFOCUS(IDC_X, OnKillfocusX)
	ON_EN_KILLFOCUS(IDC_Y, OnKillfocusY)
	ON_EN_KILLFOCUS(IDC_SPATIAL, OnKillfocusSpatial)
	ON_EN_KILLFOCUS(IDC_TEMPORAL, OnKillfocusTemporal)
	ON_EN_KILLFOCUS(IDC_ORIENTATION, OnKillfocusOrientation)
	ON_CBN_SELCHANGE(IDC_COLORVECTOR, OnSelchangeColorvector)
	ON_BN_CLICKED(IDC_SINEWAVE, OnSinewave)
	ON_BN_CLICKED(IDC_SQUAREWAVE, OnSquarewave)
	ON_BN_CLICKED(IDC_CIRCLE, OnCircle)
	ON_BN_CLICKED(IDC_SQUARE, OnSquare)
	ON_CBN_SELCHANGE(IDC_FIXATIONCOLOR, OnSelchangeFixationcolor)
	ON_EN_KILLFOCUS(IDC_FIXATIONDIAMETER, OnKillfocusFixationdiameter)
	ON_EN_KILLFOCUS(IDC_FIXATIONX, OnKillfocusFixationx)
	ON_EN_KILLFOCUS(IDC_FIXATIONY, OnKillfocusFixationy)
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


	// Fetch dialog values from VSG helper. 
	GetVSGParameters();

	m_cFrame.Create(this, CRect(25, 5, 440, 330), 0);
	m_cFrame.ShowWindow(SW_SHOW);


	return TRUE;  // return TRUE  unless you set the focus to a control
}



void CCoolerMouseDlg::GetVSGParameters()
{
//	VSGHelper* vsg = AfxGetApp()->getVSG();
	VSGHelper* vsg = theApp.getVSG();

	m_dStimX = vsg->stim().x;
	m_dStimY = vsg->stim().y;
	m_dStimDiameter = vsg->stim().w;
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
	theApp.SaveRegStimulus(theApp.getVSG()->getStimString());	
	theApp.SaveRegFixpt();
}


#if 0
// Called when something in the aperture properties has changed - x, y, diam, aperture type
void CCoolerMouseDlg::updateApertureProperties()
{
	VSGHelper* vsg = theApp.getVSG();
	vsg->setApertureProperties(m_dStimX, m_dStimY, m_dStimDiameter, getDialogApertureType());
}


void CCoolerMouseDlg::updateStimProperties()
{
	VSGHelper* vsg = theApp.getVSG();
	vsg->setStimProperties(m_dStimSF, m_dStimTF, m_iStimContrast, m_dStimOrientation, getDialogPatternType(), getDialogCV());
}
#endif

void CCoolerMouseDlg::update()
{
	VSGHelper* vsg = theApp.getVSG();

	vsg->stim().x = m_dStimX;
	vsg->stim().y = m_dStimY;
	vsg->stim().w = m_dStimDiameter;
	vsg->stim().h = m_dStimDiameter;
	vsg->stim().orientation = m_dStimOrientation;
	vsg->stim().sf = m_dStimSF;
	vsg->stim().tf = m_dStimTF;
	vsg->stim().contrast = m_iStimContrast;

	vsg->stim().pattern = getDialogPatternType();
	vsg->stim().aperture = getDialogApertureType();

	vsg->stim().cv = getDialogCV();

	vsg->fixpt().x = m_dFixationX;
	vsg->fixpt().y = m_dFixationY;
	vsg->fixpt().d = m_dFixationDiameter;
	vsg->fixpt().color = getDialogFixationColor();

}


COLOR_VECTOR_TYPE CCoolerMouseDlg::getDialogCV()
{
	COLOR_VECTOR_TYPE cv;
	switch (m_nStimColorVector)
	{
	case 0:
		cv = b_w;
		break;
	case 1:
		cv = l_cone;
		break;
	case 2:
		cv = m_cone;
		break;
	case 3:
		cv = s_cone;
		break;
	default:
		cv = b_w;
		break;
	}
	
	return cv;
}

APERTURE_TYPE CCoolerMouseDlg::getDialogApertureType()
{
	APERTURE_TYPE a;
	if (m_nAperture == 0) a = ellipse;
	else a = rectangle;
	return a;
}

PATTERN_TYPE CCoolerMouseDlg::getDialogPatternType()
{
	PATTERN_TYPE p;
	if (m_nPattern == 0) p = sinewave;
	else p = squarewave;
	return p;
}

COLOR_TYPE CCoolerMouseDlg::getDialogFixationColor()
{
	COLOR_TYPE c;		

	switch (m_nFixationColor)
	{
	case 0:
		c = red;
		break;
	case 1:
		c = green;
		break;
	case 2:
		c = blue;
		break;
	default:
		c = red;
		break;
	}
	return c;
}


void CCoolerMouseDlg::OnKillfocusContrast() 
{
	// TODO: Add your control notification handler code here
//	updateStimProperties();		
	VSGHelper* vsg = theApp.getVSG();
	UpdateData(TRUE);
	vsg->setStimContrast(m_iStimContrast);
}

void CCoolerMouseDlg::OnKillfocusW() 
{
	// TODO: Add your control notification handler code here
	VSGHelper* vsg = theApp.getVSG();
	UpdateData(TRUE);
	vsg->setStimSize(m_dStimDiameter);
}

void CCoolerMouseDlg::setStimXY(double x, double y)
{
	m_dStimX = x;
	m_dStimY = y;
	UpdateData(FALSE);
	theApp.getVSG()->setStimXY(x, y);
}

void CCoolerMouseDlg::setFixationXY(double x, double y)
{
	m_dFixationX = x;
	m_dFixationY = y;
	UpdateData(FALSE);
	theApp.getVSG()->setFixationPointXY(x, y);
}


void CCoolerMouseDlg::OnKillfocusX() 
{
	// TODO: Add your control notification handler code here
	VSGHelper* vsg = theApp.getVSG();
	UpdateData(TRUE);
	vsg->setStimXY(m_dStimX, m_dStimY);	
}

void CCoolerMouseDlg::OnKillfocusY() 
{
	// TODO: Add your control notification handler code here
	VSGHelper* vsg = theApp.getVSG();
	UpdateData(TRUE);
	vsg->setStimXY(m_dStimX, m_dStimY);		
}

void CCoolerMouseDlg::OnKillfocusSpatial() 
{
	// TODO: Add your control notification handler code here
	VSGHelper* vsg = theApp.getVSG();
	UpdateData(TRUE);
	vsg->setStimSF(m_dStimSF);		
}

void CCoolerMouseDlg::OnKillfocusTemporal() 
{
	// TODO: Add your control notification handler code here
	VSGHelper* vsg = theApp.getVSG();
	UpdateData(TRUE);
	vsg->setStimTF(m_dStimTF);	
	
}

void CCoolerMouseDlg::OnKillfocusOrientation() 
{
	// TODO: Add your control notification handler code here
	VSGHelper* vsg = theApp.getVSG();
	UpdateData(TRUE);
	vsg->setStimOrientation(m_dStimOrientation);	

}

void CCoolerMouseDlg::OnSelchangeColorvector() 
{
	// TODO: Add your control notification handler code here
	VSGHelper* vsg = theApp.getVSG();
	UpdateData(TRUE);

	// The "value" from the combo box is an integer. the vsg() func below
	// knows the values. If the contents of the combo box changes, then the vsg
	// helper needs to change also. 
	vsg->setStimColorVector(m_nStimColorVector);		
}


void CCoolerMouseDlg::OnSinewave() 
{
	// TODO: Add your control notification handler code here
	VSGHelper* vsg = theApp.getVSG();
	UpdateData(TRUE);
	vsg->setStimPattern(m_nPattern);
}

void CCoolerMouseDlg::OnSquarewave() 
{
	// TODO: Add your control notification handler code here
	VSGHelper* vsg = theApp.getVSG();
	UpdateData(TRUE);
	vsg->setStimPattern(m_nPattern);	
}

void CCoolerMouseDlg::OnCircle() 
{
	// TODO: Add your control notification handler code here
	VSGHelper* vsg = theApp.getVSG();
	UpdateData(TRUE);
	vsg->setStimAperture(m_nAperture);		
}

void CCoolerMouseDlg::OnSquare() 
{
	// TODO: Add your control notification handler code here
	VSGHelper* vsg = theApp.getVSG();
	UpdateData(TRUE);
	vsg->setStimAperture(m_nAperture);	
}

void CCoolerMouseDlg::OnSelchangeFixationcolor() 
{
	// TODO: Add your control notification handler code here
	VSGHelper* vsg = theApp.getVSG();
	UpdateData(TRUE);

	vsg->setFixationColor(m_nFixationColor);			
}

void CCoolerMouseDlg::OnKillfocusFixationdiameter() 
{
	// TODO: Add your control notification handler code here
	VSGHelper* vsg = theApp.getVSG();
	UpdateData(TRUE);

	vsg->setFixationDiameter(m_dFixationDiameter);			
}

void CCoolerMouseDlg::OnKillfocusFixationx() 
{
	// TODO: Add your control notification handler code here
	VSGHelper* vsg = theApp.getVSG();
	UpdateData(TRUE);
	vsg->setFixationPointXY(m_dFixationX, m_dFixationY);	
}

void CCoolerMouseDlg::OnKillfocusFixationy() 
{
	// TODO: Add your control notification handler code here
	VSGHelper* vsg = theApp.getVSG();
	UpdateData(TRUE);
	vsg->setFixationPointXY(m_dFixationX, m_dFixationY);	
}
