// ParameterFile.cpp
//
//////////////////////////////////////////////////////////////////////

#include "ParameterFile.h"
#include "AustinUtilities.h"

ParameterFile::ParameterFile(CString ParameterFileName)
{

	FILE *ParameterFile;
	char raw[100];
	CString line;
	int pos,pos2,length, ParameterIndex, MaxParameterIndex=0;
    CString Parameter, Value;
	int nLines=0;

	//Read in the parameter file, and use it to set member variable values
	if( (ParameterFile  = fopen( ParameterFileName, "r" )) == NULL )
	{
		char ctmp[256];
		sprintf(ctmp, "Could not open parameter file %s", ParameterFileName);
		throw ctmp;
	}

	//Read the first line of the file to determine the experiment type.
	fgets(raw,100,ParameterFile);

	if ( strncmp(raw,"[",1) )  // Check to see if the first character is [
	{
		throw "Not a valid parameter file - first line should be [experiment-type]";
	}
	else
	{
		line=TrimLine(raw);
		ExperimentType=line.Mid(1,line.GetLength()-2); //Remove the enclosing brackets
	}


	nLines=1;
	while(  fgets(raw,100,ParameterFile) != NULL ) // Read until end of file (or error)
	{
		nLines++;

		//Every line has a Parameter, an optional ParameterIndex (for the tuned Parameter only), and a Value
		ParameterIndex = -1; // Reset ParameterIndex for every line, as not every line has a ParameterIndex
		line=TrimLine(raw);

		//We check to make sure there is a closing " in the parameter name, otherwise it is an invalid line.
		if ( (pos=line.Find("\"",2)) < 2 )
		{
			char ctmp[256];
			sprintf(ctmp, "Error in parameter file %s: Line %d missing a closing \"", ParameterFileName, nLines);
			throw ctmp;
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
		if (Parameter == "Fixation Color")
			FixationColor = Value;


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
			if (ParameterIndex == -1) {
				throw "Aperture cannot appear without an index.";
			}
			else {
				TunedParameter[ParameterIndex]=atof(Value);
			}
		}
	}

	//We compare the NumberOfSteps*NumberOfRepeats against the number of Parameter indices read.

	if (NumberOfSteps*NumberOfRepeats != MaxParameterIndex)
	{
		char ctmp[256];
		sprintf(ctmp, "Incomplete Parameter File %s: Missing tuned parameter lines", ParameterFileName);
		throw ctmp;
	}

	fclose(ParameterFile);

}
