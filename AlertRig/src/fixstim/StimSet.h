#include "alertlib.h"
#include <vector>
#include <string>

using namespace alert;

class StimSet
{
private:
	double m_spatialphase;
public:
	StimSet() : m_spatialphase(0) {};
	StimSet(double spph) : m_spatialphase(spph) {};
	virtual ~StimSet() {};

	// Get spatial phase value preferred
	double get_spatial_phase() { return m_spatialphase; };

	// subclasses should return the number of pages they will need.
	virtual int num_pages() = 0;

	// subclasses should return the number of pages they will need.
	virtual int num_overlay_pages() = 0;

	// initialize the pages indicated by the elements of the vector. Subclasses may 
	// call vsgPresent() if needed. The first version has a default implementation 
	// that uses the regular vsg card. The second version should be used for master/slave
	// cases. 
	virtual int init(std::vector<int> pages);
	virtual int init(ARvsg& vsg, std::vector<int> pages) = 0;

	// handle the trigger indicated by the string s. Do not call vsgPresent! return value of 
	// 1 means vsgPresent() will be called. 
	virtual int handle_trigger(std::string& s) = 0;

	virtual std::string toString() const;

};

std::ostream& operator<<(std::ostream& out, const StimSet& sset);



class NullStimSet: public StimSet
{
public:
	NullStimSet(): StimSet() {};
	virtual ~NullStimSet() {};
	virtual int num_pages() {return 0;};
	virtual int num_overlay_pages() {return 0;};
	virtual int init(std::vector<int> pages) {return 0;};
	virtual int init(ARvsg& vsg, std::vector<int> pages) {return 0;};
	virtual int handle_trigger(std::string& s) {return 0;};		// will not trigger?!?
	virtual std::string toString() const;
};


class GratingStimSet: public StimSet
{
public:
	GratingStimSet(alert::ARGratingSpec& g, double xoffset, double yoffset, double spatialphase=0) : StimSet(spatialphase), m_page(-1), m_grating(g) {m_grating.x += xoffset; m_grating.y += yoffset;};
	virtual ~GratingStimSet() {};
	virtual int num_pages();
	virtual int num_overlay_pages();
	//virtual int init(std::vector<int> pages);
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_page;
	int m_contrast;
	alert::ARGratingSpec m_grating;
};


// This stim set can take a fixpt and grating, or just a fixpt. 
class FixptGratingStimSet: public StimSet
{
public:
	FixptGratingStimSet(alert::ARContrastFixationPointSpec& f, alert::ARGratingSpec& g, double xoffset, double yoffset, double spatialphase=0) : StimSet(spatialphase), m_fixpt(f), m_grating(g), m_page(-1), m_bHaveGrating(true) {m_grating.x += xoffset; m_grating.y += yoffset; m_fixpt.x += xoffset; m_fixpt.y += yoffset;};
	FixptGratingStimSet(alert::ARContrastFixationPointSpec& f, double xoffset, double yoffset) : StimSet(), m_fixpt(f), m_page(-1), m_bHaveGrating(false) {m_fixpt.x += xoffset; m_fixpt.y += yoffset;};
	virtual ~FixptGratingStimSet() {};
	virtual int num_pages();
	virtual int num_overlay_pages();
	//virtual int init(std::vector<int> pages);
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	bool m_bHaveGrating;
	int m_page;
	int m_contrast;
	alert::ARGratingSpec m_grating;
	alert::ARContrastFixationPointSpec m_fixpt;
};


class ContrastStimSet: public StimSet
{
public:
	ContrastStimSet(alert::ARContrastFixationPointSpec& f, alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(spatialphase), m_fixpt(f), m_grating(g), m_bHaveFixpt(true), m_contrasts(parameters) {};
	ContrastStimSet(alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(spatialphase), m_grating(g), m_bHaveFixpt(false), m_contrasts(parameters) {};
	virtual int num_pages() {return 1;};
	virtual int num_overlay_pages() {return 0;};
	//virtual int init(std::vector<int> pages);
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_page;
	std::vector<double> m_contrasts;
	std::vector<double>::const_iterator m_iterator;
	alert::ARGratingSpec m_grating;
	alert::ARContrastFixationPointSpec m_fixpt;
	bool m_bHaveFixpt;
};

class TFStimSet: public StimSet
{
public:
	TFStimSet(alert::ARContrastFixationPointSpec& f, alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(spatialphase), m_fixpt(f), m_grating(g), m_bHaveFixpt(true), m_temporal_frequencies(parameters) {};
	TFStimSet(alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(spatialphase), m_grating(g), m_bHaveFixpt(false), m_temporal_frequencies(parameters) {};
	virtual int num_pages() {return 1;};
	virtual int num_overlay_pages() {return 0;};
	//virtual int init(std::vector<int> pages);
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_page;
	int m_contrast;
	std::vector<double> m_temporal_frequencies;
	std::vector<double>::const_iterator m_iterator;
	alert::ARGratingSpec m_grating;
	alert::ARContrastFixationPointSpec m_fixpt;
	bool m_bHaveFixpt;
};


class SFStimSet: public StimSet
{
public:
	SFStimSet(alert::ARContrastFixationPointSpec& f, alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(spatialphase), m_fixpt(f), m_grating(g), m_bHaveFixpt(true), m_spatial_frequencies(parameters), m_current_page(-1) {};
	SFStimSet(alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(spatialphase), m_grating(g), m_bHaveFixpt(false), m_spatial_frequencies(parameters), m_current_page(-1) {};
	virtual int num_pages() {return 2;};
	virtual int num_overlay_pages() {return 0;};
	//virtual int init(std::vector<int> pages);
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_pages[2];
	int m_current_page;		// page flipping. This is the page currently displayed when handle_trigger is called. 
	int m_contrast;
	std::vector<double> m_spatial_frequencies;
	std::vector<double>::const_iterator m_iterator;
	alert::ARGratingSpec m_grating;
	alert::ARContrastFixationPointSpec m_fixpt;
	bool m_bHaveFixpt;
};

class OrientationStimSet: public StimSet
{
public:
	OrientationStimSet(alert::ARContrastFixationPointSpec& f, alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(spatialphase), m_fixpt(f), m_grating(g), m_bHaveFixpt(true), m_orientations(parameters), m_current_page(-1) {};
	OrientationStimSet(alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(spatialphase), m_grating(g), m_bHaveFixpt(false), m_orientations(parameters), m_current_page(-1) {};
	virtual int num_pages() {return 2;};
	virtual int num_overlay_pages() {return 0;};
	//virtual int init(std::vector<int> pages);
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_pages[2];
	int m_current_page;		// page flipping. This is the page currently displayed when handle_trigger is called. 
	int m_contrast;
	std::vector<double> m_orientations;
	std::vector<double>::const_iterator m_iterator;
	alert::ARGratingSpec m_grating;
	alert::ARContrastFixationPointSpec m_fixpt;
	bool m_bHaveFixpt;
};

class AreaStimSet: public StimSet
{
public:
	AreaStimSet(alert::ARContrastFixationPointSpec& f, alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(spatialphase), m_fixpt(f), m_grating(g), m_bHaveFixpt(true), m_diameters(parameters), m_current_page(-1) {};
	AreaStimSet(alert::ARGratingSpec& g, std::vector<double> parameters, double spatialphase=0) : StimSet(spatialphase), m_grating(g), m_bHaveFixpt(false), m_diameters(parameters), m_current_page(-1) {};
	virtual int num_pages() {return 2;};
	virtual int num_overlay_pages() {return 0;};
	//virtual int init(std::vector<int> pages);
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_pages[2];
	int m_current_page;		// page flipping. This is the page currently displayed when handle_trigger is called. 
	int m_contrast;
	std::vector<double> m_diameters;
	std::vector<double>::const_iterator m_iterator;
	alert::ARGratingSpec m_grating;
	alert::ARContrastFixationPointSpec m_fixpt;
	bool m_bHaveFixpt;
};


class CRGStimSet: public StimSet
{
public:
	CRGStimSet(alert::ARContrastFixationPointSpec& f, alert::ARGratingSpec& g, int frames_per_term, const std::string& sequence, bool balanced = false);
	CRGStimSet(alert::ARGratingSpec& g, int frames_per_term, const std::string& sequence, bool balanced = false);
	virtual int num_pages() {return 2;};
	virtual int num_overlay_pages() {return 0;};
	//virtual int init(std::vector<int> pages);
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_page0;
	int m_page1;
	int m_contrast;
	alert::ARGratingSpec m_grating0;
	alert::ARGratingSpec m_grating1;
	alert::ARContrastFixationPointSpec m_fixpt;
	bool m_bHaveFixpt;
	int m_fpt;
	std::string m_seq;
	bool m_balanced;
};

class CBarStimSet: public StimSet
{
public:
	CBarStimSet(COLOR_TYPE& c, double w, double h, double dps, std::vector<double> parameters) : StimSet(), m_barWidth(w), m_barHeight(h), m_degreesPerSecond(dps), m_orientations(parameters), m_barOffsetXPixels(1024), m_barOffsetYPixels(512), m_barMaxWidthKludge(1.2) {	m_iterator = m_orientations.begin(); m_rect.color = c; };
	virtual int num_pages() {return 2;};
	virtual int num_overlay_pages() {return 0;};
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
	void prepareCycling(double ori);
	double getDistance(long *c0, long *c1, long *c2, double *p, double *q, double *u, double *v);
	double getIntersectDistance(long *ca, long *cb, double *p, double *u);
	void set_color(COLOR_TYPE& color) { m_rect.color = color; };
private:
	int m_pageBackground;
	int m_pageStim;
	double m_barWidth;	// degrees
	double m_barHeight;	// degrees
	double m_barMaxWidthKludge;	// max bar width is diagonal times this, but why isn't diagonal big enough?
	double m_degreesPerSecond;	// um, degrees per second
	double m_barOffsetXPixels;		// drawing offset for bar in pixels
	double m_barOffsetYPixels;
	double m_barOffsetXDegrees;		// drawing offset for bar in degrees
	double m_barOffsetYDegrees;
	ARContrastRectangleSpec m_rect;
	ARContrastLineSpec m_perp;
	std::vector<double> m_orientations;
	std::vector<double>::const_iterator m_iterator;
	double m_pixels_per_frame;
};
