#include <afxwin.h>
#include <iostream>
//#include <stdio>
#include "C:\Program Files\Cambridge Research Systems\VSGV6\Windows\Win32\Msc\INCLUDE\VSGEX2.H"
#include <math.h>

using namespace std;

float Distractors[5][7];

int ReadDistractorFile();

void main(int ArgumentCount, char *Arguments[])
{
	//Parse the command line
	double X, Y, FixationPointDiameter;
	double BaseContrast, ContrastUp, ContrastDown,IncorrectBrightness;
	double TargetX, TargetY;
	CString BackgroundColor,GratingOrSpot,GratingColor;
	double TargetOrientation, TargetSF, TargetTF, TargetDiameter;
	double AnswerPointInitialBrightness, AnswerPointFullBrightness;
	int NumberOfDistractors,PixelRange;

	cout << "Starting..." << std::endl;

	NumberOfDistractors=ReadDistractorFile();
	PixelRange=79;

	X=atof(Arguments[1]);
	Y=atof(Arguments[2]);
	BackgroundColor=Arguments[3];
	FixationPointDiameter=atof(Arguments[4]);
	BaseContrast=atof(Arguments[5]);
	ContrastUp=atof(Arguments[6]);
	ContrastDown=atof(Arguments[7]);
	IncorrectBrightness=atof(Arguments[8]);
	GratingOrSpot=Arguments[9];
	TargetX=atof(Arguments[10]);
	TargetY=atof(Arguments[11]);
	TargetOrientation=atof(Arguments[12]);
	TargetSF=atof(Arguments[13]);
	TargetTF=atof(Arguments[14]);
	TargetDiameter=atof(Arguments[15]);
	if(ArgumentCount>16)
	{
		AnswerPointInitialBrightness=atof(Arguments[16]);
		AnswerPointFullBrightness=atof(Arguments[17]);
		GratingColor=Arguments[18];
	}
	else
	{
		AnswerPointInitialBrightness=100;
		AnswerPointFullBrightness=0;
		GratingColor="Grayscale";
	}

	//Initialize the vsg card then check that it initialized O.K.

	cout << "Init card..." << std::endl;

	int CheckCard;
	CheckCard = vsgInit("");
	if (CheckCard < 0)
	{  
		printf("VSG card failed to initialize\n");
		_exit(0);
	}
	vsgSetViewDistMM(630); //MAKE THIS VARIABLE!!
	vsgSetSpatialUnits(vsgDEGREEUNIT);
	vsgSetVideoMode(vsgPANSCROLLMODE);
	Sleep(2000);
	
	//Load the lut for our drawing pages

	VSGTRIVAL Red, Black, White,Incorrect,Initial,Gray,BackgroundC,From,To;
	VSGLUTBUFFER Buffer;
	Red.a=1; Red.b=0; Red.c=0;
	Black.a=0; Black.b=0; Black.c=0;
	White.a=1; White.b=1; White.c=1;
	Gray.a=.5; Gray.b=.5; Gray.c=.5;

	VSGTRIVAL LOn,LOff,MOn,MOff,SOn,SOff;
	LOn.a=1; LOn.b=.42074;	LOn.c=.5;
	LOff.a=0; LOff.b=.57926;	LOff.c=.5;
	MOn.a=0; MOn.b=.72407; MOn.c=.4863;
	MOff.a=1; MOff.b=.27593; MOff.c=.5137;
	SOn.a=.61644; SOn.b=.36301; SOn.c=1;
	SOff.a=.38356; SOff.b=.63699; SOff.c=0;


	if(strcmp(BackgroundColor,"Black")==0) BackgroundC=Black;
	if(strcmp(BackgroundColor,"Gray")==0) BackgroundC=Gray;
	if(strcmp(BackgroundColor,"White")==0) BackgroundC=White;

	if(strcmp(GratingColor,"Grayscale")==0) {From=Black; To=White;}
	if(strcmp(GratingColor,"S-cone")==0) {From=SOff; To=SOn;}
	if(strcmp(GratingColor,"M-cone")==0) {From=MOff; To=MOn;}
	if(strcmp(GratingColor,"L-cone")==0) {From=LOff; To=LOn;}

	Initial.a=.5*(1+AnswerPointInitialBrightness/100);
	Initial.b=.5*(1-AnswerPointInitialBrightness/100);
	Initial.c=.5*(1-AnswerPointInitialBrightness/100);

	
	Incorrect.a=.5*(1+IncorrectBrightness/100);
	Incorrect.b=.5*(1-IncorrectBrightness/100);
	Incorrect.c=.5*(1-IncorrectBrightness/100);

	int index,Background=0;
	//Configure the LUT initially to have 5 "special" colors up front, and 
	//generic luminance ramp thereafter.
	for(index=0; index<256; index++)
	{
		if(index==0) Buffer[index]=BackgroundC;
		if(index==1) Buffer[index]=Red;
		if(index==2) Buffer[index]=White;
		if(index==3) Buffer[index]=Initial;
		if(index==4) Buffer[index]=Incorrect;

		if(index > 4)
		{
			Buffer[index].a=(index-5)/250.0;
			Buffer[index].b=(index-5)/250.0;
			Buffer[index].c=(index-5)/250.0;
		}
	}
	vsgLUTBUFFERWrite(0,&Buffer);
	vsgLUTBUFFERtoPalette(0);

	//Setup the overlay page so our square gratings appear as circles
	//We do this by setting a solid background overlay page, and drawing
	//transparent circles around the location of our fixation point, answer points,
	//and gratings

	cout << "Set up overlay..." << std::endl;

	VSGLUTBUFFER OverlayBuffer;
	vsgSetCommand(vsgOVERLAYMASKMODE);

	OverlayBuffer[0].a=.5; OverlayBuffer[0].b=.5; OverlayBuffer[0].c=.5;
	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&OverlayBuffer[0], 1, 1);
	vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1);
	vsgSetPen1(0);
	vsgDrawOval(X,Y,FixationPointDiameter,FixationPointDiameter);
	vsgDrawOval(X,Y+5,.5,.5);
	vsgDrawOval(X,Y-5,.5,.5);
	vsgDrawOval(TargetX,TargetY,TargetDiameter,TargetDiameter);
	int OvIndex;
	if (NumberOfDistractors > 0)
	{
		for(OvIndex=0; OvIndex<NumberOfDistractors; OvIndex++)
		{
			vsgDrawOval(Distractors[OvIndex][5],Distractors[OvIndex][6],Distractors[OvIndex][3],Distractors[OvIndex][3]);
		}
	}


	//Set up our video drawing pages. Page 0 is solid background, and page 1 contains our drawing objects

	cout << "Set up video pages..." << std::endl;

	vsgSetDrawPage(vsgVIDEOPAGE,0,0);
	vsgSetDrawPage(vsgVIDEOPAGE,1,0);

	//Define object handles. Grating is our target grating, the one the monkey should attend to.
	//Confounder and Distractor are gratings (read from the distractor file) that the monkey
	//should be ignoring. Because we are limited to 255 pixel levels, we define a separate handle
	//for confounder so that we can change the contrast for one of the distractors, while the 
	//other distractors share the same pixel levels.
	//To avoid confusion, all object handle variables are prefixed with an 'h'
	VSGOBJHANDLE hGrating, hConfounder, hDistractors;
	VSGOBJHANDLE hFixationPoint, hAnswerPointTop,hAnswerPointBottom;

	//Define the fixation point object
	hFixationPoint=vsgObjCreate();
	vsgSetDrawPage(vsgVIDEOPAGE,1,vsgNOCLEAR);
	vsgObjSetPixelLevels(5,2);
	vsgObjSetColourVector((VSGTRIVAL*)&BackgroundC,(VSGTRIVAL*)&Red,vsgUNIPOLAR);
	vsgObjSetContrast(0);
	vsgDrawOval(X,Y,FixationPointDiameter,FixationPointDiameter);
	vsgPresent();

	//Define the AnswerPoint objects
	hAnswerPointTop=vsgObjCreate();
	vsgSetDrawPage(vsgVIDEOPAGE,1,vsgNOCLEAR);
	vsgObjSetPixelLevels(7,2);
	vsgObjSetContrast(0);
	vsgObjSetColourVector((VSGTRIVAL*)&BackgroundC,(VSGTRIVAL*)&Red,vsgUNIPOLAR);
	vsgDrawOval(X,Y-5,.5,.5);
	vsgPresent();

	hAnswerPointBottom=vsgObjCreate();
	vsgSetDrawPage(vsgVIDEOPAGE,1,vsgNOCLEAR);
	vsgObjSetPixelLevels(9,2);
	vsgObjSetContrast(0);
	vsgObjSetColourVector((VSGTRIVAL*)&BackgroundC,(VSGTRIVAL*)&Red,vsgUNIPOLAR);
	vsgDrawOval(X,Y+5,.5,.5);
	vsgPresent();


	// Define the grating the monkey will attend to.
	hGrating=vsgObjCreate();
	vsgSetDrawPage(vsgVIDEOPAGE,1,vsgNOCLEAR);
	vsgObjTableSinWave(vsgSWTABLE);
	vsgObjSetDriftVelocity(TargetTF);
	vsgObjSetPixelLevels(11,PixelRange);
	vsgObjSetColourVector((VSGTRIVAL*)&From,(VSGTRIVAL*)&To,vsgBIPOLAR);
	vsgObjSetContrast(0);
	if (strcmp(GratingOrSpot,"Spot")==0)
		vsgDrawOval(TargetX,TargetY,TargetDiameter,TargetDiameter);
	else
		vsgDrawGrating(TargetX,TargetY,TargetDiameter,TargetDiameter,TargetOrientation,TargetSF);

	vsgPresent();

	//Define the confounder grating, for catch trials
	hConfounder=vsgObjCreate();
	vsgSetDrawPage(vsgVIDEOPAGE,1,vsgNOCLEAR);
	vsgObjTableSinWave(vsgSWTABLE);
	vsgObjSetDriftVelocity(Distractors[0][2]);
	vsgObjSetColourVector((VSGTRIVAL*)&From,(VSGTRIVAL*)&To,vsgBIPOLAR);
	vsgObjSetPixelLevels(91,PixelRange);
	vsgObjSetContrast(0);
	if (NumberOfDistractors>0)
	{

		if (strcmp(GratingOrSpot,"Spot")==0)
			vsgDrawOval(Distractors[0][5],Distractors[0][6],Distractors[0][3],Distractors[0][3]);
		else
			vsgDrawGrating(Distractors[0][5],Distractors[0][6],Distractors[0][3],Distractors[0][3],
							Distractors[0][0],Distractors[0][1]);
	}
	vsgPresent();
	
	//Define a handle for the other distractors.
	int dindex;
	hDistractors=vsgObjCreate();
	vsgSetDrawPage(vsgVIDEOPAGE,1,vsgNOCLEAR);
	vsgObjTableSinWave(vsgSWTABLE);
	vsgObjSetDriftVelocity(Distractors[1][2]); //All distractors must have same drift velocity
	vsgObjSetColourVector((VSGTRIVAL*)&From,(VSGTRIVAL*)&To,vsgBIPOLAR);
	vsgObjSetPixelLevels(171,PixelRange);
	vsgObjSetContrast(0);

	if (NumberOfDistractors > 1) 
	{
		for(dindex=1; dindex<NumberOfDistractors; dindex++)
		{
			if (strcmp(GratingOrSpot,"Spot")==0)
				vsgDrawOval(Distractors[dindex][5],Distractors[dindex][6],Distractors[dindex][3],Distractors[dindex][3]);
			else
				vsgDrawGrating(Distractors[dindex][5],Distractors[dindex][6],Distractors[dindex][3],Distractors[dindex][3],
								Distractors[dindex][0],Distractors[dindex][1]);
		}
	}
	vsgPresent();

	long StimulusState, NonJuicePins, PreviousState;
	int Fixation=0,TargetContrastChange=0,ConfounderContrastChange=0,ContrastChangeValue=0,Stim=0,AnswerPoint=0;
	int PreviousFixation=0,PreviousTargetContrastChange=0,PreviousConfounderContrastChange=0,PreviousContrastState=0,PreviousStim=0,PreviousAnswerPoint=0;
	//Determine the initial state of the VSG digital input pins.
	StimulusState=vsgIOReadDigitalIn();
	//Digital input 0 is used by the juicer, so we ignore it
	NonJuicePins=floor(StimulusState/2);
	PreviousState=NonJuicePins;

	cout << "Starting loop, NJP=" << std::hex << NonJuicePins << std::endl;

	DWORD TriggerState=0; //We use this to keep track of our output trigger states.
	while (TRUE)
	{
		cout << "Read vsg digin" << std::endl;
		StimulusState=vsgIOReadDigitalIn();
		cout << "Done reading vsg digin" << std::endl;
		NonJuicePins=floor(StimulusState/2);

		cout << "NJP=" << std::hex << NonJuicePins << std::endl;
		
		if(NonJuicePins != PreviousState)
		{
			Fixation=((StimulusState&vsgDIG1)/vsgDIG1);
			Stim=((StimulusState&vsgDIG2)/vsgDIG2);
			TargetContrastChange=((StimulusState&vsgDIG3)/vsgDIG3);
			ConfounderContrastChange=((StimulusState&vsgDIG4)/vsgDIG4);
			ContrastChangeValue=((StimulusState&vsgDIG5)/vsgDIG5);
			AnswerPoint=((StimulusState&vsgDIG6)/vsgDIG6);
			cout << " fix=" << Fixation << " stim=" << Stim << " tgt_contrast=" << TargetContrastChange << " cfd_contrast=" << ConfounderContrastChange << " ans=" << AnswerPoint << std::endl;

			//If the screen is supposed to be completely blank, we clear the screen by switching video pages.
			//This avoids the potential problem of items disappearing one at a time, rather than simultaneously.
			if (Fixation + Stim + AnswerPoint==0) //Special case for when the screen is supposed to be completely clear.
			{
				vsgSetDrawPage(vsgVIDEOPAGE,0,vsgNOCLEAR); //set the drawing page to the blank video page.
				TriggerState=0;
				vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER+vsgTRIG_TOGGLEMODE,TriggerState,0);
				vsgPresent(); //vsgPresent automatically switches the display page to the current drawing page.
				//Reset all the object contrasts to 0
				vsgObjSelect(hFixationPoint);
				vsgObjSetContrast(0);

				vsgObjSelect(hAnswerPointTop);
				vsgObjSetContrast(0);
				vsgObjSelect(hAnswerPointBottom);
				vsgObjSetContrast(0);

				
				vsgObjSelect(hGrating);
				vsgObjSetContrast(0);

				vsgObjSelect(hConfounder);
				vsgObjSetContrast(0);

				vsgObjSelect(hDistractors);
				vsgObjSetContrast(0);
				vsgPresent();

				PreviousState=NonJuicePins;
				PreviousFixation=0;
				PreviousStim=0;
				PreviousAnswerPoint=0;
				PreviousTargetContrastChange=0;
				PreviousConfounderContrastChange=0;
				continue;
			}

			//Look for a change in the state of the Fixation flag. If Fixation = 1,
			//set the contrast of the fixation point to 100%. If Fixation = 0, set
			//the contrast of the fixation point to 0%/
			if (Fixation != PreviousFixation)
			{	
				vsgObjSelect(hFixationPoint);
				vsgSetDrawPage(vsgVIDEOPAGE,1,vsgNOCLEAR);
				vsgObjSetContrast(100*Fixation);
				TriggerState=TriggerState+(2*Fixation-1)*vsgDIG0;
				vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER + vsgTRIG_TOGGLEMODE,TriggerState,0);
				// DJS DEBUG
				cout << "TriggerState=" << std::hex << TriggerState << std::endl;
				vsgPresent();
				PreviousFixation=Fixation;
			}

			//Look for a change in the state of the Stim flag. If Stim = 1,
			//set the contrast of the target grating, confounder grating, and distractor gratings
			//to 100%. If Fixation = 0, set the contrast of those objects to 0%/
			if (Stim != PreviousStim)
			{	
				vsgObjSelect(hGrating);
				vsgSetDrawPage(vsgVIDEOPAGE,1,vsgNOCLEAR);
				vsgObjSetContrast(BaseContrast*Stim);
				vsgObjSelect(hConfounder);
				vsgObjSetContrast(BaseContrast*Stim);
				vsgObjSelect(hDistractors);
				vsgObjSetContrast(BaseContrast*Stim);
				TriggerState=TriggerState+(2*Stim-1)*vsgDIG1;
				vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER + vsgTRIG_TOGGLEMODE,TriggerState,0);
				vsgPresent();
				// DJS DEBUG
				cout << "TriggerState=" << std::hex << TriggerState << std::endl;
				PreviousStim=Stim;
			}

			//Look for a change in the state of the Fixation flag. If Fixation = 1,
			//set the contrast of the fixation point to 100%. If Fixation = 0, set
			//the contrast of the fixation point to 0%/
			if (AnswerPoint != PreviousAnswerPoint)
			{	
				vsgObjSelect(hAnswerPointTop);
				vsgSetDrawPage(vsgVIDEOPAGE,1,vsgNOCLEAR);
				if (ContrastChangeValue==1)
					vsgObjSetColourVector((VSGTRIVAL*)&BackgroundC,(VSGTRIVAL*)&Red,vsgUNIPOLAR);
				else
					vsgObjSetColourVector((VSGTRIVAL*)&BackgroundC,(VSGTRIVAL*)&Incorrect,vsgUNIPOLAR);
				vsgObjSetContrast(100*AnswerPoint);

				vsgObjSelect(hAnswerPointBottom);
				vsgSetDrawPage(vsgVIDEOPAGE,1,vsgNOCLEAR);
				if (ContrastChangeValue==1)
					vsgObjSetColourVector((VSGTRIVAL*)&BackgroundC,(VSGTRIVAL*)&Incorrect,vsgUNIPOLAR);
				else
					vsgObjSetColourVector((VSGTRIVAL*)&BackgroundC,(VSGTRIVAL*)&Red,vsgUNIPOLAR);
				vsgObjSetContrast(100*AnswerPoint);

				TriggerState=TriggerState+(2*AnswerPoint-1)*vsgDIG2;
				vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER + vsgTRIG_TOGGLEMODE,TriggerState,0);
				vsgPresent();
				// DJS DEBUG
				cout << "TriggerState=" << std::hex << TriggerState << std::endl;
				PreviousAnswerPoint=AnswerPoint;
			}

			//Look for a change in the state of the TargetContrastChange flag. If it has
			//changed to 1, use the ContrastChangeValue flag to determine how to change the target grating.
			//If it is 1, set the contrast of the target grating to ContrastUp. If it is 0, set
			//the contrast of the target grating to ContrastDown.
			if ((TargetContrastChange != PreviousTargetContrastChange) & (TargetContrastChange==1))
			{	
				vsgObjSelect(hGrating);
				vsgSetDrawPage(vsgVIDEOPAGE,1,vsgNOCLEAR);
				switch (ContrastChangeValue)
				{
				case 1:
					{
						cout << "Contrast Up " <<ContrastUp << endl;
						vsgObjSetContrast(ContrastUp);
					}
					break;
				case 0:
					{
						cout << "Contrast Down " <<ContrastDown << endl;
						vsgObjSetContrast(ContrastDown);
					}
					break;
				}
				TriggerState=TriggerState+vsgDIG3;
				vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER + vsgTRIG_TOGGLEMODE,TriggerState,0);
				vsgPresent();
				// DJS DEBUG
				cout << "TriggerState=" << std::hex << TriggerState << std::endl;
				PreviousTargetContrastChange=TargetContrastChange;
			}

			if ((ConfounderContrastChange != PreviousConfounderContrastChange) & (ConfounderContrastChange==1))
			{	
				vsgObjSelect(hConfounder);
				vsgSetDrawPage(vsgVIDEOPAGE,1,vsgNOCLEAR);
				switch (ContrastChangeValue)
				{
				case 1:
					{
						cout << "Contrast Up " <<ContrastUp << endl;
						vsgObjSetContrast(ContrastUp);
					}
					break;
				case 0:
					{
						cout << "Contrast Down " <<ContrastDown << endl;
						vsgObjSetContrast(ContrastDown);
					}
					break;
				}
				TriggerState=TriggerState+vsgDIG4;
				vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER + vsgTRIG_TOGGLEMODE,TriggerState,0);
				vsgPresent();
				// DJS DEBUG
				cout << "TriggerState=" << std::hex << TriggerState << std::endl;
				PreviousConfounderContrastChange=ConfounderContrastChange;
			}

			PreviousState=NonJuicePins;
		}
	}
}

int ReadDistractorFile()
{
	FILE *DistractorFile;
	int DistractorCount=0;
	DistractorFile=fopen("c:\\AlertRig\\DistractorList","r");
	if(DistractorFile == NULL) 
	{
		cout << "Could not open distractor file" << endl;
		_exit(0);
	}
	while(!feof(DistractorFile))
	{
		DistractorCount++;
		fscanf(DistractorFile,"%f %f %f %f %f %f %f\n",&Distractors[DistractorCount-1][0],&Distractors[DistractorCount-1][1],
				&Distractors[DistractorCount-1][2],&Distractors[DistractorCount-1][3],&Distractors[DistractorCount-1][4],
				&Distractors[DistractorCount-1][5],&Distractors[DistractorCount-1][6]);
	}
	if (Distractors[0][0] == -1000) DistractorCount=0; 
	return DistractorCount;
}


/*
	VSGTRIVAL Gray, Black, White,Red;

	vsgSetPen1(0);

	//Draw circles at the position of the target grating, confounder grating, and
	//distractor gratings
	vsgDrawOval(TargetX,TargetY,TargetDiameter,TargetDiameter);
	int OvIndex;
	if (NumberOfDistractors > 0)
	{
		for(OvIndex=1; OvIndex<=NumberOfDistractors; OvIndex++)
			vsgDrawOval(Distractors[OvIndex][5],Distractors[OvIndex][6],Distractors[OvIndex][3],Distractors[OvIndex][3]);
	}
	
	return 1;
}


*/
