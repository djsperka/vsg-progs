#pragma once
#include <afxwin.h>
#include <iostream>
#include "vsgv8.h"

CString TrimLine(char input[100]);

CString GetParameterFileName(void);

int GetDistanceToScreen(void);

int InitializeVSG(void);

void SetupOverlay(double FixationX,double FixationY,double FixationDiameter, CString &FixationColor);
