#include "alertlib.h"
#include <vector>
#include <string>
#include <boost/ptr_container/ptr_vector.hpp>

using namespace std;
using namespace alert;
using namespace boost;


class StimParameterList
{
public:
	StimParameterList() {};
	virtual ~StimParameterList() {};
	virtual ARGratingSpec& advance(ARGratingSpec& donut) = 0;
	virtual ARGratingSpec& set_current_parameter(ARGratingSpec& grating) = 0;
	virtual StimParameterList* clone() const = 0;
};

class StimContrastList: public StimParameterList
{
public:
	StimContrastList(vector<double> contrasts) : StimParameterList(), m_vec(contrasts) { m_iter = m_vec.begin(); };
	StimContrastList(const StimContrastList& list) : StimParameterList(), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}

	virtual ~StimContrastList() {};
	virtual StimContrastList *clone() const
	{
		return new StimContrastList(*this);
	}
	virtual ARGratingSpec& advance(ARGratingSpec& grating)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(grating);
	}

	virtual ARGratingSpec& set_current_parameter(ARGratingSpec& grating)
	{
		grating.setContrast((int)*m_iter);
		return grating;
	}

private:
	vector<double> m_vec;
	vector<double>::const_iterator m_iter;
};

class StimOrientationList: public StimParameterList
{
public:
	StimOrientationList(vector<double> ori) : StimParameterList(), m_vec(ori) { m_iter = m_vec.begin(); };
	StimOrientationList(const StimOrientationList& list) : StimParameterList(), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimOrientationList() {};
	virtual StimOrientationList *clone() const
	{
		return new StimOrientationList(*this);
	}
	virtual ARGratingSpec& advance(ARGratingSpec& grating)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(grating);
	}

	virtual ARGratingSpec& set_current_parameter(ARGratingSpec& grating)
	{
		grating.orientation = *m_iter;
		return grating;
	}

private:
	vector<double> m_vec;
	vector<double>::const_iterator m_iter;
};

class StimAreaList: public StimParameterList
{
public:
	StimAreaList(vector<double> areas) : StimParameterList(), m_vec(areas) { m_iter = m_vec.begin(); };
	StimAreaList(const StimAreaList& list) : StimParameterList(), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimAreaList() {};
	virtual StimAreaList *clone() const
	{
		return new StimAreaList(*this);
	}
	virtual ARGratingSpec& advance(ARGratingSpec& grating)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(grating);
	}

	virtual ARGratingSpec& set_current_parameter(ARGratingSpec& grating)
	{
		grating.w = grating.h = *m_iter;
		return grating;
	}

private:
	vector<double> m_vec;
	vector<double>::const_iterator m_iter;
};

class StimSFList: public StimParameterList
{
public:
	StimSFList(vector<double> sfs) : StimParameterList(), m_vec(sfs) { m_iter = m_vec.begin(); };
	StimSFList(const StimSFList& list) : StimParameterList(), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimSFList() {};
	virtual StimSFList *clone() const
	{
		return new StimSFList(*this);
	}
	virtual ARGratingSpec& advance(ARGratingSpec& grating)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(grating);
	}

	virtual ARGratingSpec& set_current_parameter(ARGratingSpec& grating)
	{
		grating.sf = *m_iter;
		return grating;
	}

private:
	vector<double> m_vec;
	vector<double>::const_iterator m_iter;
};

class StimTFList: public StimParameterList
{
public:
	StimTFList(vector<double> tfs) : StimParameterList(), m_vec(tfs) { m_iter = m_vec.begin(); };
	StimTFList(const StimTFList& list) : StimParameterList(), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimTFList() {};
	virtual StimTFList *clone() const
	{
		return new StimTFList(*this);
	}
	virtual ARGratingSpec& advance(ARGratingSpec& grating)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(grating);
	}

	virtual ARGratingSpec& set_current_parameter(ARGratingSpec& grating)
	{
		grating.setTemporalFrequency(*m_iter);
		return grating;
	}

private:
	vector<double> m_vec;
	vector<double>::const_iterator m_iter;
};

class StimHoleList: public StimParameterList
{
public:
	StimHoleList(vector<double> diams) : StimParameterList()
	{
		init_diameters(diams);
		m_iter = m_vec.begin(); 
	};
	StimHoleList(const StimHoleList& list) : StimParameterList(), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimHoleList() {};
	virtual StimHoleList *clone() const
	{
		return new StimHoleList(*this);
	}

	virtual ARGratingSpec& advance(ARGratingSpec& grating)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(grating);
	}

	virtual ARGratingSpec& set_current_parameter(ARGratingSpec& grating)
	{
		grating.w = grating.h = m_iter->first;
		grating.wd = grating.hd = m_iter->second;
		return grating;
	}

private:
	void init_diameters(vector<double>diams)
	{
		double d1, d2;
		std::vector<double>::const_iterator iter = diams.begin();
		for (; iter != diams.end(); iter++)
		{
			d1 = *iter;
			iter++;
			assert(iter != diams.end());
			d2 = *iter;
			m_vec.push_back(make_pair(d1, d2));
		}
		return;
	}
	vector< pair<double, double> > m_vec;
	vector< pair<double, double> >::const_iterator m_iter;
};


class StimXList: public StimParameterList
{
public:
	StimXList(vector<double> xs) : StimParameterList(), m_vec(xs) { m_iter = m_vec.begin(); };
	StimXList(const StimXList& list) : StimParameterList(), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimXList() {};
	virtual StimXList *clone() const
	{
		return new StimXList(*this);
	}
	virtual ARGratingSpec& advance(ARGratingSpec& grating)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(grating);
	}

	virtual ARGratingSpec& set_current_parameter(ARGratingSpec& grating)
	{
		grating.x = *m_iter;
		return grating;
	}

private:
	vector<double> m_vec;
	vector<double>::const_iterator m_iter;
};

class StimYList: public StimParameterList
{
public:
	StimYList(vector<double> ys) : StimParameterList(), m_vec(ys) { m_iter = m_vec.begin(); };
	StimYList(const StimYList& list) : StimParameterList(), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimYList() {};
	virtual StimYList *clone() const
	{
		return new StimYList(*this);
	}
	virtual ARGratingSpec& advance(ARGratingSpec& grating)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(grating);
	}

	virtual ARGratingSpec& set_current_parameter(ARGratingSpec& grating)
	{
		grating.y = *m_iter;
		return grating;
	}

private:
	vector<double> m_vec;
	vector<double>::const_iterator m_iter;
};


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



#if 0
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


#endif