#if !defined(_FGGXSTIMSET_H_)
#define _FGGXSTIMSET_H_


#include "StimSet.h"
#include "SSInfo.h"
#include <boost/shared_ptr.hpp>

using namespace boost;

class FGGXStimSet: public FXMultiGStimSet
{
private:
	void initialize(bool bDonut, double xOffset, double yOffset);
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
	FGGXStimSet(shared_ptr<SSInfo> pssinfo, bool bDonut=false, double xOffset=0.0, double yOffset=0.0);
	FGGXStimSet(ARContrastFixationPointSpec& fixpt, shared_ptr<SSInfo> pssinfo, bool bDonut=false, double xOffset=0.0, double yOffset=0.0);
	FGGXStimSet(ARContrastFixationPointSpec& fixpt, ARXhairSpec& xhair, shared_ptr<SSInfo> pssinfo, bool bDonut=false, double xOffset=0.0, double yOffset=0.0);
	~FGGXStimSet() {};
	virtual void advance();
	virtual int setup_cycling();
};



class FGGXCoreStimSet: public FGGXStimSet
{
private:
	virtual void per_trial_predraw_updates();
	virtual void draw_pages(bool updates_only);

protected:

public:
	FGGXCoreStimSet(ARContrastFixationPointSpec& fixpt, ARXhairSpec& xhair, shared_ptr<SSInfo> pssinfo, double xOffset=0.0, double yOffset=0.0): FGGXStimSet(fixpt, xhair, pssinfo, false, xOffset, yOffset) {};
	FGGXCoreStimSet(ARContrastFixationPointSpec& fixpt, shared_ptr<SSInfo> pssinfo, double xOffset=0.0, double yOffset=0.0): FGGXStimSet(fixpt, pssinfo, false, xOffset, yOffset) {};
	FGGXCoreStimSet(shared_ptr<SSInfo> pssinfo, double xOffset=0.0, double yOffset=0.0): FGGXStimSet(pssinfo, false, xOffset, yOffset) {};
	~FGGXCoreStimSet() {};
	virtual int handle_trigger(std::string& s);
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual std::string toString() const;
	virtual int num_pages() {return 6;};
	virtual int num_overlay_pages() {return 0;};
};



class FGGXDonutStimSet: public FGGXStimSet
{
private:
	virtual void per_trial_predraw_updates();
protected:
	virtual void draw_pages(bool updates_only);
public:
	FGGXDonutStimSet(ARContrastFixationPointSpec& fixpt, ARXhairSpec& xhair, shared_ptr<SSInfo> pssinfo, double xOffset=0.0, double yOffset=0.0): FGGXStimSet(fixpt, xhair, pssinfo, true, xOffset, yOffset) {};
	FGGXDonutStimSet(ARContrastFixationPointSpec& fixpt, shared_ptr<SSInfo> pssinfo, double xOffset=0.0, double yOffset=0.0): FGGXStimSet(fixpt, pssinfo, true, xOffset, yOffset) {};
	FGGXDonutStimSet(shared_ptr<SSInfo> pssinfo, double xOffset=0.0, double yOffset=0.0): FGGXStimSet(pssinfo, true, xOffset, yOffset) {};
	~FGGXDonutStimSet() {};
	virtual int handle_trigger(std::string& s);
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual std::string toString() const;
	virtual int num_pages() {return 6;};
	virtual int num_overlay_pages() {return 0;};};

class FGGXDanishStimSet: public FGGXStimSet
{
private:
	virtual void per_trial_predraw_updates();
	void initialize(double xoffset, double yoffset);
protected:
	virtual void draw_pages(bool updates_only);
public:
	FGGXDanishStimSet(ARContrastFixationPointSpec& fixpt, ARXhairSpec& xhair, shared_ptr<SSInfo> pssinfo, double xOffset=0.0, double yOffset=0.0): FGGXStimSet(fixpt, xhair, pssinfo, false, xOffset, yOffset) { initialize(xOffset, yOffset); };
	FGGXDanishStimSet(ARContrastFixationPointSpec& fixpt, shared_ptr<SSInfo> pssinfo, double xOffset=0.0, double yOffset=0.0): FGGXStimSet(fixpt, pssinfo, false, xOffset, yOffset) { initialize(xOffset, yOffset); };
	FGGXDanishStimSet(shared_ptr<SSInfo> pssinfo, double xOffset=0.0, double yOffset=0.0): FGGXStimSet(pssinfo, false, xOffset, yOffset) { initialize(xOffset, yOffset); };
	~FGGXDanishStimSet() {};
	virtual int handle_trigger(std::string& s);
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual std::string toString() const;
	virtual int num_pages() {return 6;};
	virtual int num_overlay_pages() {return 0;};};

class FGGXNullStimSet: public FGGXStimSet
{
private:
	virtual void per_trial_predraw_updates() {};
protected:
	virtual void draw_pages(bool updates_only);
public:
	FGGXNullStimSet(ARContrastFixationPointSpec& fixpt, ARXhairSpec& xhair, shared_ptr<SSInfo> pssinfo, double xOffset=0.0, double yOffset=0.0): FGGXStimSet(fixpt, xhair, pssinfo, false, xOffset, yOffset) {};
	FGGXNullStimSet(ARContrastFixationPointSpec& fixpt, shared_ptr<SSInfo> pssinfo, double xOffset=0.0, double yOffset=0.0): FGGXStimSet(fixpt, pssinfo, false, xOffset, yOffset) {};
	FGGXNullStimSet(shared_ptr<SSInfo> pssinfo, double xOffset=0.0, double yOffset=0.0): FGGXStimSet(pssinfo, false, xOffset, yOffset) {};
	~FGGXNullStimSet() {};
	virtual int handle_trigger(std::string& s);
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual std::string toString() const;
	virtual int num_pages() {return 6;};
	virtual int num_overlay_pages() {return 0;};
};

#endif