#include "alertlib.h"
#include <vector>
#include <cassert>

using namespace alert;
using namespace std;

// This class represents a list of parameter values over which a grating may vary. 
// The advance() method steps to the next value in the list and sets it in the grating.
// The set_current_parameter() method is an initialization - it sets the grating to the
// first value in the list.

class StimParameterList
{
public:
	StimParameterList() {};
	virtual ~StimParameterList() {};
	virtual ARGratingSpec& advance(ARGratingSpec& donut) = 0;
	virtual ARGratingSpec& set_current_parameter(ARGratingSpec& grating) = 0;
	virtual StimParameterList* clone() const = 0;
};

// Parameter value list for contrast

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


// Parameter value list for orientation

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

// Parameter value list for area (list of diameters)

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

// Parameter value list for spatial frequency

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

// Parameter value list for temporal frequency

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

// Parameter value list for donuts. Input is a list of diameter pairs: od1,id1,od2,id2,... where
// od is outer diameter and id is inner diameter. Setting id to 0 yields a grating 
// with no hole. 

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

// Parameter value list for x position

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

// Parameter value list for y position

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


class StimXYList: public StimParameterList
{
public:
	StimXYList(vector<double> xys) : StimParameterList()
	{
		init_xys(xys);
		m_iter = m_vec.begin(); 
	};
	StimXYList(const StimXYList& list) : StimParameterList(), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimXYList() {};
	virtual StimXYList *clone() const
	{
		return new StimXYList(*this);
	}

	virtual ARGratingSpec& advance(ARGratingSpec& grating)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(grating);
	}

	virtual ARGratingSpec& set_current_parameter(ARGratingSpec& grating)
	{
		grating.x = m_iter->first;
		grating.y = m_iter->second;
		return grating;
	}

private:
	void init_xys(vector<double>xys)
	{
		double x, y;
		std::vector<double>::const_iterator iter = xys.begin();
		for (; iter != xys.end(); iter++)
		{
			x = *iter;
			iter++;
			assert(iter != xys.end());
			y = *iter;
			m_vec.push_back(make_pair(x, y));
		}
		return;
	}
	vector< pair<double, double> > m_vec;
	vector< pair<double, double> >::const_iterator m_iter;
};
