#include "alertlib.h"
#include <vector>
#include <string>

using namespace std;
using namespace alert;



class DualStimSet
{
private:
public:
	DualStimSet() {};
	virtual ~DualStimSet() {};	// TODO: clean up vsg objects here?

	// Initialize vsg objects for master/slave. Assume that the vsg cards have been 
	// init'd and that all pages have been cleared to background. 
	virtual int init(ARvsg& master, ARvsg& slave) = 0;
	virtual int cleanup(ARvsg& master, ARvsg& slave) = 0;

	// Initialize triggers for this stim. 
	virtual int init_triggers(TriggerVector& triggers) = 0;

	// Dump something relevant for verbosity's sake
	virtual std::string toString() const;
};

std::ostream& operator<<(std::ostream& out, const DualStimSet& dsset);



class NullDualStimSet: public DualStimSet
{
public:
	NullDualStimSet(): DualStimSet() {};
	virtual ~NullDualStimSet() {};
	virtual int init(ARvsg& master, ARvsg& slave);
	virtual int cleanup(ARvsg& master, ARvsg& slave);
	virtual int init_triggers(TriggerVector& triggers);
	virtual std::string toString() const;
};



class StimSet
{
private:
	double m_spatialphase;
	int m_nframes_on;
	int m_nframes_delay;
	int m_nframes_fixpt_delay;
	ARContrastFixationPointSpec m_fixpt;
	ARXhairSpec m_xhair;
	bool m_have_fixpt;
	bool m_have_xhair;
public:
	StimSet() : m_spatialphase(0), m_have_fixpt(false), m_have_xhair(false) {};
	StimSet(double spph) : m_spatialphase(spph), m_have_fixpt(false), m_have_xhair(false) {};
	StimSet(ARContrastFixationPointSpec& fixpt, double spph=0) : m_have_xhair(false), m_have_fixpt(true), m_fixpt(fixpt), m_spatialphase(spph) {};
	StimSet(ARContrastFixationPointSpec& fixpt, ARXhairSpec& xhair, double spph=0) : m_have_xhair(true), m_have_fixpt(true), m_fixpt(fixpt), m_xhair(xhair), m_spatialphase(spph) {};

	virtual ~StimSet() {};

	double get_spatial_phase() { return m_spatialphase; };
	int get_frames_on() { return m_nframes_on; };
	int get_frames_delay() { return m_nframes_delay; };
	void set_frames_delay(int nframes_delay) { m_nframes_delay = nframes_delay;};
	void set_frames_on(int nframes_on) { m_nframes_on = nframes_on;};
	int get_frames_fixpt_delay() { return m_nframes_fixpt_delay; };

	virtual bool has_fixpt() const { return m_have_fixpt; };
	virtual bool has_xhair() const { return m_have_xhair; };
	virtual bool has_grating() const { return false; };		// most classes will want to override this
	ARContrastFixationPointSpec& fixpt() { return m_fixpt; };
	const ARContrastFixationPointSpec& fixpt() const { return m_fixpt; };
	ARXhairSpec& xhair() { return m_xhair; };

	int init(ARvsg& vsg, int nframes_delay, int nframes_on, int nframes_fixpt_delay)
	{
		m_nframes_on = nframes_on;
		m_nframes_delay = nframes_delay;
		m_nframes_fixpt_delay = nframes_fixpt_delay;
		return init(vsg);
	}
	int setup_cycling(int firstpage, int stimpage, int lastpage);
	int setup_fixpt_cycling(int firstpage, int fixptpage);


	virtual int init(ARvsg& vsg) = 0;

	// handle the trigger indicated by the string s. Do not call vsgPresent! return value of 
	// 1 means vsgPresent() will be called. 
	virtual int handle_trigger(std::string& s) = 0;

	virtual std::string toString() const = 0;

};

std::ostream& operator<<(std::ostream& out, const StimSet& sset);


class NullStimSet: public StimSet
{
public:
	NullStimSet(): StimSet() {};
	virtual ~NullStimSet() {};
	virtual int init(ARvsg& vsg) {return 0;};
	virtual int handle_trigger(std::string& s) {return 0;};		// will not trigger?!?
	virtual std::string toString() const;
};

// This stim set can take a fixpt and grating, or just a fixpt. 
class FixptGratingStimSet: public StimSet
{
public:
	FixptGratingStimSet(alert::ARContrastFixationPointSpec& f, alert::ARGratingSpec& g, double spatialphase=0) : StimSet(f, spatialphase), m_grating(g), m_bHaveGrating(true) {};
	FixptGratingStimSet(alert::ARContrastFixationPointSpec& f, double spatialphase=0) : StimSet(f, spatialphase), m_bHaveGrating(false) {};
	FixptGratingStimSet(alert::ARContrastFixationPointSpec& f, alert::ARXhairSpec& x, alert::ARGratingSpec& g, double spatialphase=0) : StimSet(f, x, spatialphase), m_grating(g), m_bHaveGrating(true) {};
	FixptGratingStimSet(alert::ARContrastFixationPointSpec& f, alert::ARXhairSpec& x, double spatialphase=0) : StimSet(f, x, spatialphase), m_bHaveGrating(false) {};
	FixptGratingStimSet(alert::ARGratingSpec& g, double spatialphase=0) : StimSet(spatialphase), m_grating(g),  m_bHaveGrating(true) {};
	virtual ~FixptGratingStimSet() {};
	virtual int init(ARvsg& vsg);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
	virtual bool has_grating() const { return m_bHaveGrating; };
private:
	bool m_bHaveGrating;
	int m_contrast;
	alert::ARGratingSpec m_grating;
};

class ContrastStimSet: public StimSet
{
public:
	ContrastStimSet(alert::ARContrastFixationPointSpec& f, alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(f, spatialphase), m_grating(g), m_contrasts(parameters) {};
	ContrastStimSet(alert::ARContrastFixationPointSpec& f, alert::ARXhairSpec& x, alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(f, x, spatialphase), m_grating(g), m_contrasts(parameters) {};
	ContrastStimSet(alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(spatialphase), m_grating(g), m_contrasts(parameters) {};
	virtual int init(ARvsg& vsg);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
	virtual bool has_grating() const { return true; };
private:
	int m_contrast;
	std::vector<double> m_contrasts;
	std::vector<double>::const_iterator m_iterator;
	alert::ARGratingSpec m_grating;
};

class TFStimSet: public StimSet
{
public:
	TFStimSet(alert::ARContrastFixationPointSpec& f, alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(f, spatialphase), m_grating(g), m_temporal_frequencies(parameters) {};
	TFStimSet(alert::ARContrastFixationPointSpec& f, alert::ARXhairSpec& x, alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(f, x, spatialphase), m_grating(g), m_temporal_frequencies(parameters) {};
	TFStimSet(alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(spatialphase), m_grating(g), m_temporal_frequencies(parameters) {};
	virtual int init(ARvsg& vsg);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
	virtual bool has_grating() const { return true; };
private:
	int m_contrast;
	std::vector<double> m_temporal_frequencies;
	std::vector<double>::const_iterator m_iterator;
	alert::ARGratingSpec m_grating;
};


class SFStimSet: public StimSet
{
public:
	SFStimSet(alert::ARContrastFixationPointSpec& f, alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(f, spatialphase), m_grating(g), m_spatial_frequencies(parameters) {};
	SFStimSet(alert::ARContrastFixationPointSpec& f, alert::ARXhairSpec& x, alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(f, x, spatialphase), m_grating(g), m_spatial_frequencies(parameters) {};
	SFStimSet(alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(spatialphase), m_grating(g), m_spatial_frequencies(parameters) {};
	virtual int init(ARvsg& vsg);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
	virtual bool has_grating() const { return true; };
private:
	int m_contrast;
	std::vector<double> m_spatial_frequencies;
	std::vector<double>::const_iterator m_iterator;
	alert::ARGratingSpec m_grating;
};

class OrientationStimSet: public StimSet
{
public:
	OrientationStimSet(alert::ARContrastFixationPointSpec& f, alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(f, spatialphase), m_grating(g), m_orientations(parameters) {};
	OrientationStimSet(alert::ARContrastFixationPointSpec& f, alert::ARXhairSpec& x, alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(f, x, spatialphase), m_grating(g), m_orientations(parameters) {};
	OrientationStimSet(alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(spatialphase), m_grating(g), m_orientations(parameters) {};
	virtual int init(ARvsg& vsg);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
	virtual bool has_grating() const { return true; };
private:
	int m_contrast;
	std::vector<double> m_orientations;
	std::vector<double>::const_iterator m_iterator;
	alert::ARGratingSpec m_grating;
};

class AreaStimSet: public StimSet
{
public:
	AreaStimSet(alert::ARContrastFixationPointSpec& f, alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(f, spatialphase), m_grating(g), m_diameters(parameters) {};
	AreaStimSet(alert::ARContrastFixationPointSpec& f, alert::ARXhairSpec& x, alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(f, x, spatialphase), m_grating(g), m_diameters(parameters) {};
	AreaStimSet(alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(spatialphase), m_grating(g), m_diameters(parameters) {};
	virtual int init(ARvsg& vsg);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
	virtual bool has_grating() const { return true; };
private:
	int m_contrast;
	std::vector<double> m_diameters;
	std::vector<double>::const_iterator m_iterator;
	alert::ARGratingSpec m_grating;
};

class DonutStimSet: public StimSet
{
public:
	DonutStimSet(alert::ARContrastFixationPointSpec& f, alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(f, spatialphase), m_grating(g) {init_diameters(parameters);};
	DonutStimSet(alert::ARContrastFixationPointSpec& f, alert::ARXhairSpec& x, alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(f, x, spatialphase), m_grating(g) {init_diameters(parameters);};
	DonutStimSet(alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(spatialphase), m_grating(g) {init_diameters(parameters);};
	virtual int init(ARvsg& vsg);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
	virtual bool has_grating() const { return true; };
private:
	int init_diameters(std::vector<double>diams);
	int m_contrast;
	std::vector<std::pair<double,double>> m_diameters;
	std::vector<std::pair<double,double>>::const_iterator m_iterator;
	alert::ARDonutSpec m_grating;
};


