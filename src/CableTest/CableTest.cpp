#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__
#include <conio.h>
#include "vsgv8.h"
#include "Alertlib.h"
#include "windows.h"

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib,"vsgv8.lib")

bool f_binaryTriggers = true;
bool f_dio = false;
bool f_bUseLock = true;
bool f_quit = false;
using namespace std;
using namespace alert;

int args(int argc, char **argv);

#define IS_VISAGE (vsgGetSystemAttribute(vsgDEVICECLASS)==7)

BOOL CtrlHandler( DWORD fdwCtrlType ) 
{ 
  switch( fdwCtrlType ) 
  { 
    // Handle the CTRL-C signal. 
    case CTRL_C_EVENT: 
		f_quit = true;
	    printf( "Ctrl-C event\n\n" );
		return( TRUE );
	default:
		return FALSE;
  }
}

int args(int argc, char **argv)
{	
	string s;
	int c;
	extern char *optarg;
	extern int optind;
	int errflg = 0;
	while ((c = getopt(argc, argv, "adn")) != -1)
	{
		switch (c) 
		{
		case 'a':
			f_binaryTriggers = false;
			break;
		case 'd':
			f_dio = true;
			break;
		case 'n':
			f_bUseLock = false;
			break;
		default:
			errflg++;
			break;
		}
	}

	if (errflg) return 1;
	return 0;
}


void DisplayState(long DigState)
{
	cout << "VSG digital IO Input bits:" << endl;
	cout << "vsgDIG0: "<<(DigState&vsgDIG0)/vsgDIG0<<endl;
	cout << "vsgDIG1: "<<(DigState&vsgDIG1)/vsgDIG1<<endl;
	cout << "vsgDIG2: "<<(DigState&vsgDIG2)/vsgDIG2<<endl;
	cout << "vsgDIG3: "<<(DigState&vsgDIG3)/vsgDIG3<<endl;
	cout << "vsgDIG4: "<<(DigState&vsgDIG4)/vsgDIG4<<endl;
	cout << "vsgDIG5: "<<(DigState&vsgDIG5)/vsgDIG5<<endl;
	cout << "vsgDIG6: "<<(DigState&vsgDIG6)/vsgDIG6<<endl;
	cout << "vsgDIG7: "<<(DigState&vsgDIG7)/vsgDIG7<<endl;
	cout << "vsgDIG8: "<<(DigState&vsgDIG8)/vsgDIG8<<endl;
	if (f_binaryTriggers)
	{
		cout << "waiting for binary trigger...." << endl;
	}
	else
	{
		cout << "Enter key: q-quit, 0 clear, 1-8 set bit" << endl;
	}
}


int main(int argc, char *argv[])
{
	COLOR_TYPE g = { gray, {0.5, 0.5, 0.5} };
	long diginState, lastDiginState;

	SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler, TRUE );

	if (args(argc, argv))
		return -1;

	if (ARvsg::instance().init(1000, g, f_bUseLock))
	{
		cerr << "Error initializing vsg." << endl;
		return(-1);
	}

	diginState=vsgIOReadDigitalIn() & 0xff;
	lastDiginState=diginState;
	DisplayState(diginState);

	// Create an object unless using vsgIO
	if (!f_dio)
	{
		vsgObjCreate();
		vsgObjSetPixelLevels(1, 2);
	}

	if (f_binaryTriggers)
	{
		while(!f_quit)
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
			diginState=vsgIOReadDigitalIn() & 0xff;

			if ((diginState&0xff) == 0xf0) break;

			if (diginState!=lastDiginState)
			{
				cout << diginState << endl;
				DisplayState(diginState);
				lastDiginState=diginState;
				if (!f_dio)
				{
					if (IS_VISAGE)
					{
						vsgSetTriggerOptions(vsgTRIGOPT_PRESENT, 0, vsgTRIG_OUTPUTMARKER, 0.5, 0, diginState, 0x1FE);
					}
					else
					{
						vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, diginState, 0);
					}
					vsgPresent();
				}
				else
				{
					vsgIOWriteDigitalOut(diginState, 0xff);
				}
			}
		}
	}
	else
	{
		bool bQuit = false;
		int key=0;
		int num=0;
		while (!bQuit)
		{
			if (_kbhit())
			{
				key = _getch();
				switch(key)
				{
				case 'q':
					bQuit = true;
					break;
				case '0':
					vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0x0, 0);
					vsgPresent();
					break;
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					num = key - '0';
					vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, (0x1 << (num-1)), 0);
					vsgPresent();
					break;
				default:
					cout << "Bad input." << endl;
					break;
				}
			}
			else
			{
				Sleep(500);
			}
		}
	}
	return 0;
}


