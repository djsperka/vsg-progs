#include "alertlib.h"
#include "StimParameterList.h"
#include <string>
#include <boost/ptr_container/ptr_vector.hpp>

using namespace std;
using namespace alert;
using namespace boost;





class StimSet: public ptr_vector<StimParameterList>
{
private:
	int m_nframes_on;
	int m_nframes_delay;
	int m_nframes_fixpt_delay;
	ARContrastFixationPointSpec m_fixpt;
	ARXhairSpec m_xhair;
	ARGratingSpec m_grating;
	bool m_have_fixpt;
	bool m_have_xhair;
	bool m_have_grating;
	int m_contrast;
public:
	StimSet() : m_have_fixpt(false), m_have_xhair(false), m_have_grating(false) {};
	virtual ~StimSet() {};

	int get_frames_on() { return m_nframes_on; };
	int get_frames_delay() { return m_nframes_delay; };
	void set_frames_delay(int nframes_delay) { m_nframes_delay = nframes_delay;};
	void set_frames_on(int nframes_on) { m_nframes_on = nframes_on;};
	int get_frames_fixpt_delay() { return m_nframes_fixpt_delay; };

	void set_fixpt(ARFixationPointSpec& fixpt);
	void set_fixpt(ARFixationPointSpec& fixpt, double xoffset, double yoffset);
	void set_xhair(ARXhairSpec& xhair);
	void set_xhair(ARXhairSpec& xhair, double xoffset, double yoffset);
	void set_grating(ARGratingSpec& grating);
	void set_grating(ARGratingSpec& grating, double xoffset, double yoffset);

	virtual bool has_fixpt() const { return m_have_fixpt; };
	virtual bool has_xhair() const { return m_have_xhair; };
	virtual bool has_grating() const { return m_have_grating; };
	virtual bool is_empty() const { return !(m_have_fixpt || m_have_xhair || m_have_grating); };
	ARContrastFixationPointSpec& fixpt() { return m_fixpt; };
	const ARContrastFixationPointSpec& fixpt() const { return m_fixpt; };
	ARXhairSpec& xhair() { return m_xhair; };
	ARGratingSpec& grating() { return m_grating; };

	void advance()
	{
		if (m_have_grating)
		{
			for (ptr_vector<StimParameterList>::iterator it = begin(); it != end(); it++)
			{
				it->advance(m_grating);
			}
		}
		return;
	}

	void set_initial_parameters()
	{
		if (m_have_grating)
		{
			for (ptr_vector<StimParameterList>::iterator it = begin(); it != end(); it++)
			{
				it->set_current_parameter(m_grating);
			}
		}
		return;
	}

	int init(ARvsg& vsg, int nframes_delay, double dStimTime, int nframes_fixpt_delay)
	{
		vsg.select();
		m_nframes_on = (int)(dStimTime * 1000000 /(double)vsgGetSystemAttribute(vsgFRAMETIME));
		m_nframes_delay = nframes_delay;
		m_nframes_fixpt_delay = nframes_fixpt_delay;
		return init(vsg);
	}
	int init(ARvsg& vsg);
	int setup_cycling(int firstpage, int stimpage, int lastpage);
	int setup_fixpt_cycling(int firstpage, int fixptpage);


	// handle the trigger indicated by the string s. Do not call vsgPresent! return value of 
	// 1 means vsgPresent() will be called. 
	virtual int handle_trigger(std::string& s);

	//virtual std::string toString();

};

std::ostream& operator<<(std::ostream& out, const StimSet& sset);


