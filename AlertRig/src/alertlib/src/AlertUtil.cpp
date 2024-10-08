#include "AlertUtil.h"
#include <iostream>
#include <cmath>
#include <boost/algorithm/string.hpp> 
using namespace std;
using namespace boost;

int arutil_color_to_overlay_palette(ARFixationPointSpec& fp, PIXEL_LEVEL level)
{
	int status=0;
	return arutil_color_to_overlay_palette(fp.color, level);
}


int arutil_color_to_overlay_palette(COLOR_TYPE ct, PIXEL_LEVEL level)
{
	int status=0;
	VSGTRIVAL c = ct.trival();
	vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&c, level, 1);
	return status;
}

int arutil_color_to_palette(COLOR_TYPE ct, PIXEL_LEVEL level)
{
	int status=0;
	VSGTRIVAL c = ct.trival();
	if (vsgPaletteSet(level, level, &c) < 0)
		cerr << "ERROR vsgPaletteSet" << endl;
	return status;
}


int arutil_ramp_to_palette(COLOR_TYPE from, COLOR_TYPE to, PIXEL_LEVEL from_level, PIXEL_LEVEL to_level)
{
	int status = 0;
	int i;
	VSGTRIVAL f, t, step;
	VSGLUTBUFFER buffer;
	f = from.trival();
	t = to.trival();
	step.a = (t.a - f.a) / (to_level - from_level);
	step.b = (t.b - f.b) / (to_level - from_level);
	step.c = (t.c - f.c) / (to_level - from_level);

	// end of ramp is the "to" color
	buffer[to_level - from_level] = t;
	// now fill in the rest of the ramp
	//cerr << "ramp levels " << from_level << ":" << to_level << endl;
	for (i = 0; i < (to_level - from_level); i++)
	{
		buffer[i].a = f.a + i * step.a;
		buffer[i].b = f.b + i * step.b;
		buffer[i].c = f.c + i * step.c;
		//cerr << i << "\t" << buffer[i].a << "\t" << buffer[i].b << "\t" << buffer[i].c << endl;
	}

	// now set palette
	if (vsgPaletteWrite(&buffer, from_level, to_level-from_level+1) < 0)
	{
		cerr << "ERROR vsgPaletteSet" << endl;
		status = -1;
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

int	arutil_draw_video(ARFixationPointSpec& fp, PIXEL_LEVEL level, int videoPage)
{
	int status=0;
	int numVideoPages = vsgGetSystemAttribute(vsgNUMVIDEOPAGES);
	if (videoPage>=0 && videoPage < numVideoPages)
	{
		vsgSetDrawPage(vsgVIDEOPAGE, videoPage, vsgNOCLEAR);
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

		// In some circumstances this call turns off temporal frequency, and subsequent calls to vsgObjSetTemporal.. 
		// don't change it.We probably don't need this call anyways.....
//		vsgObjSetDefaults();
		vsgObjSetPixelLevels(gr.getFirstLevel(), gr.getNumLevels());

		// Set spatial waveform
		if (gr.swt == sinewave)
		{
			vsgObjTableSinWave(vsgSWTABLE);
		}
		else
		{	
			// Set up standard 50:50 square wave
			vsgObjTableSquareWave(vsgSWTABLE, (DWORD)(vsgObjGetTableSize(vsgSWTABLE)*0.25), (DWORD)(vsgObjGetTableSize(vsgSWTABLE)*0.75));
		}

		// set temporal freq
		vsgObjSetDriftVelocity(gr.tf);

		// Set contrast
		vsgObjSetContrast(gr.contrast);

		// set color vector
		from = gr.cv.from();
		to = gr.cv.to();
		vsgObjSetColourVector(&from, &to, vsgBIPOLAR);

		double dWidth = vsgGetScreenWidthPixels();
		double dHeight = vsgGetScreenHeightPixels();
		vsgUnit2Unit(vsgPIXELUNIT, dWidth, vsgDEGREEUNIT, &dWidth);
		vsgUnit2Unit(vsgPIXELUNIT, dHeight, vsgDEGREEUNIT, &dHeight);

		// Now draw
		vsgSetDrawMode(vsgCENTREXY);
		//vsgDrawGrating(gr.x, -gr.y, 2 * dWidth, 2 * dHeight, gr.orientation, gr.sf);
		vsgDrawGrating(0, 0, dWidth, dHeight, gr.orientation, gr.sf);

		cerr << "arutil_draw_grating_fullscreen: " << gr << endl;
	}
	else status = -1;
	return status;
}


int arutil_draw_grating(ARGratingSpec& gr, int videoPage)
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
		if (gr.swt == sinewave)
		{
			vsgObjTableSinWave(vsgSWTABLE);
		}
		else
		{	
			// Set up standard 50:50 square wave
			vsgObjTableSquareWave(vsgSWTABLE, (DWORD)(vsgObjGetTableSize(vsgSWTABLE)*0.25), (DWORD)(vsgObjGetTableSize(vsgSWTABLE)*0.75));
		}

		// set temporal freq
		vsgObjSetDriftVelocity(gr.tf);

		// Set contrast
		vsgObjSetContrast(gr.contrast);

		// set color vector
		from = gr.cv.from();
		to = gr.cv.to();
		vsgObjSetColourVector(&from, &to, vsgBIPOLAR);

		// Now draw
		if (gr.w > 0 && gr.h > 0)
		{
			vsgDrawGrating(gr.x, -gr.y, gr.w, gr.h, gr.orientation, gr.sf);
		}
		else
		{
			if (vsgGetSystemAttribute(vsgSPATIALUNITS) == vsgPIXELUNIT)
			{
				long lWidth = vsgGetScreenWidthPixels();
				long lHeight = vsgGetScreenHeightPixels();
				vsgDrawGrating(0, 0, lWidth, lHeight, gr.orientation, gr.sf);
			}
			else
			{
				long lWidth = vsgGetScreenWidthPixels();
				long lHeight = vsgGetScreenHeightPixels();
				double degWidth, degHeight;
				vsgUnitToUnit(vsgPIXELUNIT, (double)lWidth, vsgDEGREEUNIT, &degWidth);
				vsgUnitToUnit(vsgPIXELUNIT, (double)lHeight, vsgDEGREEUNIT, &degHeight);
				vsgDrawGrating(0, 0, degWidth, degHeight, gr.orientation, gr.sf);
			}
		}
	}
	else status = -1;
	return status;
}

int arutil_draw_grating_noaperture(ARGratingSpec& gr, int videoPage)
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
		if (gr.swt == sinewave)
		{
			vsgObjTableSinWave(vsgSWTABLE);
		}
		else
		{	
			// Set up standard 50:50 square wave
			vsgObjTableSquareWave(vsgSWTABLE, (DWORD)(vsgObjGetTableSize(vsgSWTABLE)*0.25), (DWORD)(vsgObjGetTableSize(vsgSWTABLE)*0.75));
		}

		// set temporal freq
		vsgObjSetDriftVelocity(gr.tf);

		// Set contrast
		vsgObjSetContrast(gr.contrast);

		// set color vector
		from = gr.cv.from();
		to = gr.cv.to();
		vsgObjSetColourVector(&from, &to, vsgBIPOLAR);

		if (gr.aperture == ellipse)
		{
			vsgSetPen1(250);
			vsgSetPen2(251);
			vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
			vsgDrawOval(gr.x, -1*gr.y, gr.w, gr.h);
			vsgSetDrawMode(vsgCENTREXY + vsgTRANSONHIGHER);
		}

		// Now draw grating
		vsgSetPen1(gr.getFirstLevel());
		vsgSetPen2(gr.getFirstLevel() + gr.getNumLevels());
		vsgDrawGrating(gr.x, -gr.y, gr.w, gr.h, gr.orientation, gr.sf);
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

		if (gr.w<=0 || gr.h <= 0)
		{
			// clear the whole page
			vsgSetDrawPage(vsgOVERLAYPAGE, overlayPage, 0);
		}
		else
		{
			if (gr.aperture == ellipse)
			{
				vsgDrawOval(gr.x, -gr.y, gr.w, gr.h);
			}
			else
			{
				vsgDrawRect(gr.x, -gr.y, gr.w, gr.h);
			}
		}
	}
	else status = -1;
	return status;
}



int arutil_load_mseq(string& seq, string& filename, int iOrder)
{
	int istatus=0;
	int nterms = (int)pow(2.0f, iOrder) -1;

	istatus = arutil_load_sequence(seq, filename);
	if (!istatus)
	{
		if (seq.length() != nterms)
		{
			istatus=3;
			cerr << "Expected " << nterms << " terms in seq. Found " << seq.length() << ". Check mseq file." << endl;
		}
	}
	return istatus;
}



int arutil_load_sequence(string& seq, string& filename)
{
	int istatus=0;
	int nterms = 0;
	string line;
	ifstream seqfile(filename.c_str());
	if (seqfile.is_open())
	{
		getline(seqfile, line);
		seqfile.close();
	}
	else istatus = -1;
	return istatus;
}


int arutil_load_sequences(vector<string>& sequences, string& filename)
{
	int istatus = 0;
	string line;
	ifstream seqfile(filename.c_str());
	if (seqfile.is_open())
	{
		while (seqfile.good())
		{
			getline (seqfile, line);
			trim(line);
			if (!line.empty() || line[0]!='#')
			{
				sequences.push_back(line);
			}
		}
	    seqfile.close();
    }
	else
	{
		istatus = -1;
	}
	return istatus;
}