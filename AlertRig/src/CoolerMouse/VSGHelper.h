#if !defined(_VSGHELPER_H_)
#define _VSGHELPER_H_

#include "alertlib.h"
#include <string>


class VSGHelper
{
public:
	VSGHelper(bool useVSG, int distToScreen, COLOR_TYPE bg=gray);
	virtual ~VSGHelper() {};

	void setStimString(std::string s);
	std::string getStimString();

	void setFixationPointString(std::string s);
	std::string getFixationPointString();

	void start();
	void stop();

	void setStimXY(double x, double y);
//	void setStimProperties(double x, double y, double w, double h, 
//							double sf, double tf, double orientation, int contrast, 
//							PATTERN_TYPE pattern, APERTURE_TYPE aperture, 
//							COLOR_VECTOR_TYPE cv);

	void setFixationPointXY(double x, double y);
//	void setFixationPointProperties(double x, double y, double diam, COLOR_TYPE color);

	alert::ARContrastFixationPointSpec& fixpt() { return m_fp; };
	alert::ARGratingSpec& stim() { return m_stim; };

private:
	bool m_bUsingVSG;
	alert::ARContrastFixationPointSpec m_fp;
	alert::ARGratingSpec m_stim;
	bool m_bHaveFP;
	bool m_bHaveStim;

};

#endif