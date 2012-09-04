#include "StimSetFGX.h"
#include "SSInfo.h"
//#include <windows.h>
#include <string>
#include <boost/shared_ptr.hpp>

using namespace std;
using namespace alert;
using namespace boost;

class StimSetFGGX: public StimSetMultipleGrating
{
private:
	virtual void per_trial_predraw_updates() {};
protected:
	int m_itrial;
	int m_ngridpages;
	int m_firstgridpage;
	shared_ptr<SSInfo> m_pssinfo;
	ARRandomGridSpec m_grid0;
	ARRandomGridSpec m_grid1;
	//virtual void draw_pages(bool updates_only);
	virtual void draw_scratch_pages();
	virtual void copy_scratch_page(int dest);
	void getRandomList(vector<int>& result, int N, int num);

	// Get the max outer diameter from m_pssinfo
	double getGridDiam();

public:
	StimSetFGGX(shared_ptr<SSInfo> pssinfo, double xOffset=0.0, double yOffset=0.0);
	~StimSetFGGX() {};
	virtual void advance();
	virtual int setup_cycling();
	virtual int init(ARvsg& vsg) = 0;
	virtual int handle_trigger(std::string& s) = 0;
};


// s/b StimSetFGGXCore
class StimSetFGGXCore: public StimSetFGGX
{
private:
//	HANDLE m_thread;
//	HANDLE m_event;
	virtual void per_trial_predraw_updates();
	virtual void draw_pages(bool updates_only);
//	void do_updates();
protected:
public:
	StimSetFGGXCore(shared_ptr<SSInfo> pssinfo, double xOffset=0.0, double yOffset=0.0);
	~StimSetFGGXCore() {};
	virtual int handle_trigger(std::string& s);
//	static void threadfunc(void *obj);
	virtual int init(ARvsg& vsg);
};

// s/b StimSetFGGXDonut
class StimSetFGGXDonut: public StimSetFGGX
{
private:
//	HANDLE m_thread;
//	HANDLE m_event;
	virtual void per_trial_predraw_updates();
protected:
//	void threadfunc(void *obj);
	virtual void draw_pages(bool updates_only);
public:
	StimSetFGGXDonut(shared_ptr<SSInfo> pssinfo, double xOffset=0.0, double yOffset=0.0);
	~StimSetFGGXDonut() {};
	virtual int handle_trigger(std::string& s);
	virtual int init(ARvsg& vsg);
};
