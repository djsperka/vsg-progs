#include "AlertUtil.h"
#include <iostream>
using namespace std;

int arutil_color_to_overlay_palette(ARFixationPointSpec& fp, PIXEL_LEVEL level)
{
	int status=0;
	VSGTRIVAL c;
	if (get_color(fp.color, c))
	{
		cerr << "Cannot get trival for fp color " << fp.color << endl;
		status = 2;
	}
	else
	{
		vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&c, level, 1);
	}
	return status;
}

int	arutil_draw_overlay(ARFixationPointSpec& fp, PIXEL_LEVEL level, int overlayPage)
{
	int status=0;
	int numOverlayPages = vsgGetSystemAttribute(vsgNUMOVERLAYPAGES);
	if (overlayPage>=0 && overlayPage < numOverlayPages)
	{
		vsgSetDrawPage(vsgOVERLAYPAGE, overlayPage, vsgNOCLEAR);
		vsgSetPen1(level);
		vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
		vsgDrawOval(fp.x, -fp.y, fp.d, fp.d);
	}
	else status=-1;

	return status;
}

int arutil_draw_grating_fullscreen(ARGratingSpec& gr, int videoPage)
{
	int status=0;
	VSGTRIVAL from, to;
	int numVideoPages = vsgGetSystemAttribute(vsgNUMVIDEOPAGES);

	if (videoPage>=0 && videoPage < numVideoPages)
	{
		vsgSetDrawPage(vsgVIDEOPAGE, videoPage, vsgNOCLEAR);
		gr.select();

		// We assume that the handle is created and selected. In order to make this grating appear, you still must
		// assign pixel levels (vsgObjSetPixels). Note also that the contrast is initially set to 100% by the call to 
		// vsgObjSetDefaults().

		vsgObjSetDefaults();
		vsgObjSetPixelLevels(gr.getFirstLevel(), gr.getNumLevels());

		// Set spatial waveform
		if (gr.pattern == sinewave)
		{
			vsgObjTableSinWave(vsgSWTABLE);
		}
		else
		{	
			// Set up standard 50:50 square wave
			vsgObjTableSquareWave(vsgSWTABLE, vsgObjGetTableSize(vsgSWTABLE)*0.25, vsgObjGetTableSize(vsgSWTABLE)*0.75);
		}

		// set temporal freq
		vsgObjSetDriftVelocity(gr.tf);

		// set color vector
		if (get_colorvector(gr.cv, from, to))
		{
			cerr << "Cannot get color vector for type " << gr.cv << endl;
		}
		vsgObjSetColourVector(&from, &to, vsgBIPOLAR);

		double dWidth = vsgGetScreenWidthPixels();
		double dHeight = vsgGetScreenHeightPixels();
		vsgUnit2Unit(vsgPIXELUNIT, dWidth, vsgDEGREEUNIT, &dWidth);
		vsgUnit2Unit(vsgPIXELUNIT, dHeight, vsgDEGREEUNIT, &dHeight);

		// Now draw
		vsgSetDrawMode(vsgCENTREXY);
		vsgDrawGrating(0, 0, dWidth, dHeight, gr.orientation, gr.sf);
	}
	else status = -1;
	return status;
}

int arutil_draw_aperture(ARGratingSpec& gr, int overlayPage)
{
	int status=0;
	int numOverlayPages = vsgGetSystemAttribute(vsgNUMOVERLAYPAGES);
	if (overlayPage>=0 && overlayPage < numOverlayPages)
	{
		vsgSetDrawPage(vsgOVERLAYPAGE, overlayPage, vsgNOCLEAR);
		vsgSetPen1(0);
		vsgSetDrawMode(vsgCENTREXY);
		if (gr.aperture == ellipse)
		{
			vsgDrawOval(gr.x, -gr.y, gr.w, gr.h);
		}
		else
		{
			vsgDrawRect(gr.x, -gr.y, gr.w, gr.h);
		}
	}
	else status = -1;
	return 0;
}
