#ifndef _STIMPARAMETERLIST_H_
#define _STIMPARAMETERLIST_H_

#include "alertlib.h"
#include "AlertUtil.h"
#include "MultiParameterFXMultiGStimSet.h"
#include <vector>
#include <cassert>
#include <tuple>

using namespace alert;
using namespace std;

// This class represents a list of parameter values over which a grating may vary. 
// The advance() method steps to the next value in the list and sets it in the grating.
// The set_current_parameter() method is an initialization - it sets the grating to the
// first value in the list.

class FXGStimParameterList
{
	unsigned int m_gratingIndex;
	bool m_bDistractor;
public:
	FXGStimParameterList(unsigned int index=0, bool bDistractor=false): m_gratingIndex(index), m_bDistractor(bDistractor) {};
	virtual ~FXGStimParameterList() {};
	virtual bool advance(MultiParameterFXMultiGStimSet* pstimset) = 0;
	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset) = 0;
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

		// no distractors in test
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
	StimXYList(vector<double> xys, unsigned int index, bool bDistractor) : FXGStimParameterList(index, bDistractor)
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

class GratingWHList : public StimXYList
{
public:
	GratingWHList(vector<double> xys, unsigned int index, bool bDistractor) : StimXYList(xys, index, bDistractor)
	{
	};

	GratingWHList(const GratingXYList& list) : StimXYList(list.getXY(), list.index(), list.isDistractor())
	{
	}

	virtual ~GratingWHList() {};
	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset)
	{
		if (!isDistractor())
		{
			pstimset->grating(index()).w = m_iter->first;
			pstimset->grating(index()).h = m_iter->second;
		}
		else
		{
			pstimset->distractor(index()).w = m_iter->first;
			pstimset->distractor(index()).h = m_iter->second;
		}
		return true;
	}
};


class ColorVectorList : public FXGStimParameterList
{
public:
	ColorVectorList(vector<COLOR_VECTOR_TYPE> colorvectors, unsigned int index, bool bDistractor) : FXGStimParameterList(index, bDistractor), m_vec(colorvectors) { m_iter = m_vec.begin(); };
	ColorVectorList(const ColorVectorList& list) : FXGStimParameterList(list.index(), list.isDistractor()), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~ColorVectorList() {};
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
			pstimset->grating(index()).cv = *m_iter;
		}
		else
		{
			pstimset->distractor(index()).cv = *m_iter;
		}
		return true;
	}

private:
	vector<COLOR_VECTOR_TYPE> m_vec;
	vector<COLOR_VECTOR_TYPE>::const_iterator m_iter;
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
	DotList(vector<vector<alert::ARFixationPointSpec> >& dots, int index) : FXGStimParameterList(index), m_vec(dots) { m_iter = m_vec.begin(); };
	DotList(const DotList& list) : FXGStimParameterList(list.index()), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}

	virtual ~DotList() {};
	virtual bool advance(MultiParameterFXMultiGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset)
	{
		if (index() < pstimset->dot_count())
		{
			pstimset->dot(index()).setMulti(*m_iter);
		}
		return true;
	}

private:
	vector<vector<alert::ARFixationPointSpec> >m_vec;
	vector<vector<alert::ARFixationPointSpec> >::const_iterator m_iter;
};

class RectangleList : public FXGStimParameterList
{
public:
	RectangleList(vector<vector<alert::ARRectangleSpec> >& rects, int index) : FXGStimParameterList(index), m_vec(rects) { m_iter = m_vec.begin(); };
	RectangleList(const RectangleList& list) : FXGStimParameterList(list.index()), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}

	virtual ~RectangleList() {};
	virtual bool advance(MultiParameterFXMultiGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset)
	{
		if (index() < pstimset->rectangle_count())
		{
			pstimset->rectangle(index()).setMulti(*m_iter);
		}
		return true;
	}

private:
	vector<vector<alert::ARRectangleSpec> >m_vec;
	vector<vector<alert::ARRectangleSpec> >::const_iterator m_iter;
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

typedef std::tuple<double, double, double> PursuitParams;

class PursuitList : public FXGStimParameterList
{
public:
	PursuitList(vector < double > pp) : FXGStimParameterList() { create_pursuit_list(pp);  m_iter = m_vec.begin(); };
	PursuitList(const PursuitList& list) : FXGStimParameterList(), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~PursuitList() {};
	virtual bool advance(MultiParameterFXMultiGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset)
	{
		pstimset->setPursuitParameters(std::get<0>(*m_iter), std::get<1>(*m_iter), std::get<2>(*m_iter));
		return true;
	}

private:
	vector<PursuitParams> m_vec;
	vector<PursuitParams>::const_iterator m_iter;

	void create_pursuit_list(vector<double> pp)
	{
		int n = (int)pp.size() / 3;
		for (int i = 0; i < n; i++)
		{
			m_vec.push_back(std::make_tuple(pp[i * 3], pp[i * 3 + 1], pp[i * 3 + 2]));
//			cout << "tuple " << i  << " " << std::get<0>(m_vec[i]) << " " << std::get<1>(m_vec[i]) << " " << std::get<2>(m_vec[i]) << std::endl;
		}
	}
};

class MultiGratingOriList : public FXGStimParameterList
{
public:
	MultiGratingOriList(const vector<vector<std::tuple<double, double, double> > >& groups, unsigned int index, bool bDistractor) : FXGStimParameterList(index, bDistractor), m_vec(groups) 
	{ 
		m_iter = m_vec.begin(); 
	};
	MultiGratingOriList(const MultiGratingOriList& list) : FXGStimParameterList(list.index(), list.isDistractor()), m_vec(list.m_vec)
	{
		m_iter = m_vec.begin();
	}
	virtual ~MultiGratingOriList() {};
	virtual bool advance(MultiParameterFXMultiGStimSet* pstimset)
	{
		m_iter++;
		if (m_iter == m_vec.end()) m_iter = m_vec.begin();
		return set_current_parameter(pstimset);
	}

	virtual bool set_current_parameter(MultiParameterFXMultiGStimSet* pstimset)
	{
		if (!isDistractor())
			pstimset->grating(index()).setMulti(*m_iter);
		else
			pstimset->distractor(index()).setMulti(*m_iter);
		return true;
	}

private:
	vector<vector<std::tuple<double, double, double> > > m_vec;
	vector<vector<std::tuple<double, double, double> > >::const_iterator m_iter;
};



#endif
