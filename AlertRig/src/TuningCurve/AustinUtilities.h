#pragma once
#include <afxwin.h>
#include <iostream.h>
#include "C:\Program Files\Cambridge Research Systems\VSGV6\Windows\Win32\Msc\INCLUDE\VSGEX2.H"

CString TrimLine(char input[100]);

CString GetParameterFileName(void);

int GetDistanceToScreen(void);

int InitializeVSG(void);

void SetupOverlay(double FixationX,double FixationY,double FixationDiameter, CString &FixationColor);
