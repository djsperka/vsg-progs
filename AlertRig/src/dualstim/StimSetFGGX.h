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
	ARContrastFixationPointSpec m_ap0;
	ARContrastFixationPointSpec m_ap1;
	ARChessboardSpec m_cb0;
	ARChessboardSpec m_cb1;
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


class StimSetFGGXCore: public StimSetFGGX
{
private:
	virtual void per_trial_predraw_updates();
	virtual void draw_pages(bool updates_only);
protected:
public:
	StimSetFGGXCore(shared_ptr<SSInfo> pssinfo, double xOffset=0.0, double yOffset=0.0);
	~StimSetFGGXCore() {};
	virtual int handle_trigger(std::string& s);
	virtual int init(ARvsg& vsg);
};

class StimSetFGGXDonut: public StimSetFGGX
{
private:
	virtual void per_trial_predraw_updates();
protected:
	virtual void draw_pages(bool updates_only);
public:
	StimSetFGGXDonut(shared_ptr<SSInfo> pssinfo, double xOffset=0.0, double yOffset=0.0);
	~StimSetFGGXDonut() {};
	virtual int handle_trigger(std::string& s);
	virtual int init(ARvsg& vsg);
};
