#ifndef _STIM_SET_BASE

#include "alertlib.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/assert.hpp>
#include <vector>
using namespace alert;
using namespace boost;
using namespace std;

class StimSetBase
{
private:
	unsigned int m_nframes_on;
	unsigned int m_nframes_delay;
	unsigned int m_nframes_fixpt_delay;
	double m_dstimtime_sec;
	ARContrastFixationPointSpec m_fixpt;
	ARXhairSpec m_xhair;
	bool m_have_fixpt;
	bool m_have_xhair;

protected:
	ARGratingSpec m_grating;
	bool m_have_grating;

public:
	StimSetBase(): m_nframes_on(1), m_nframes_delay(0), m_nframes_fixpt_delay(0), m_dstimtime_sec(0.0), m_have_fixpt(false), m_have_xhair(false), m_have_grating(false) {};
	virtual ~StimSetBase() {};

	virtual int init(alert::ARvsg& vsg) = 0;
	virtual int handle_trigger(std::string& s) = 0;

	int get_frames_on();
	void set_frames_on(int nframes_on) { m_nframes_on = nframes_on;};
	int get_frames_delay() { return m_nframes_delay; };
	void set_frames_delay(int nframes_delay) { m_nframes_delay = nframes_delay;};
	int get_frames_fixpt_delay() { return m_nframes_fixpt_delay; };
	void set_frames_fixpt_delay(int nframes_fixpt_delay) { m_nframes_fixpt_delay = nframes_fixpt_delay; };
	void set_stimtime(double dstimtime_sec) { m_dstimtime_sec = dstimtime_sec; };

	// Same as above, call prior to init().
	void set_fixpt(const ARFixationPointSpec& fixpt, double xoffset=0.0, double yoffset=0.0);
	void set_xhair(const ARXhairSpec& xhair, double xoffset=0.0, double yoffset=0.0);
#if 0
	virtual void set_grating(ARGratingSpec& grating);
	virtual void set_grating(ARGratingSpec& grating, double xoffset, double yoffset);
	virtual bool has_grating() const { return m_have_grating; };
	virtual ARGratingSpec& grating() { return m_grating; };
#endif

	virtual bool has_fixpt() const { return m_have_fixpt; };
	virtual bool has_xhair() const { return m_have_xhair; };
	virtual bool is_empty() const { return !(m_have_fixpt || m_have_xhair || m_have_grating); };
	ARContrastFixationPointSpec& fixpt() { return m_fixpt; };
	const ARContrastFixationPointSpec& fixpt() const { return m_fixpt; };
	ARXhairSpec& xhair() { return m_xhair; };
};


class StimSetSingleGrating: public StimSetBase
{
public:
	StimSetSingleGrating() : StimSetBase(), m_have_grating(false) {};
	virtual ~StimSetSingleGrating() {};

	virtual int init(alert::ARvsg& vsg) = 0;
	virtual int handle_trigger(std::string& s) = 0;

	virtual void set_grating(const ARGratingSpec& grating, double xoffset=0.0, double yoffset=0.0);
	virtual bool has_grating() const { return m_have_grating; };
	virtual ARGratingSpec& grating() { return m_grating; };

protected:
	ARGratingSpec m_grating;
	bool m_have_grating;

private:

};

class StimSetMultipleGrating: public StimSetBase
{
public:
	StimSetMultipleGrating() : StimSetBase() { };
	virtual ~StimSetMultipleGrating() {};

	virtual int init(alert::ARvsg& vsg) = 0;
	virtual int handle_trigger(std::string& s) = 0;

	virtual void set_grating(const ARGratingSpec& grating, double xoffset=0.0, double yoffset=0.0);
	virtual bool has_grating() const { return m_have_grating; };
	virtual ARGratingSpec& grating() 
	{ 
		BOOST_ASSERT(m_gratings.size() > 0);
		return m_gratings[0]; 
	};

	ARGratingSpec& grating(int i)
	{
		BOOST_ASSERT(m_gratings.size() > i);
		return m_gratings[i];
	};

	int contrast(int i)
	{
		BOOST_ASSERT(m_contrasts.size() > i);
		return m_contrasts[i];
	}

	int count()
	{
		return m_gratings.size();
	};

protected:

private:
	ptr_vector<ARGratingSpec> m_gratings;
	vector<int> m_contrasts;

};



#define _STIM_SET_BASE
#endif





