#pragma once
#include "StimSet.h"


class PlaidStimSet : public StimSet
{
	std::vector<int> m_vecContrast;
	std::vector<double> m_vecSF;
	std::vector<double> m_vecTF;
	std::vector<double> m_vecOriDegrees;
	ARPlaidSpec m_plaid;
	int m_pages[2];
	int m_pageIndex;
	unsigned int m_current;

	void setupCycling(double ppd, double tf, const alert::ARPlaidSubGr& gr1, const alert::ARPlaidSubGr& gr2, double endXDrift, double endYDrift);
	void getDriftPos(double t, double ppd, double tf, const alert::ARPlaidSubGr& gr1, const alert::ARPlaidSubGr& gr2, double& xpos, double& ypos);

public:
	PlaidStimSet(const ARGratingSpec& g0, const ARGratingSpec& g1, const std::vector<int> vecContrast, const std::vector<double> vecSF, const std::vector<double> vecTF, const std::vector<double>& oris);
	PlaidStimSet(const ARGratingSpec& g0, const ARGratingSpec& g1, int contrast, double sf, double tf, double ori);
	//PlaidStimSet(const std::vector<int> vecContrast, const std::vector<double> vecSF, const std::vector<double> vecTF, const std::vector<double>& oris);
	//PlaidStimSet(int contrast, double sf, double tf, double ori);

	virtual ~PlaidStimSet() {};

	virtual int num_pages() { return 2; };
	virtual int num_overlay_pages() { return 0; };
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;

};