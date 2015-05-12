#ifndef _STIMSET_H_
#define _STIMSET_H_

#include "alertlib.h"
#include <windows.h>	// mutex
#include <vector>
#include <string>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/assert.hpp>
using namespace alert;
using namespace boost;
using namespace std;

class StimSet
{
private:
public:
	StimSet() {};
	virtual ~StimSet() {};

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


class FXStimSet : public StimSet
{
private:
	ARContrastFixationPointSpec m_fixpt;
	bool m_have_fixpt;
	ARXhairSpec m_xhair;
	bool m_have_xhair;

public:
	FXStimSet(): m_have_fixpt(false), m_have_xhair(false) {};
	FXStimSet(ARContrastFixationPointSpec& fixpt): m_fixpt(fixpt), m_have_fixpt(true), m_have_xhair(false) {}; 
	FXStimSet(ARXhairSpec& xhair): m_have_fixpt(false), m_xhair(xhair), m_have_xhair(true) {}; 
	FXStimSet(ARContrastFixationPointSpec& fixpt, ARXhairSpec& xhair): m_fixpt(fixpt), m_have_fixpt(true), m_xhair(xhair), m_have_xhair(true) {}; 
	virtual ~FXStimSet() {};

	void set_fixpt(const ARFixationPointSpec& fixpt, double xoffset=0.0, double yoffset=0.0);
	void set_xhair(const ARXhairSpec& xhair, double xoffset=0.0, double yoffset=0.0);
	bool has_fixpt() const { return m_have_fixpt; };
	bool has_xhair() const { return m_have_xhair; };
	ARContrastFixationPointSpec& fixpt() { return m_fixpt; };
	const ARContrastFixationPointSpec& fixpt() const { return m_fixpt; };
	ARXhairSpec& xhair() { return m_xhair; };
	const ARXhairSpec& xhair() const { return m_xhair; };

};


class FXGStimSet : public FXStimSet
{
private:
	ARGratingSpec m_grating;
	bool m_has_grating;
	int m_contrast;

public:
	FXGStimSet(): FXStimSet(), m_has_grating(false), m_contrast(0) {};
	FXGStimSet(ARGratingSpec& grating): FXStimSet(), m_grating(grating), m_has_grating(true), m_contrast(grating.contrast) {};
	FXGStimSet(ARGratingSpec& grating, ARContrastFixationPointSpec& fixpt): FXStimSet(fixpt), m_grating(grating), m_has_grating(true), m_contrast(grating.contrast) {}; 
	FXGStimSet(ARContrastFixationPointSpec& fixpt): FXStimSet(fixpt), m_has_grating(false), m_contrast(0) {}; 
	FXGStimSet(ARContrastFixationPointSpec& fixpt, ARXhairSpec& xhair): FXStimSet(fixpt, xhair), m_has_grating(false), m_contrast(0) {}; 
	FXGStimSet(ARGratingSpec& grating, ARXhairSpec& xhair): FXStimSet(xhair), m_grating(grating), m_has_grating(true), m_contrast(grating.contrast) {}; 
	FXGStimSet(ARGratingSpec& grating, ARContrastFixationPointSpec& fixpt, ARXhairSpec& xhair): FXStimSet(fixpt, xhair), m_grating(grating), m_has_grating(true), m_contrast(grating.contrast) {}; 
	virtual ~FXGStimSet() {};

	// Must call this if a StimParameterList object will change the grating's contrast. The updated contrast
	// is not saved here in that case.
	void setSavedContrast(int contrast) { m_contrast = contrast; };
	int contrast() const { return m_contrast; };
	const bool has_grating() const { return m_has_grating; };
	ARGratingSpec& grating() { return m_grating; };
	const ARGratingSpec& grating() const { return m_grating; };
};

class FXMultiGStimSet : public FXStimSet
{
private:
	ptr_vector<ARGratingSpec> m_gratings;
	ptr_vector<ARGratingSpec> m_distractors;
	vector<int> m_contrasts;
	vector<int> m_distractor_contrasts;

public:
	FXMultiGStimSet(): FXStimSet() {};
	FXMultiGStimSet(ARContrastFixationPointSpec& fixpt): FXStimSet(fixpt) {}; 
	FXMultiGStimSet(ARXhairSpec& xhair): FXStimSet(xhair) {}; 
	FXMultiGStimSet(ARContrastFixationPointSpec& fixpt, ARXhairSpec& xhair): FXStimSet(fixpt, xhair) {}; 
	virtual ~FXMultiGStimSet() {};

	ptr_vector<ARGratingSpec>& gratings() { return m_gratings; };
	ptr_vector<ARGratingSpec>& distractors() { return m_distractors; };
	const ptr_vector<ARGratingSpec>& gratings() const { return m_gratings; };
	const ptr_vector<ARGratingSpec>& distractors() const { return m_distractors; };

	virtual void set_grating(const ARGratingSpec& grating, double xoffset=0.0, double yoffset=0.0);
	virtual bool has_grating() const { return count()>0; };

	ARGratingSpec& grating() 
	{ 
		BOOST_ASSERT(m_gratings.size() > 0);
		return m_gratings[0]; 
	};

	const ARGratingSpec& grating() const 
	{ 
		BOOST_ASSERT(m_gratings.size() > 0);
		return m_gratings[0]; 
	};

	ARGratingSpec& grating(int i)
	{
		BOOST_ASSERT(m_gratings.size() > i);
		return m_gratings[i];
	};

	const ARGratingSpec& grating(int i) const
	{
		BOOST_ASSERT(m_gratings.size() > i);
		return m_gratings[i];
	};

	int contrast(int i) const
	{
		BOOST_ASSERT(m_contrasts.size() > i);
		return m_contrasts[i];
	}

	int distractor_contrast(int i) const
	{
		BOOST_ASSERT(m_distractor_contrasts.size() > i);
		return m_distractor_contrasts[i];
	}

	int count() const
	{
		return m_gratings.size();
	};

	int distractor_count() const
	{
		return m_distractors.size();
	};

	virtual void set_distractor(const ARGratingSpec& grating, double xoffset=0.0, double yoffset=0.0);
	virtual bool has_distractor() const { return distractor_count()>0; };

	ARGratingSpec& distractor() 
	{ 
		BOOST_ASSERT(m_distractors.size() > 0);
		return m_distractors[0]; 
	};

	const ARGratingSpec& distractor() const 
	{ 
		BOOST_ASSERT(m_distractors.size() > 0);
		return m_distractors[0]; 
	};

	ARGratingSpec& distractor(int i)
	{
		BOOST_ASSERT(m_distractors.size() > i);
		return m_distractors[i];
	};

	const ARGratingSpec& distractor(int i) const
	{
		BOOST_ASSERT(m_distractors.size() > i);
		return m_distractors[i];
	};

};







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


class GratingStimSet: public FXGStimSet
{
public:
	GratingStimSet(alert::ARGratingSpec& g) : FXGStimSet(g), m_page(-1) {};
	virtual ~GratingStimSet() {};
	virtual int num_pages() { return 1;};
	virtual int num_overlay_pages() { return 0;};
	//virtual int init(std::vector<int> pages);
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_page;
};


// This stim set can take a fixpt and grating, or just a fixpt. 
class FixptGratingStimSet: public FXGStimSet
{
public:
	FixptGratingStimSet(ARContrastFixationPointSpec& f, ARXhairSpec& h, ARGratingSpec& g) : FXGStimSet(g, f, h), m_page(-1) {};
	FixptGratingStimSet(ARContrastFixationPointSpec& f, alert::ARGratingSpec& g) : FXGStimSet(g, f), m_page(-1) {};
	FixptGratingStimSet(ARContrastFixationPointSpec& f, ARXhairSpec& h) : FXGStimSet(f, h), m_page(-1) {};
	FixptGratingStimSet(ARContrastFixationPointSpec& f) : FXGStimSet(f) {};
	FixptGratingStimSet() : FXGStimSet() {};
	virtual ~FixptGratingStimSet() {};
	virtual int num_pages() { return 1; };
	virtual int num_overlay_pages() { return 0; };
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_page;
};

class FixptMultiGratingStimSet: public FXMultiGStimSet
{
public:
	FixptMultiGratingStimSet(ARContrastFixationPointSpec& f, ARXhairSpec& h) : FXMultiGStimSet(f, h), m_page(-1) {};
	FixptMultiGratingStimSet(ARContrastFixationPointSpec& f) : FXMultiGStimSet(f), m_page(-1) {};
	FixptMultiGratingStimSet() : FXMultiGStimSet() {};
	virtual ~FixptMultiGratingStimSet() {};
	virtual int num_pages() { return 1; };
	virtual int num_overlay_pages() { return 0; };
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_page;
	bool m_bDistractors;
};

class ContrastStimSet: public FXGStimSet
{
public:
	ContrastStimSet(ARContrastFixationPointSpec& f, ARXhairSpec& h, ARGratingSpec& g, std::vector<double> parameters) : FXGStimSet(g, f, h), m_contrasts(parameters) {};
	ContrastStimSet(alert::ARContrastFixationPointSpec& f, alert::ARGratingSpec& g, std::vector<double> parameters) : FXGStimSet(g, f), m_contrasts(parameters) {};
	ContrastStimSet(alert::ARGratingSpec& g, std::vector<double> parameters) : FXGStimSet(g), m_contrasts(parameters) {};
	virtual int num_pages() {return 1;};
	virtual int num_overlay_pages() {return 0;};
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_page;
	std::vector<double> m_contrasts;
	std::vector<double>::const_iterator m_iterator;
};

class TFStimSet: public FXGStimSet
{
public:
	TFStimSet(ARContrastFixationPointSpec& f, ARXhairSpec& h, ARGratingSpec& g, std::vector<double> parameters) : FXGStimSet(g, f, h), m_temporal_frequencies(parameters) {};
	TFStimSet(ARContrastFixationPointSpec& f, ARGratingSpec& g, std::vector<double> parameters) : FXGStimSet(g, f), m_temporal_frequencies(parameters) {};
	TFStimSet(ARGratingSpec& g, std::vector<double> parameters) : FXGStimSet(g), m_temporal_frequencies(parameters) {};
	virtual int num_pages() {return 1;};
	virtual int num_overlay_pages() {return 0;};
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_page;
	std::vector<double> m_temporal_frequencies;
	std::vector<double>::const_iterator m_iterator;
};


class SFStimSet: public FXGStimSet
{
public:
	SFStimSet(ARContrastFixationPointSpec& f, ARXhairSpec& h, ARGratingSpec& g, std::vector<double> parameters) : FXGStimSet(g, f, h), m_spatial_frequencies(parameters), m_current_page(-1) {};
	SFStimSet(ARContrastFixationPointSpec& f, ARGratingSpec& g, std::vector<double> parameters) : FXGStimSet(g, f), m_spatial_frequencies(parameters), m_current_page(-1) {};
	SFStimSet(ARGratingSpec& g, std::vector<double> parameters) : FXGStimSet(g), m_spatial_frequencies(parameters), m_current_page(-1) {};
	virtual int num_pages() {return 2;};
	virtual int num_overlay_pages() {return 0;};
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_pages[2];
	int m_current_page;		// page flipping. This is the page currently displayed when handle_trigger is called. 
	std::vector<double> m_spatial_frequencies;
	std::vector<double>::const_iterator m_iterator;
};

class OrientationStimSet: public FXGStimSet
{
public:
	OrientationStimSet(ARContrastFixationPointSpec& f, ARXhairSpec& h, ARGratingSpec& g, std::vector<double> parameters) : FXGStimSet(g, f, h), m_orientations(parameters), m_current_page(-1) {};
	OrientationStimSet(ARContrastFixationPointSpec& f, ARGratingSpec& g, std::vector<double> parameters) : FXGStimSet(g, f), m_orientations(parameters), m_current_page(-1) {};
	OrientationStimSet(ARGratingSpec& g, std::vector<double> parameters) : FXGStimSet(g), m_orientations(parameters), m_current_page(-1) {};
	virtual int num_pages() {return 2;};
	virtual int num_overlay_pages() {return 0;};
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_pages[2];
	int m_current_page;		// page flipping. This is the page currently displayed when handle_trigger is called. 
	std::vector<double> m_orientations;
	std::vector<double>::const_iterator m_iterator;
};

class AreaStimSet: public FXGStimSet
{
public:
	AreaStimSet(ARContrastFixationPointSpec& f, ARXhairSpec& h, ARGratingSpec& g, std::vector<double> parameters) : FXGStimSet(g, f, h), m_diameters(parameters), m_current_page(-1) {};
	AreaStimSet(ARContrastFixationPointSpec& f, ARGratingSpec& g, std::vector<double> parameters) : FXGStimSet(g, f), m_diameters(parameters), m_current_page(-1) {};
	AreaStimSet(ARGratingSpec& g, std::vector<double> parameters) : FXGStimSet(g), m_diameters(parameters), m_current_page(-1) {};
	virtual int num_pages() {return 2;};
	virtual int num_overlay_pages() {return 0;};
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_pages[2];
	int m_current_page;		// page flipping. This is the page currently displayed when handle_trigger is called. 
	std::vector<double> m_diameters;
	std::vector<double>::const_iterator m_iterator;
};

class InnerDiameterStimSet: public FXGStimSet
{
public:
	InnerDiameterStimSet(ARContrastFixationPointSpec& f, ARXhairSpec& h, ARGratingSpec& g, std::vector<double> parameters) : FXGStimSet(g, f, h), m_diameters(parameters), m_current_page(-1) {};
	InnerDiameterStimSet(ARContrastFixationPointSpec& f, ARGratingSpec& g, std::vector<double> parameters) : FXGStimSet(g, f), m_diameters(parameters), m_current_page(-1) {};
	InnerDiameterStimSet(ARGratingSpec& g, std::vector<double> parameters) : FXGStimSet(g), m_diameters(parameters), m_current_page(-1) {};
	virtual int num_pages() {return 2;};
	virtual int num_overlay_pages() {return 0;};
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_pages[2];
	int m_current_page;		// page flipping. This is the page currently displayed when handle_trigger is called. 
	std::vector<double> m_diameters;
	std::vector<double>::const_iterator m_iterator;
};

class AnnulusStimSet: public FXGStimSet
{
public:
	AnnulusStimSet(ARContrastFixationPointSpec& f, ARXhairSpec& h, ARGratingSpec& g, std::vector<double> parameters) : FXGStimSet(g, f, h), m_diameters(parameters), m_current_page(-1) {};
	AnnulusStimSet(ARContrastFixationPointSpec& f, ARGratingSpec& g, std::vector<double> parameters) : FXGStimSet(g, f), m_diameters(parameters), m_current_page(-1) {};
	AnnulusStimSet(ARGratingSpec& g, std::vector<double> parameters) : FXGStimSet(g), m_diameters(parameters), m_current_page(-1) {};
	virtual int num_pages() {return 2;};
	virtual int num_overlay_pages() {return 0;};
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_pages[2];
	int m_current_page;		// page flipping. This is the page currently displayed when handle_trigger is called. 
	std::vector<double> m_diameters;
	std::vector<double>::const_iterator m_iterator;
};


class PositionStimSet: public FXGStimSet
{
public:
	PositionStimSet(ARContrastFixationPointSpec& f, ARXhairSpec& h, ARGratingSpec& g, std::vector<double> positions) : FXGStimSet(g, f, h), m_positions(positions), m_current_page(-1) {};
	PositionStimSet(ARContrastFixationPointSpec& f, ARGratingSpec& g, std::vector<double> positions) : FXGStimSet(g, f), m_positions(positions), m_current_page(-1) {};
	PositionStimSet(ARGratingSpec& g, std::vector<double> positions) : FXGStimSet(g), m_positions(positions), m_current_page(-1) {};
	virtual int num_pages() {return 2;};
	virtual int num_overlay_pages() {return 0;};
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_pages[2];
	int m_current_page;		// page flipping. This is the page currently displayed when handle_trigger is called. 
	std::vector<double> m_positions;
	std::vector<double>::const_iterator m_iterator;
};


class CounterphaseStimSet: public FXGStimSet
{
public:
	CounterphaseStimSet(ARContrastFixationPointSpec& f, ARXhairSpec& h, ARGratingSpec& g, std::vector<double> parameters, double tf, bool bStepTW) : FXGStimSet(g, f), m_phases(parameters), m_tf(tf), m_bStepTW(bStepTW), m_current_page(-1) {};
	CounterphaseStimSet(ARContrastFixationPointSpec& f, ARGratingSpec& g, std::vector<double> parameters, double tf, bool bStepTW) : FXGStimSet(g, f), m_phases(parameters), m_tf(tf), m_bStepTW(bStepTW), m_current_page(-1) {};
	CounterphaseStimSet(ARGratingSpec& g, std::vector<double> parameters, double tf, bool bStepTW) : FXGStimSet(g), m_phases(parameters), m_tf(tf), m_bStepTW(bStepTW), m_current_page(-1) {};
	virtual int num_pages() {return 2;};
	virtual int num_overlay_pages() {return 0;};
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_pages[2];
	int m_current_page;		// page flipping. This is the page currently displayed when handle_trigger is called. 
	std::vector<double> m_phases;
	double m_tf;
	bool m_bStepTW;			// Step temporal waveform function. default is sinusoid.
	std::vector<double>::const_iterator m_iterator;
};


class CRGStimSet: public StimSet
{
public:
	CRGStimSet(alert::ARContrastFixationPointSpec& f, alert::ARGratingSpec& g, int frames_per_term, const std::string& sequence, std::vector<double> contrasts, bool balanced = false);
	CRGStimSet(alert::ARGratingSpec& g, int frames_per_term, const std::string& sequence, std::vector<double> contrasts, bool balanced = false);
	CRGStimSet(alert::ARContrastFixationPointSpec& f, alert::ARGratingSpec& g, int frames_per_term, const std::string& sequence, bool balanced = false);
	CRGStimSet(alert::ARGratingSpec& g, int frames_per_term, const std::string& sequence, bool balanced = false);
	virtual int num_pages() {return 2;};
	virtual int num_overlay_pages() {return 0;};
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_page0;
	int m_page1;
	//int m_contrast;
	vector<double> m_contrasts;
	vector<double>::const_iterator m_iterator;
	alert::ARGratingSpec m_grating0;
	alert::ARGratingSpec m_grating1;
	alert::ARContrastFixationPointSpec m_fixpt;
	bool m_bHaveFixpt;
	int m_fpt;
	std::string m_seq;
	bool m_balanced;
};

class FlashStimSet: public StimSet
{
public:
	FlashStimSet(alert::ARContrastFixationPointSpec& f, std::vector< COLOR_TYPE >& colors, int frames_per_term, const std::string& sequence, bool balanced = false);
	FlashStimSet(std::vector< COLOR_TYPE >& colors, int frames_per_term, const std::string& sequence, bool balanced = false);
	FlashStimSet(alert::ARContrastFixationPointSpec& f, int frames_per_term, const std::string& sequence, bool balanced = false);
	FlashStimSet(int frames_per_term, const std::string& sequence, bool balanced = false);
	virtual int num_pages() {return 1+m_colors.size();};
	virtual int num_overlay_pages() {return 0;};
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	vector<COLOR_TYPE> m_colors;
	alert::ARContrastFixationPointSpec m_fixpt;
	bool m_bHaveFixpt;
	int m_fpt;
	std::string m_seq;
	bool m_balanced;
	int m_pageBlank;
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

class DotStimSet: public StimSet
{
public:
	DotStimSet(alert::ARContrastFixationPointSpec& f, double x, double y, COLOR_TYPE color, double diameter, double speed, double density, int dotsize, vector<double>& angles);
	DotStimSet(double x, double y, COLOR_TYPE color, double speed, double diameter, double density, int dotsize, vector<double>& angles);
	~DotStimSet();
	virtual int num_pages() {return 3;};
	virtual int num_overlay_pages() {return 0;};
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
	static void threadfunc(void *);
private:
	// Generate initial set of dots. 
	void generate_dots();

	// Draw fixpt/dots on current draw page
	void update_page();

	// update thread function
	void do_dots();

	// func to shift dots and wrap if necessary
	void shift_dots();

	// thread handle
	HANDLE m_thread;

	// event handle
	HANDLE m_event;

	// page tracking
	int m_pages[2];
	int m_ipage;
	int m_pageBackground;

	// data we need to keep
	vector<double> m_angles;
	vector<double>::const_iterator m_iterator;
	alert::ARContrastFixationPointSpec m_fixpt;
	bool m_bHaveFixpt;
	double m_x;				// position in degrees
	double m_y;				// position in degrees
	double m_diameter;		// diameter in degrees (circular patch)
	double m_speed;			// degrees per second
	double m_xPixel;		// position converted to pixels (positive-down, ULH origin)
	double m_yPixel;		// position converted to pixels (positive-down, ULH origin)
	double m_diameterPixel;	// diameter converted to pixels
	double m_speedPixelsPerFrame;	// pixels per frame (NOT per second)

	// these do not require pixel conversion
	double m_density;		// density in dots per square degree
	int m_dotsize;
	COLOR_TYPE m_color;

	// dot storage
	int m_nDots;
	double *m_dots;
	PIXEL_LEVEL m_dotLevel;

	// status of stim
	bool m_bDotsOn;
	bool m_bFixptOn;
};


class DanishStimSet: public FXMultiGStimSet
{
public:
	DanishStimSet(alert::ARContrastFixationPointSpec& f, alert::ARGratingSpec& g, std::vector<double> parameters) : FXMultiGStimSet(f), m_ods(parameters), m_current_page(-1) { set_grating(g); };
	DanishStimSet(alert::ARGratingSpec& g, std::vector<double> parameters) : FXMultiGStimSet(), m_ods(parameters), m_current_page(-1) { set_grating(g); };
	DanishStimSet(alert::ARContrastFixationPointSpec& f, alert::ARGratingSpec& g, alert::ARGratingSpec& hole, std::vector<double> parameters) : FXMultiGStimSet(f), m_ods(parameters), m_current_page(-1) { set_grating(g); set_grating(hole); };
	DanishStimSet(alert::ARGratingSpec& g, alert::ARGratingSpec& hole, std::vector<double> parameters) : FXMultiGStimSet(), m_ods(parameters), m_current_page(-1) {set_grating(g); set_grating(hole); };
	virtual int num_pages() {return 2;};
	virtual int num_overlay_pages() {return 0;};
	virtual int init(ARvsg& vsg, std::vector<int> pages);
	virtual int handle_trigger(std::string& s);
	virtual std::string toString() const;
private:
	int m_pages[2];
	int m_current_page;		// page flipping. This is the page currently displayed when handle_trigger is called. 
	std::vector<double> m_ods;	// outer diameters
	std::vector<double>::const_iterator m_iterator;
};


#endif