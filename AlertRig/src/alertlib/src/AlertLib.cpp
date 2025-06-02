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
	if (parse_fixation_point(s, fixpt))
	{
		in.setstate(std::ios::failbit);
	}

#if 0
	stringstream ss(s);
	bool bCross = false;

	if (s.size() > 0 && s[0] == '+')
	{
		bool bNoMoreChars = false;	// set when we run out of stuff, in case string is shortened
		fixpt.isDot = true;
		ss.ignore(2);
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
		if (ss)
		{
			// anything left? 
			std::streampos pos = ss.tellg();
			ss.seekg(0, std::ios::end);
			std::streamsize len = ss.tellg() - pos;
			ss.seekg(pos);
			if (len > 0)
			{
				ss.ignore(1);
				ss >> fixpt.penSizePixels;
			}
			else
				bNoMoreChars = true;
		}
		if (ss && !bNoMoreChars)
		{
			// anything left? 
			std::streampos pos = ss.tellg();
			ss.seekg(0, std::ios::end);
			std::streamsize len = ss.tellg() - pos;
			ss.seekg(pos);
			if (len > 0)
			{
				ss.ignore(1);
				ss >> fixpt.crossOriDeg;
			}
		}
		if (!ss) in.setstate(std::ios::failbit);
	}
	else
	{
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
	}
#endif
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

std::istream& operator>>(std::istream& in, alert::ARRectangleSpec& arrect)
{
	string s;
	in >> s;
	if (parse_rectangle(s, arrect))
	{
		in.setstate(std::ios::failbit);
	}
	return in;
}


//static COLOR_TYPE default_red(red);


std::ostream& operator<<(std::ostream& out, const ARFixationPointSpec& arfps)
{
	if (arfps.isDot)
		out << arfps.x << "," << arfps.y << "," << arfps.d << "," << arfps.color;
	else
		out << "+," << arfps.x << "," << arfps.y << "," << arfps.d << "," << arfps.color << "," << arfps.penSizePixels << "," << arfps.crossOriDeg;

	return out;
}

std::ostream& operator<<(std::ostream& out, const ARRectangleSpec& arrect)
{
	if (!arrect.isMulti())
		out << arrect.x << "," << arrect.y << "," << arrect.w << "," << arrect.h << "," << arrect.color;
	else
		for (auto r : arrect.getMulti())
			out << r.x << "," << r.y << "," << r.w << "," << r.h << "," << r.color << ";";

	return out;
}

std::ostream& operator<<(std::ostream& out, const ARContrastRectangleSpec& arrect)
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

std::ostream& operator<<(std::ostream& out, const alert::ARConteSpec& conte)
{
	string s;
	switch (conte.iHorizontal)
	{
	case 0:		s = "V"; break;
	case 1:		s = "H"; break;
	default:	s = "N"; break;
	}
	out << conte.x << "," << conte.y << "," << conte.w << "," << conte.h << "," << conte.orientation << "," << conte.sf << "," << conte.divisor << "," << conte.phase << "," << s << "," << conte.cueLineWidth << "," << conte.gaborContrast << "," << conte.flankerContrast << "," << conte.cueContrast << conte.cueColor;
	return out;
}

std::ostream& operator<<(std::ostream& out, const alert::ARImageSpec& img)
{
	out << "FILE=" << img.filename << "," << img.x << "," << img.y << "," << img.durSeconds << "," << img.dlySeconds;
	return out;
}


ARObject::ARObject() : m_handle(0), m_iDrawGroups(-1), m_use_master(false), m_use_slave(false) 
{
};

ARObject::ARObject(const ARObject& obj)
{
	m_handle = obj.m_handle;
	m_first = obj.m_first;
	m_numlevels = obj.m_numlevels;
	m_iDrawGroups = obj.m_iDrawGroups;
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
		m_iDrawGroups = obj.m_iDrawGroups;
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
		return vsgObjSelect(m_handle);
	}
	else return -1;
};


void ARObject::init(int numlevels, bool bcreate)
{
	PIXEL_LEVEL level;
	if (numlevels == vsgFIXATION)
	{
		level = vsgFIXATION;
		init(level, 1, bcreate);
	}
	else if (numlevels>1)
	{
		if (getVSG().request_range(numlevels, level))
			cerr << "Error in init: request_range failed to return " << numlevels << " levels." << endl;
		else
			init(level, numlevels, bcreate);
	}
	else if (numlevels==1)
	{
		getVSG().request_single(level);
		init(level, numlevels, bcreate);
	}
	else
	{
		cerr << "Error - number of levels must be >0!" << endl;
	}
}

void ARObject::init(PIXEL_LEVEL first, int numlevels, bool bcreate)
{
	m_first = first;
	m_numlevels = numlevels;
	if (bcreate)
	{
		m_handle = vsgObjCreate();
		vsgObjSetDefaults();
		vsgObjSetContrast(100);
		vsgObjSetPixelLevels(first, numlevels);
		m_initialized = true;
		cout << "init obj(" << (int)m_handle << ") on level " << first << ", with " << numlevels << " levels" << endl;
	}
	else
		cout << "init obj(***) on level " << first << ", with " << numlevels << " levels" << endl;

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

int ARXhairSpec::drawOverlay(PIXEL_LEVEL)
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


int ARRandomGridSpec::drawOverlay(PIXEL_LEVEL)
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

int ARChessboardSpec::drawOverlay(PIXEL_LEVEL)
{
	cerr << "ARChessboardSpec::drawOverlay() not implemented!" << endl;
	return -1;
}

int ARRectangleSpec::draw()
{
	if (!bIsMulti)
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
		vsgSetPen1(this->getFirstLevel());
		vsgDrawBar(x, -y, w, h, orientation);
	}
	else
	{
		PIXEL_LEVEL first = this->getFirstLevel();
		PIXEL_LEVEL num = this->getNumLevels();

		for (int i = 0; i < m_multi.size(); i++)
		{
			// The dot should be init'd with enough levels for all dots. This is just in case it isn't.
			PIXEL_LEVEL n = first + (i % num);

			// Draw. Use level 'n' only when palette mode
			if (vsgGetSystemAttribute(vsgVIDEOMODE) == vsg8BITPALETTEMODE)
			{
				// assign color to level 'n'
				//cerr << "multidot " << i << " color " << m_multi[i].color << " level " << n << endl;
				arutil_color_to_palette(m_multi[i].color, n);
				vsgSetPen1(n);
			}
			else
			{
				vsgSetPen1((DWORD)(255 * m_multi[i].color.trival().a + 256 * 255 * m_multi[i].color.trival().b + 256 * 256 * 255 * m_multi[i].color.trival().c));
			}
			vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
			vsgDrawBar(m_multi[i].x, -1 * m_multi[i].y, m_multi[i].w, m_multi[i].h, m_multi[i].orientation);
		}
		return 0;
	}



	return 0;
}

int ARRectangleSpec::drawOverlay(PIXEL_LEVEL ovLevel)
{
	int status = 0;
	arutil_color_to_overlay_palette(this->color, ovLevel);
	vsgSetPen1(ovLevel);	// overlay page transparent
	if (!bIsMulti)
	{
		DWORD oldmode = vsgGetDrawMode();
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
		vsgSetDrawMode(oldmode);
	}
	else
	{
		cerr << "ARRectangleSpec::drawOverlay() not implemented!" << endl;
		status = -1;
	}
	return status;
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

int ARContrastRectangleSpec::drawOverlay(PIXEL_LEVEL overlayLevel)
{
	cerr << "ARContrastRectangleSpec::drawOverlay() not implemented!" << endl;
	return -1;
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

int ARMultiContrastRectangleSpec::drawOverlay(PIXEL_LEVEL)
{
	cerr << "ARMultiContrastRectangleSpec::drawOverlay() not implemented!" << endl;
	return -1;
}

ARFixationPointSpec::ARFixationPointSpec()
	: x(0)
	, y(0)
	, d(1)
	, color(COLOR_TYPE(red))
	, isDot(true)
	, penSizePixels(1)
	, crossOriDeg(0)
{};

ARFixationPointSpec::ARFixationPointSpec(const ARFixationPointSpec& fixpt) 
	: ARSpec(fixpt)
	, x(fixpt.x)
	, y(fixpt.y)
	, color(fixpt.color)
	, d(fixpt.d)
	, isDot(fixpt.isDot)
	, penSizePixels(fixpt.penSizePixels)
	, crossOriDeg(fixpt.crossOriDeg)
{
}

ARFixationPointSpec::ARFixationPointSpec(const ARContrastFixationPointSpec& fixpt) 
	: ARSpec(fixpt)
	, x(fixpt.x)
	, y(fixpt.y)
	, color(fixpt.color)
	, d(fixpt.d)
	, isDot(fixpt.isDot)
	, penSizePixels(fixpt.penSizePixels)
	, crossOriDeg(fixpt.crossOriDeg)
{
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
		isDot = fixpt.isDot;
		penSizePixels = fixpt.penSizePixels;
		crossOriDeg = fixpt.crossOriDeg;
	}
	return *this;
}

void getCrossEndpoints(double x, double y, double oriDeg, double l, double (&xc)[4], double (&yc)[4])
{
	double c = cos(oriDeg * 3.14159 / 180);
	double s = sin(oriDeg * 3.14159 / 180);
	xc[0] = x + l / 2 * c;
	yc[0] = y + l / 2 * s;
	xc[1] = x + -l / 2 * c;
	yc[1] = y + -l / 2 * s;
	xc[2] = x + -l / 2 * s;
	yc[2] = y + l / 2 * c;
	xc[3] = x + l / 2 * s;
	yc[3] = y + -l / 2 * c;
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

	if (isDot)
	{
		vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
		vsgDrawOval(x, -1 * y, d, d);
	}
	else
	{
		// Leaving this as originally written. Uses vsg's "pixel pen" when drawing lines. 
		// VSG draws line as a rectangle, with one edge from the coordinate locations you supply.
		// The rest of the "width" of the line lay to one side or the other - it is not centered. 
		// This can be noticable when the line width is big enough and/or there are other 
		// objects drawn relative to the "center" of the cross. 
#if 1
		double xc[4], yc[4];
		vsgSetDrawMode(vsgCENTREXY + vsgSOLIDPEN);
		vsgSetPenSize(penSizePixels, penSizePixels);
		getCrossEndpoints(this->x, -this->y, this->crossOriDeg, this->d, xc, yc);
		vsgDrawLine(xc[0], yc[0], xc[1], yc[1]);
		vsgDrawLine(xc[2], yc[2], xc[3], yc[3]);
#else
		// draw cross as two rectangles ('bars' to the vsg). Determine height of box in degrees
		double h = this->penSizePixels;
		//vsgUnit2Unit(vsgPIXELUNIT, this->penSizePixels, vsgDEGREEUNIT, &h);
		vsgSetDrawMode(vsgCENTREXY);
		vsgDrawBar(this->x, this->y, this->d, h, this->crossOriDeg);
		vsgDrawBar(this->x, this->y, this->d, h, this->crossOriDeg+90);
#endif

	}
	return 0;
}

int ARFixationPointSpec::drawOverlay(PIXEL_LEVEL ovLevel)
{
	int status=0;
	arutil_color_to_overlay_palette(this->color, ovLevel);
	vsgSetPen1(ovLevel);	// overlay page transparent
	if (isDot)
	{
		vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
		vsgDrawOval(x, -1 * y, d, d);
	}
	else
	{
		// see comment in draw() above. Do NOT change the #if below 
		// without also changing draw()!!!
#if 1
		double xc[4], yc[4];
		vsgSetDrawMode(vsgCENTREXY + vsgSOLIDPEN);
		vsgSetPenSize(penSizePixels, penSizePixels);
		getCrossEndpoints(this->x, -this->y, this->crossOriDeg, this->d, xc, yc);
		vsgDrawLine(xc[0], yc[0], xc[1], yc[1]);
		vsgDrawLine(xc[2], yc[2], xc[3], yc[3]);
#else
		// draw cross as two rectangles ('bars' to the vsg). Determine height of box in degrees
		double h = this->penSizePixels;
		vsgSetDrawMode(vsgCENTREXY);
		vsgDrawBar(this->x, this->y, this->d, h, this->crossOriDeg);
		vsgDrawBar(this->x, this->y, this->d, h, this->crossOriDeg + 90);
#endif

	}
	return status;
}


ARDotSpec& ARDotSpec::operator=(const ARFixationPointSpec& fixpt)
{
	if (this != &fixpt)
	{
		ARObject::operator=(fixpt);
		x = fixpt.x;
		y = fixpt.y;
		d = fixpt.d;
		color = fixpt.color;
		bIsMulti = false;
	}
	return *this;

}

int ARDotSpec::draw()
{
	if (!bIsMulti)
		return ARFixationPointSpec::draw();
	else
	{
		PIXEL_LEVEL first = this->getFirstLevel();
		PIXEL_LEVEL num = this->getNumLevels();

		for (int i = 0; i<m_multi.size(); i++)
		{
			// The dot should be init'd with enough levels for all dots. This is just in case it isn't.
			PIXEL_LEVEL n = first + (i % num);

			// Draw. Use level 'n' only when palette mode
			if (vsgGetSystemAttribute(vsgVIDEOMODE) == vsg8BITPALETTEMODE)
			{
				// assign color to level 'n'
				//cerr << "multidot " << i << " color " << m_multi[i].color << " level " << n << endl;
				arutil_color_to_palette(m_multi[i].color, n);
				vsgSetPen1(n);
			}
			else
			{
				vsgSetPen1((DWORD)(255 * m_multi[i].color.trival().a + 256 * 255 * m_multi[i].color.trival().b + 256 * 256 * 255 * m_multi[i].color.trival().c));
			}
			vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
			vsgDrawOval(m_multi[i].x, -1 * m_multi[i].y, m_multi[i].d, m_multi[i].d);
		}
		return 0;
	}
}

int ARDotSpec::drawOverlay(PIXEL_LEVEL)
{
	cerr << "ARDotSpec::drawOverlay(): not implemented!" << endl;
	return -1;
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

int ARContrastFixationPointSpec::drawOverlay(PIXEL_LEVEL ovLevel)
{
	return ARFixationPointSpec::drawOverlay(ovLevel);
	//vsgSetPen1(0);
	//vsgDrawOval(x, -1*y, d, d);
	//return 0;
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

int ARContrastCircleSpec::drawOverlay(PIXEL_LEVEL)
{
	cerr << "ARContrastCircleSpec::drawOverlay(): not implemented!" << endl;
	return -1;
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
int ARContrastCueCircleSpec::drawOverlay(PIXEL_LEVEL)
{
	cerr << "ARContrastCueCircleSpec::drawOverlay(): not implemented!" << endl;
	return -1;
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

int ARCircleSpec::drawOverlay(PIXEL_LEVEL)
{
	cerr << "ARCircleSpec::drawOverlay(): not implemented!" << endl;
	return -1;
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


int ARCueCircleSpec::drawOverlay(PIXEL_LEVEL)
{
	cerr << "ARCueCircleSpec::drawOverlay(): not implemented!" << endl;
	return -1;
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
int ARContrastLineSpec::drawOverlay(PIXEL_LEVEL)
{
	cerr << "ARContrastLineSpec::drawOverlay(): not implemented!" << endl;
	return -1;
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
	bIsMulti = g.bIsMulti;
	m_multi = g.m_multi;
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
		bIsMulti = g.bIsMulti;
		m_multi = g.m_multi;
	}
	return *this;
}

void ARGratingSpec::getBBoxWH(double x, double y, double w, double h, double ori, double& bboxWidth, double& bboxHeight)
{
	MyRot mr(ori);
	double llx = 99999999;
	double lly = 99999999;
	double urx = -99999999;
	double ury = -99999999;
	double xp, yp;
	// rotate upper right corner of box, measured relative to center pt. 
	mr.rotatePoint(w / 2, h / 2, xp, yp);
	if (xp > urx) urx = xp;
	if (yp > ury) ury = yp;
	if (xp < llx) llx = xp;
	if (yp < lly) lly = yp;

	// rotate lower right corner of box, measured relative to center pt. 
	mr.rotatePoint(w / 2, -h / 2, xp, yp);
	if (xp > urx) urx = xp;
	if (yp > ury) ury = yp;
	if (xp < llx) llx = xp;
	if (yp < lly) lly = yp;

	// rotate lower left corner of box, measured relative to center pt. 
	mr.rotatePoint(-w / 2, -h / 2, xp, yp);
	if (xp > urx) urx = xp;
	if (yp > ury) ury = yp;
	if (xp < llx) llx = xp;
	if (yp < lly) lly = yp;

	// rotate upper left corner of box, measured relative to center pt. 
	mr.rotatePoint(-w / 2, h / 2, xp, yp);
	if (xp > urx) urx = xp;
	if (yp > ury) ury = yp;
	if (xp < llx) llx = xp;
	if (yp < lly) lly = yp;

	// difference between LL and UR give width and height
	bboxWidth = urx - llx;
	bboxHeight = ury - lly;

	return;
}

int ARGratingSpec::drawOrientedRectangle()
{
	int status = 0;

	// get bbox
	double bboxW, bboxH;
	getBBoxWH(this->x, this->y, this->w, this->h, this->wd, bboxW, bboxH);

	// Get current draw page - we'll use it later.
	VSGPAGEDESCRIPTOR descr;
	descr._StructSize = sizeof(VSGPAGEDESCRIPTOR);
	vsgGetCurrentDrawPage(&descr);

	// Now set the dummy host page as drawing page
	vsgSetDrawPage(vsgHOSTPAGE, getVSG().hostpage_handle(), vsgNOCLEAR);

	// draw bbox rect on the HOST page. Set a HIGH level, which should be higher than 
	// anything that will or was drawn (except vsgBACKGROUND).
	vsgSetPen1(getVSG().highlevel());
	vsgDrawRect(this->x, -this->y, bboxW, bboxH);

	// draw oriented rectangle. This is on a lower level than the grating levels, 
	// so using MAXMODE (TRANSONLOWER) ensures that the grating shape is drawn wherever
	// the rect is drawn, but the background remains at the higher level.
	vsgSetPen1(getVSG().lowlevel());
	vsgDrawBar(this->x, -this->y, this->w, this->h, this->wd);

	// Set spatial, temporal waveforms, tf, etc stuff.
	this->select();
	setGratingObjProperties();

	// draw grating on HOST page
	vsgSetPen1(getFirstLevel());
	vsgSetPen2(getFirstLevel() + getNumLevels() - 1);
	vsgSetDrawMode(vsgCENTREXY + vsgTRANSONLOWER); //vsgSetDrawMode(vsgCENTREXY + vsgTRANSONHIGHER);
	vsgDrawGrating(this->x, -this->y, bboxW, bboxH, this->orientation, this->sf);

	// restore the original draw page -- this is where we want our specially-shaped grating
	vsgSetDrawPage(descr.PageZone, descr.Page, vsgNOCLEAR);

	vsgSetDrawMode(vsgCENTREXY);
	vsgSetPen1(getVSG().lowlevel());
	vsgDrawBar(this->x, -this->y, this->w, this->h, this->wd);

	// Now move the rect back to the original draw page. 
	// TRANSONLOWER (MAX mode) ensures that the grating shape will overwrite the rect that was just drawn on the low level. 
	// We need TRANSONSOURCE, however, to keep the extra "background" surrounding the shape from overwriting the stuff on the draw page.
	// The TRANSONSOURCE makes that level transparent - meaning the destination level wins out.
	vsgSetDrawMode(vsgCENTREXY + vsgTRANSONSOURCE + vsgTRANSONLOWER);
	vsgSetPen2(getVSG().highlevel()); //vsgSetPen2(lvLow);
	vsgDrawMoveRect(vsgHOSTPAGE, getVSG().hostpage_handle(), this->x, -this->y, bboxW, bboxH, this->x, -this->y, bboxW, bboxH);

	// restore a normal draw mode
	vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);

	return status;
}


int ARGratingSpec::draw()
{
	if (this->aperture == oriented_rectangle)
		return drawOrientedRectangle();
	else
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


	// If this is NOT a multigrating, fake it by adding a single dummy element to m_mutli
	if (!bIsMulti)
	{
		m_multi.push_back(make_tuple(this->x, this->y, this->orientation));
	}


	// draw each element of m_multi
	for (std::tuple<double, double, double> tup : m_multi)
	{
		this->x = std::get<0>(tup);
		this->y = std::get<1>(tup);
		this->orientation = std::get<2>(tup);
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
				vsgDrawOval(x, -1 * y, w, h);
			}
		}

		// Set spatial, temporal waveforms, tf, etc stuff.
		setGratingObjProperties();

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
			vsgSetPen2(getFirstLevel() + getNumLevels() - 1);
			vsgDrawGrating(this->x, -1 * this->y, this->w, this->h, this->orientation, this->sf);
		}
		else
		{
			vsgSetDrawMode(vsgCENTREXY);
			vsgSetPen1(getFirstLevel());
			vsgSetPen2(getFirstLevel() + getNumLevels() - 1);
			vsgDrawGrating(this->x, -1 * this->y, this->w, this->h, this->orientation, this->sf);
		}

		// Draw hole if this is a donut. Having both hd, wd > 0 makes this a donut.
		// Check for dumb situation w,h == 0.
		if (w > 0 && h > 0)
		{
			vsgSetPen1(vsgBACKGROUND);

			// Now draw the hole, but only if the diam is >0
			if (wd > 0 && hd > 0)
			{
				if (this->aperture == ellipse)
				{
					vsgSetDrawMode(vsgCENTREXY);
					vsgDrawOval(x, -1 * y, wd, hd);
				}
				else
				{
					vsgSetDrawMode(vsgCENTREXY);
					vsgDrawRect(this->x, -1 * this->y, this->wd, this->hd);
				}
			}
		}
	}

	if (!bIsMulti)
		m_multi.clear();

	return 0;
}

void ARGratingSpec::setGratingObjProperties()
{
	// Set spatial waveform
	if (this->swt == sinewave)
	{
		vsgObjTableSinWave(vsgSWTABLE);
	}
	else
	{
		// Set up standard 50:50 square wave
		vsgObjTableSquareWave(vsgSWTABLE, (DWORD)(vsgObjGetTableSize(vsgSWTABLE) * 0.25), (DWORD)(vsgObjGetTableSize(vsgSWTABLE) * 0.75));
	}

	// get color vector
	VSGTRIVAL from = this->cv.from();
	VSGTRIVAL to = this->cv.to();
	vsgObjSetColourVector(&from, &to, vsgBIPOLAR);

	// set temporal waveform
	if (this->twt == sinewave)
	{
		vsgObjTableSinWave(vsgTWTABLE);
	}
	else
	{
		// Set up standard 50:50 square wave
		vsgObjTableSquareWave(vsgTWTABLE, (DWORD)(0), (DWORD)(vsgObjGetTableSize(vsgTWTABLE) * 0.5));
	}
	vsgObjSetTemporalPhase(0);

	vsgObjSetContrast(contrast);
	vsgObjSetSpatialPhase(phase);
	vsgObjSetDriftVelocity(tf);
	vsgObjSetTemporalFrequency(ttf);

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

int ARGratingSpec::drawOverlay(PIXEL_LEVEL)
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
	from = this->cv.from();
	to = this->cv.to();
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


// call this instead of init()
void ARConteSpec::init(int nlevels, bool bCreate)
{
	// reserve the levels, but do not create a vsg object. 
	// do not call vsgObj*** on this thing. 
	ARObject::init(nlevels, false);

	m_level_cue = this->getFirstLevel();
	PIXEL_LEVEL d = (nlevels - 1) / 3;
	m_level_low_gabor = m_level_cue + 1;
	m_level_high_gabor = m_level_low_gabor + 2 * d;
	m_level_low_flanker = m_level_high_gabor + 1;
	m_level_high_flanker = this->getFirstLevel() + nlevels - 1;
	cerr << "conte levels " << m_level_cue << " " << m_level_low_gabor << " " << m_level_high_gabor << " " << m_level_low_flanker << " " << m_level_high_flanker << endl;

	//// black-white ramp. Cue color should be set elsewhere (spec just holds level)
	//m_ramp_low = this->getFirstLevel() + 1;
	//m_ramp_high = this->getFirstLevel() + this->getNumLevels() - 1;
	//m_ramp_mid = (m_ramp_high + m_ramp_low) / 2;
	//arutil_ramp_to_palette(COLOR_TYPE(black), COLOR_TYPE(white), m_ramp_low, m_ramp_high);
}


// get coords for drawing the gaussians: (x[0], y[0]) and (x[1],y[1])
// also coords of border in rect (in VSG positive-down coord system)
// No need to modify the y-coord (*-1) for actual drawing
// rect[0]: left-hand x coord
// rect[1]: right-hand
// rect[2]: top
// rect[3]: bottom
void ARConteSpec::getDrawingCoordinates(double(&x)[2], double(&y)[2], double(&rect)[4])
{
	if (this->iHorizontal == 1 || this->iHorizontal == -1)
	{
		x[0] = this->x - this->w;
		x[1] = this->x + this->w;
		y[0] = y[1] = -1 * this->y;
		rect[0] = this->x - 1.5 * this->w;
		rect[1] = this->x + 1.5 * this->w;
		rect[2] = -1 * this->y - 0.5 * this->h;
		rect[3] = -1 * this->y + 0.5 * this->h;
	}
	else if (this->iHorizontal == 0 || this->iHorizontal == -2)
	{
		y[0] = -1 * this->y - this->h;
		y[1] = -1 * this->y + this->h;
		x[0] = x[1] = this->x;
		rect[0] = this->x - 0.5 * this->w;
		rect[1] = this->x + 0.5 * this->w;
		rect[2] = -1 * this->y - 1.5 * this->h;
		rect[3] = -1 * this->y + 1.5 * this->h;
	}
	else
	{
		y[0] = -1 * this->y - this->h;
		y[1] = -1 * this->y + this->h;
		x[0] = x[1] = this->x;
		rect[0] = this->x - 0.5 * this->w;
		rect[1] = this->x + 0.5 * this->w;
		rect[2] = -1 * this->y - 0.5 * this->h;
		rect[3] = -1 * this->y + 0.5 * this->h;
	}
}

// When drawing filled rect because cue lwt>=99, this returns the width/height
// for the rect
void ARConteSpec::getDrawRectArgs(double &w, double &h)
{
	if (this->iHorizontal == 1 || this->iHorizontal == -1)
	{
		w = 3 * this->w;
		h = this->h;
	}
	else if (this->iHorizontal == 0 || this->iHorizontal == -2)
	{
		h = 3 * this->w;
		w = this->h;
	}
	else
	{
		cerr << "ARConteSpec::getDrawRectArgs - iHorizontal = " << this->iHorizontal << ": not handled." << endl;
	}
}






int ARConteSpec::draw()
{
	double xx[2], yy[2];	// coordinates (centers) for the two gaussian flankers (xx[0], yy[0]) and (xx[1], yy[1])
	double rect[4];			// coordinates (corners) for the border cue rectangle
	DWORD old_mode = vsgGetDrawMode();
	VSGTRIVAL trival_from = COLOR_TYPE(black).trival();
	VSGTRIVAL trival_to = COLOR_TYPE(white).trival();

	// Set up colors for gabor
	COLOR_TYPE diff = (COLOR_TYPE(white) - COLOR_TYPE(gray)) * (gaborContrast / 100.0);
	COLOR_TYPE color0 = COLOR_TYPE(gray) - diff;
	COLOR_TYPE color1 = COLOR_TYPE(gray) + diff;
	arutil_ramp_to_palette(color0, color1, m_level_low_gabor, m_level_high_gabor);

	// color for flankers
	COLOR_TYPE flankerHigh = COLOR_TYPE(gray) + (COLOR_TYPE(white) - COLOR_TYPE(gray)) * (flankerContrast / 100.0);
	arutil_ramp_to_palette(COLOR_TYPE(gray), flankerHigh, m_level_low_flanker, m_level_high_flanker);


	// if the cue line width is large (>=99) it means that we don't draw the gaussian and flankers, 
	// because instead of a box around the cue, the cue will be a filled rectangle. In that case, the
	// cue 'border', drawn on the overlay, will obscure the grating/flanker so no need to draw. 
		// color for the cue rectangle
	COLOR_TYPE c = COLOR_TYPE(gray) + (this->cueColor - COLOR_TYPE(gray)) * (this->cueContrast / 100.0);
	arutil_color_to_palette(c, m_level_cue);
	if (this->cueLineWidth < 99)
	{
		// draw gabor
		double gabor_dev = 0.5 * (this->w + this->h) / this->divisor;
		vsgSetDrawMode(vsgCENTREXY);
		vsgSetPen1(m_level_low_gabor);
		vsgSetPen2(m_level_high_gabor);
		vsgDrawGabor(this->x, -1 * this->y, this->w, this->h, this->orientation, this->sf, gabor_dev, this->phase);

		// get coords for drawing gaussian and border cues
		getDrawingCoordinates(xx, yy, rect);

		// now draw the gaussian flankers, but only if is_horizontal is not negative (which means do not draw)
		if (this->iHorizontal > -1)
		{
			vsgSetPen1(this->m_level_high_flanker);
			vsgSetPen2(this->m_level_low_flanker);
			vsgDrawGaussian(xx[0], yy[0], this->w, this->h, gabor_dev);
			vsgDrawGaussian(xx[1], yy[1], this->w, this->h, gabor_dev);
		}

		// draw border cue, even when flankers are not drawn (borders should be set correctly)
		// TEST - if cue line width > 5 (arbitrary), draw rectangles which tend towards the center
		if (this->cueLineWidth > 0)
		{
			if (this->cueLineWidth > 5)
			{
				// convert pixels to degrees
				double wdeg;
				vsgUnit2Unit(vsgPIXELUNIT, this->cueLineWidth, vsgDEGREEUNIT, &wdeg);

				// set draw mode
				vsgSetDrawMode(vsgSOLIDFILL);	// do not use CENTREXY here

				// color
				vsgSetPen1(this->m_level_cue);

				// draw 4 rectangles as the "border". in drawRect, x,y pos is top-left
				// of the rect!
				// Note depending on order of rect elements!
				double rectWidthDeg = rect[1] - rect[0];
				double rectHeightDeg = rect[3] - rect[2];
				vsgDrawRect(rect[0], rect[2], wdeg, rectHeightDeg);			// left-hand side
				vsgDrawRect(rect[1] - wdeg, rect[2], wdeg, rectHeightDeg);	// right-hand side
				vsgDrawRect(rect[0], rect[2], rectWidthDeg, wdeg);			// top
				vsgDrawRect(rect[0], rect[3] - wdeg, rectWidthDeg, wdeg);	// bottom
			}
			else
			{
				vsgSetDrawMode(vsgSOLIDPEN);
				vsgSetPenSize(this->cueLineWidth, this->cueLineWidth);
				vsgSetPen1(this->m_level_cue);
				vsgDrawLine(rect[0], rect[2], rect[1], rect[2]);	// top
				vsgDrawLine(rect[1], rect[2], rect[1], rect[3]);	// right
				vsgDrawLine(rect[1], rect[3], rect[0], rect[3]);	// bottom
				vsgDrawLine(rect[0], rect[3], rect[0], rect[2]);	// left
			}
		}
	}
	else
	{
		vsgSetDrawMode(vsgCENTREXY);
		vsgSetPen1(this->m_level_cue);
		double hh=0, ww=0;
		getDrawRectArgs(ww, hh);
		vsgDrawRect(this->x, -this->y, ww, hh);
	}

	// restore draw mode
	vsgSetDrawMode(old_mode);

	return 0;
}

int ARConteSpec::drawOverlay(PIXEL_LEVEL ovLevel)
{
	double x[2], y[2], rect[4];
	DWORD old_mode = vsgGetDrawMode();

	// We're only going to draw the cue rectangles! Get the color into the overlay palette first
	COLOR_TYPE c = COLOR_TYPE(gray) + (this->cueColor - COLOR_TYPE(gray)) * (this->cueContrast / 100.0);
	arutil_color_to_overlay_palette(c, ovLevel);

	// If the line width is a large number (>=99), then draw as a solid rectangle.

	if (this->cueLineWidth < 99)
	{
		vsgSetDrawMode(vsgSOLIDPEN);
		vsgSetPenSize(this->cueLineWidth, this->cueLineWidth);
		vsgSetPen1(ovLevel);
		getDrawingCoordinates(x, y, rect);
		vsgDrawLine(rect[0], rect[2], rect[1], rect[2]);	// top
		vsgDrawLine(rect[1], rect[2], rect[1], rect[3]);	// right
		vsgDrawLine(rect[1], rect[3], rect[0], rect[3]);	// bottom
		vsgDrawLine(rect[0], rect[3], rect[0], rect[2]);	// left
	}
	else
	{
		vsgSetDrawMode(vsgCENTREXY);
		vsgSetPen1(ovLevel);
		double hh = 0, ww = 0;
		getDrawRectArgs(ww, hh);
		vsgDrawRect(this->x, -this->y, ww, hh);
	}

	// restore draw mode
	vsgSetDrawMode(old_mode);

	return 0;
}


int ARImageSpec::drawOverlay(PIXEL_LEVEL ovLevel)
{
	return -1;
}

int ARImageSpec::draw()
{
	// fetch and write palette
	// TODO: load palette when loading file
	// TODO: palette write should respect first/nlevels? 
	VSGLUTBUFFER lut;
	long pstatus;
	pstatus = vsgImageGetPalette(0, this->filename, &lut);
	if (pstatus)
	{
		switch (pstatus)
		{
		case vsgerrorERRORREADINGFILE: cerr << "get palette vsgerrorERRORREADINGFILE" << endl; break;
		case vsgerrorUNSUPPORTEDIMAGETYPE: cerr << "get palette vsgerrorUNSUPPORTEDIMAGETYPE" << endl; break;
		case vsgerrorUNSUPPORTEDBITMAPFORMAT: cerr << "get palette vsgerrorUNSUPPORTEDBITMAPFORMAT" << endl; break;
		case vsgerrorOUTOFPCMEMORY: cerr << "get palette vsgerrorOUTOFPCMEMORY" << endl; break;
		case vsgerrorIMAGEHASNOPALETTE: cerr << "get palette vsgerrorIMAGEHASNOPALETTE" << endl; break;
		default: cerr << "get palette error: " << pstatus << " filename " << this->filename << " len " << strlen(this->filename) << endl; break;
		}
	}
	vsgPaletteWrite((VSGLUTBUFFER*)lut, 0, 64);

	// draw image
	if (strlen(this->filename)) vsgDrawImage(vsgBMPPICTURE, this->x, -this->y, this->filename);

	return 0;
}
