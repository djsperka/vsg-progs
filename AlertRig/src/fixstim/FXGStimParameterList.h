#ifndef _STIMPARAMETERLIST_H_
#define _STIMPARAMETERLIST_H_

#include "alertlib.h"
#include "MultiParameterFXMultiGStimSet.h"
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
	unsigned  int m_gratingIndex;
	bool m_bDistractor;
public:
	FXGStimParameterList(unsigned int index=0, bool bDistractor=false): m_gratingIndex(index), m_bDistractor(bDistractor) {};
	virtual ~FXGStimParameterList() {};
	virtual bool advance(MultiParameterFXMultiGStimSet* pstimset) = 0;
	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset) = 0;
	virtual FXGStimParameterList* clone() const = 0;
	unsigned int index() const { return m_gratingIndex; };
	bool isDistractor() const { return m_bDistractor; };
};

// Parameter value list for contrast

class StimContrastList: public FXGStimParameterList
{
public:
	StimContrastList(vector<double> contrasts, unsigned int index, bool bDistractor) : FXGStimParameterList(index, bDistractor), m_vec(contrasts) { m_iter = m_vec.begin(); };
	StimContrastList(const StimContrastList& list) : FXGStimParameterList(list.index(), list.isDistractor()), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}

	virtual ~StimContrastList() {};
	virtual StimContrastList *clone() const
	{
		return new StimContrastList(*this);
	}
	virtual bool advance(MultiParameterFXMultiGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset)
	{
		// Tell the stim set to save this contrast as its "good" contrast.
		pstimset->setSavedContrast((int)*m_iter, index(), isDistractor());

		// and set the contrast.
		if (isDistractor())
			pstimset->distractor(index()).setContrast((int)*m_iter);
		else
			pstimset->grating(index()).setContrast((int)*m_iter);
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
	StimOrientationList(vector<double> ori, unsigned int index, bool bDistractor) : FXGStimParameterList(index, bDistractor), m_vec(ori) { m_iter = m_vec.begin(); };
	StimOrientationList(const StimOrientationList& list) : FXGStimParameterList(list.index(), list.isDistractor()), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimOrientationList() {};
	virtual StimOrientationList *clone() const
	{
		return new StimOrientationList(*this);
	}
	virtual bool advance(MultiParameterFXMultiGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset)
	{
		if (!isDistractor())
			pstimset->grating(index()).orientation = *m_iter;
		else
			pstimset->distractor(index()).orientation = *m_iter;
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
	StimAreaList(vector<double> areas, unsigned int index, bool bDistractor) : FXGStimParameterList(index, bDistractor), m_vec(areas) { m_iter = m_vec.begin(); };
	StimAreaList(const StimAreaList& list) : FXGStimParameterList(list.index(), list.isDistractor()), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimAreaList() {};
	virtual StimAreaList *clone() const
	{
		return new StimAreaList(*this);
	}
	virtual bool advance(MultiParameterFXMultiGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset)
	{
		if (!isDistractor())
			pstimset->grating(index()).w = pstimset->grating(index()).h = *m_iter;
		else
			pstimset->distractor(index()).w = pstimset->distractor(index()).h = *m_iter;

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
	StimSFList(vector<double> sfs, unsigned int index, bool bDistractor) : FXGStimParameterList(index, bDistractor), m_vec(sfs) { m_iter = m_vec.begin(); };
	StimSFList(const StimSFList& list) : FXGStimParameterList(list.index(), list.isDistractor()), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimSFList() {};
	virtual StimSFList *clone() const
	{
		return new StimSFList(*this);
	}
	virtual bool advance(MultiParameterFXMultiGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset)
	{
		if (!isDistractor())
			pstimset->grating(index()).sf = *m_iter;
		else
			pstimset->distractor(index()).sf = *m_iter;

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
	StimTFList(vector<double> tfs, unsigned int index, bool bDistractor) : FXGStimParameterList(index, bDistractor), m_vec(tfs) { m_iter = m_vec.begin(); };
	StimTFList(const StimTFList& list) : FXGStimParameterList(list.index(), list.isDistractor()), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimTFList() {};
	virtual StimTFList *clone() const
	{
		return new StimTFList(*this);
	}
	virtual bool advance(MultiParameterFXMultiGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset)
	{
		if (!isDistractor())
		{
			pstimset->grating(index()).select();
			pstimset->grating(index()).setTemporalFrequency(*m_iter);
		}
		else
		{
			pstimset->distractor(index()).select();
			pstimset->distractor(index()).setTemporalFrequency(*m_iter);
		}
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
	StimHoleList(vector<double> diams, unsigned int index, bool bDistractor) : FXGStimParameterList(index, bDistractor)
	{
		init_diameters(diams);
		m_iter = m_vec.begin(); 
	};
	StimHoleList(const StimHoleList& list) : FXGStimParameterList(list.index(), list.isDistractor()), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimHoleList() {};
	virtual StimHoleList *clone() const
	{
		return new StimHoleList(*this);
	}

	virtual bool advance(MultiParameterFXMultiGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset)
	{
		if (!isDistractor())
		{
			pstimset->grating(index()).w = pstimset->grating(index()).h = m_iter->first;
			pstimset->grating(index()).wd = pstimset->grating(index()).hd = m_iter->second;
		}
		else
		{
			pstimset->distractor(index()).w = pstimset->grating(index()).h = m_iter->first;
			pstimset->distractor(index()).wd = pstimset->grating(index()).hd = m_iter->second;
		}
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
	StimXList(vector<double> xs, unsigned int index, bool bDistractor) : FXGStimParameterList(index, bDistractor), m_vec(xs) { m_iter = m_vec.begin(); };
	StimXList(const StimXList& list) : FXGStimParameterList(list.index(), list.isDistractor()), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimXList() {};
	virtual StimXList *clone() const
	{
		return new StimXList(*this);
	}
	virtual bool advance(MultiParameterFXMultiGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset)
	{
		if (!isDistractor())
			pstimset->grating(index()).x = *m_iter;
		else
			pstimset->distractor(index()).x = *m_iter;

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
	StimYList(vector<double> ys, unsigned int index, bool bDistractor) : FXGStimParameterList(index, bDistractor), m_vec(ys) { m_iter = m_vec.begin(); };
	StimYList(const StimYList& list) : FXGStimParameterList(list.index(), list.isDistractor()), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimYList() {};
	virtual StimYList *clone() const
	{
		return new StimYList(*this);
	}
	virtual bool advance(MultiParameterFXMultiGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset)
	{
		if (!isDistractor())
			pstimset->grating(index()).y = *m_iter;
		else
			pstimset->distractor(index()).y = *m_iter;
		return true;
	}

private:
	vector<double> m_vec;
	vector<double>::const_iterator m_iter;
};


class StimXYList: public FXGStimParameterList
{
public:
	StimXYList(vector<double> xys, unsigned int index, bool bDistractor) : FXGStimParameterList(index)
	{
		init_xys(xys);
		m_iter = m_vec.begin(); 
	};

	StimXYList(const vector <pair <double, double> >& xys, unsigned int index, bool bDistractor) : FXGStimParameterList(index, bDistractor), m_vec(xys)
	{
		m_iter = m_vec.begin();
	}

	StimXYList(const StimXYList& list) : FXGStimParameterList(list.index(), list.isDistractor()), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}

	virtual ~StimXYList() {};
	virtual StimXYList *clone() const = 0;

	virtual bool advance(MultiParameterFXMultiGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset) = 0;

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
	StimPhaseList(vector<double> phases, unsigned int index, bool bDistractor) : FXGStimParameterList(index, bDistractor), m_vec(phases) { m_iter = m_vec.begin(); };
	StimPhaseList(const StimPhaseList& list) : FXGStimParameterList(list.index(), list.isDistractor()), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimPhaseList() {};
	virtual StimPhaseList *clone() const
	{
		return new StimPhaseList(*this);
	}
	virtual bool advance(MultiParameterFXMultiGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset)
	{
		if (!isDistractor())
			pstimset->grating(index()).phase = *m_iter;
		else
			pstimset->distractor(index()).phase = *m_iter;
		return true;
	}

private:
	vector<double> m_vec;
	vector<double>::const_iterator m_iter;
};


class StimTTFList : public FXGStimParameterList
{
public:
	StimTTFList(vector<double> ttfs, unsigned int index, bool bDistractor) : FXGStimParameterList(index, bDistractor), m_vec(ttfs) { m_iter = m_vec.begin(); };
	StimTTFList(const StimTTFList& list) : FXGStimParameterList(list.index(), list.isDistractor()), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimTTFList() {};
	virtual StimTTFList *clone() const
	{
		return new StimTTFList(*this);
	}
	virtual bool advance(MultiParameterFXMultiGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset)
	{
		if (!isDistractor())
			pstimset->grating(index()).ttf = *m_iter;
		else
			pstimset->distractor(index()).ttf = *m_iter;
		return true;
	}

private:
	vector<double> m_vec;
	vector<double>::const_iterator m_iter;
};





class GratingXYList: public StimXYList
{
public:
	GratingXYList(vector<double> xys, unsigned int index, bool bDistractor) : StimXYList(xys, index, bDistractor)
	{
	};

	GratingXYList(const GratingXYList& list) : StimXYList(list.getXY(), list.index(), list.isDistractor())
	{
	}

	virtual ~GratingXYList() {};
	virtual GratingXYList *clone() const
	{
		return new GratingXYList(*this);
	}

	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset)
	{
		if (!isDistractor())
		{
			pstimset->grating(index()).x = m_iter->first;
			pstimset->grating(index()).y = m_iter->second;
		}
		else
		{
			pstimset->distractor(index()).x = m_iter->first;
			pstimset->distractor(index()).y = m_iter->second;
		}
		return true;
	}
};



class FixptXYList: public StimXYList
{
public:
	FixptXYList(vector<double> xys) : StimXYList(xys, -1, false)
	{
	};

	FixptXYList(const FixptXYList& list) : StimXYList(list.getXY(), -1, false)
	{
		m_iter = m_vec.begin();
	}
	virtual ~FixptXYList() {};
	virtual FixptXYList *clone() const
	{
		return new FixptXYList(*this);
	}

	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset)
	{
		if (pstimset->has_fixpt())
		{
			pstimset->fixpt().x = m_iter->first;
			pstimset->fixpt().y = m_iter->second;
		}
		return true;
	}
};



class FixptColorList : public FXGStimParameterList
{
public:
	FixptColorList(vector<COLOR_TYPE> colors) : FXGStimParameterList(-1), m_vec(colors) { m_iter = m_vec.begin(); };
	FixptColorList(const FixptColorList& list) : FXGStimParameterList(list.index()), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}

	virtual ~FixptColorList() {};
	virtual FixptColorList *clone() const
	{
		return new FixptColorList(*this);
	}
	virtual bool advance(MultiParameterFXMultiGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset)
	{
		if (pstimset->has_fixpt())
		{
			pstimset->fixpt().color = *m_iter;
		}
		return true;
	}

private:
	vector<COLOR_TYPE> m_vec;
	vector<COLOR_TYPE>::const_iterator m_iter;
};


class DotList : public FXGStimParameterList
{
public:
	DotList(vector<alert::ARFixationPointSpec>& dots, int index) : FXGStimParameterList(index), m_vec(dots) { m_iter = m_vec.begin(); };
	DotList(const DotList& list) : FXGStimParameterList(list.index()), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}

	virtual ~DotList() {};
	virtual DotList* clone() const
	{
		return new DotList(*this);
	}
	virtual bool advance(MultiParameterFXMultiGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset)
	{
		if (index() > -1 && index() < pstimset->size())
		{
			pstimset->dot(index()).x = m_iter->x;
			pstimset->dot(index()).y = m_iter->y;
			pstimset->dot(index()).d = m_iter->d;
			pstimset->dot(index()).color = m_iter->color;
		}
		return true;
	}

private:
	vector<alert::ARFixationPointSpec> m_vec;
	vector<alert::ARFixationPointSpec>::const_iterator m_iter;
};



class StimDelayList: public FXGStimParameterList
{
public:
	StimDelayList(vector<double> delays) : FXGStimParameterList(), m_vec(delays) { m_iter = m_vec.begin(); };
	StimDelayList(const StimDelayList& list) : FXGStimParameterList(), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimDelayList() {};
	virtual StimDelayList *clone() const
	{
		return new StimDelayList(*this);
	}
	virtual bool advance(MultiParameterFXMultiGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset)
	{
		pstimset->setCyclingDelay((int)(*m_iter));
		return true;
	}

private:
	vector<double> m_vec;
	vector<double>::const_iterator m_iter;
};

class StimDurationList : public FXGStimParameterList
{
public:
	StimDurationList(vector<double> durations) : FXGStimParameterList(), m_vec(durations) { m_iter = m_vec.begin(); };
	StimDurationList(const StimDurationList& list) : FXGStimParameterList(), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~StimDurationList() {};
	virtual StimDurationList *clone() const
	{
		return new StimDurationList(*this);
	}
	virtual bool advance(MultiParameterFXMultiGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset)
	{
		pstimset->setStimDuration(*m_iter);
		return true;
	}

private:
	vector<double> m_vec;
	vector<double>::const_iterator m_iter;
};


#endif
