#include "StimSetCRG.h"

template <typename A, typename B>
class DoubleCircular
{
	vector<A> m_avec;
	vector<B> m_bvec;
	vector<A>::iterator m_a_it;
	vector<B>::iterator m_b_it;
public:
	DoubleCircular(vector<A> avec, vector<B> bvec) : m_avec(avec), m_bvec(bvec) 
	{
		m_a_it = m_avec.begin();
		m_b_it = m_bvec.begin();
	}
	~DoubleCircular() {};

	pair<A, B> next()
	{
		A a = *m_a_it;	// assuming avec and bvec in constructor are not empty
		B b = *m_b_it;
		m_a_it++;
		if (m_a_it == m_avec.end()) m_a_it = m_avec.begin();
		m_b_it++;
		if (m_b_it == m_bvec.end()) m_b_it = m_bvec.begin();
		return pair<A, B>(a, b);
	}
};

class StimSetCRGG: public StimSetBase, public StimSequenceList
{
protected:
	int m_contrast;
	int m_contrastUp;
	double x0,y0,x1,y1;
	ARGratingSpec m_gratings[4];
	unsigned int m_fpt;

public:
	StimSetCRG(unsigned int ifpt, vector<string> sequences, vector<int> order): m_fpt(ifpt), StimSequenceList(sequences, order) {};
	virtual ~StimSetCRG() {};

	void set_initial_parameters();
	int setup_cycling(int firstpage, int stim1page, int stim0page, int lastpage);

	virtual int init(ARvsg& vsg);
	virtual int handle_trigger(std::string& s);
};


class StimSetCRGG: public StimSetCRG
{
private:
	ARGratingSpec m_grating2, m_grating3;
	vector<int> m_transitions;
	vector<double> m_crggParams;
public:
	StimSetCRGG(unsigned int ifpt, vector<string> sequences, vector<int> order, vector<double> crggParams): StimSetCRG(ifpt, sequences, order)
	{
		unsigned int i;
		for (i=0; i<6; i++)
		{
			m_crggParams.push_back(crggParams[i]);
		}
		for (i=6; i<crggParams.size(); i++)
		{
			m_transitions.push_back((int)crggParams[i]);
		}
	};
	virtual ~StimSetCRGG() {};

	void set_initial_parameters();
	int setup_cycling(int firstpage, int stim1page, int stim0page, int lastpage);

	virtual int init(ARvsg& vsg);
	virtual int handle_trigger(std::string& s);


	virtual void set_grating(ARGratingSpec& grating);
	virtual void set_grating(ARGratingSpec& grating, double xoffset, double yoffset);
	virtual ARGratingSpec& grating(int i) 
	{
		switch (i)
		{
		case 0: return m_grating; break;
		case 1: return m_grating1; break;
		case 2: return m_grating2; break;
		case 3: return m_grating3; break;
		default: return m_grating; break;
		}
	}

};

