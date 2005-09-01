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
	void setStimContrast(int contrast);
	void setStimSize(double diameter);
	void setStimSF(double sf);
	void setStimTF(double tf);
	void setStimOrientation(long orientation);
	void setStimColorVector(int cv);
	void setStimPattern(int pattern);
	void setStimAperture(int aperture);
	void setFixationPointXY(double x, double y);
	void setFixationColor(int color);
	void setFixationDiameter(double diameter);
	alert::ARFixationPointSpec& fixpt() { return m_fp; };
	alert::ARApertureGratingSpec& stim() { return m_stim; };

	inline double getVSGWidthDegrees() { return m_dScreenWidthDegrees; };
	inline double getVSGHeightDegrees() { return m_dScreenHeightDegrees; };

	void drawOverlay(BOOL bpresent);

	bool stimFixed() { return m_bStimFixed; };
	bool stimFixed(bool b) { m_bStimFixed = b; return b; };
	bool fixptFixed() { return m_bFPFixed; };
	bool fixptFixed(bool b) { m_bFPFixed = b; return b; };
	void fixptChanged(bool changeIsDone);
	void fixptChanged();

private:
	bool m_bStimFixed;
	bool m_bFPFixed;
	bool m_bUsingVSG;
	alert::ARFixationPointSpec m_fp;
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
