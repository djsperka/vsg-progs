#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <boost/ptr_container/ptr_vector.hpp>
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__

#include "vsgv8.h"
#include "Alertlib.h"
#include "AlertUtil.h"
//#include "StimSet.h"
using namespace std;
using namespace alert;
using namespace boost;


// libs

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif
#pragma comment (lib, "vsgv8.lib")

bool f_verbose = true;
bool f_binaryTriggers = false;
int f_nlevels = 40;
int f_iDistanceToScreenMM = 1000;
COLOR_TYPE f_background = { gray, { 0.5, 0.5, 0.5}};
TriggerVector triggers;
int f_contrast;
ARFixationPointSpec f_fixpt;


void init_triggers();
int callback(int &output, const CallbackTrigger* ptrig);


class StimParameterList
{
public:
	StimParameterList() {};
	virtual ~StimParameterList() {};
	virtual ARDonutSpec& advance(ARDonutSpec& donut) = 0;
	virtual ARDonutSpec& set_current_parameter(ARDonutSpec& donut) = 0;
};

class StimContrastList: public StimParameterList
{
public:
	StimContrastList(vector<int> contrasts) : StimParameterList(), m_vec(contrasts) { m_iter = m_vec.begin(); };
	virtual ~StimContrastList() {};
	virtual ARDonutSpec& advance(ARDonutSpec& donut)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(donut);
	}

	virtual ARDonutSpec& set_current_parameter(ARDonutSpec& donut)
	{
		donut.setContrast(*m_iter);
		return donut;
	}

private:
	vector<int> m_vec;
	vector<int>::const_iterator m_iter;
};


class StimSFList: public StimParameterList
{
public:
	StimSFList(vector<double> sfs) : StimParameterList(), m_vec(sfs) { m_iter = m_vec.begin(); };
	virtual ~StimSFList() {};
	virtual ARDonutSpec& advance(ARDonutSpec& donut)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(donut);
	}

	virtual ARDonutSpec& set_current_parameter(ARDonutSpec& donut)
	{
		donut.sf = *m_iter;
		return donut;
	}

private:
	vector<double> m_vec;
	vector<double>::const_iterator m_iter;
};


class StimXList: public StimParameterList
{
public:
	StimXList(vector<double> xs) : StimParameterList(), m_vec(xs) { m_iter = m_vec.begin(); };
	virtual ~StimXList() {};
	virtual ARDonutSpec& advance(ARDonutSpec& donut)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(donut);
	}

	virtual ARDonutSpec& set_current_parameter(ARDonutSpec& donut)
	{
		donut.x = *m_iter;
		return donut;
	}

private:
	vector<double> m_vec;
	vector<double>::const_iterator m_iter;
};



class MultiParameterDonut: public ARDonutSpec, public ptr_vector<StimParameterList>
{
public:
	MultiParameterDonut() {};
	virtual ~MultiParameterDonut() {};
	void add_parameter_list(StimParameterList *plist)
	{
		plist->set_current_parameter(*this);
		push_back(plist);
		return;
	}

	void advance()
	{
		for (ptr_vector<StimParameterList>::iterator it = begin(); it != end(); it++)
			it->advance(*this);
		return;
	}
};
MultiParameterDonut f_mpdonut;


int main (int argc, char *argv[])
{

	if (parse_donut(string("0.0,0.0,10.0,10.0,5.0,5.0,100,0.10,4.00,90.0,b,s,e"), f_mpdonut))
		cerr << "Error parsing donut" << endl;
	parse_fixation_point(string("0.0,0.0,0.5,red"), f_fixpt);


	if (ARvsg::instance().init(f_iDistanceToScreenMM, f_background))
	{
		cerr << "VSG init failed!" << endl;
		return 1;
	}

	vector<int> contrasts;
	contrasts.push_back(10);
	contrasts.push_back(50);
	contrasts.push_back(100);
	StimContrastList *pstim = new StimContrastList(contrasts);
	f_mpdonut.push_back(pstim);

	vector<double> sfs;
	sfs.push_back(0.1);
	sfs.push_back(0.8);
	sfs.push_back(3.2);
	StimSFList *psfs = new StimSFList(sfs);
	f_mpdonut.push_back(psfs);

	vector<double> xs;
	xs.push_back(0);
	xs.push_back(2);
	xs.push_back(-4);
	xs.push_back(7);
	StimXList *pxlist = new StimXList(xs);
	f_mpdonut.push_back(pxlist);

	init_triggers();

	if (f_verbose)
	{
		for (unsigned int i=0; i<triggers.size(); i++)
		{
			std::cout << "Trigger " << i << " " << *(triggers[i]) << std::endl;
		}
	}

	f_fixpt.init(2);
	f_mpdonut.init(ARvsg::instance(), f_nlevels);

	// draw on page 2
	vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgBACKGROUND);
	f_contrast = f_mpdonut.contrast;
	f_mpdonut.setContrast(0);
	f_mpdonut.draw();
	f_fixpt.setContrast(0);
	f_fixpt.draw();
	vsgPresent();

	// All right, start monitoring triggers........
	int last_output_trigger=0;
	long input_trigger = 0;
	string s;

	while (1)
	{
		// If user-triggered, get a trigger entry. 
		if (!f_binaryTriggers)
		{
			// Get a new "trigger" from user
			cout << "Enter trigger/key: ";
			cin >> s;
		}
		else
		{
			input_trigger = vsgIOReadDigitalIn();
		}

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

void init_triggers()
{
	triggers.addTrigger(new CallbackTrigger("F", 0x2, 0x2, 0x2, 0x2, callback));
	triggers.addTrigger(new CallbackTrigger("S", 0x4, 0x4, 0x4, 0x4, callback));
	triggers.addTrigger(new CallbackTrigger("s", 0x4, 0x0, 0x4, 0x0, callback));
	triggers.addTrigger(new CallbackTrigger("X", 0x6, 0x0, 0x6, 0x0, callback));
	triggers.addTrigger(new CallbackTrigger("a", 0x8, 0x8|AR_TRIGGER_TOGGLE, 0x8, 0x8|AR_TRIGGER_TOGGLE, callback));

	// quit trigger
	triggers.addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));

	return;
}

int callback(int &output, const CallbackTrigger* ptrig)
{
	int ival=1;
	string key = ptrig->getKey();

	if (key == "S")
	{
		f_mpdonut.setContrast(f_contrast);
	}
	else if (key == "F")
	{
		f_fixpt.setContrast(100);
	}
	else if (key=="X")
	{
		f_fixpt.setContrast(0);
		f_mpdonut.setContrast(0);
	}
	else if (key=="a")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgBACKGROUND);
		f_mpdonut.advance();
		f_contrast = f_mpdonut.contrast;
		f_mpdonut.setContrast(0);
		f_mpdonut.draw();
		f_fixpt.draw();
	}
	else
	{
		cerr << "ERROR! Unknown trigger sent to callback!" << endl;
	}

	return ival;
}
