// Grating.cpp: implementation of the Grating class.
//
//////////////////////////////////////////////////////////////////////

#include "Grating.h"
#include "trimline.h"
#include "AustinUtilities.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Grating::Grating()
{
	CurrentIndex=0;
}

Grating::~Grating()
{

}
/*
int Grating::ReadParameterFile(CString ParameterFileName)
{

	FILE *ParameterFile;
	char raw[100];
	CString line;
	int pos,pos2,length, ParameterIndex, MaxParameterIndex=0;
    CString Parameter, Value;

	//Read in the parameter file, and use it to set member variable values
	if( (ParameterFile  = fopen( ParameterFileName, "r" )) == NULL )
	{
		cout << "Could not open parameter file " << ParameterFileName << "\n";
		return -1;
	}

	//Read the first line of the file to determine the experiment type.
	fgets(raw,100,ParameterFile);

	if ( strncmp(raw,"[",1) )  // Check to see if the first character is [
	{
		cout << "Not a valid parameter file" << "\n";
		return -1;
	}
	else
	{
		line=TrimLine(raw);
		ExperimentType=line.Mid(1,line.GetLength()-2); //Remove the enclosing brackets
	}


	while(  fgets(raw,100,ParameterFile) != NULL ) // Read until end of file (or error)
	{
	//Every line has a Parameter, an optional ParameterIndex (for the tuned Parameter only), and a Value
		ParameterIndex = -1; // Reset ParameterIndex for every line, as not every line has a ParameterIndex
		line=TrimLine(raw);

		//We check to make sure there is a closing " in the parameter name, otherwise it is an invalid line.
		if ( (pos=line.Find("\"",2)) < 2 )
		{
			cout << "Invalid line in parameter file " << ParameterFileName << "\n";
			return -1;
		}
		Parameter = line.Mid(1,pos-1);
		//pos is the position of the first whitespace character
		pos=line.Find(" ",pos);
		//pos2 is the position of the second whitespace character -- if there is one
		pos2=line.Find(" ",pos+1);
		length=line.GetLength();

		//If pos2 < 0, there is only a Parameter and Value, no Parameter index
		if (pos2 < 0)
			Value = line.Right(length - (pos+1));
		else // If pos2 > 0, there are values for Parameter, ParameterIndex and Value
		{
			ParameterIndex = atoi( line.Mid(pos+1,pos2 - (pos+1) ) );
			if (ParameterIndex > MaxParameterIndex) MaxParameterIndex=ParameterIndex;
			Value = line.Right(length - (pos2+1));
		}

		//Since the C switch statement requires integral expressions, we use many if statements
		//to assign the appropriate Values to the appropriate Parameters


		//These parameters are not tune-able, so we need not check the ParameterIndex
		if (Parameter == "Grating Pattern")
			Pattern=Value;
		if (Parameter == "Number Of Steps")
			NumberOfSteps=atoi(Value);
		if (Parameter == "Number Of Repeats")
			NumberOfRepeats=atoi(Value);
		if (Parameter == "Aperture Type")
			ApertureType=Value;
		if (Parameter == "Aperture Center X")
			ApertureX=atof(Value);
		if (Parameter == "Aperture Center Y")
			ApertureY=atof(Value);
		if (Parameter == "Signal Duration")
			SignalDuration=atof(Value);
		if (Parameter == "Blank Duration")
			BlankDuration=atof(Value);
		if (Parameter == "Background Color")
			BackgroundColor=Value;
		if (Parameter == "Aperture Height")
			ApertureHeight=atof(Value);
		if (Parameter == "Aperture Width" )
			ApertureWidth=atof(Value);
		if (Parameter == "Color" )
			GratingColor = Value;


		//These parameters are potentially tune-able, so we check ParameterIndex. For un-tuned
		//values, ParameterIndex = -1

		if (Parameter == "Grating Contrast")
		{
			if (ParameterIndex == -1) Contrast=atof(Value); else TunedParameter[ParameterIndex]=atof(Value);
		}

		if (Parameter == "Spatial Frequency")
		{
			if (ParameterIndex == -1) SpatialFrequency=atof(Value); else TunedParameter[ParameterIndex]=atof(Value);
		}

		if (Parameter == "Temporal Frequency")
		{
			if (ParameterIndex == -1) TemporalFrequency=atof(Value); else TunedParameter[ParameterIndex]=atof(Value);
		}

		if (Parameter == "Orientation")
		{
			if (ParameterIndex == -1) Orientation=atof(Value); else TunedParameter[ParameterIndex]=atof(Value);
		}

		if (Parameter == "Aperture") // Aperture should never appear without an index
		{
			if (ParameterIndex == -1) return -1; else TunedParameter[ParameterIndex]=atof(Value);
		}
	}

	//We compare the NumberOfSteps*NumberOfRepeats against the number of Parameter indices read.

	if (NumberOfSteps*NumberOfRepeats != MaxParameterIndex)
	{
		cout << "Incomplete Parameter File " << ParameterFileName << "\n";
		return -1;
	}

	fclose(ParameterFile);

	return 1;
}
*/

int Grating::NextGrating()
{

	//CurrentIndex tells us which parameter in TunedValue we use
	CurrentIndex++;

	if(CurrentIndex > (p->NumberOfSteps*p->NumberOfRepeats))
	{
		cout << "Tuned parameter index exceeds number of values read from parameter file";
		cout <<"\n";
		return -1;
	}
	//Based on ExperimentType, we increment the value of the tuned parameter
	if( strcmp(p->ExperimentType,"Orientation tuning") == 0)
		p->Orientation=p->TunedParameter[CurrentIndex];
	if( strcmp(p->ExperimentType,"Contrast tuning") == 0)
		p->Contrast=p->TunedParameter[CurrentIndex];
	if( strcmp(p->ExperimentType,"Spatial frequency tuning") == 0)
		p->SpatialFrequency=p->TunedParameter[CurrentIndex];
	if( strcmp(p->ExperimentType,"Temporal frequency tuning") == 0)
		p->TemporalFrequency=p->TunedParameter[CurrentIndex];
	if( strcmp(p->ExperimentType,"Spatial frequency tuning") == 0)
		p->SpatialFrequency=p->TunedParameter[CurrentIndex];
	if( strcmp(p->ExperimentType,"Aperture area tuning") == 0)
	{
		p->ApertureHeight=p->TunedParameter[CurrentIndex];
		p->ApertureWidth=p->TunedParameter[CurrentIndex];
	}

	return CurrentIndex;
}

void Grating::DrawGrating(double FixationX,double FixationY,double FixationDiameter)
{

	vsgObjSelect(Handle);
	vsgSetDrawPage(vsgVIDEOPAGE,0,vsgNOCLEAR);
	//We must reset the pixel levels to prevent standing wave artefacts from overlaying gratings
	//on each other
	vsgObjSetPixelLevels(2,240);

	// djs temporal freq needs this set!
	vsgObjSetDriftVelocity(p->TemporalFrequency);

	vsgDrawGrating(p->ApertureX,p->ApertureY,p->ApertureWidth,p->ApertureHeight,p->Orientation,p->SpatialFrequency);

//	vsgDrawGrating(0,0,p->ApertureWidth,p->ApertureHeight,p->Orientation,p->SpatialFrequency);
	//made change here, hja
//	vsgObjSetSpatialPhase(90);
	vsgSetDrawPage(vsgOVERLAYPAGE,2,1);
	vsgSetPen1(0);
	if(strcmp(p->ApertureType,"Ellipse")==0)
		vsgDrawOval(p->ApertureX,p->ApertureY,p->ApertureWidth,p->ApertureHeight);
	if(strcmp(p->ApertureType,"Rectangle")==0)
		vsgDrawRect(p->ApertureX,p->ApertureY,p->ApertureWidth,p->ApertureHeight);
	vsgSetPen1(2);
	vsgDrawOval(FixationX,FixationY,FixationDiameter,FixationDiameter);
	vsgPresent();

}

void Grating::InitGrating()
{
	VSGTRIVAL Black, White,SOn,SOff,MOn,MOff,LOn,LOff,From,To;
	Black.a=0; Black.b=0; Black.c=0;
	White.a=1; White.b=1; White.c=1;

	LOn.a=1; LOn.b=.42074;	LOn.c=.5;
	LOff.a=0; LOff.b=.57926;	LOff.c=.5;
	MOn.a=0; MOn.b=.72407; MOn.c=.4863;
	MOff.a=1; MOff.b=.27593; MOff.c=.5137;
	SOn.a=.61644; SOn.b=.36301; SOn.c=1;
	SOff.a=.38356; SOff.b=.63699; SOff.c=0;

	cout << "Grating color is "<< p->GratingColor << endl;
	if( strcmp(p->GratingColor,"Grayscale") == 0)
	{ From=Black; To=White;}
	if( strcmp(p->GratingColor,"S-cone") == 0)
	{ From=SOff; To=SOn;}
	if( strcmp(p->GratingColor,"M-cone") == 0)
	{ From=MOff; To=MOn;}
	if( strcmp(p->GratingColor,"L-cone") == 0)
	{ From=LOff; To=LOn;}



	//At this point, all of the grating parameters are set EXCEPT for the 
	//tuned parameter. We set the value of the tuned parameter to the first value in the list.

	if( strcmp(p->ExperimentType,"Orientation tuning") == 0)
		p->Orientation=p->TunedParameter[0];
	if( strcmp(p->ExperimentType,"Contrast tuning") == 0)
		p->Contrast=p->TunedParameter[0];
	if( strcmp(p->ExperimentType,"Spatial frequency tuning") == 0)
		p->SpatialFrequency=p->TunedParameter[0];
	if( strcmp(p->ExperimentType,"Temporal frequency tuning") == 0)
		p->TemporalFrequency=p->TunedParameter[0];
	if( strcmp(p->ExperimentType,"Spatial frequency tuning") == 0)
		p->SpatialFrequency=p->TunedParameter[0];
	if( strcmp(p->ExperimentType,"Aperture area tuning") == 0)
	{
		p->ApertureHeight=p->TunedParameter[0];
		p->ApertureWidth=p->TunedParameter[0];
	}

	//Create the object handle with which we will refer to the graphical object
	vsgSetDrawPage(vsgVIDEOPAGE,0,vsgNOCLEAR);
	Handle=vsgObjCreate();
	vsgObjSetPixelLevels(2,240);

	//Set the grating pattern

	if(strcmp(p->Pattern,"Sine")==0)
		vsgObjTableSinWave(vsgSWTABLE);
	if(strcmp(p->Pattern,"Square Wave")==0)
		vsgObjTableSquareWave(vsgSWTABLE,vsgObjGetTableSize(vsgSWTABLE)*.25,vsgObjGetTableSize(vsgSWTABLE)*.75);
	vsgObjSetDriftVelocity(p->TemporalFrequency);
	vsgObjSetColourVector((VSGTRIVAL*)&From,(VSGTRIVAL*)&To,vsgBIPOLAR);
	vsgObjSetContrast(p->Contrast);
	vsgPresent();
}

void Grating::ShowGrating()
{
	vsgIOWriteDigitalOut(0xFFFF, vsgDIG0+vsgDIG1);
	vsgSetZoneDisplayPage(vsgOVERLAYPAGE,2+vsgTRIGGERPAGE);
}

double Grating::GetSignalDuration()
{
	return p->SignalDuration;
}

double Grating::GetBlankDuration()
{
	return p->BlankDuration;
}
