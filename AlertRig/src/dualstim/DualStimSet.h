#include "alertlib.h"
using namespace alert;

#ifndef _STIM_SET_BASE

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
	~StimSetBase() {};

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
	void set_fixpt(ARFixationPointSpec& fixpt);
	void set_fixpt(ARFixationPointSpec& fixpt, double xoffset, double yoffset);
	void set_xhair(ARXhairSpec& xhair);
	void set_xhair(ARXhairSpec& xhair, double xoffset, double yoffset);
	virtual void set_grating(ARGratingSpec& grating);
	virtual void set_grating(ARGratingSpec& grating, double xoffset, double yoffset);

	virtual bool has_fixpt() const { return m_have_fixpt; };
	virtual bool has_xhair() const { return m_have_xhair; };
	virtual bool has_grating() const { return m_have_grating; };
	virtual bool is_empty() const { return !(m_have_fixpt || m_have_xhair || m_have_grating); };
	ARContrastFixationPointSpec& fixpt() { return m_fixpt; };
	const ARContrastFixationPointSpec& fixpt() const { return m_fixpt; };
	ARXhairSpec& xhair() { return m_xhair; };
	virtual ARGratingSpec& grating() { return m_grating; };
};

#define _STIM_SET_BASE
#endif





