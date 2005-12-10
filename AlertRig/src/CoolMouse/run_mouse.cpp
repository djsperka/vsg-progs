//****************************************************************
//
//
//		Mouse Control Program
//		This program displays a drifting grating
//		whose position is determined by the position
//		of the mouse pointer.
//		
//		The grating parameters can be changed by
//		typing (o) orientaion, (s) spatial frequency
//		(t) temporal frequency, (a) aperture size.
//
//		Michael Sceniak 4/30/02
//
//*****************************************************************



#include "stdafx.h"

#include "vsgv8.h"
#include <conio.h>
#include "alertlib.h"
#include "reghelper.h"
#include <iostream>
#include <sstream>
using namespace alert;
using namespace std;

#pragma comment (lib, "vsgv8.lib")

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

double ScrWidth, ScrHeight;

double vsgWidthPixels;
double vsgHeightPixels;
double vsgWidthDegrees;
double vsgHeightDegrees;
double monWidthPixels;
double monHeightPixels;

int UpdateGrating(int objHandle, double Orientation, double SpatialFrequency, double DriftVelocity);
void UpdateAperture(float Aperture);
void UpdateDriftVelocity(double DriftVelocity);
void UpdateContrast(double Contrast);	
void UpdateBackground(double BackgroundLuminance);
void mousePosToVSGDrawDegrees(int pixMouseX, int pixMouseY, double* pvsgDegX, double* pvsgDegY);
void vsgDrawDegreesToMousePos(double vsgDegX, double vsgDegY, long* pixMouseX, long* pixMouseY);
void UpdateOverlay(bool bFixationOn, double fixX, double fixY, double fixD, double apertureX, double apertureY, double apertureDiameter);
void InitializeOverlay(double B, VSGTRIVAL fixation_color);


int InitializeCard(int dist)
{
	int CheckCard;
	CheckCard = vsgInit("");
	if (CheckCard < 0) 
	{ 
		cout << "VSG card failed to initialize" << endl;
		return(-1);
	}
	vsgSetVideoMode(vsgPANSCROLLMODE);
	vsgSetSpatialUnits(vsgDEGREEUNIT);
	vsgSetViewDistMM(dist);
	return(1);
}

void InitializeOverlay(double b, VSGTRIVAL color)
{
	VSGLUTBUFFER  Buffer;

	vsgSetCommand(vsgOVERLAYMASKMODE);

	// initialize lut buffer for overlay
	Buffer[1].a = b;
	Buffer[1].b = b;
	Buffer[1].c = b;

	Buffer[2].a = 0;
	Buffer[2].b = 0;
	Buffer[2].c = 0;

	Buffer[3].a = color.a;
	Buffer[3].b = color.b;
	Buffer[3].c = color.c;

	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&Buffer, 0, 4);

	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1);   // <--------------- Last value gives the overlay page color from Buffer
	vsgSetDrawPage(vsgOVERLAYPAGE,1,1);
	vsgSetDrawPage(vsgOVERLAYPAGE,2,1);
	vsgSetDrawPage(vsgOVERLAYPAGE,3,1);
}

void run_mouse()
{	
	int	i=0;
	int key;
	float ApertureDiameter=1;
	int objHandle=0;
	double DriftVelocity=4, SpatialFrequency=3, Orientation=45, Contrast=99;
	double CurrentDriftVelocity=4, PreviousDriftVelocity=4;
	double BackgroundLuminance = 0.5;
	COLOR_TYPE FixationColor;
	VSGTRIVAL FixationColorTrival;
	int Page=0;										// current page. Use for page flipping
	double FixationX,FixationY,FixationDiameter;	// fixation point parameters
	int DistanceToScreen;
	bool bUseManualTriggers = false;
	int iStepSize = 1;

	// check registry args

	if (!GetRegScreenDistance(DistanceToScreen))
	{
		_exit(0);
	}

	if (!GetRegFixpt(FixationX, FixationY, FixationDiameter, FixationColor))
	{
		_exit(0);
	}
	else
	{
		get_color(FixationColor, FixationColorTrival);
	}

	ARApertureGratingSpec stim;
	if (!GetRegStimulus(stim))
	{
		_exit(0);
	}
	else
	{
		DriftVelocity = stim.tf;
		Contrast = stim.contrast;
		SpatialFrequency = stim.sf;
		ApertureDiameter = stim.w;
		Orientation = stim.orientation;
	}


	cout << " ____________________________________ " << endl;
	cout << "|                                    |" << endl;
	cout << "| Mouse Control stimulation program. |" << endl;
	cout << "|    Type <h> for help.              |" << endl;
	cout << "|____________________________________|" << endl;


	if (InitializeCard(DistanceToScreen) == -1) _exit(0);

	// Initilize globals for cnversions
	vsgWidthPixels = vsgGetScreenWidthPixels();
	vsgHeightPixels = vsgGetScreenHeightPixels();
	monWidthPixels = GetSystemMetrics(SM_CXSCREEN);
	monHeightPixels = GetSystemMetrics(SM_CYSCREEN);
	vsgUnit2Unit(vsgPIXELUNIT, vsgWidthPixels, vsgDEGREEUNIT, &vsgWidthDegrees);
	vsgUnit2Unit(vsgPIXELUNIT, vsgHeightPixels, vsgDEGREEUNIT, &vsgHeightDegrees);


	// draw grating and init overlay
	objHandle = UpdateGrating(objHandle, Orientation, SpatialFrequency, DriftVelocity);
	InitializeOverlay(BackgroundLuminance, FixationColorTrival);





	// Issue "ready" triggers to spike2.
	// These commands pulse spike2 port 6. 
	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x20, 0);
	vsgPresent();

	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x00, 0);
	vsgPresent();




	
	double degMouseX, degMouseY;
	POINT Pos;
	bool bFixationOn=false;
	bool bSendTrigger=false;
	long lDigitalIO=0;
	long lDigitalIOLast=0;
	bool bMouseOn = true;
	while(i < 1)
	{

		// get cursor position, convert to degrees
		if (bMouseOn)
		{
			GetCursorPos(&Pos);
		}
		if (Pos.x > monWidthPixels) Pos.x = (long)monWidthPixels;
		mousePosToVSGDrawDegrees(Pos.x, Pos.y, &degMouseX, &degMouseY);

		// read vsg io for fixation pt signal
		if (!bUseManualTriggers)
		{
			lDigitalIO = vsgIOReadDigitalIn() & vsgDIG1;
			bFixationOn =  lDigitalIO != 0;
			bSendTrigger = lDigitalIO!=lDigitalIOLast;
			lDigitalIOLast = lDigitalIO;
		}


		// flip overlay page
		Page=1-Page;
		vsgSetDrawPage(vsgOVERLAYPAGE,Page,1);

		// draw overlay page, fixation point if required, and aperture
		UpdateOverlay(bFixationOn, FixationX, FixationY, FixationDiameter, degMouseX, degMouseY, ApertureDiameter);

		// put fresh overlay page up
		if (!bSendTrigger)
		{
			vsgSetZoneDisplayPage(vsgOVERLAYPAGE, Page);
		}
		else
		{
			if (bFixationOn)
			{
				vsgIOWriteDigitalOut(0x2, 0x2);
			}
			else 
			{
				vsgIOWriteDigitalOut(0x0, 0x2);
			}
			vsgSetZoneDisplayPage(vsgOVERLAYPAGE, Page + vsgTRIGGERPAGE);
		}

		// check for keyboard hit
		while(_kbhit())
		{

			key = _getch();
			
			switch(key) 
			{
			case 'a':
			{  
				// get aperture size. It will be updated next time through.
				float a = -1;
				cout << "Aperture Diameter (degrees) = ";
				cin >> a;
				if ( a>0 )
				{
					ApertureDiameter = a;
				}
				else 
				{
					cerr << "Error in input: Aperture diam must be a number>0." << endl;
				}
				break;
			}
			case 'b':
			{
				double b = -1;
				cout << "Background = ";
				cin >> b;
				if (b>=0 && b<=1)
				{
					BackgroundLuminance = b;
					InitializeOverlay(BackgroundLuminance, FixationColorTrival);
				}
				else 
				{
					cout << "Bad luminance value (0<=luminance<=1)" << endl;
				}
				break;
			}
			case 'h':
			{
				cout << "Help Menu" << endl << endl;
				cout << "<a> aperture size" << endl;
				cout << "<o> orientation" << endl;
				cout << "<s> spatial frequency" << endl;
				cout << "<t> temporal frequency" << endl;
				cout << "<c> contrast [0-100]" << endl;
				cout << "<b> background luminance [0-1]" << endl;
				cout << "<p> position information" << endl;
				cout << "<v> current spatiotemporal values" << endl;
				cout << "<space bar> toggles between drifting and static" << endl;
				cout << "<M> use manual triggers for fixation point (<1>/<0>)" << endl;
				cout << "<f> toggle mouse movement on/off" << endl;
				cout << "<S> change step size when mouse movement off" << endl;
				cout << "<Enter> save current stim parameters in registry" << endl;
				cout << "<esc> end program" << endl;
				cout << endl << endl;
				break;
			}
			case 's':
			{
				double s;
				cout << "Spatial Frequency = ";
				cin >> s;
				if (s>0.1 && s<100)
				{
					SpatialFrequency = s;
					objHandle = UpdateGrating(objHandle, Orientation, SpatialFrequency, DriftVelocity);
				}
				else
				{
					cout << "Error in input: spatial freq must be a number between 0.1 and 100." << endl;
				}
				break;
			}
			case 'v':
			{
				cout << "Orientation = " << Orientation << ", Spatial Frequency = " << SpatialFrequency << ", Temporal Frequency = " << DriftVelocity << endl;
				break;
			}
			case 't':
			{
				double d = -1;
				cout << "Temporal Frequency = ";
				cin >> d;
				if (d>=0 && d<50)
				{
					DriftVelocity = d;
					UpdateDriftVelocity(DriftVelocity);
					CurrentDriftVelocity = DriftVelocity;
					PreviousDriftVelocity = DriftVelocity;
				}
				else 
				{
					cout << "Error in input: Temporal freq must be between 0 and 50." << endl;
				}
				break;
			}
			case 'M':
			{
				if (bUseManualTriggers)
				{
					cout << "Manual triggers OFF" << endl;
					bUseManualTriggers = false;
				}
				else
				{
					cout << "Manual triggers ON" << endl;
					bUseManualTriggers = true;
				}
				break;
			}
			case 'f':
			{
				if (bMouseOn)
				{
					cout << "Mouse movement OFF" << endl;
					bMouseOn = false;
				}
				else
				{
					cout << "Mouse movement ON" << endl;
					bMouseOn = true;
				}
				break;
			}
			case '8':
			{
				if (!bMouseOn)
				{
					Pos.y -= iStepSize;
				}
				else
				{
					cout << "Mouse movement is ON - turn off with 'f'" << endl;
				}
				break;
			}
			case '2':
			{
				if (!bMouseOn)
				{
					Pos.y += iStepSize;
				}
				else
				{
					cout << "Mouse movement is ON - turn off with 'f'" << endl;
				}
				break;
			}
			case '4':
			{
				if (!bMouseOn)
				{
					Pos.x -= iStepSize;
				}
				else
				{
					cout << "Mouse movement is ON - turn off with 'f'" << endl;
				}
				break;
			}
			case '6':
			{
				if (!bMouseOn)
				{
					Pos.x += iStepSize;
				}
				else
				{
					cout << "Mouse movement is ON - turn off with 'f'" << endl;
				}
				break;
			}
			case '5':
			{
				if (!bMouseOn)
				{
					ARApertureGratingSpec stim;
					if (!GetRegStimulus(stim))
					{
						cout << "Error getting stimulus spec from registry!" << endl;
					}
					else
					{
						DriftVelocity = stim.tf;
						Contrast = stim.contrast;
						SpatialFrequency = stim.sf;
						ApertureDiameter = stim.w;
						Orientation = stim.orientation;
						vsgDrawDegreesToMousePos(stim.x, stim.y, &Pos.x, &Pos.y);
					}
				}
				else
				{
					cout << "Mouse movement is ON - turn off with 'f'" << endl;
				}
				break;
			}
			case '1':
			{
				if (bUseManualTriggers)
				{
					cout << "Fixation point ON" << endl;
					bFixationOn = true;
				}
				else
				{
					cout << "Manual triggers are OFF: use <M> first" << endl;
				}
				break;
			}
			case '0':
			{
				if (bUseManualTriggers)
				{
					cout << "Fixation point OFF" << endl;
					bFixationOn = false;
				}
				else
				{
					cout << "Manual triggers are OFF: use <M> first" << endl;
				}
				break;
			}
			case 'o':
			{
				double oo = 999;
				cout <<"Orientation = ";
				cin >> oo;
				if (oo >=0 && oo <=360)
				{
					Orientation = oo;
					objHandle = UpdateGrating(objHandle, Orientation, SpatialFrequency, DriftVelocity);
				}
				else
				{
					cout << "Error in input: Orientation must be a number between 0 and 360" << endl;
				}
				break;
			}
			case ' ':
			{
				if(CurrentDriftVelocity!=0)
				{
					PreviousDriftVelocity = DriftVelocity;
					UpdateDriftVelocity(0);
					CurrentDriftVelocity = 0;
				}
				else
				{
					UpdateDriftVelocity(PreviousDriftVelocity);
					CurrentDriftVelocity = PreviousDriftVelocity;
				}
				break;
			}
			case 'c':
			{
				double c;
				cout << "Contrast = ";
				cin >> c;
				if (c>=0 && c<=100)
				{
					Contrast = c;
					UpdateContrast(Contrast);
				}
				else
				{
					cout << "Error in input: Contrast must be between 0 and 100." << endl;
				}
				break;
			}
			case 'p':
			{
				cout << "pixels:X position = " << Pos.x << ", Y position = " << Pos.y << endl;
				cout << "degrees:X position = " << degMouseX << ", Y position = " << -degMouseY << endl;
				break;
			}
			case 13:
			{
				alert::ARApertureGratingSpec stim;
				stim.aperture = ellipse;
				stim.contrast = Contrast;
				stim.cv = b_w;
				stim.h = stim.w = ApertureDiameter;
				stim.orientation = Orientation;
				stim.pattern = sinewave;
				stim.sf = SpatialFrequency;
				stim.tf = CurrentDriftVelocity;
				stim.x = degMouseX;
				stim.y = -degMouseY;
				std::ostringstream oss;
				oss << stim;
				cout << "reg string=" <<  oss.str() << endl;
				SaveRegStimulus(oss.str());
				break;
			}
			case 'S':
			{
				int i;
				cout << "Step size (pixels) = ";
				cin >> i;
				if (i > 0)
				{
					iStepSize = i;
				}
				else
				{
					cout << "Error in input: step size must be positive" << endl;
				}
				break;
			}
			case 27:
			{
				vsgSetZoneDisplayPage(vsgOVERLAYPAGE, 2);
				vsgPresent();
				exit(0);
			}
			default:
			{
				cout << "Unknown key '" << key << "'" << endl;
				break;
			}
			};	// end case

		}
	}
}


void UpdateOverlay(bool bFixationOn, double fixX, double fixY, double fixD, double apertureX, double apertureY, double apertureDiameter)
{
	// draw aperture (color 0 is clear)
	vsgSetPen1(0);
	vsgDrawOval(apertureX, apertureY, apertureDiameter, apertureDiameter);

	if (bFixationOn) 
	{
		vsgSetPen1(3);
		vsgDrawOval(fixX, fixY, fixD, fixD);
	}
}


void mousePosToVSGDrawDegrees(int pixMouseX, int pixMouseY, double* pvsgDegX, double* pvsgDegY)
{
	vsgUnit2Unit(vsgPIXELUNIT, pixMouseX * vsgWidthPixels/monWidthPixels - vsgWidthPixels/2, vsgDEGREEUNIT, pvsgDegX);
	vsgUnit2Unit(vsgPIXELUNIT, pixMouseY * vsgHeightPixels/monHeightPixels - vsgHeightPixels/2, vsgDEGREEUNIT, pvsgDegY);
}

void vsgDrawDegreesToMousePos(double vsgDegX, double vsgDegY, long* pixMouseX, long* pixMouseY)
{
	double x, y;
	vsgUnit2Unit(vsgDEGREEUNIT, vsgDegX, vsgPIXELUNIT, &x);
	vsgUnit2Unit(vsgDEGREEUNIT, vsgDegY, vsgPIXELUNIT, &y);
	*pixMouseX = (x + vsgWidthPixels/2) * monWidthPixels/vsgWidthPixels;
	*pixMouseY = (y + vsgHeightPixels/2) * monHeightPixels/vsgHeightPixels;
}


void UpdateAperture(float Aperture)
{
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1);
	vsgSetPen1(0);	
	vsgDrawOval(0, 0, Aperture, Aperture);
}

void UpdateBackground(double BackgroundLuminance)
{
	VSGLUTBUFFER Buffer;

	Buffer[1].a = BackgroundLuminance;
	Buffer[1].b = BackgroundLuminance;
	Buffer[1].c = BackgroundLuminance;

	Buffer[2].a = 0;
	Buffer[2].b = 0;
	Buffer[2].c = 0;

	Buffer[3].a = 1;
	Buffer[3].b = 1;
	Buffer[3].c = 1;

	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&Buffer, 0, 4);
	vsgSetDrawPage(vsgOVERLAYPAGE,2,1);
	vsgSetDrawPage(vsgOVERLAYPAGE,3,1);
}



int UpdateGrating(int objHandle, double Orientation, double SpatialFrequency, double DriftVelocity)
{


	VSGTRIVAL From, Too, Back;
	From.a = 0.0;
	From.b = 0.0;
	From.c = 0.0;

	Too.a = 1.0;
	Too.b = 1.0;
	Too.c = 1.0;

	Back.a = (From.a+Too.a)/2;
	Back.b = (From.b+Too.b)/2;
	Back.c = (From.c+Too.c)/2;

	vsgObjDestroy(objHandle);
	vsgSetCommand(vsgPALETTECLEAR);

	vsgSetDrawPage(vsgVIDEOPAGE,0,vsgBACKGROUND);
	objHandle = vsgObjCreate();

	vsgSetBackgroundColour((VSGTRIVAL*)&Back);
	vsgObjSetDefaults();
	vsgObjSetPixelLevels(1, 120);
	vsgObjTableSinWave(vsgSWTABLE);
	vsgObjSetColourVector((VSGTRIVAL*)&From, (VSGTRIVAL*)&Too, vsgBIPOLAR);
	
	vsgSetPen1(1);
	vsgSetPen2(120);
	vsgObjSetDriftVelocity(DriftVelocity);
	vsgDrawGrating(0, 0, vsgWidthDegrees, vsgHeightDegrees, Orientation, SpatialFrequency);
	vsgPresent();
	return(objHandle);

}


void UpdateDriftVelocity(double DriftVelocity)
{
	vsgObjSetDriftVelocity(DriftVelocity);
	vsgPresent();
}


void UpdateContrast(double Contrast)
{
	vsgObjSetContrast(Contrast);
	vsgPresent();
}
