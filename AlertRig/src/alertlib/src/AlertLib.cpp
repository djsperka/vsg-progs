#include "Alertlib.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>



using namespace std;
using namespace alert;

std::ostream& operator<<(std::ostream& out, const alert::ARFixationPointSpec& arfps)
{
	out << "(x,y): (" << arfps.x << ", " << arfps.y << ") diameter: " << arfps.d << " color: " << arfps.color;
	return out;
}

std::ostream& operator<<(std::ostream& out, const alert::ARGratingSpec& args)
{
	out << "(x,y): (" << args.x << ", " << args.y << ") (w,h): (" << args.w << ", " << args.h << ") con%: " << args.contrast << " sf: " << args.sf << " tf: " << args.tf << " orient: " << args.orientation << " cv: " << args.cv << " patt: " << args.pattern << " aper: " << args.aperture;
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
	vsgDrawOval(x, y, d, d);
	return 0;
}

int alert::ARFixationPointSpec::drawOverlay()
{
	vsgSetPen1(0);	// overlay page transparent
	vsgDrawOval(x, y, d, d);
	return 0;
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
		ARFixationPointSpec::draw();
	}
	return status;
}

int alert::ARContrastFixationPointSpec::drawOverlay()
{
	vsgSetPen1(0);
	vsgDrawOval(x, y, d, d);
	return 0;
}


int alert::ARGratingSpec::draw()
{
	return draw(false);
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
			vsgSetPen1(0);
//			vsgSetPen2(0);
			vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
			vsgDrawOval(x, y, w, h);
		}
	}

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
		vsgObjTableSquareWave(vsgSWTABLE, vsgObjGetTableSize(vsgSWTABLE)*0.25, vsgObjGetTableSize(vsgSWTABLE)*0.75);
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
			vsgSetDrawMode(vsgCENTREXY + vsgTRANSONLOWER);
			vsgSetPen1(getFirstLevel());
			vsgSetPen2(getFirstLevel() + getNumLevels());
			vsgDrawGrating(this->x, this->y, this->w, this->h, this->orientation, this->sf);
//			vsgSetDrawMode(vsgCENTREXY);
		}
		else
		{
			vsgSetDrawMode(vsgCENTREXY);
			vsgDrawGrating(this->x, this->y, this->w, this->h, this->orientation, this->sf);
		}
	}
	else
	{
		vsgSetDrawMode(vsgCENTREXY);
		vsgDrawGrating(this->x, this->y, this->w, this->h, this->orientation, this->sf);
	}

	return 0;
}


int alert::ARGratingSpec::drawOverlay()
{
	vsgSetPen1(0);
	vsgSetDrawMode(vsgCENTREXY);
	if (this->aperture == ellipse)
	{
		vsgDrawOval(x, y, w, h);
	}
	else
	{
		vsgDrawRect(x, y, w, h);
	}
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


int alert::ARvsg::init(int screenDistanceMM, COLOR_TYPE i_bg)
{
	VSGTRIVAL background;
	int status=0;
	if (!m_initialized)
	{
		if (vsgInit(""))
		{
			cerr << "Error in vsgInit()." << endl;
			status=1;
		}
		else
		{
			Sleep(2000);
			m_initialized = true;
			vsgSetViewDistMM(screenDistanceMM);
			vsgSetSpatialUnits(vsgDEGREEUNIT);
			vsgSetCommand(vsgPALETTECLEAR);
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
			vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&background, 1, 1);
			vsgSetDrawPage(vsgOVERLAYPAGE, 0, 1);
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
		vsgPaletteWrite((VSGLUTBUFFER*)&background, m_background_level, 1);


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



