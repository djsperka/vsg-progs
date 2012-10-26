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

std::ostream& operator<<(std::ostream& out, const ARFixationPointSpec& arfps)
{
	out << arfps.x << ", " << arfps.y << "," << arfps.d << "," << arfps.color;
	return out;
}

std::ostream& operator<<(std::ostream& out, const ARGratingSpec& args)
{
	out << args.x << "," << args.y << "," << args.w << "," << args.h << "," << args.wd << "," << args.hd << "," << args.contrast << "," << args.sf << "," << args.tf << "," << args.orientation << "," << args.phase << "," << args.cv << "," << args.pattern << "," << args.aperture;
	//out << "x,y=(" << args.x << "," << args.y << ") w,h,wd,hd=(" << args.w << "," << args.h << "," << args.wd << "," << args.hd << ") c=" << args.contrast << " sf=" << args.sf << " tf=" << args.tf << " ori=" << args.orientation << " ph=" << args.phase << " cv=" << args.cv << " p=" << args.pattern << " ap=" << args.aperture;
	return out;
}

std::ostream& operator<<(std::ostream& out, const Trigger& t)
{
	out << t.toString();
	return out;
}


ARvsg& ARvsg::instance() 
{
	static ARvsg vsg;
	return vsg; 
};

ARvsg& ARvsg::instance(bool is_master, bool is_slave)
{
	if (is_master && !is_slave) return ARvsg::master();
	if (!is_master && is_slave) return ARvsg::slave();
	return ARvsg::instance();
}

ARvsg& ARvsg::master() 
{
	static ARvsg master(true, false);
	return master; 
};

ARvsg& ARvsg::slave() 
{
	static ARvsg slave(false, true);
	return slave; 
};

COLOR_TYPE ARvsg::background_color() 
{ 
	return m_background_color; 
};

PIXEL_LEVEL ARvsg::background_level() 
{ 
	return m_background_level; 
};

long ARvsg::getScreenHeightPixels() 
{ 
	return m_heightPixels; 
};

long ARvsg::getScreenWidthPixels() 
{ 
	return m_widthPixels; 
};

double ARvsg::getScreenHeightDegrees() 
{ 
	return m_heightDegrees; 
};

double ARvsg::getScreenWidthDegrees() 
{ 
	return m_widthDegrees; 
};

ARvsg::ARvsg(bool bMaster, bool bSlave) : m_initialized(false), m_is_master(bMaster), m_is_slave(bSlave), m_device_handle(-999), m_next_available_level(0)
{
	cout << "ARvsg::ARvsg(" << bMaster << ", " << bSlave << ")" << endl;
}

ARvsg::~ARvsg()
{ 
	clear();
	if (c_bHaveLock) 
	{
		release_lock();
	} 
};

// static member variable. The lock need only be obtained once, even if using master/slave. 
bool ARvsg::c_bHaveLock = false;


bool ARvsg::acquire_lock()
{
	string name;
	int fd;
	int status;

	// If the lock was already obtained, return.
	if (c_bHaveLock) return true;



	// Get lock file name
	if (!GetRegLockFile(name))
	{
		cerr << "acquire_lock(): Cannot get lock file name from registry!" << endl;
		return false;
	}

	status = _sopen_s(&fd, name.c_str(), O_WRONLY | O_CREAT | O_EXCL, _SH_DENYRW, _S_IREAD | _S_IWRITE);

	if (fd < 0)
	{
		cerr << "acquire_lock(): Cannot get lock." << endl;
		return false;
	}
	else
	{
		cerr << "acquire_lock(): got vsg lock." << endl;
		c_bHaveLock = true;
		_close(fd);
	}
	return true;
}


void ARvsg::release_lock()
{
	string name;

	// If the lock was already released, return.
	if (!c_bHaveLock) return;

	// Get lock file name
	if (!GetRegLockFile(name))
	{
		cerr << "release_lock(): Cannot get lock file name from registry!" << endl;
		return;
	}
	remove(name.c_str());
	c_bHaveLock = false;
	return;
}

bool ARvsg::is_master()
{
	return m_is_master;
}

bool ARvsg::is_slave()
{
	return m_is_slave;
}


void ARvsg::select()
{
	vsgInitSelectDevice(m_device_handle);
}


int ARvsg::init(int screenDistanceMM, COLOR_TYPE i_bg,  bool bUseLockFile, bool bSlaveSynch)
{
	VSGTRIVAL background;
	int status=0;
	if (!m_initialized)
	{
		if (bUseLockFile)
		{
			if (!acquire_lock())
			{
				cerr << "ARvsg::init(): cannot acquire VSG lock!" << endl;
				return 1;
			}
		}
		else
		{
			cout << "ARvsg::init(): ignoring lock file!" << endl;
		}


		// Initialize VSG card. 
		// Exactly WHICH vsg card should be initialize depends on the particulars of what this ARvsg represents. 
		// If running master/slave setup, then m_is_master==true or m_is_slave==true, and then specific 
		// initializations should be done using the VSG configurations indicated in the registry key/value 
		// HKCU\Software\Spike2\VSGMaster (or VSGSlave). 
		// If running single VSG, then m_is_master == m_is_slave == false. In this case use the VSG configuration
		// indicated in HKCU\Software\Spike2\VSGConfig IF IT EXISTS. If it does not exist, then init with the current
		// default VSG card (i.e. use "" as the arg to vsgInit). 
		
		if (!m_is_master && !m_is_slave)
		{
			string s;
			if (!GetRegVSGConfig(s))
			{
				cout << "Initialize VSG using currently selected configuration (see VSG Desktop)" << endl;
				m_device_handle = vsgInit("");
			}
			else
			{
				cout << "Initialize VSG using configuration file \"" << s << "\"" << endl;
				m_device_handle = vsgInit(const_cast<char *>(s.c_str()));
			}
		}
		else if (m_is_master)
		{
			string s;
			if (!GetRegVSGMaster(s))
			{
				cerr << "ERROR: Registry key HKCU\\Software\\Spike2\\VSGMaster not found!" << endl;
				return 1;
			}
			else
			{
				cout << "Initialize Master VSG using configuration file \"" << s << "\"" << endl;
				m_device_handle = vsgInit(const_cast<char *>(s.c_str()));
			}
		}
		else if (m_is_slave)
		{
			string s;
			if (!GetRegVSGSlave(s))
			{
				cerr << "ERROR: Registry key HKCU\\Software\\Spike2\\VSGSlave not found!" << endl;
				return 1;
			}
			else
			{
				cout << "Initialize Slave VSG using configuration file \"" << s << "\"" << endl;
				if (bSlaveSynch)
				{
					m_device_handle = vsgAdvancedInit(const_cast<char *>(s.c_str()), vsgADVINITSLAVEVIDEO);
				}
				else
				{
					cout << "WARNING: Non-enslavement initialization!!! No frame sync!!" << endl;
					m_device_handle = vsgInit(const_cast<char *>(s.c_str()));
				}
			}
		}

		if (m_device_handle < 0)
		{
			cerr << "Error (" << m_device_handle << ")in vsgInit()." << endl;
			return 1;
		}

		Sleep(2000);
		m_initialized = true;

		/*
		 * If screen distance is negative, then set spatial units to PIXEL units. 
		 */

		vsgInitSelectDevice(m_device_handle);

		if (screenDistanceMM <= 0) 
		{
			// djs 3-22-11
			// spatial units are set to pixel units with the init() call. 
			//vsgSetSpatialUnits(vsgPIXELUNIT);
			m_heightPixels = vsgGetScreenHeightPixels();
			m_widthPixels = vsgGetScreenWidthPixels();
			m_background_color = i_bg;
			request_single(m_background_level);
			if (get_color(m_background_color, background))
			{
				cerr << "Cannot get trival for background color " << m_background_color << endl;
				status = 2;
			}
			arutil_color_to_palette(m_background_color, m_background_level);
			cout << "Background level " << m_background_level << " color set to " << m_background_color.color.a << "," << m_background_color.color.b << "," << m_background_color.color.c << endl;
		}
		else
		{
			vsgSetViewDistMM(screenDistanceMM);
			vsgSetSpatialUnits(vsgDEGREEUNIT);
			m_heightPixels = vsgGetScreenHeightPixels();
			m_widthPixels = vsgGetScreenWidthPixels();
			vsgUnitToUnit(vsgPIXELUNIT, m_heightPixels, vsgDEGREEUNIT, &m_heightDegrees);
			vsgUnitToUnit(vsgPIXELUNIT, m_widthPixels, vsgDEGREEUNIT, &m_widthDegrees);

			m_background_color = i_bg;
			// this level gets used later, but we request it now to insure we get level 0
			request_single(m_background_level);

			// Create single dummy object and assign it a level
			m_handle = vsgObjCreate();
			vsgObjSetPixelLevels(m_background_level, 1);
			
			// The background level was obtained in the init() call.
			if (get_color(m_background_color, background))
			{
				cerr << "Cannot get trival for background color " << m_background_color << endl;
				status = 2;
			}
			//vsgPaletteSet(m_background_level, m_background_level, &background);
			vsgPresent();
			vsgSetBackgroundColour(&background);
			cout << "Background color set to " << background.a << "," << background.b << "," << background.c << endl;
		}
	}
	return status;
}



int ARvsg::init_overlay()
{
	int status=0;
	VSGTRIVAL background;

	if (!m_initialized)
	{
		cerr << "init_overlay: Must call init first!" << endl;
		status=1;
	}
	else 
	{		
		vsgSetCommand(vsgOVERLAYMASKMODE);
		if (get_color(m_background_color, background))
		{
			cerr << "Cannot get trival for background color " << m_background_color << endl;
			status = 2;
		}
		else
		{
			// Get the number of overlay pages, then clear them all. 
			int npages = vsgGetSystemAttribute(vsgNUMOVERLAYPAGES);
			cout << "There are " << npages << " overlay pages." << endl;
			vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&background, 1, 1);
			for (int i=npages-1; i>=0; i--)
			{
				vsgSetDrawPage(vsgOVERLAYPAGE, i, 1);
			}
		}
	}
	return status;
}



int ARvsg::init_video()
{
	return init_video_pages(NULL, NULL, NULL);
}




int ARvsg::init_video_pages(voidfunc func_before_objects, voidfunc func_after_objects, void *data)
{
	int i;
	int status=0;
	VSGTRIVAL background;
	PIXEL_LEVEL dummy_level;

	if (!m_initialized)
	{
		cerr << "Must call ARvsg::init first!" << endl;
		return 1;
	}
	else 
	{		
		// set background color, er colour, in palette
		// We'll set the vsgBACKGROUND color later, after the vsgObject is created. 
		// The background level was obtained in the init() call.
		if (get_color(m_background_color, background))
		{
			cerr << "Cannot get trival for background color " << m_background_color << endl;
			status = 2;
		}


		VSGLUTBUFFER buffer;
		for(i=0; i<256; i++) buffer[i] = background;
		vsgLUTBUFFERWrite(0,&buffer);
		vsgLUTBUFFERtoPalette(0);
		
		
		
		
		
//		vsgPaletteWrite((VSGLUTBUFFER*)&background, m_background_level, 1);
//		vsgPaletteWrite((VSGLUTBUFFER*)&background, 250, 1);
//		for (i = 0; i<252; i++) vsgPaletteWrite((VSGLUTBUFFER*)&background, i, 1);

		// Now clear all pages to background level, then call the before_objects callback, 
		// if it exists. 
		for (i=0; i<vsgGetSystemAttribute(vsgNUMVIDEOPAGES); i++)
		{
			vsgSetDrawPage(vsgVIDEOPAGE, i, m_background_level);
			if (func_before_objects)
			{
				func_before_objects(i, data);
			}
		}
		

		// Create a single vsg object and set vsgBACKGROUND color
		m_handle = vsgObjCreate();
		request_single(dummy_level);
		vsgObjSetPixelLevels(dummy_level, 1);
		cerr << " Dummy object " << (int)m_handle << " level " << (int)dummy_level << endl;
		vsgSetBackgroundColour(&background);


		// Now call after_objects callback if not null
		if (func_after_objects)
		{
			for (i=0; i<vsgGetSystemAttribute(vsgNUMVIDEOPAGES); i++)
			{
				vsgSetDrawPage(vsgVIDEOPAGE, i, vsgNOCLEAR);
				func_after_objects(i, data);
			}
		}

		// Finally, set page 0 as the current page and present. 
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		vsgPresent();
	}
	return status;
};

void ARvsg::clear(int ipage)
{
	vsgSetDrawPage(vsgVIDEOPAGE, ipage, vsgBACKGROUND);

	// djs 4-30-2010 
	// modify this to set the display page to the just-cleared page. Not sure why the vsgPresent() was here before. 
	vsgSetZoneDisplayPage(vsgVIDEOPAGE, ipage);
	//vsgPresent();
}

void ARvsg::clear()
{
	clear(0);
}

void ARvsg::ready_pulse(int wait_msecs, unsigned int which_bit)
{
	// Issue "ready" triggers to spike2.
	// These commands pulse spike2 port 6. 
	Sleep(wait_msecs);
	vsgIOWriteDigitalOut(0xff, which_bit);
	Sleep(10);
	vsgIOWriteDigitalOut(0, which_bit);
}


int ARvsg::remaining() 
{ 
	return 250-m_next_available_level; 
}

int ARvsg::request_single(PIXEL_LEVEL& level)
{
	int status=0;
	if (m_next_available_level < 250)
	{
		level = m_next_available_level++;
	}
	else status=1;
	return status;
}

int ARvsg::request_range(int num, PIXEL_LEVEL& first)
{
	int status=0;
	if (remaining() >= num)
	{
		first = m_next_available_level;
		m_next_available_level += num;
	}
	else status=1;
	return status;
}





//static ARvsg& f_vsg_default = ARvsg::instance();

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
		getVSG().request_range(numlevels, level);
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
	m_use_master = vsg.is_master();
	m_use_slave = vsg.is_slave();
	init(first, numlevels);
}

void ARObject::init(ARvsg& vsg, int numlevels)
{
	m_use_master = vsg.is_master();
	m_use_slave = vsg.is_slave();
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

void ARObject::setContrast(int contrast) 
{ 
	select(); 
	vsgObjSetContrast(contrast); 
};

VSGOBJHANDLE ARObject::handle() 
{ 
	return m_handle; 
};

bool ARObject::initialized() 
{ 
	return m_initialized; 
};

PIXEL_LEVEL ARObject::getFirstLevel() 
{ 
	return m_first; 
};

int ARObject::getNumLevels() 
{ 
	return m_numlevels; 
};

ARvsg& ARObject::getVSG()
{
	return ARvsg::instance(m_use_master, m_use_slave);
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
	vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
	vsgDrawBar(x, y, w, h, orientation);
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

	if (get_color(this->color, to))
	{
		cerr << "Cannot get color trival for point: " << this->color << endl;
		status=1;
	}
	else if (get_color(getVSG().background_color(), from))
	{
		cerr << "Cannot get background color trival for point: " << getVSG().background_color() << endl;
		status=1;
	}
	else 
	{
		vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);
	}

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
	if (get_color(this->color, to))
	{
		cerr << "Cannot get color trival for point: " << this->color << endl;
		status=1;
	}
	else if (get_color(getVSG().background_color(), from))
	{
		cerr << "Cannot get background color trival for point: " << getVSG().background_color() << endl;
		status=1;
	}
	else 
	{
		vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);
	}

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
	vsgSetPen1(getFirstLevel());
	vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
	vsgDrawOval(x, -1*y, d, d);
	return 0;
}


// oh-oh! The fixpt will hijack overlay palette color level 2!
int ARFixationPointSpec::drawOverlay()
{
	int status=0;
	VSGTRIVAL c;
	if (get_color(this->color, c))
	{
		cerr << "Cannot get color for fix pt: " << this->color << endl;
		status=1;
	}
	else
	{
		vsgPaletteWriteOverlayCols((VSGLUTBUFFER *)&c, 2, 1);
		vsgSetPen1(2);	// overlay page transparent
		vsgDrawOval(x, -1*y, d, d);
	}
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

	if (get_color(this->color, to))
	{
		cerr << "Cannot get color trival for point: " << this->color << endl;
		status=1;
	}
	else if (get_color(getVSG().background_color(), from))
	{
		cerr << "Cannot get background color trival for point: " << getVSG().background_color() << endl;
		status=1;
	}
	else 
	{
		vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);
	}

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

ARContrastCircleSpec::ARContrastCircleSpec(const ARContrastCircleSpec& c) : ARContrastFixationPointSpec(c)
{
}

ARContrastCircleSpec& ARContrastCircleSpec::operator=(const ARContrastCircleSpec& c)
{
	if (this != &c)
	{
		ARContrastFixationPointSpec::operator=(c);
	}
	return *this;
}


int ARContrastCircleSpec::draw()
{
	int status=0;
	VSGTRIVAL from, to;

	select();

	if (get_color(this->color, to))
	{
		cerr << "Cannot get color trival for point: " << this->color << endl;
		status=1;
	}
	else if (get_color(getVSG().background_color(), from))
	{
		cerr << "Cannot get background color trival for point: " << getVSG().background_color() << endl;
		status=1;
	}
	else 
	{
		vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);
	}

	if (!status)
	{
		vsgSetPen1(getFirstLevel());
		vsgSetDrawMode(vsgCENTREXY + vsgPIXELPEN);
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

	if (get_color(this->color, to))
	{
		cerr << "Cannot get color trival for point: " << this->color << endl;
		status=1;
	}
	else if (get_color(getVSG().background_color(), from))
	{
		cerr << "Cannot get background color trival for point: " << getVSG().background_color() << endl;
		status=1;
	}
	else 
	{
		vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);
	}

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
	pattern = g.pattern;
	aperture = g.aperture;
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
		pattern = g.pattern;
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


int ARGratingSpec::draw(long mode)
{
	int status=0;
	int ipen;
	VSGTRIVAL from, to;

	select();

	// We assume that the handle is created and selected. In order to make this grating appear, you still must
	// assign pixel levels (vsgObjSetPixels). Note also that the contrast is initially set to 100% by the call to 
	// vsgObjSetDefaults().

	if (mode == vsgTRANSONHIGHER) ipen = 255;
	else ipen = 0;

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

	if (!m_bDrawInitDone)
	{
		// Set object defauilts. setDefaults() sets contrast to 100% 
		// -- this may not be what we want, so reset contrast to the 
		// stim's current value
		//vsgObjSetDefaults();
		//vsgObjSetContrast(contrast);

		// assign pixel levels for object
		vsgObjSetPixelLevels(getFirstLevel(), getNumLevels());

		// Set spatial waveform
		if (this->pattern == sinewave)
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

		m_bDrawInitDone = true;
	}

	vsgObjSetContrast(contrast);
	vsgObjSetSpatialPhase(phase);
	vsgObjSetDriftVelocity(tf);

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
	vsgSetPen1(getVSG().background_level());
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
	if (this->pattern == sinewave)
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
