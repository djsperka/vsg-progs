#include "Alertlib.h"
#include "AlertUtil.h"
#include "RegHelper.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cmath>

using namespace std;
using namespace alert;


std::istream& operator>>(std::istream& in, ARFixationPointSpec& fixpt)
{
	string s;
	in >> s;
	stringstream ss(s);
	ss >> fixpt.x;
	if (ss)
	{
		ss.ignore(1);
		ss >> fixpt.y;
	}
	if (ss)
	{
		ss.ignore(1);
		ss >> fixpt.d;
	}
	if (ss)
	{
		ss.ignore(1);
		ss >> fixpt.color;
	}
	if (!ss) in.setstate(std::ios::failbit);
	return in;
}


std::istream& operator>>(std::istream& in, alert::ARGratingSpec& grating)
{
	string s;
	in >> s;
	if (parse_grating(s, grating))
	{
		in.setstate(std::ios::failbit);
	}
	return in;
}



//static COLOR_TYPE default_red(red);


ARFixationPointSpec::ARFixationPointSpec()
: x(0)
, y(0)
, d(1)
, color(COLOR_TYPE(red)) 
{};

std::ostream& operator<<(std::ostream& out, const ARFixationPointSpec& arfps)
{
	out << arfps.x << "," << arfps.y << "," << arfps.d << "," << arfps.color;
	return out;
}

std::ostream& operator<<(std::ostream& out, const ARRectangleSpec& arrect)
{
	out << arrect.x << "," << arrect.y << "," << arrect.w << "," << arrect.h << "," << arrect.color;
	return out;
}

std::ostream& operator<<(std::ostream& out, const ARGratingSpec& args)
{
	out << args.x << "," << args.y << "," << args.w << "," << args.h << "," << args.wd << "," << args.hd << "," << args.contrast << "," << args.sf << "," << args.tf << "," << args.orientation << "," << args.phase << "," << args.cv << "," << args.swt << args.twt << "," << args.aperture << "," << args.ttf;
	//out << "x,y=(" << args.x << "," << args.y << ") w,h,wd,hd=(" << args.w << "," << args.h << "," << args.wd << "," << args.hd << ") c=" << args.contrast << " sf=" << args.sf << " tf=" << args.tf << " ori=" << args.orientation << " ph=" << args.phase << " cv=" << args.cv << " p=" << args.pattern << " ap=" << args.aperture;
	return out;
}

std::ostream& operator<<(std::ostream& out, const alert::ARXhairSpec& arx)
{
	out << arx.x << "," << arx.y << "," << arx.ri << "," << arx.ro << "," << arx.nc << "," << arx.r1 << "," << arx.r2;
	return out;
}



ARObject::ARObject() : m_handle(0), m_use_master(false), m_use_slave(false) 
{
};

ARObject::ARObject(const ARObject& obj)
{
	m_handle = obj.m_handle;
	m_first = obj.m_first;
	m_numlevels = obj.m_numlevels;
	m_use_master = obj.m_use_master;
	m_use_slave = obj.m_use_slave;
}

ARObject::~ARObject() {};

ARObject& ARObject::operator=(const ARObject& obj)
{
	if (this != &obj)
	{
		m_handle = obj.m_handle;
		m_first = obj.m_first;
		m_numlevels = obj.m_numlevels;
		m_use_master = obj.m_use_master;
		m_use_slave = obj.m_use_slave;
	}
	return *this;
}


void ARObject::destroy() 
{ 
	if (m_handle > 0) vsgObjDestroy(m_handle); 
	m_handle = 0; 
	return;
};

int ARObject::select()
{
	if (m_handle > 0) 
	{
		vsgObjSelect(m_handle);
		return 0;
	}
	else return -1;
};


void ARObject::init(int numlevels)
{
	PIXEL_LEVEL level;
	if (numlevels == vsgFIXATION)
	{
		level = vsgFIXATION;
		init(level, 1);
	}
	else if (numlevels>1)
	{
		if (getVSG().request_range(numlevels, level))
			cerr << "Error in init: request_range failed to return " << numlevels << " levels." << endl;
		else
			init(level, numlevels);
	}
	else if (numlevels==1)
	{
		getVSG().request_single(level);
		init(level, numlevels);
	}
	else
	{
		cerr << "Error - number of levels must be >0!" << endl;
	}
}

void ARObject::init(ARvsg& vsg, PIXEL_LEVEL first, int numlevels)
{
#if 0
	m_use_master = vsg.is_master();
	m_use_slave = vsg.is_slave();
#endif
	m_use_master = m_use_slave = false;
	init(first, numlevels);
}

void ARObject::init(ARvsg& vsg, int numlevels)
{
#if 0
	m_use_master = vsg.is_master();
	m_use_slave = vsg.is_slave();
#endif
	m_use_master = m_use_slave = false;
	init(numlevels);
}


void ARObject::init(PIXEL_LEVEL first, int numlevels)
{
	m_handle = vsgObjCreate();
	m_first = first;
	m_numlevels = numlevels;
	vsgObjSetDefaults();
	vsgObjSetContrast(100);
	vsgObjSetPixelLevels(first, numlevels);
	m_initialized = true;

	cout << "init obj(" << (int)m_handle << ") on level " << first << ", with " << numlevels << " levels" << endl;
}

void ARObject::init(const ARObject& obj)
{
	m_initialized = obj.initialized();
	m_handle = obj.handle();
	m_first = obj.getFirstLevel();
	m_numlevels = obj.getNumLevels();
	m_use_master = obj.getUseMaster();
	m_use_slave = obj.getUseSlave();
}

void ARObject::setContrast(int contrast) 
{ 
	select(); 
	vsgObjSetContrast(contrast); 
};

VSGOBJHANDLE ARObject::handle() const 
{ 
	return m_handle; 
};

bool ARObject::initialized() const 
{ 
	return m_initialized; 
};

PIXEL_LEVEL ARObject::getFirstLevel() const 
{ 
	return m_first; 
};

int ARObject::getNumLevels() const 
{ 
	return m_numlevels; 
};

ARvsg& ARObject::getVSG()
{
#if 0
	return ARvsg::instance(m_use_master, m_use_slave);
#endif
	return ARvsg::instance();
};

// This object works a little differently than the others. 
// First of all, there is no color vector - it will always be black/white (though that can be changed 
// after this function is called). Second, it really should be init'd with AT LEAST 3 levels, preferably 
// more. 

int ARXhairSpec::draw()
{
	int status=0;
	VSGTRIVAL from = {0, 0, 0};
	VSGTRIVAL to = {1, 1, 1};
	PIXEL_LEVEL level_first, level_mid, level_x, level_last;

	select();
	level_first = getFirstLevel();
	level_mid = getFirstLevel() + getNumLevels()/2;
	level_x = this->getFirstLevel() + this->getNumLevels()/8;
	level_last = getFirstLevel() + getNumLevels() - 1;
	if (getNumLevels() < 3)
	{
		cerr << "WARNING: ARXhair objects should be initialized with at least 3 levels!" << endl;
	}
	vsgObjSetColourVector(&from, &to, vsgBIPOLAR);
	drawPie(this->nc, level_first, level_mid, this->x, -this->y, this->ro);
	drawPie(this->nc, level_mid, level_first, this->x, -this->y, this->rm);
	vsgSetPen1(vsgBACKGROUND);
	vsgDrawOval(this->x, -this->y, 2*this->ri, 2*this->ri);

	double pixels=4, delta;
	vsgUnit2Unit(vsgPIXELUNIT, pixels, vsgDEGREEUNIT, &delta);

#ifdef DRAW_RED_XHAIR
	COLOR_TYPE r = {red, {0}};
	arutil_color_to_palette(r, 245);
#endif

	if (this->r1 > 0 && this->r2 > 0)
	{
		vsgSetPen1(level_mid);
		//vsgSetPen1(245);
		vsgSetDrawMode(vsgCENTREXY + vsgSOLIDPEN);
		vsgSetPenSize(2, 2);

		vsgDrawLine(this->x + this->r1 - delta/2, -this->y - delta/2, this->x + this->r2 - delta/2, -this->y - delta/2);
		vsgDrawLine(this->x-delta/2, -this->y + this->r1 - delta/2, this->x-delta/2, -this->y + this->r2 - delta/2);
		vsgDrawLine(this->x - this->r1 - delta/2, -this->y - delta/2, this->x - this->r2 - delta/2, -this->y - delta/2);
		vsgDrawLine(this->x-delta/2, -this->y - this->r1 - delta/2, this->x-delta/2, -this->y - this->r2 - delta/2);
	}
	return 0;
}

int ARXhairSpec::drawPie(int n, PIXEL_LEVEL first, PIXEL_LEVEL second, double x, double y, double r)
{
	double astep;
	int i;
	astep = 360.0/n;
	vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
	for (i=0; i<n; i++)
	{
		if (i % 2 == 0)
		{
			vsgSetPen1(first);
		}
		else
		{
			vsgSetPen1(second);
		}
		vsgDrawPieArc(x, y, 2*r, 2*r, i*astep, astep*1.5);
	}
	vsgSetPen1(first);
	vsgDrawPieArc(x, y, 2*r, 2*r, 0, astep);
	return 0;
}

int ARXhairSpec::drawOverlay()
{
	cerr << "ARXhairSpec::drawOverlay() not implemented!" << endl;
	return -1;
}

int ARRandomGridSpec::draw()
{
	int status=0;
	double xbox, ybox;
	double xstart, ystart;
	VSGTRIVAL from = {0, 0, 0};
	VSGTRIVAL to = {1, 1, 1};
	PIXEL_LEVEL level_black, level_white;
	long startPixelLevel, numPixelLevels;

	select();
	level_black = getFirstLevel();
	level_white = getFirstLevel() + getNumLevels()/2;
	if (getNumLevels() < 3)
	{
		cerr << "WARNING: ARRandomGrid objects should be initialized with at least 3 levels!" << endl;
	}
	vsgObjSetColourVector(&from, &to, vsgBIPOLAR);
	vsgObjGetPixelLevels(&startPixelLevel, &numPixelLevels);

	// WARNING: All positions will be calculated in pixels!!!

	// xbox and ybox are the width and height of the individual boxes in the grid
	xbox = w/nc;
	ybox = h/nr;

	// xstart, ystart is the center of the small box in the corner of the overall grid
	// xstart and ystart are in Y-up screen coords (origin at center, y pos up). 
	// We'll do all calculations in this frame (call it "yup" frame)
	// and only transform to actual drawing coords (origin upper lh corner, y positive down)
	// when we actually draw.
	// 
	// conversions: x_draw = x_yup - W/2  (W, H screen width, height in current units)
	//              y_draw = H/2 - y_yup

	xstart = x - w/2 + xbox/2;
	ystart = y - h/2 + ybox/2;

	// now draw the boxes
	vsgSetDrawMode(vsgCENTREXY);
	vsgSetPen1(level_black);
	vsgDrawRect(x, -y, w, h);
	vsgSetPen1(level_white);
	for (int i=0; i<nc; i++)
	{
		for (int j=0; j<nr; j++)
		{
			if (RAND_MAX-rand() > RAND_MAX/2) 
			{
				vsgDrawRect(xstart+i*xbox, -(ystart+j*ybox), xbox, ybox);
			}
		}
	}
	return 0;
}


int ARRandomGridSpec::drawOverlay()
{
	cerr << "ARRandomGridSpec::drawOverlay() not implemented!" << endl;
	return -1;
}




int ARChessboardSpec::draw()
{
	int status=0;
	double xbox, ybox, ndiv, ww, hh;
	VSGTRIVAL from = {0, 0, 0};
	VSGTRIVAL to = {1, 1, 1};

	// Need to get dx and dy - individual box dimensions
	ndiv = (double)nr;
	if (nr<=0) ndiv = 1;
	ybox = h/ndiv;
	hh = ybox*ndiv;

	ndiv = (double)nc;
	if (nc <= 0) ndiv = 1;
	xbox = w/ndiv;
	ww = xbox * ndiv;

	select();
	vsgObjTableSinWave(vsgSWTABLE);
	vsgObjSetColourVector(&from, &to, vsgBIPOLAR);
	//vsgObjTableSinWave(vsgTWTABLE);
	vsgObjTableSquareWave(vsgTWTABLE, (WORD)(vsgObjGetTableSize(vsgTWTABLE)*.25), (WORD)(vsgObjGetTableSize(vsgTWTABLE)*.75));
	vsgObjSetTemporalFrequency(tf);
	vsgSetPen1(getFirstLevel());
	vsgSetPen2(getFirstLevel()+getNumLevels()-1);
	vsgSetDrawMode(vsgCENTREXY);
	vsgDrawChessboard(x, -y, ww, hh, xbox, ybox, 0);
	return 0;
}

int ARChessboardSpec::drawOverlay()
{
	cerr << "ARChessboardSpec::drawOverlay() not implemented!" << endl;
	return -1;
}

int ARRectangleSpec::draw()
{
	if (drawmode)
	{
		vsgSetDrawMode(drawmode);
		if (drawmode & vsgSOLIDPEN)
		{
			vsgSetPenSize(linewidth, linewidth);
		}
	}
	else
		vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
	vsgDrawBar(x, -y, w, h, orientation);
	return 0;
}

int ARRectangleSpec::drawOverlay()
{
	cerr << "ARRectangleSpec::drawOverlay() not implemented!" << endl;
	return -1;
}

int ARContrastRectangleSpec::draw()
{
	int status=0;
	VSGTRIVAL from, to;

	select();

	to = this->color.trival();
	from = getVSG().background_color().trival();
	vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);

	if (!status)
	{
		vsgSetPen1(getFirstLevel());
		ARRectangleSpec::draw();
	}
	return status;
}

int ARContrastRectangleSpec::drawOverlay()
{
	vsgSetPen1(0);
	vsgDrawBar(x, -1*y, w, h, orientation);
	return 0;
}

int ARMultiContrastRectangleSpec::draw()
{
	int status=0;
	VSGTRIVAL from, to;

	select();
	vsgSetDrawMode(vsgSOLIDFILL);
	to = this->color.trival();
	from = getVSG().background_color().trival();
	vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);

	if (!status)
	{
		vsgSetPen1(getFirstLevel());
		for (vector<XYWH>::iterator it= this->begin(); it != this->end(); ++it)
		{
			vsgDrawBar(it->x, it->y, it->w, it->h, 0);
			cout << "draw rect " << it->x << ", " << it->y << ", " << it->w << ", " <<  it->h << endl;
		}
	}
	return status;
}

int ARMultiContrastRectangleSpec::drawOverlay()
{
	// not implemented!
	return 0;
}

ARFixationPointSpec::ARFixationPointSpec(const ARFixationPointSpec& fixpt) : ARSpec(fixpt)
{
	x = fixpt.x;
	y = fixpt.y;
	color = fixpt.color;
	d = fixpt.d;
}

ARFixationPointSpec::ARFixationPointSpec(const ARContrastFixationPointSpec& fixpt) : ARSpec(fixpt)
{
	x = fixpt.x;
	y = fixpt.y;
	color = fixpt.color;
	d = fixpt.d;
}

ARFixationPointSpec& ARFixationPointSpec::operator=(const ARFixationPointSpec& fixpt)
{
	if (this != &fixpt)
	{
		ARObject::operator=(fixpt);
		x = fixpt.x;
		y = fixpt.y;
		d = fixpt.d;
		color = fixpt.color;
	}
	return *this;
}


int ARFixationPointSpec::draw()
{
	// check video mode
	if (vsgGetSystemAttribute(vsgVIDEOMODE) == vsg8BITPALETTEMODE)
	{
		vsgSetPen1(getFirstLevel());
	}
	else
	{
		vsgSetPen1((DWORD)(255 * this->color.trival().a + 256 * 255 * this->color.trival().b + 256 * 256 * 255 * this->color.trival().c));
	}
	vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
	vsgDrawOval(x, -1*y, d, d);
	return 0;
}


// oh-oh! The fixpt will hijack overlay palette color level 2!
int ARFixationPointSpec::drawOverlay()
{
	int status=0;
	VSGTRIVAL c = this->color.trival();
	vsgPaletteWriteOverlayCols((VSGLUTBUFFER *)&c, 2, 1);
	vsgSetPen1(2);	// overlay page transparent
	vsgDrawOval(x, -1*y, d, d);
	return status;
}

ARContrastFixationPointSpec& ARContrastFixationPointSpec::operator=(const ARContrastFixationPointSpec& fixpt)
{
	if (this != &fixpt)
	{
		ARFixationPointSpec::operator=(fixpt);
	}
	return *this;
}

ARContrastFixationPointSpec& ARContrastFixationPointSpec::operator=(const ARFixationPointSpec& fixpt)
{
	if (this != &fixpt)
	{
		ARFixationPointSpec::operator=(fixpt);
	}
	return *this;
}


int ARContrastFixationPointSpec::draw()
{
	int status=0;
	VSGTRIVAL from, to;

	select();

	to = this->color.trival();
	from = getVSG().background_color().trival();
	vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);

	if (!status)
	{
		vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
		vsgSetPen1(getFirstLevel());
		ARFixationPointSpec::draw();
	}
	return status;
}

int ARContrastFixationPointSpec::drawOverlay()
{
	vsgSetPen1(0);
	vsgDrawOval(x, -1*y, d, d);
	return 0;
}

ARContrastCircleSpec::ARContrastCircleSpec(const ARContrastCircleSpec& c) : ARContrastFixationPointSpec(c), linewidth(c.linewidth)
{
}

ARContrastCircleSpec& ARContrastCircleSpec::operator=(const ARContrastCircleSpec& c)
{
	if (this != &c)
	{
		ARContrastFixationPointSpec::operator=(c);
		this->linewidth = c.linewidth;
	}
	return *this;
}


int ARContrastCircleSpec::draw()
{
	int status=0;
	VSGTRIVAL from, to;

	select();
	to = this->color.trival();
	from = getVSG().background_color().trival();
	vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);

	if (!status)
	{
		vsgSetPen1(getFirstLevel());
		vsgSetPenSize(this->linewidth, this->linewidth);
		vsgSetDrawMode(vsgCENTREXY + vsgSOLIDPEN);
//		vsgSetDrawMode(vsgCENTREXY + vsgPIXELPEN);
		vsgDrawOval(x, -1*y, d, d);
	}
	return status;
}

// this function doesn't make sense. Should actually draw on the overlay, not draw clear. 
int ARContrastCircleSpec::drawOverlay()
{
	vsgSetPen1(0);
	vsgDrawOval(x, -1*y, d, d);
	return 0;
}



ARContrastCueCircleSpec::ARContrastCueCircleSpec(const ARContrastCueCircleSpec& c) : ARContrastCircleSpec(c), dCentral(c.dCentral), bCentralIsDot(c.bCentralIsDot), bCircleEnabled(c.bCircleEnabled)
{
}

ARContrastCueCircleSpec& ARContrastCueCircleSpec::operator=(const ARContrastCueCircleSpec& c)
{
	if (this != &c)
	{
		ARContrastCircleSpec::operator=(c);
		this->dCentral = c.dCentral;
		this->bCentralIsDot = c.bCentralIsDot;
		this->bCircleEnabled = c.bCircleEnabled;
	}
	return *this;
}


int ARContrastCueCircleSpec::drawCircle()
{
	int status = 0;
	VSGTRIVAL from, to;

	select();
	to = this->color.trival();
	from = getVSG().background_color().trival();
	vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);

	if (!status)
	{
		vsgSetPen1(getFirstLevel());
		vsgSetPenSize(this->linewidth, this->linewidth);
		vsgSetDrawMode(vsgCENTREXY + vsgSOLIDPEN);

		if (bCircleEnabled)
		{
			cerr << "ARContrastCueCircleSpec::drawCircle() - drawOval d=" << d << endl;
			vsgDrawOval(x, -1 * y, d, d);
		}

		//// draw central dot if diam is set
		//if (dCentral > 0)
		//{
		//	if (bCentralIsDot)
		//	{
		//		vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
		//		vsgDrawOval(x, -1 * y, d, d);
		//	}
		//	else
		//	{
		//		vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
		//		vsgDrawBar(x, -y, dCentral, dCentral, 0);
		//	}
		//}
	}
	return status;
}

int ARContrastCueCircleSpec::drawPoint()
{
	int status = 0;
	VSGTRIVAL from, to;

	select();
	to = this->color.trival();
	from = getVSG().background_color().trival();
	vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);

	if (!status)
	{
		vsgSetPen1(getFirstLevel());
		vsgSetPenSize(this->linewidth, this->linewidth);
		// draw central dot if diam is set
		if (dCentral > 0)
		{
			if (bCentralIsDot)
			{
				cerr << "ARContrastCueCircleSpec::drawPoint() - drawOval dCentral=" << dCentral << endl;
				vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
				vsgDrawOval(x, -1 * y, dCentral, dCentral);
			}
			else
			{
				cerr << "ARContrastCueCircleSpec::drawPoint() - drawBar dCentral=" << dCentral << endl;
				vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
				vsgDrawBar(x, -y, dCentral, dCentral, 0);
			}
		}
	}
	return status;
}

// not intended to be used this way but should work
int ARContrastCueCircleSpec::draw()
{
	return drawCircle() + drawPoint();
}


// this function doesn't make sense. Should actually draw on the overlay, not draw clear. 
int ARContrastCueCircleSpec::drawOverlay()
{
	vsgSetPen1(0);
	vsgDrawOval(x, -1 * y, d, d);
	return 0;
}







ARCircleSpec::ARCircleSpec(const ARCircleSpec& c) : ARFixationPointSpec(c), linewidth(c.linewidth)
{
}

ARCircleSpec& ARCircleSpec::operator=(const ARCircleSpec& c)
{
	if (this != &c)
	{
		ARFixationPointSpec::operator=(c);
		this->linewidth = c.linewidth;
	}
	return *this;
}


int ARCircleSpec::draw()
{
	int status = 0;
	VSGTRIVAL from, to;

	select();
	to = this->color.trival();
	from = getVSG().background_color().trival();
	vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);

	if (!status)
	{
		vsgSetPen1(getFirstLevel());
		vsgSetPenSize(this->linewidth, this->linewidth);
		vsgSetDrawMode(vsgCENTREXY + vsgSOLIDPEN);
		//		vsgSetDrawMode(vsgCENTREXY + vsgPIXELPEN);
		vsgDrawOval(x, -1 * y, d, d);
	}
	return status;
}

// this function doesn't make sense. Should actually draw on the overlay, not draw clear. 
int ARCircleSpec::drawOverlay()
{
	vsgSetPen1(0);
	vsgDrawOval(x, -1 * y, d, d);
	return 0;
}


ARCueCircleSpec::ARCueCircleSpec(const ARCueCircleSpec& c) : ARCircleSpec(c), dCentral(c.dCentral), bCentralIsDot(c.bCentralIsDot), bCircleEnabled(c.bCircleEnabled)
{
}

ARCueCircleSpec& ARCueCircleSpec::operator=(const ARCueCircleSpec& c)
{
	if (this != &c)
	{
		ARCircleSpec::operator=(c);
		this->dCentral = c.dCentral;
		this->bCentralIsDot = c.bCentralIsDot;
		this->bCircleEnabled = c.bCircleEnabled;
	}
	return *this;
}


int ARCueCircleSpec::drawCircle()
{
	int status = 0;
	// check video mode
	if (vsgGetSystemAttribute(vsgVIDEOMODE) == vsg8BITPALETTEMODE)
	{
		vsgSetPen1(getFirstLevel());
	}
	else
	{
		vsgSetPen1((DWORD)(255 * this->color.trival().a + 256 * 255 * this->color.trival().b + 256 * 256 * 255 * this->color.trival().c));
	}
	vsgSetPenSize(this->linewidth, this->linewidth);
	vsgSetDrawMode(vsgCENTREXY + vsgSOLIDPEN);

	if (bCircleEnabled)
	{
		cerr << "ARContrastCueCircleSpec::drawCircle() - drawOval d=" << d << endl;
		vsgDrawOval(x, -1 * y, d, d);
	}
	return status;
}

int ARCueCircleSpec::drawPoint()
{
	int status = 0;
	if (!status)
	{
		// check video mode
		if (vsgGetSystemAttribute(vsgVIDEOMODE) == vsg8BITPALETTEMODE)
		{
			vsgSetPen1(getFirstLevel());
		}
		else
		{
			vsgSetPen1((DWORD)(255 * this->color.trival().a + 256 * 255 * this->color.trival().b + 256 * 256 * 255 * this->color.trival().c));
		}
		vsgSetPenSize(this->linewidth, this->linewidth);
		// draw central dot if diam is set
		if (dCentral > 0)
		{
			if (bCentralIsDot)
			{
				//cerr << "ARContrastCueCircleSpec::drawPoint() - drawOval dCentral=" << dCentral << endl;
				vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
				vsgDrawOval(x, -1 * y, dCentral, dCentral);
			}
			else
			{
				//cerr << "ARContrastCueCircleSpec::drawPoint() - drawBar dCentral=" << dCentral << endl;
				vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
				vsgDrawBar(x, -y, dCentral, dCentral, 0);
			}
		}
	}
	return status;
}

// not intended to be used this way but should work
int ARCueCircleSpec::draw()
{
	return drawCircle() + drawPoint();
}


// this function doesn't make sense. Should actually draw on the overlay, not draw clear. 
int ARCueCircleSpec::drawOverlay()
{
	vsgSetPen1(0);
	vsgDrawOval(x, -1 * y, d, d);
	return 0;
}


ARContrastLineSpec::ARContrastLineSpec(const ARContrastLineSpec& line) : ARSpec(line)
{
	x0 = line.x0;
	x1 = line.x1;
	y0 = line.y0;
	y1 = line.y1;
	color = line.color;
}

ARContrastLineSpec& ARContrastLineSpec::operator=(const ARContrastLineSpec& line)
{
	if (this != &line)
	{
		ARSpec::operator=(line);
		x0 = line.x0;
		x1 = line.x1;
		y0 = line.y0;
		y1 = line.y1;
		color = line.color;
	}
	return *this;
}



int ARContrastLineSpec::draw()
{
	int status=0;
	VSGTRIVAL from, to;

	select();

	to = this->color.trival();
	from = getVSG().background_color().trival();
	vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);

	if (!status)
	{
		vsgSetPen1(getFirstLevel());
		vsgSetDrawMode(vsgPIXELPEN);
		vsgDrawLine(x0, y0, x1, y1);
	}
	return status;
}

// this function doesn't make sense. Should actually draw on the overlay, not draw clear. 
int ARContrastLineSpec::drawOverlay()
{
	vsgSetPen1(0);
	vsgDrawLine(x0, y0, x1, y1);
	return 0;
}


ARGratingSpec::ARGratingSpec(const ARGratingSpec& g) : ARSpec(g)
{
	x = g.x;
	y = g.y;
	w = g.w;
	h = g.h;
	wd = g.wd;
	hd = g.hd;
	contrast = g.contrast;
	sf = g.sf;
	tf = g.tf;
	orientation = g.orientation;
	phase = g.phase;
	cv = g.cv;
	swt = g.swt;
	twt = g.twt;
	ttf = g.ttf;
	aperture = g.aperture;
	bDrawInitDone = false;
}

ARGratingSpec& ARGratingSpec::operator=(const ARGratingSpec& g)
{
	if (this != &g)
	{
		ARObject::operator=(g);
		x = g.x;
		y = g.y;
		w = g.w;
		h = g.h;
		wd = g.wd;
		hd = g.hd;
		contrast = g.contrast;
		sf = g.sf;
		tf = g.tf;
		orientation = g.orientation;
		phase = g.phase;
		cv = g.cv;
		swt = g.swt;
		twt = g.twt;
		ttf = g.ttf;
		aperture = g.aperture;
	}
	return *this;
}



int ARGratingSpec::draw()
{
	return draw(false);
}

int ARGratingSpec::drawOnce()
{
	return draw(true);
}

int ARGratingSpec::draw(bool useTransOnHigher)
{
	if (useTransOnHigher)
	{
		return draw((long)vsgTRANSONHIGHER);
	}
	else
	{
		return draw((long)vsgTRANSONLOWER);
	}
}


int ARGratingSpec::draw(long mode, int apertureLevel)
{
	int status=0;
	int ipen;
	VSGTRIVAL from, to;

	select();

	// We assume that the handle is created and selected. In order to make this grating appear, you still must
	// assign pixel levels (vsgObjSetPixels). Note also that the contrast is initially set to 100% by the call to 
	// vsgObjSetDefaults().

	if (apertureLevel >= 0) ipen = apertureLevel;
	else
	{
		if (mode == vsgTRANSONHIGHER) ipen = 255;
		else ipen = 0;
	}

	if (mode == vsgTRANSONHIGHER || mode == vsgTRANSONLOWER)
	{
		// djs TRANSONLOWER seems to leave artifacts on the screen when we turn the contrast off. !!!
		// if ellipse, draw an ellipse on level 0 for TRANSONLOWER
		// djs. Specifically setting the level seems to lead to artifacts. I found that leaving the levels
		// unassigned solves this. I suspect this may lead to trouble someday.....
		if (this->aperture == ellipse)
		{
			vsgSetPen1(ipen);
			vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
			vsgDrawOval(x, -1*y, w, h);
		}
	}

	if (!bDrawInitDone)
	{
		// Set object defauilts. setDefaults() sets contrast to 100% 
		// -- this may not be what we want, so reset contrast to the 
		// stim's current value
		//vsgObjSetDefaults();
		//vsgObjSetContrast(contrast);

		// assign pixel levels for object
		vsgObjSetPixelLevels(getFirstLevel(), getNumLevels());

		// Set spatial waveform
		if (this->swt == sinewave)
		{
			vsgObjTableSinWave(vsgSWTABLE);
		}
		else
		{	
			// Set up standard 50:50 square wave
			vsgObjTableSquareWave(vsgSWTABLE, (DWORD)(vsgObjGetTableSize(vsgSWTABLE)*0.25), (DWORD)(vsgObjGetTableSize(vsgSWTABLE)*0.75));
		}

		// set color vector
		if (get_colorvector(this->cv, from, to))
		{
			cerr << "Cannot get color vector for type " << this->cv << endl;
		}
		vsgObjSetColourVector(&from, &to, vsgBIPOLAR);

		// set temporal waveform
		if (this->twt == sinewave)
		{
			vsgObjTableSinWave(vsgTWTABLE);
		}
		else
		{
			// Set up standard 50:50 square wave
			vsgObjTableSquareWave(vsgTWTABLE, (DWORD)(0), (DWORD)(vsgObjGetTableSize(vsgTWTABLE)*0.5));
		}
		vsgObjSetTemporalPhase(0);



		bDrawInitDone = true;
	}

	vsgObjSetContrast(contrast);
	vsgObjSetSpatialPhase(phase);
	vsgObjSetDriftVelocity(tf);
	vsgObjSetTemporalFrequency(ttf);

	// Now draw
	if (this->aperture == ellipse)
	{
		if (mode == vsgTRANSONLOWER || mode == vsgTRANSONHIGHER)
		{
			vsgSetDrawMode(vsgCENTREXY + mode);
		}
		else
		{
			vsgSetDrawMode(vsgCENTREXY);
		}
		vsgSetPen1(getFirstLevel());
		vsgSetPen2(getFirstLevel() + getNumLevels() -1);
		vsgDrawGrating(this->x, -1*this->y, this->w, this->h, this->orientation, this->sf);
	}
	else
	{
		vsgSetDrawMode(vsgCENTREXY);
		vsgSetPen1(getFirstLevel());
		vsgSetPen2(getFirstLevel() + getNumLevels() - 1);
		vsgDrawGrating(this->x, -1*this->y, this->w, this->h, this->orientation, this->sf);
	}

	// Draw hole if this is a donut. Having both hd, wd > 0 makes this a donut.
	// Check for dumb situation w,h == 0.
	if (w > 0  && h > 0)
	{
		vsgSetPen1(vsgBACKGROUND);

		// Now draw the hole, but only if the diam is >0
		if (wd > 0 && hd > 0)
		{
			if (this->aperture == ellipse)
			{
				vsgSetDrawMode(vsgCENTREXY);
				vsgDrawOval(x, -1*y, wd, hd);
			}
			else
			{
				vsgSetDrawMode(vsgCENTREXY);
				vsgDrawRect(this->x, -1*this->y, this->wd, this->hd);
			}
		}
	}

	return 0;
}

int ARGratingSpec::drawBackground()
{
	vsgSetPen1(vsgBACKGROUND);
	//vsgSetPen1(getVSG().background_level());
	vsgSetDrawMode(vsgCENTREXY);
	if (this->aperture == ellipse)
	{
		vsgDrawOval(x, -1*y, w, h);
	}
	else
	{
		vsgDrawRect(x, -1*y, w, h);
	}
	return 0;
}

int ARGratingSpec::drawOverlay()
{
	vsgSetPen1(0);
	vsgSetDrawMode(vsgCENTREXY);
	if (this->aperture == ellipse)
	{
		vsgDrawOval(x, -1*y, w, h);
	}
	else
	{
		vsgDrawRect(x, -1*y, w, h);
	}
	return 0;
}



void ARGratingSpec::setContrast(int contrast) 
{ 
	select(); 
	this->contrast = contrast; 
	vsgObjSetContrast(contrast); 
};

void ARGratingSpec::setTemporalFrequency(double tf) 
{ 
	select(); 
	this->tf = tf; 
	vsgObjSetDriftVelocity(tf); 
};

void ARGratingSpec::resetDriftPhase()
{
	select();
	vsgObjResetDriftPhase();
}

int ARApertureGratingSpec::draw()
{
	int status=0;
	VSGTRIVAL from, to;

	select();

	// We assume that the handle is created and selected. In order to make this grating appear, you still must
	// assign pixel levels (vsgObjSetPixels). Note also that the contrast is initially set to 100% by the call to 
	// vsgObjSetDefaults().

	vsgObjSetDefaults();
	vsgObjSetPixelLevels(getFirstLevel(), getNumLevels());

	// Set spatial waveform
	if (this->swt == sinewave)
	{
		vsgObjTableSinWave(vsgSWTABLE);
	}
	else
	{	
		// Set up standard 50:50 square wave
		vsgObjTableSquareWave(vsgSWTABLE, (DWORD)(vsgObjGetTableSize(vsgSWTABLE)*0.25), (DWORD)(vsgObjGetTableSize(vsgSWTABLE)*0.75));
	}

	// set temporal freq
	vsgObjSetDriftVelocity(tf);

	// set color vector
	if (get_colorvector(this->cv, from, to))
	{
		cerr << "Cannot get color vector for type " << this->cv << endl;
	}
	vsgObjSetColourVector(&from, &to, vsgBIPOLAR);

	// 
	double dWidth = vsgGetScreenWidthPixels();
	double dHeight = vsgGetScreenHeightPixels();
	vsgUnit2Unit(vsgPIXELUNIT,dWidth,vsgDEGREEUNIT,&dWidth);
	vsgUnit2Unit(vsgPIXELUNIT,dHeight,vsgDEGREEUNIT,&dHeight);

	// Now draw
	vsgSetDrawMode(vsgCENTREXY);
	vsgDrawGrating(0, 0, dWidth, dHeight, this->orientation, this->sf);

	return 0;
}
