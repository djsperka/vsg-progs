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
	void setApertureProperties(double x, double y, double diam, APERTURE_TYPE a);
	void setStimProperties(double sf, double tf, int contrast, double orient, PATTERN_TYPE p, COLOR_VECTOR_TYPE cv);
	void setFixationPointXY(double x, double y);

	alert::ARContrastFixationPointSpec& fixpt() { return m_fp; };
	alert::ARApertureGratingSpec& stim() { return m_stim; };

	inline double getVSGWidthDegrees() { return m_dScreenWidthDegrees; };
	inline double getVSGHeightDegrees() { return m_dScreenHeightDegrees; };

	// Update dirty stuff
	void update(); 

	void setDirtyAperture() { m_bDirtyAperture = true; };
	void setDirtyStim() { m_bDirtyStim = true; };
private:
	bool m_bUsingVSG;
	alert::ARContrastFixationPointSpec m_fp;
	alert::ARApertureGratingSpec m_stim;
	bool m_bHaveFP;
	bool m_bHaveStim;
	double m_dScreenHeightDegrees;
	double m_dScreenWidthDegrees;
	int m_iOverlayPage;
	int m_iPage;
	bool m_bDirtyAperture;
	bool m_bDirtyStim;
};

#endif
