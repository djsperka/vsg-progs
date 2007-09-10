//This is the version used AFTER 060106
#include <afxwin.h>
#include <iostream.h>
#include <stdio.h>
#include "VSGV8.H"
#include <math.h>


float Distractors[5][7];

int ReadDistractorFile();

void main(int ArgumentCount, char *Arguments[])
{
	
	//Parse the command line
	double X, Y, FixationPointDiameter;
	double ContrastUp[20000], ContrastDown[20000],DisContrastUp[20000], DisContrastDown[20000];
	double TargetX, TargetY;
	CString BackgroundColor,GratingOrSpot,GratingColor;
	double TargetOrientation, TargetSF, TargetTF, TargetDiameter;
	int index,StimOver,AdvanceCatch;
	int NumberOfDistractors,PixelRange, temp2,temp3,ConIndex;
	int GratingConfig=0;
	double NumberBaseContrasts,NumberPerChanges,LowBaseContrast, HighBaseContrast;
	double BaseContrast[20],PerChange[20],temp,LowPerChange,HighPerChange;
	double DisRandIntContrast[20000],RandIntContrast[20000];
	NumberOfDistractors=ReadDistractorFile();
	PixelRange=79;
	AdvanceCatch=0;
	X=atof(Arguments[1]);
	Y=atof(Arguments[2]);
	FixationPointDiameter=atof(Arguments[3]);
	LowBaseContrast=atof(Arguments[4]);
	HighBaseContrast=atof(Arguments[5]);
	NumberBaseContrasts=atof(Arguments[6]);
	LowPerChange=atof(Arguments[7]);
	HighPerChange=atof(Arguments[8]);
	NumberPerChanges=atof(Arguments[9]);
	printf("NumberPer Changes = %f\n",NumberPerChanges);
	TargetX=atof(Arguments[10]);
	TargetY=atof(Arguments[11]);
	TargetOrientation=atof(Arguments[12]);
	TargetSF=atof(Arguments[13]);
	TargetTF=atof(Arguments[14]);
	TargetDiameter=atof(Arguments[15]);
	
	temp=LowBaseContrast;
	for (index=0;index<NumberBaseContrasts;index++)
	{
		BaseContrast[index]=temp;
		temp=exp(log(BaseContrast[index])+((log(HighBaseContrast)-log(LowBaseContrast))/(NumberBaseContrasts-1)));
	}

	temp=LowPerChange;
	for (index=0;index<NumberPerChanges;index++)
	{
		PerChange[index]=temp;
		temp=exp(log(PerChange[index])+((log(HighPerChange)-log(LowPerChange))/(NumberPerChanges-1)));
		printf("PerChange = %f\n",PerChange[index]);
	}
	
	FILE *RandConFile;
	FILE *ProgActFile;
	FILE *FileInfo;
	FILE *FileInfo2;
	char CurrentExperiment[200];
	char CurrentExperiment2[200];
	char CurrentExperiment3[200];
	char RunNum[10];
	FileInfo = fopen("d:\\AlertRig-050715\\Experiments\\CurrentExperiment.par","r");
	fscanf(FileInfo,"%s",CurrentExperiment);
	fclose(FileInfo);
	printf("051208\n");
	strcat(CurrentExperiment,"\\RunNumber.par");
	FileInfo2 = fopen(CurrentExperiment,"r");
	fscanf(FileInfo2,"%s",RunNum);
	FileInfo = fopen("d:\\AlertRig-050715\\Experiments\\CurrentExperiment.par","r");
	fscanf(FileInfo,"%s",CurrentExperiment2);
	fclose(FileInfo);
	FileInfo = fopen("d:\\AlertRig-050715\\Experiments\\CurrentExperiment.par","r");
	fscanf(FileInfo,"%s",CurrentExperiment3);
	strcat(CurrentExperiment3,"\\DataFiles\\ProgAct_");
	strcat(CurrentExperiment2,"\\DataFiles\\ConInfoRun_");
	strcat(CurrentExperiment2,RunNum);
	strcat(CurrentExperiment2,".par");
	strcat(CurrentExperiment3,RunNum);
	strcat(CurrentExperiment3,".par");
	printf("%s\n",CurrentExperiment3);
	
	//RandConFile = fopen("d:\\AlertRig-050715\\AttentionData\\RandConFile.txt","w");
	RandConFile = fopen(CurrentExperiment2,"w");
	ProgActFile = fopen(CurrentExperiment3,"w");
	double NumComb;
	double CombPick;
	int CurrentPerm[20], tempint;
	int CurrentComCount, index2;
	NumComb = pow(NumberBaseContrasts,3);
	printf("NumComb = %f\n",NumComb);
	srand( (unsigned)time( NULL ) );
	for (index=0; index<5000;index++)
	{
		CombPick=floor((NumComb*rand())/RAND_MAX);

		for (index2=2; index2>=0; index2--)
		{
			temp = floor(CombPick/pow(NumberBaseContrasts,index2));
			
			CombPick=CombPick-(temp*pow(NumberBaseContrasts,index2));
			CurrentPerm[index2]=temp;
		}

		RandIntContrast[index]=BaseContrast[CurrentPerm[0]];
		ContrastUp[index]=BaseContrast[CurrentPerm[0]]+(BaseContrast[CurrentPerm[0]]*PerChange[CurrentPerm[2]]);
		ContrastDown[index]=BaseContrast[CurrentPerm[0]]-(BaseContrast[CurrentPerm[0]]*PerChange[CurrentPerm[2]]);
		DisRandIntContrast[index]=BaseContrast[CurrentPerm[1]];
		DisContrastUp[index]=BaseContrast[CurrentPerm[1]]+(BaseContrast[CurrentPerm[1]]*PerChange[CurrentPerm[2]]);
		DisContrastDown[index]=BaseContrast[CurrentPerm[1]]-(BaseContrast[CurrentPerm[1]]*PerChange[CurrentPerm[2]]);
		
		fprintf(RandConFile,"%f\t%f\t%f\t%f\t%f\t%f\n",RandIntContrast[index],ContrastUp[index],ContrastDown[index],DisRandIntContrast[index],DisContrastUp[index],DisContrastDown[index]);
	}

	fclose(RandConFile);	

	//Initialize the vsg card then check that it initialized O.K.
	int CheckCard;
	CheckCard = vsgInit("");
	if (CheckCard < 0)
	{  
		printf("VSG card failed to initialize\n");
		//_exit(0);
	}
	vsgSetViewDistMM(500); //MAKE THIS VARIABLE!!
	vsgSetSpatialUnits(vsgDEGREEUNIT);
	vsgSetVideoMode(vsgPANSCROLLMODE);

	
	//Load the lut for our drawing pages

	VSGTRIVAL Red,Green, Black, White,Incorrect,Initial,Gray,BackgroundC,From,To;

	VSGLUTBUFFER Buffer;
	BackgroundC.a =.5; BackgroundC.b =.5; BackgroundC.c =.5;
	GratingOrSpot="Grating";
	Red.a=1; Red.b=0; Red.c=0;
	Green.a=0; Green.b=1; Green.c=0;
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



	From=Black; To=White;

	Initial.a=.5*(1+100/100);
	Initial.b=.5*(1-100/100);
	Initial.c=.5*(1-100/100);

	
	Incorrect.a=.5*(1+100/100);
	Incorrect.b=.5*(1-100/100);
	Incorrect.c=.5*(1-100/100);

	int Background=0;
	//Configure the LUT initially to have 5 "special" colors up front, and 
	//generic luminance ramp thereafter.
	for(index=0; index<256; index++)
	{
		if(index==0) Buffer[index].a=.5;
		if(index==0) Buffer[index].b=.5;
		if(index==0) Buffer[index].c=.5;
		if(index==1) Buffer[index]=Red;
		if(index==2) Buffer[index]=White;
		if(index==3) Buffer[index]=Red;
		if(index==4) Buffer[index]=Red;
		if(index > 4)// was 4
		{
			Buffer[index].a=(index-5)/250.0; //Buffer[index].a=(index-5)/250.0; 
			Buffer[index].b=(index-7)/250.0;
			Buffer[index].c=(index-7)/250.0;
		}
	}
	vsgLUTBUFFERWrite(0,&Buffer);
	vsgLUTBUFFERtoPalette(0);

	//Setup the overlay page so our square gratings appear as circles
	//We do this by setting a solid background overlay page, and drawing
	//transparent circles around the location of our fixation point, answer points,
	//and gratings
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
	{
		vsgDrawGrating(TargetX,TargetY,TargetDiameter,TargetDiameter,TargetOrientation,TargetSF);
		printf("%f\n",TargetSF);
	}

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
	StimOver=0;
	
	int Fixation=0,TargetContrastChange=0,ConfounderContrastChange=0,ContrastChangeValue=0,Stim=0,AnswerPoint=0;
	int PreviousFixation=0,PreviousTargetContrastChange=0,PreviousConfounderContrastChange=0,PreviousContrastState=0,PreviousStim=0,PreviousAnswerPoint=0;
	int AdvanceTrial=0, PreviousAdvanceTrial=0;
	//Determine the initial state of the VSG digital input pins.
	StimulusState=vsgIOReadDigitalIn();
	//Digital input 0 is used by the juicer, so we ignore it
	NonJuicePins=floor(StimulusState/2);
	PreviousState=NonJuicePins;
	ConIndex=0;
	int TrialNumber=0;
	DWORD TriggerState=0; //We use this to keep track of our output trigger states.
	while (TRUE)
	{
		StimulusState=vsgIOReadDigitalIn();
		NonJuicePins=floor(StimulusState/2);

		if(NonJuicePins != PreviousState)
		{
	
			Fixation=((StimulusState&vsgDIG1)/vsgDIG1);
			Stim=((StimulusState&vsgDIG2)/vsgDIG2);
			TargetContrastChange=((StimulusState&vsgDIG3)/vsgDIG3);
			ConfounderContrastChange=((StimulusState&vsgDIG4)/vsgDIG4);
			ContrastChangeValue=((StimulusState&vsgDIG5)/vsgDIG5);
			AnswerPoint=((StimulusState&vsgDIG6)/vsgDIG6);
			AdvanceTrial=((StimulusState&vsgDIG7)/vsgDIG7);
		//	printf("Advance trial = %i/n",AdvanceTrial);

			//If the screen is supposed to be completely blank, we clear the screen by switching video pages.
			//This avoids the potential problem of items disappearing one at a time, rather than simultaneously.
			if (Fixation + Stim + AnswerPoint==0) //Special case for when the screen is supposed to be completely clear.
			{
				StimOver=0;
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
				fprintf(ProgActFile,"Answer Point Off \n");
				
				vsgObjSelect(hGrating);
				vsgObjSetContrast(0);

				vsgObjSelect(hConfounder);
				vsgObjSetContrast(0);
				fprintf(ProgActFile,"Stimuli Off \n");
				vsgObjSelect(hDistractors);
				vsgObjSetContrast(0);
				vsgPresent();
				fprintf(ProgActFile,"Fixation Point Off \n\n\n");
				PreviousState=NonJuicePins;
				PreviousFixation=0;
				PreviousStim=0;
				PreviousAnswerPoint=0;
				PreviousTargetContrastChange=0;
				PreviousConfounderContrastChange=0;
				PreviousAdvanceTrial=0;
				AdvanceCatch=0;
				fclose(ProgActFile);
				ProgActFile = fopen(CurrentExperiment3,"a");
				continue;
			}
			if (AdvanceTrial != PreviousAdvanceTrial & AdvanceCatch==0)
			{	
				printf("Advance Trial Here\n");
				
				TrialNumber=TrialNumber+1;
				ConIndex=ConIndex+1;
				fprintf(ProgActFile,"Adavance Trial to Trial %i\n",ConIndex);
				PreviousAdvanceTrial=AdvanceTrial;
				AdvanceCatch=1;
				if(TrialNumber == 20)
				{
					//HoldHandle = hGrating;
					//hGrating =hConfounder;
					//hConfounder = HoldHandle;
					TrialNumber=0;
					if (GratingConfig==0)
					{
						fprintf(ProgActFile,"Switch Grating Config to Reversed \n");
						GratingConfig=1;
						vsgObjDestroy(hFixationPoint);
						hFixationPoint=vsgObjCreate();
						vsgSetDrawPage(vsgVIDEOPAGE,1,vsgNOCLEAR);
						vsgObjSetPixelLevels(5,2);
						vsgObjSetColourVector((VSGTRIVAL*)&BackgroundC,(VSGTRIVAL*)&Green,vsgUNIPOLAR);
						vsgObjSetContrast(0);
						vsgDrawOval(X,Y,FixationPointDiameter,FixationPointDiameter);
						vsgPresent();
					}
					else
					{
						GratingConfig=0;
						fprintf(ProgActFile,"Switch Grating Config to Original \n");
						vsgObjDestroy(hFixationPoint);
						hFixationPoint=vsgObjCreate();
						vsgSetDrawPage(vsgVIDEOPAGE,1,vsgNOCLEAR);
						vsgObjSetPixelLevels(5,2);
						vsgObjSetColourVector((VSGTRIVAL*)&BackgroundC,(VSGTRIVAL*)&Red,vsgUNIPOLAR);
						vsgObjSetContrast(0);
						vsgDrawOval(X,Y,FixationPointDiameter,FixationPointDiameter);
						vsgPresent();
					}

				}
				
			}

			//Look for a change in the state of the Fixation flag. If Fixation = 1,
			//set the contrast of the fixation point to 100%. If Fixation = 0, set
			//the contrast of the fixation point to 0%/
			if (Fixation != PreviousFixation)
			{	
				fprintf(ProgActFile,"Fixation Point Up \n");
				vsgObjSelect(hFixationPoint);
				vsgSetDrawPage(vsgVIDEOPAGE,1,vsgNOCLEAR);
				vsgObjSetContrast(100*Fixation);
				TriggerState=TriggerState+(2*Fixation-1)*vsgDIG0;
				vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER + vsgTRIG_TOGGLEMODE,TriggerState,0);
				vsgPresent();
				PreviousFixation=Fixation;
				
			}

			//Look for a change in the state of the Stim flag. If Stim = 1,
			//set the contrast of the target grating, confounder grating, and distractor gratings
			//to 100%. If Fixation = 0, set the contrast of those objects to 0%/
			printf('Stim = %i\n',Stim);
			if (Stim != PreviousStim & StimOver==0)
			{	
				vsgSetDrawPage(vsgVIDEOPAGE,1,vsgNOCLEAR);
				printf("Grating Config = %i\n",GratingConfig);
				fprintf(ProgActFile,"Stimulus Up \n");
				//vsgObjSelect(hConfounder);
				//vsgObjSetContrast(BaseContrast[ConIndex]*Stim);
				fprintf(ProgActFile,"Cue trial \n");
				ContrastUp[ConIndex]=RandIntContrast[ConIndex]+ RandIntContrast[ConIndex]*.7;
				ContrastDown[ConIndex]=RandIntContrast[ConIndex]- RandIntContrast[ConIndex]*.7;
				DisContrastUp[ConIndex]=DisRandIntContrast[ConIndex]+ DisRandIntContrast[ConIndex]*.7;
				DisContrastDown[ConIndex]=DisRandIntContrast[ConIndex]- DisRandIntContrast[ConIndex]*.7;
				if (GratingConfig==0)
				{
						
					vsgObjSelect(hConfounder);
					vsgObjSetContrast(0);
					vsgObjSelect(hGrating);
					vsgObjSetContrast(RandIntContrast[ConIndex]);
				}
				else
				{
					vsgObjSelect(hGrating);
					vsgObjSetContrast(0);
					vsgObjSelect(hConfounder);
					vsgObjSetContrast(DisRandIntContrast[ConIndex]);
				}
			


				vsgObjSelect(hDistractors);
				vsgObjSetContrast(BaseContrast[ConIndex]*Stim);
				TriggerState=TriggerState+(2*Stim-1)*vsgDIG1;
				vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER + vsgTRIG_TOGGLEMODE,TriggerState,0);
				vsgPresent();
				PreviousStim=Stim;
			}

			//Look for a change in the state of the Fixation flag. If Fixation = 1,
			//set the contrast of the fixation point to 100%. If Fixation = 0, set
			//the contrast of the fixation point to 0%/

			if (AnswerPoint != PreviousAnswerPoint)
			{	
				fprintf(ProgActFile,"Answer Points Up \n");
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
						fprintf(ProgActFile,"Contrast Up Original Target Trial \n");
						printf("Initial Contrast =%f\n",RandIntContrast[ConIndex]);
						cout << "Contrast Up " <<ContrastUp[ConIndex] << endl;
						printf("Trial number =%i\n",TrialNumber);
						printf("Contrast Index =%i\n",ConIndex);
						vsgObjSetContrast(ContrastUp[ConIndex]);
						StimOver=1;
					}
					break;
				case 0:
					{
						fprintf(ProgActFile,"Contrast Down Original Target Trial \n");
						printf("Initial Contrast =%f\n",RandIntContrast[ConIndex]);
						cout << "Contrast Down " <<ContrastDown[ConIndex] << endl;
						printf("Trial number =%i\n",TrialNumber);
						printf("Contrast Index =%i\n",ConIndex);
						vsgObjSetContrast(ContrastDown[ConIndex]);
						StimOver=1;
					}
					break;
				}
				
				TriggerState=TriggerState+vsgDIG3;
				vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER + vsgTRIG_TOGGLEMODE,TriggerState,0);
				vsgPresent();
				PreviousTargetContrastChange=TargetContrastChange;
			}

			if ((ConfounderContrastChange != PreviousConfounderContrastChange) & (ConfounderContrastChange==1))// & (TrialNumber>=4))
			{	
				vsgObjSelect(hConfounder);
				vsgSetDrawPage(vsgVIDEOPAGE,1,vsgNOCLEAR);
				switch (ContrastChangeValue)
				{
				case 1:
					{
						fprintf(ProgActFile,"Contrast Up Original Distractor Trial \n");
						printf("Initial Contrast =%f\n",DisRandIntContrast[ConIndex]);
						cout << "Contrast Up " <<DisContrastUp[ConIndex] << endl;
						vsgObjSetContrast(DisContrastUp[ConIndex]);
						printf("Trial number =%i\n",TrialNumber);
						printf("Contrast Index =%i\n",ConIndex);
						StimOver=1;
					}
					break;
				case 0:
					{
						fprintf(ProgActFile,"Contrast Down Original Distractor Trial \n");
						printf("Initial Contrast =%f\n",DisRandIntContrast[ConIndex]);
						cout << "Contrast Down " <<DisContrastDown[ConIndex] << endl;
						vsgObjSetContrast(DisContrastDown[ConIndex]);
						printf("Trial number =%i\n",TrialNumber);
						printf("Contrast Index =%i\n",ConIndex);
						StimOver=1;
					}
					break;
				}
				TriggerState=TriggerState+vsgDIG4;
				vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER + vsgTRIG_TOGGLEMODE,TriggerState,0);
				vsgPresent();
		//		if (TrialNumber<4)
		//		{
		//		PreviousTargetContrastChange=TargetContrastChange;
		//		}
		//		else
		//		{
				PreviousConfounderContrastChange=ConfounderContrastChange;
		//		}
				
			}
		/////////////////New code to end stim presntation after short delay
			if (Stim != PreviousStim & StimOver==1)
			{
				vsgObjSelect(hGrating);
				vsgObjSetContrast(0);
				vsgObjSelect(hConfounder);
				vsgObjSetContrast(0);
				vsgPresent();
				fprintf(ProgActFile,"Stimuli off \n");
			}
			PreviousState=NonJuicePins;
		}
	}
}

int ReadDistractorFile()
{
	FILE *DistractorFile;
	int DistractorCount=0;
	DistractorFile=fopen("d:\\AlertRig-050715\\AttentionData\\DistractorList.txt","r");
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
