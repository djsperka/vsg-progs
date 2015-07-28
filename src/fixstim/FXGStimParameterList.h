#ifndef _STIMPARAMETERLIST_H_
#define _STIMPARAMETERLIST_H_

#include "alertlib.h"
#include "StimSet.h"
#include <vector>
#include <cassert>

using namespace alert;
using namespace std;

// This class represents a list of parameter values over which a grating may vary. 
// The advance() method steps to the next value in the list and sets it in the grating.
// The set_current_parameter() method is an initialization - it sets the grating to the
// first value in the list.

class FXGStimParameterList
{
public:
	FXGStimParameterList() {};
	virtual ~FXGStimParameterList() {};
	virtual bool advance(FXGStimSet* pstimset) = 0;
	virtual bool set_current_parameter(FXGStimSet* pstimset) = 0;
	virtual FXGStimParameterList* clone() const = 0;
};

// Parameter value list for contrast

class StimContrastList: public FXGStimParameterList
{
public:
	StimContrastList(vector<double> contrasts) : FXGStimParameterList(), m_vec(contrasts) { m_iter = m_vec.begin(); };
	StimContrastList(const StimContrastList& list) : FXGStimParameterList(), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}

	virtual ~StimContrastList() {};
	virtual StimContrastList *clone() const
	{
		return new StimContrastList(*this);
	}
	virtual bool advance(FXGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(FXGStimSet* pstimset)
	{
		// Tell the stim set to save this contrast as its "good" contrast.
		pstimset->setSavedContrast((int)*m_iter);

		// and set the contrast.
		pstimset->grating().setContrast((int)*m_iter);
		return true;
	}

private:
	vector<double> m_vec;
	vector<double>::const_iterator m_iter;
};


// Parameter value list for orientation
class StimOrientationList: public FXGStimParameterList
{
public:
	StimOrientationList(vector<double> ori) : FXGStimParameterList(), m_vec(ori) { m_iter = m_vec.begin(); };
	StimOrientationList(const StimOrientationList& list) : FXGStimParameterList(), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimOrientationList() {};
	virtual StimOrientationList *clone() const
	{
		return new StimOrientationList(*this);
	}
	virtual bool advance(FXGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(FXGStimSet* pstimset)
	{
		pstimset->grating().orientation = *m_iter;
		return true;
	}

private:
	vector<double> m_vec;
	vector<double>::const_iterator m_iter;
};

// Parameter value list for area (list of diameters)

class StimAreaList: public FXGStimParameterList
{
public:
	StimAreaList(vector<double> areas) : FXGStimParameterList(), m_vec(areas) { m_iter = m_vec.begin(); };
	StimAreaList(const StimAreaList& list) : FXGStimParameterList(), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimAreaList() {};
	virtual StimAreaList *clone() const
	{
		return new StimAreaList(*this);
	}
	virtual bool advance(FXGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(FXGStimSet* pstimset)
	{
		pstimset->grating().w = pstimset->grating().h = *m_iter;
		return true;
	}

private:
	vector<double> m_vec;
	vector<double>::const_iterator m_iter;
};

// Parameter value list for spatial frequency

class StimSFList: public FXGStimParameterList
{
public:
	StimSFList(vector<double> sfs) : FXGStimParameterList(), m_vec(sfs) { m_iter = m_vec.begin(); };
	StimSFList(const StimSFList& list) : FXGStimParameterList(), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimSFList() {};
	virtual StimSFList *clone() const
	{
		return new StimSFList(*this);
	}
	virtual bool advance(FXGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(FXGStimSet* pstimset)
	{
		pstimset->grating().sf = *m_iter;
		return true;
	}

private:
	vector<double> m_vec;
	vector<double>::const_iterator m_iter;
};

// Parameter value list for temporal frequency

class StimTFList: public FXGStimParameterList
{
public:
	StimTFList(vector<double> tfs) : FXGStimParameterList(), m_vec(tfs) { m_iter = m_vec.begin(); };
	StimTFList(const StimTFList& list) : FXGStimParameterList(), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimTFList() {};
	virtual StimTFList *clone() const
	{
		return new StimTFList(*this);
	}
	virtual bool advance(FXGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(FXGStimSet* pstimset)
	{
		pstimset->grating().select();
		pstimset->grating().setTemporalFrequency(*m_iter);
		return true;
	}

private:
	vector<double> m_vec;
	vector<double>::const_iterator m_iter;
};

// Parameter value list for donuts. Input is a list of diameter pairs: od1,id1,od2,id2,... where
// od is outer diameter and id is inner diameter. Setting id to 0 yields a grating 
// with no hole. 

class StimHoleList: public FXGStimParameterList
{
public:
	StimHoleList(vector<double> diams) : FXGStimParameterList()
	{
		init_diameters(diams);
		m_iter = m_vec.begin(); 
	};
	StimHoleList(const StimHoleList& list) : FXGStimParameterList(), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimHoleList() {};
	virtual StimHoleList *clone() const
	{
		return new StimHoleList(*this);
	}

	virtual bool advance(FXGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(FXGStimSet* pstimset)
	{
		pstimset->grating().w = pstimset->grating().h = m_iter->first;
		pstimset->grating().wd = pstimset->grating().hd = m_iter->second;
		return true;
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

class StimXList: public FXGStimParameterList
{
public:
	StimXList(vector<double> xs) : FXGStimParameterList(), m_vec(xs) { m_iter = m_vec.begin(); };
	StimXList(const StimXList& list) : FXGStimParameterList(), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimXList() {};
	virtual StimXList *clone() const
	{
		return new StimXList(*this);
	}
	virtual bool advance(FXGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(FXGStimSet* pstimset)
	{
		pstimset->grating().x = *m_iter;
		return true;
	}

private:
	vector<double> m_vec;
	vector<double>::const_iterator m_iter;
};

// Parameter value list for y position

class StimYList: public FXGStimParameterList
{
public:
	StimYList(vector<double> ys) : FXGStimParameterList(), m_vec(ys) { m_iter = m_vec.begin(); };
	StimYList(const StimYList& list) : FXGStimParameterList(), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimYList() {};
	virtual StimYList *clone() const
	{
		return new StimYList(*this);
	}
	virtual bool advance(FXGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(FXGStimSet* pstimset)
	{
		pstimset->grating().y = *m_iter;
		return true;
	}

private:
	vector<double> m_vec;
	vector<double>::const_iterator m_iter;
};


class StimXYList: public FXGStimParameterList
{
public:
	StimXYList(vector<double> xys) : FXGStimParameterList()
	{
		init_xys(xys);
		m_iter = m_vec.begin(); 
	};

	StimXYList(const vector <pair <double, double> >& xys) : m_vec(xys)
	{
		m_iter = m_vec.begin();
	}

	StimXYList(const StimXYList& list) : FXGStimParameterList(), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}

	virtual ~StimXYList() {};
	virtual StimXYList *clone() const = 0;

	virtual bool advance(FXGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(FXGStimSet* pstimset) = 0;

	const vector< pair<double, double> >& getXY() const { return m_vec; };

protected:
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

// Parameter value list for grating initial phase

class StimPhaseList: public FXGStimParameterList
{
public:
	StimPhaseList(vector<double> phases) : FXGStimParameterList(), m_vec(phases) { m_iter = m_vec.begin(); };
	StimPhaseList(const StimPhaseList& list) : FXGStimParameterList(), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimPhaseList() {};
	virtual StimPhaseList *clone() const
	{
		return new StimPhaseList(*this);
	}
	virtual bool advance(FXGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(FXGStimSet* pstimset)
	{
		pstimset->grating().phase = *m_iter;
		return true;
	}

private:
	vector<double> m_vec;
	vector<double>::const_iterator m_iter;
};




class GratingXYList: public StimXYList
{
public:
	GratingXYList(vector<double> xys) : StimXYList(xys)
	{
	};

	GratingXYList(const GratingXYList& list) : StimXYList(list.getXY())
	{
	}

	virtual ~GratingXYList() {};
	virtual GratingXYList *clone() const
	{
		return new GratingXYList(*this);
	}

	virtual bool set_current_parameter(FXGStimSet* pstimset)
	{
		pstimset->grating().x = m_iter->first;
		pstimset->grating().y = m_iter->second;
		return true;
	}
};



class FixptXYList: public StimXYList
{
public:
	FixptXYList(vector<double> xys) : StimXYList(xys)
	{
	};

	FixptXYList(const FixptXYList& list) : StimXYList(list.getXY())
	{
		m_iter = m_vec.begin();
	}
	virtual ~FixptXYList() {};
	virtual FixptXYList *clone() const
	{
		return new FixptXYList(*this);
	}

	virtual bool set_current_parameter(FXGStimSet* pstimset)
	{
		if (pstimset->has_fixpt())
		{
			pstimset->fixpt().x = m_iter->first;
			pstimset->fixpt().y = m_iter->second;
		}
		return true;
	}
};




#endif
