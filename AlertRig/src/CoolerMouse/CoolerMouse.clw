; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CCoolerMouseDlg
LastTemplate=CView
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "CoolerMouse.h"

ClassCount=5
Class1=CCoolerMouseApp
Class2=CCoolerMouseDlg
Class3=CAboutDlg

ResourceCount=3
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Class4=CVSGFrameCtrl
Class5=CVSGView
Resource3=IDD_COOLERMOUSE_DIALOG

[CLS:CCoolerMouseApp]
Type=0
HeaderFile=CoolerMouse.h
ImplementationFile=CoolerMouse.cpp
Filter=N
LastObject=CCoolerMouseApp

[CLS:CCoolerMouseDlg]
Type=0
HeaderFile=CoolerMouseDlg.h
ImplementationFile=CoolerMouseDlg.cpp
Filter=D
LastObject=IDC_CONTRAST
BaseClass=CDialog
VirtualFilter=dWC

[CLS:CAboutDlg]
Type=0
HeaderFile=CoolerMouseDlg.h
ImplementationFile=CoolerMouseDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_COOLERMOUSE_DIALOG]
Type=1
Class=CCoolerMouseDlg
ControlCount=34
Control1=IDOK,button,1342242817
Control2=IDC_APPLY,button,1342242816
Control3=IDC_STATIC,button,1342177287
Control4=IDC_STATIC,static,1342308352
Control5=IDC_X,edit,1350631552
Control6=IDC_STATIC,static,1342308352
Control7=IDC_Y,edit,1350631552
Control8=IDC_STATIC,static,1342308352
Control9=IDC_W,edit,1350631552
Control10=IDC_STATIC,static,1342308352
Control11=IDC_CONTRAST,edit,1350631552
Control12=IDC_STATIC,static,1342308352
Control13=IDC_SPATIAL,edit,1350631552
Control14=IDC_STATIC,static,1342308352
Control15=IDC_TEMPORAL,edit,1350631552
Control16=IDC_STATIC,static,1342308352
Control17=IDC_ORIENTATION,edit,1350631552
Control18=IDC_COLORVECTOR,combobox,1344340227
Control19=IDC_STATIC,static,1342308352
Control20=IDC_SINEWAVE,button,1342308361
Control21=IDC_SQUAREWAVE,button,1342177289
Control22=IDC_STATIC,button,1342177287
Control23=IDC_CIRCLE,button,1342308361
Control24=IDC_SQUARE,button,1342177289
Control25=IDC_STATIC,button,1342177287
Control26=IDC_STATIC,static,1342308352
Control27=IDC_STATIC,static,1342308352
Control28=IDC_STATIC,static,1342308352
Control29=IDC_STATIC,static,1342308352
Control30=IDC_FIXATIONX,edit,1350631552
Control31=IDC_FIXATIONY,edit,1350631552
Control32=IDC_FIXATIONDIAMETER,edit,1350631552
Control33=IDC_FIXATIONCOLOR,combobox,1344340227
Control34=IDC_STATIC,button,1342177287

[CLS:CVSGFrameCtrl]
Type=0
HeaderFile=VSGFrameCtrl.h
ImplementationFile=VSGFrameCtrl.cpp
BaseClass=CFrameWnd
Filter=T
LastObject=CVSGFrameCtrl
VirtualFilter=fWC

[CLS:CVSGView]
Type=0
HeaderFile=VSGView.h
ImplementationFile=VSGView.cpp
BaseClass=CView
Filter=C
LastObject=CVSGView
VirtualFilter=VWC

