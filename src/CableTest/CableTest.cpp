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
		// Read digital input bits. Our cabling aligns the 1401's DIGOUT and VSG's digital input
		// bits in the following way. 
		// A spike2 sequence [00000001] is seen (vsgIOReadDigitalIn) here as 0x1. 
		// We turn around and set the output triggers with that and send it back to spike2. 
		// Spike2 sees that on its digital input -- its event ports -- at event port 1. 
		// In other words, if spike2 sends 0x1 out its DIGOUT it is seen by the VSG as 0x1. 
		// But if VSG sends back trigger bits set 0x1, spike2 sees it on event port 1. 
		// The VSG reserves its own digital output bit for its frame trigger pulses.
		// If you call vsgObjSetTriggers with vsgTRIG_ONPRESENT, then you get a pulse when the frame
		// is presented, and that pulse is seen at the 1401 at event port 0. If you call vsgObjSetTriggers
		// with vsgTRIG_OUTPUTMARKER, then the 8 bits you can set are received by the 1401 on event ports 1-8
		// (actually event port 8 doesn't exist). 
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


