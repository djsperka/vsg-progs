#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__

#include "vsgv8.h"
#include "Alertlib.h"
#include "AlertUtil.h"
#include "..\dualstim\StimSetFGGX.h"

using namespace std;
using namespace alert;

#pragma comment (lib, "Winmm.lib")

COLOR_TYPE f_background = {gray, { 0.5, 0.5, 0.5}};
int f_iDistanceToScreenMM = 1000;
TriggerVector triggers;
StimSetBase *f_pstimset = NULL;
bool f_binaryTriggers = false;

int callback(int &output, const CallbackTrigger* ptrig);
void init_triggers();
StimSetBase* create_stimset(string filename, bool useCore);

int main(int argc, char **argv)
{

	if (argc < 3) 
	{
		cerr << "Need 2 args: example1 ssinfofile core|donut" << endl;
		return 1;
	}

	if (ARvsg::instance().init(f_iDistanceToScreenMM, f_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}

	init_triggers();

	f_pstimset = create_stimset(argv[1], (argv[2][0]=='d' ? false : true));
	f_pstimset->init(ARvsg::instance());


	// All right, start monitoring triggers........
	int last_output_trigger=0;
	long input_trigger = 0;
	string s;

	while (1)
	{
		// Get a new "trigger" from user
		cout << "Enter trigger/key: ";
		cin >> s;

		TriggerFunc	tf = std::for_each(triggers.begin(), triggers.end(), 
			(f_binaryTriggers ? TriggerFunc(input_trigger, last_output_trigger) : TriggerFunc(s, last_output_trigger)));

		// Now analyze input trigger
	 	
		if (tf.quit()) break;
		else if (tf.present())
		{	
			last_output_trigger = tf.output_trigger();
			if (IS_VISAGE)
			{
				vsgSetTriggerOptions(vsgTRIGOPT_PRESENT, 0, vsgTRIG_OUTPUTMARKER, 0.5, 0, tf.output_trigger() << 1, 0x1FE);
			}
			else
			{
				vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, tf.output_trigger(), 0);
			}
			vsgPresent();
		}
		Sleep(10);
	}

	ARvsg::instance().clear();

	return 0;
}

StimSetBase* create_stimset(string filename, bool useCore)
{
	ARContrastFixationPointSpec fixpt;
	ARXhairSpec xhair;
	shared_ptr<SSInfo> pssinfo(new SSInfo());
	if (!SSInfo::load(filename, *pssinfo))
	{
		cerr << "Error parsing stim set info file " << filename << endl;
	}
	parse_fixation_point(string("0,0,0.5,red"), fixpt);
	parse_xhair(string("0,0,0.5,0.75,1,14,0,0"), xhair);

	StimSetBase *p;
	if (useCore) p = new StimSetFGGXCore(pssinfo);
	else p = new StimSetFGGXDonut(pssinfo);
	p->set_fixpt(fixpt);
	p->set_xhair(xhair);
	return p;
}

void init_triggers()
{
	triggers.addTrigger(new CallbackTrigger("F", 0x2, 0x2, 0x2, 0x2, callback));
	triggers.addTrigger(new CallbackTrigger("S", 0x4, 0x4, 0x4, 0x4, callback));
	triggers.addTrigger(new CallbackTrigger("s", 0x4, 0x0, 0x4, 0x0, callback));
	triggers.addTrigger(new CallbackTrigger("X", 0x6, 0x0, 0x6, 0x0, callback));
	triggers.addTrigger(new CallbackTrigger("a", 0x8, 0x8|AR_TRIGGER_TOGGLE, 0x8, 0x8|AR_TRIGGER_TOGGLE, callback));

	// new triggers, unused, but available.
	triggers.addTrigger(new CallbackTrigger("C", 0x20, 0x20, 0x1, 0x1, callback));
	triggers.addTrigger(new CallbackTrigger("D", 0x40, 0x40, 0x1, 0x1, callback));
	triggers.addTrigger(new CallbackTrigger("E", 0x80, 0x80, 0x0, 0x0, callback));

	// quit trigger
	triggers.addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));

	return;
}

int callback(int &output, const CallbackTrigger* ptrig)
{
	int ival=1;
	string key = ptrig->getKey();

	if (key == "S" || key == "F" || key=="X" || key=="a" || key=="s" || key == "C" || key == "D" || key == "E")
	{
		ival = f_pstimset->handle_trigger(key);
	}
	else
	{
		cerr << "ERROR! Unknown trigger sent to callback!" << endl;
	}

	return ival;
}

