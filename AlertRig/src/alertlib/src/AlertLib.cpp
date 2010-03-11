#include "Alertlib.h"
#include "RegHelper.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

using namespace std;
using namespace alert;

std::ostream& operator<<(std::ostream& out, const alert::ARFixationPointSpec& arfps)
{
	out << arfps.x << ", " << arfps.y << "," << arfps.d << "," << arfps.color;
	return out;
}

std::ostream& operator<<(std::ostream& out, const alert::ARGratingSpec& args)
{
	out << args.x << "," << args.y << "," << args.w << "," << args.h << "," << args.contrast << "," << args.sf << "," << args.tf << "," << args.orientation << "," << args.cv << "," << args.pattern << "," << args.aperture;
	return out;
}

std::ostream& operator<<(std::ostream& out, const alert::Trigger& t)
{
	out << t.toString();
	return out;
}




int alert::ARObject::select()
{
	if (m_initialized) 
	{
		vsgObjSelect(m_handle);
		return 0;
	}
	else return -1;
};


void alert::ARObject::init(int numlevels)
{
	PIXEL_LEVEL level;
	if (numlevels>1)
	{
		LevelManager::instance().request_range(numlevels, level);
		init(level, numlevels);
	}
	else if (numlevels==1)
	{
		LevelManager::instance().request_single(level);
		init(level, numlevels);
	}
	else
	{
		cerr << "Error - number of levels must be >0!" << endl;
	}
}



void alert::ARObject::init(PIXEL_LEVEL first, int numlevels)
{
	m_handle = vsgObjCreate();
	m_first = first;
	m_numlevels = numlevels;

	vsgObjSetDefaults();
	vsgObjSetContrast(100);
	vsgObjSetPixelLevels(first, numlevels);
	m_initialized = true;

	cerr << "init obj on level " << first << ", with " << numlevels << " levels" << endl;
}


int alert::ARFixationPointSpec::draw()
{
	vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
	vsgDrawOval(x, -1*y, d, d);
	return 0;
}


// oh-oh! The fixpt will hijack overlay palette color level 2!
int alert::ARFixationPointSpec::drawOverlay()
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


int alert::ARContrastFixationPointSpec::draw()
{
	int status=0;
	VSGTRIVAL from, to;

	select();

	if (get_color(this->color, to))
	{
		cerr << "Cannot get color trival for point: " << this->color << endl;
		status=1;
	}
	else if (get_color(alert::ARvsg::instance().background_color(), from))
	{
		cerr << "Cannot get background color trival for point: " << alert::ARvsg::instance().background_color() << endl;
		status=1;
	}
	else 
	{
		vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);
		cerr << "from=" << from.a << "," << from.b << "," << from.c << " to=" << to.a << "," << to.b << "," << to.c << endl;
	}

	if (!status)
	{
		vsgSetPen1(getFirstLevel());
		ARFixationPointSpec::draw();
	}
	return status;
}

int alert::ARContrastFixationPointSpec::drawOverlay()
{
	vsgSetPen1(0);
	vsgDrawOval(x, -1*y, d, d);
	return 0;
}


int alert::ARContrastCircleSpec::draw()
{
	int status=0;
	VSGTRIVAL from, to;

	select();

	if (get_color(this->color, to))
	{
		cerr << "Cannot get color trival for point: " << this->color << endl;
		status=1;
	}
	else if (get_color(alert::ARvsg::instance().background_color(), from))
	{
		cerr << "Cannot get background color trival for point: " << alert::ARvsg::instance().background_color() << endl;
		status=1;
	}
	else 
	{
		vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);
		cerr << "from=" << from.a << "," << from.b << "," << from.c << " to=" << to.a << "," << to.b << "," << to.c << endl;
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
int alert::ARContrastCircleSpec::drawOverlay()
{
	vsgSetPen1(0);
	vsgDrawOval(x, -1*y, d, d);
	return 0;
}


int alert::ARContrastLineSpec::draw()
{
	int status=0;
	VSGTRIVAL from, to;

	select();

	if (get_color(this->color, to))
	{
		cerr << "Cannot get color trival for point: " << this->color << endl;
		status=1;
	}
	else if (get_color(alert::ARvsg::instance().background_color(), from))
	{
		cerr << "Cannot get background color trival for point: " << alert::ARvsg::instance().background_color() << endl;
		status=1;
	}
	else 
	{
		vsgObjSetColourVector(&from, &to, vsgUNIPOLAR);
		cerr << "from=" << from.a << "," << from.b << "," << from.c << " to=" << to.a << "," << to.b << "," << to.c << endl;
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
int alert::ARContrastLineSpec::drawOverlay()
{
	vsgSetPen1(0);
	vsgDrawLine(x0, y0, x1, y1);
	return 0;
}



int alert::ARGratingSpec::draw()
{
	return draw(false);
}

int alert::ARGratingSpec::redraw(bool useTransOnLower)
{
	int status=0;
	select();
	if (useTransOnLower)
	{
		// djs TRANSONLOWER seems to leave artifacts on the screen when we turn the contrast off. !!!
		// if ellipse, draw an ellipse on level 0 for TRANSONLOWER
		// djs. Specifically setting the level seems to lead to artifacts. I found that leaving the levels
		// unassigned solves this. I suspect this may lead to trouble someday.....
		if (this->aperture == ellipse)
		{
			vsgSetPen1(250);
//			vsgSetPen2(0);
			vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
			vsgDrawOval(x, -1*y, w, h);
		}
	}


	// Now draw
	if (this->aperture == ellipse)
	{
		if (useTransOnLower)
		{
			vsgSetDrawMode(vsgCENTREXY + vsgTRANSONLOWER);
			vsgSetPen1(getFirstLevel());
			vsgSetPen2(getFirstLevel() + getNumLevels());
			vsgDrawGrating(this->x, -1*this->y, this->w, this->h, this->orientation, this->sf);
//			vsgSetDrawMode(vsgCENTREXY);
		}
		else
		{
			vsgSetDrawMode(vsgCENTREXY);
			vsgDrawGrating(this->x, -1*this->y, this->w, this->h, this->orientation, this->sf);
		}
	}
	else
	{
		vsgSetDrawMode(vsgCENTREXY);
		vsgDrawGrating(this->x, -1*this->y, this->w, this->h, this->orientation, this->sf);
	}


	return 0;
}


int alert::ARGratingSpec::drawOnce()
{
	return draw(true);
}

int alert::ARGratingSpec::draw(bool useTransOnLower)
{
	int status=0;
	VSGTRIVAL from, to;

	select();

	// We assume that the handle is created and selected. In order to make this grating appear, you still must
	// assign pixel levels (vsgObjSetPixels). Note also that the contrast is initially set to 100% by the call to 
	// vsgObjSetDefaults().

	if (useTransOnLower)
	{
		// djs TRANSONLOWER seems to leave artifacts on the screen when we turn the contrast off. !!!
		// if ellipse, draw an ellipse on level 0 for TRANSONLOWER
		// djs. Specifically setting the level seems to lead to artifacts. I found that leaving the levels
		// unassigned solves this. I suspect this may lead to trouble someday.....
		if (this->aperture == ellipse)
		{
			vsgSetPen1(250);
//			vsgSetPen2(0);
			vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
			vsgDrawOval(x, -1*y, w, h);
		}
	}

	// Set object defauilts. setDefaults() sets contrast to 100% 
	// -- this may not be what we want, so reset contrast to the 
	// stim's current value
	vsgObjSetDefaults();
	vsgObjSetContrast(contrast);

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

	// set temporal freq
	vsgObjSetDriftVelocity(tf);

	// set color vector
	if (get_colorvector(this->cv, from, to))
	{
		cerr << "Cannot get color vector for type " << this->cv << endl;
	}
	vsgObjSetColourVector(&from, &to, vsgBIPOLAR);

	// Now draw
	if (this->aperture == ellipse)
	{
		if (useTransOnLower)
		{
			vsgSetDrawMode(vsgCENTREXY + vsgTRANSONHIGHER);
			vsgSetPen1(getFirstLevel());
			vsgSetPen2(getFirstLevel() + getNumLevels() -1);
			vsgDrawGrating(this->x, -1*this->y, this->w, this->h, this->orientation, this->sf);
//			vsgSetDrawMode(vsgCENTREXY);
		}
		else
		{
			vsgSetDrawMode(vsgCENTREXY);
			vsgDrawGrating(this->x, -1*this->y, this->w, this->h, this->orientation, this->sf);
		}
	}
	else
	{
		vsgSetDrawMode(vsgCENTREXY);
		vsgDrawGrating(this->x, -1*this->y, this->w, this->h, this->orientation, this->sf);
	}

	return 0;
}

int alert::ARGratingSpec::drawBackground()
{
	vsgSetPen1(ARvsg::instance().background_level());
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

int alert::ARGratingSpec::drawOverlay()
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






int alert::ARApertureGratingSpec::draw()
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






int alert::LevelManager::request_single(PIXEL_LEVEL& level)
{
	int status=0;
	if (m_next < 250)
	{
		level = m_next++;
	}
	else status=1;
	return status;
}

int alert::LevelManager::request_range(int num, PIXEL_LEVEL& first)
{
	int status=0;
	if (250-m_next >= num)
	{
		first = m_next;
		m_next += num;
	}
	else status=1;
	return status;
}


alert::ARvsg::~ARvsg()
{ 
	if (m_bHaveLock) 
	{
		clear(); 
		release_lock();
	} 
};

bool alert::ARvsg::acquire_lock()
{
	string name;
	int fd;
	int status;

	m_bHaveLock = false;

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
		m_bHaveLock = true;
		_close(fd);
	}
	return true;
}


void alert::ARvsg::release_lock()
{
	string name;

	// Get lock file name
	if (!GetRegLockFile(name))
	{
		cerr << "release_lock(): Cannot get lock file name from registry!" << endl;
		return;
	}
	remove(name.c_str());
	m_bHaveLock = false;
	return;
}



int alert::ARvsg::init(int screenDistanceMM, COLOR_TYPE i_bg)
{
	VSGTRIVAL background;
	int status=0;
	if (!m_initialized)
	{
		if (!acquire_lock())
		{
			cerr << "ARvsg::init(): cannot acquire VSG lock!" << endl;
			return 1;
		}
		
		if (vsgInit(""))
		{
			cerr << "Error in vsgInit()." << endl;
			return 1;
		}

		Sleep(2000);
		m_initialized = true;

		/*
		 * If screen distance is negative, then set spatial units to PIXEL units. 
		 */
		if (screenDistanceMM <= 0) 
		{
			vsgSetSpatialUnits(vsgPIXELUNIT);
			m_heightPixels = vsgGetScreenHeightPixels();
			m_widthPixels = vsgGetScreenWidthPixels();
		}
		else
		{
			vsgSetViewDistMM(screenDistanceMM);
			vsgSetSpatialUnits(vsgDEGREEUNIT);
			m_heightPixels = vsgGetScreenHeightPixels();
			m_widthPixels = vsgGetScreenWidthPixels();
			vsgUnitToUnit(vsgPIXELUNIT, m_heightPixels, vsgDEGREEUNIT, &m_heightDegrees);
			vsgUnitToUnit(vsgPIXELUNIT, m_widthPixels, vsgDEGREEUNIT, &m_widthDegrees);
		}
		m_background_color = i_bg;

		// this level gets used later, but we request it now to insure we get level 0
		alert::LevelManager::instance().request_single(m_background_level);
		
		// The background level was obtained in the init() call.
		if (get_color(m_background_color, background))
		{
			cerr << "Cannot get trival for background color " << m_background_color << endl;
			status = 2;
		}
		vsgSetBackgroundColour(&background);
	}
	return status;
}



int alert::ARvsg::init_overlay()
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
			int npages = vsgGetSystemAttribute(vsgNUMOVERLAYPAGES);
			// Get the number of overlay pages, then clear them all. 
			vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&background, 1, 1);
			for (int i=npages-1; i>=0; i--)
			{
				vsgSetDrawPage(vsgOVERLAYPAGE, i, 1);
			}
		}
	}
	return status;
}



int alert::ARvsg::init_video()
{
	return init_video_pages(NULL, NULL, NULL);
}




int alert::ARvsg::init_video_pages(voidfunc func_before_objects, voidfunc func_after_objects, void *data)
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
		alert::LevelManager::instance().request_single(dummy_level);
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

void alert::ARvsg::clear(int ipage)
{
	vsgSetDrawPage(vsgVIDEOPAGE, ipage, vsgBACKGROUND);
	vsgPresent();
}

void alert::ARvsg::clear()
{
	clear(0);
}



