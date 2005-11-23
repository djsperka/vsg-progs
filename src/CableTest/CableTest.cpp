#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__

#include "vsgv8.h"
#include "Alertlib.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib,"vsgv8.lib")

using namespace std;
using namespace alert;



void DisplayState(long DigState)
{
	cout <<"Bit 0: "<<(DigState&vsgDIG0)/vsgDIG0<<endl;
	cout <<"Bit 1: "<<(DigState&vsgDIG1)/vsgDIG1<<endl;
	cout <<"Bit 2: "<<(DigState&vsgDIG2)/vsgDIG2<<endl;
	cout <<"Bit 3: "<<(DigState&vsgDIG3)/vsgDIG3<<endl;
	cout <<"Bit 4: "<<(DigState&vsgDIG4)/vsgDIG4<<endl;
	cout <<"Bit 5: "<<(DigState&vsgDIG5)/vsgDIG5<<endl;
	cout <<"Bit 6: "<<(DigState&vsgDIG6)/vsgDIG6<<endl;
	cout <<"Bit 7: "<<(DigState&vsgDIG7)/vsgDIG7<<endl;
}

int main(int argc, char *argv[])
{
	int nRetCode = 0;


	vsgInit("");
	Sleep(3000);
	long StimulusState,PreviousState;
	StimulusState=vsgIOReadDigitalIn() & 0xff;
	PreviousState=StimulusState;
	DisplayState(StimulusState);

	vsgObjCreate();
	vsgObjSetPixelLevels(1, 2);


	while(TRUE)
	{
		StimulusState=vsgIOReadDigitalIn() & 0xff;
		if (StimulusState!=PreviousState)
		{
			cout << StimulusState << endl;
			DisplayState(StimulusState);
			PreviousState=StimulusState;
			vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, StimulusState, 0);
			vsgPresent();
		}
	}

	return nRetCode;
}


