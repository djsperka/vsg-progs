#include "ARPlaidSpec.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
using namespace std;
namespace alert
{
	//int ARPlaidSpec::draw()
	//{
	//	return drawDriftingPlaid(0, 0);
	//}

	int ARPlaidSpec::draw()
	{
		int status = 0;
		int W, H;
		double ppd;
		double ll[2], ur[2];
		double xOrgVSG, yOrgVSG;
		double xWinVSG, yWinVSG;	// upper left corner of initial window, vsg coords
		double bbTopLeftXVSG;
		bool bOverlay = false;

		vsgUnit2Unit(vsgDEGREEUNIT, 1.0, vsgPIXELUNIT, &ppd);
		W = vsgGetScreenWidthPixels();
		H = vsgGetScreenHeightPixels();

		// select the object - must have called init()!
		select();

		// color vector is b-w
		VSGTRIVAL from, to;
		from.a = from.b = from.c = 0;
		to.a = to.b = to.c = 1.0;
		vsgObjSetColourVector(&from, &to, vsgBIPOLAR);

		// OVERLAY PAGE?
		if (m_w > 1 && m_h > 1)
		{
//			vsgSetCommand(vsgOVERLAYMASKMODE + vsgOVERLAYDRIFT + vsgVIDEODRIFT);
			bOverlay = true;
		}
		else
		{
//			vsgSetCommand(vsgVIDEODRIFT);
		}


		// set contrast for plaid
		// drift velocity is implicit in the definition of the plaid pattern (in the drift, that is)
		// do not set it at the object level (set it to zero just to make sure)
		vsgObjSetContrast(m_contrast);
		vsgObjSetTemporalFrequency(0);
		vsgObjTableRampWave(vsgSWTABLE);
		vsgObjSetSpatialPhase(180);
		//vsgObjTableSinWave(vsgSWTABLE);

		// what is current draw page? will need it for cycling struct.
		VSGPAGEDESCRIPTOR descr;
		descr._StructSize = sizeof(VSGPAGEDESCRIPTOR);
		vsgGetCurrentDrawPage(&descr);

		if (m_tf == 0)
		{
			cerr << "tf=0, no drift" << endl;
			getBB(ll, ur, W, H, 0, 0);
			cerr << "Got BB " << ll[0] << ", " << ll[1] << "  " << ur[0] << ", " << ur[1] << endl;
			bbTopLeftXVSG = (vsgGetSystemAttribute(vsgPAGEWIDTH) - (ur[0] - ll[0])) / 2;	// top left Y is zero, this is VSG coords
			xOrgVSG = bbTopLeftXVSG + W / 2;
			yOrgVSG = H / 2;
			xWinVSG = bbTopLeftXVSG;
			yWinVSG = 0;
			cerr << "origin, in VSG coord " << xOrgVSG << ", " << yOrgVSG << endl;
			cerr << "iniwin, in VSG coord " << xWinVSG << ", " << yWinVSG << endl;

			drawPlaidArea(ll, ur, ppd, xOrgVSG, yOrgVSG, g1(), g2());

			// if an aperture is needed, prepare overlay page.
			if (bOverlay) drawOverlayPage();
			VSGCYCLEPAGEENTRY cycle[1];	// warning! No check on usage. You have been warned. 
			cycle[0].Page = descr.Page;
			cycle[0].Xpos = xWinVSG;
			cycle[0].Ypos = yWinVSG;
			if (bOverlay)
			{
				cycle[0].Page += vsgDUALPAGE;
				cycle[0].ovPage = 1;
				cycle[0].ovXpos = 0;
			}
			else
			{
				cycle[0].ovPage = 0;
				cycle[0].ovXpos = cycle[0].ovYpos = 0;
			}
			cycle[0].Frames = 1;
			cycle[0].Stop = 0;

			vsgPageCyclingSetup(1, &cycle[0]);

		}
		else
		{
			unsigned int frameUS;	// microseconds per frame
			unsigned int nFramesPerCycle; // use 1 LESS than this items in cycling array
			frameUS = vsgGetSystemAttribute(vsgFRAMETIME);
			nFramesPerCycle = 1 / tf() * 1 / frameUS * 1.0e6;
			cerr << "tf " << tf() << " nfpc " << nFramesPerCycle << " max " << vsgGetSystemAttribute(vsgPAGECYCLEARRAYSIZE) << endl;
			// check that frames per cycle is do-able
			if ((nFramesPerCycle - 1) < vsgGetSystemAttribute(vsgPAGECYCLEARRAYSIZE))
			{
				double xpos0, ypos0;
				double xpos1, ypos1;
				double xdtot, ydtot;	// total drift vector - CAUTION this is in plaid coords (Y coord opposite vsg)
				//cerr << "ppd " << ppd << endl;
				//cerr << "D " << vsgGetViewDistMM() << endl;
				getDriftPos(0, ppd, this->tf(), this->g1(), this->g2(), xpos0, ypos0);
				//cerr << "init drift position: " << xpos0 << ", " << ypos0 << endl;
				getDriftPos((nFramesPerCycle - 1) * frameUS / 1.0e6, ppd, this->tf(), this->g1(), this->g2(), xpos1, ypos1);
				//cerr << "last drift position: " << xpos1 << ", " << ypos1 << endl;
				xdtot = xpos0 - xpos1;
				ydtot = ypos0 - ypos1;
				//cerr << "tot drift (plaid coords) " << xdtot << ", " << ydtot << endl;

				// determine bbox. 
				// bbox determined with plaid coords (y up).
				getBB(ll, ur, W, H, xpos0 - xpos1, ypos0 - ypos1);
				bbTopLeftXVSG = (vsgGetSystemAttribute(vsgPAGEWIDTH) - (ur[0] - ll[0])) / 2;	// top left Y is zero, this is VSG coords
				cerr << "Got BB " << ll[0] << ", " << ll[1] << "  " << ur[0] << ", " << ur[1] << endl;

				// where should base offset be? In other words, in the VSG coords (0,0, top left of video mem, y pos down)
				// what is the coord of origin of plaid image. 
				// This initial image should be such that this is the origin used for position of plaid (if using aperture). 
				// If no aperture, center of screen is used as origin.
				double ydtotVSG = -ydtot;
				if (ydtotVSG > 0)
				{
					if (xdtot > 0)
					{
						xOrgVSG = bbTopLeftXVSG + W / 2;
						yOrgVSG = H / 2;
						xWinVSG = bbTopLeftXVSG;
						yWinVSG = 0;
						// will move down, to right. Init screen is upper left of bb. 
					}
					else
					{
						xOrgVSG = bbTopLeftXVSG + (ur[0] - ll[0]) - W / 2;
						yOrgVSG = H / 2;
						xWinVSG = bbTopLeftXVSG + (ur[0] - ll[0]) - W;
						yWinVSG = 0;
						// down, to left. Init screen upper right of bb.
					}
				}
				else
				{
					if (xdtot > 0)
					{
						xOrgVSG = bbTopLeftXVSG + W / 2;
						yOrgVSG = (ur[1] - ll[1]) - H / 2;
						xWinVSG = bbTopLeftXVSG;
						yWinVSG = (ur[1] - ll[1]) - H;
						// will move up, to right. Init screen is lower left of bb. 
					}
					else
					{
						xOrgVSG = bbTopLeftXVSG + (ur[0] - ll[0]) - W / 2;
						yOrgVSG = (ur[1] - ll[1]) - H / 2;
						xWinVSG = bbTopLeftXVSG + (ur[0] - ll[0]) - W;
						yWinVSG = (ur[1] - ll[1]) - H;
						// up, to left. Init screen lower right of bb.
					}
				}

				cerr << "origin, in VSG coord " << xOrgVSG << ", " << yOrgVSG << endl;
				cerr << "iniwin, in VSG coord " << xWinVSG << ", " << yWinVSG << endl;

				// Now begin drawing the bbox area into video memory. 
				// The bbox coords are in plaid coords. 
				// To convert (x,y) in plaid coords to VSG coords (needed for setting up 
				// cycling and for drawing the plaid), 
				// xVSG = xPLAID + xorgVSG;
				// yVSG = -yPLAID + yorgVSG;
				drawPlaidArea(ll, ur, ppd, xOrgVSG, yOrgVSG, g1(), g2());

			}
			else
			{
				cerr << "not enough pages in cycling array" << endl;
				status = 1;
			}


			// if an aperture is needed, prepare overlay page.
			if (bOverlay) drawOverlayPage();


			VSGCYCLEPAGEENTRY cycle[32768];	// warning! No check on usage. You have been warned. 
			for (int i = 0; i < (nFramesPerCycle - 1); i++)
			{
				double xpos, ypos;
				getDriftPos(i * frameUS / 1.0e6, ppd, this->tf(), this->g1(), this->g2(), xpos, ypos);
				cycle[i].Page = descr.Page;
				cycle[i].Xpos = xWinVSG - xpos;
				cycle[i].Ypos = yWinVSG + ypos;
				if (bOverlay)
				{
					cycle[i].Page += vsgDUALPAGE;
					cycle[i].ovPage = 1;
					cycle[i].ovXpos = cycle[i].ovYpos = 0;
				}
				else
				{
					cycle[i].ovPage = 0;
					cycle[i].ovXpos = cycle[i].ovYpos = 0;
				}
				cycle[i].Frames = 1;
				cycle[i].Stop = 0;
			}

			cerr << "First frame Xpos, Ypos " << cycle[0].Xpos << ", " << cycle[0].Ypos << endl;
			vsgPageCyclingSetup(nFramesPerCycle-1, &cycle[0]);

		}
		return status;
	}

	void ARPlaidSpec::drawOverlayPage()
	{
		// assume vsg.init_overlay was called. 
		vsgSetDrawPage(vsgOVERLAYPAGE, 0, 0);	
		vsgSetDrawPage(vsgOVERLAYPAGE, 1, 1);	// level 1 should be background on overlay pages. 

		// Note - the center pos of the plaid is in eye-coords. Must convert to pixels and change coords before drawing.
		double xpix, ypix, wpix, hpix;
		vsgUnit2Unit(vsgDEGREEUNIT, m_x, vsgPIXELUNIT, &xpix);
		vsgUnit2Unit(vsgDEGREEUNIT, m_y, vsgPIXELUNIT, &ypix);
		vsgUnit2Unit(vsgDEGREEUNIT, m_w, vsgPIXELUNIT, &wpix);
		vsgUnit2Unit(vsgDEGREEUNIT, m_h, vsgPIXELUNIT, &hpix);
		xpix = xpix;
		ypix = -ypix;
		vsgSetDrawMode(vsgCENTREXY);
		//int units = vsgGetSystemAttribute(vsgSPATIALUNITS);
		//vsgSetSpatialUnits(vsgDEGREEUNIT);
		vsgSetPen1(0);		// shouild be clear on overlay page
		vsgDrawOval(xpix + vsgGetScreenWidthPixels()/2, ypix+vsgGetScreenHeightPixels()/2, wpix, hpix);
		//vsgDrawOval(m_x, -m_y, m_w, m_h);
		//vsgSetSpatialUnits(units);
		cerr << "draw overlay oval at " << xpix << ", " << ypix << " size " << wpix << "x" << hpix << endl;
		//cerr << "draw overlay (deg)at " << m_x << ", " << m_y << " size " << m_w << "x" << m_h << endl;
	}

	void ARPlaidSpec::drawPlaidArea(double *ll, double *ur, double ppd, double xOrgVSG, double yOrgVSG, const alert::ARPlaidSubGr& gr1, const alert::ARPlaidSubGr& gr2)
	{
		unsigned int bbWidth = ur[0] - ll[0];
		unsigned int bbHeight = ur[1] - ll[1];
		int xMin = (int)ll[0];
		int yMin = (int)ll[1];
		double C1 = g1().contrast() / 100.0;
		double C2 = g2().contrast() / 100.0;
		unsigned char *buffer = new unsigned char[bbWidth];
		vsgSetDrawOrigin(0, 0);
		for (int uy = 0; uy < bbHeight; uy++)
		{
			for (int ux = 0; ux < bbWidth; ux++)
			{
				double d = getFirstLevel() + (getNumLevels() * (0.5 + 0.25 *
					(C1 * cos(getGrPhase(xMin + ux, yMin + uy, ppd, g1())) + C2 * cos(getGrPhase(xMin + ux, yMin + uy, ppd, g2())))));
				buffer[ux] = (unsigned char)d;
			}
			vsgDrawPixelLineFast(xOrgVSG + xMin, yOrgVSG - (yMin + uy), buffer, bbWidth);
		}

		delete buffer;
	}

	void ARPlaidSpec::getBB(double *ll, double *ur, int W, int H, double xdr, double ydr)
	{
		ll[0] = -W / 2;
		ll[1] = -H / 2;
		ur[0] = W / 2;
		ur[1] = H / 2;	
		compareBBPoint(ll, ur, -W / 2 + xdr, -H / 2 + ydr);
		compareBBPoint(ll, ur, -W / 2 + xdr, H / 2 + ydr);
		compareBBPoint(ll, ur, W / 2 + xdr, H / 2 + ydr);
		compareBBPoint(ll, ur, W / 2 + xdr, -H / 2 + ydr);
		return;
	}

	void ARPlaidSpec::compareBBPoint(double *ll, double *ur, double x, double y)
	{
		if (x < ll[0]) ll[0] = x;
		if (y < ll[1]) ll[1] = y;
		if (x > ur[0]) ur[0] = x;
		if (y > ur[1]) ur[1] = y;
	}

	void ARPlaidSpec::getDriftPos(double t, double ppd, double tf, const alert::ARPlaidSubGr& gr1, const alert::ARPlaidSubGr& gr2, double& xpos, double& ypos)
	{
		// The values returned assume a coord system where x, y are pos right, up. 
		// When using values from this method to use in vsg cycling setup, remember that the coord system 
		// for that call is y pos down! The args for Xpos, Ypos specify the movement of the viewing window relative
		// to the unmoved case. A value of Xpos > 0 moves viewing window to the right, and it appears that the stim moves left. 
		// A value of Ypos > 0 moves viewing window down, and the stim appears to move UP. 
		xpos = t * tf * ppd / (gr1.sf() * gr2.sf()) * (gr1.sf() * gr1.salpha() - gr2.sf() * gr2.salpha()) / (gr1.salpha() * gr2.calpha() - gr1.calpha() * gr2.salpha());
		ypos = t * tf * ppd / (gr1.sf() * gr2.sf()) * (gr2.sf() * gr2.calpha() - gr1.sf() * gr1.calpha()) / (gr1.salpha() * gr2.calpha() - gr1.calpha() * gr2.salpha());
	}

#if 0
	int ARPlaidSpec::drawDriftingPlaid(double xpos, double ypos)
	{
		double pixelsPerDegree;
		int W, H;
		unsigned char *buffer;
		long units;
		int xmin, xmax, ymin, ymax;

		// select the object - must have called init()!
		select();

		// test
		drawAlt();

		// color vector is b-w
		VSGTRIVAL from, to;
		from.a = from.b = from.c = 0;
		to.a = to.b = to.c = 1.0;
		vsgObjSetColourVector(&from, &to, vsgBIPOLAR);

		// set contrast for plaid
		// drift velocity is implicit in the definition of the plaid pattern (in the drift, that is)
		// do not set it at the object level (set it to zero just to make sure)
		vsgObjSetContrast(m_contrast);
		vsgObjSetTemporalFrequency(0);

		// Now generate the plaid pattern. 
		// TODO It will have to be larger than the screen size due to drifting, though we can simulate the drift
		vsgUnit2Unit(vsgDEGREEUNIT, 1.0, vsgPIXELUNIT, &pixelsPerDegree);
		W = vsgGetScreenWidthPixels();
		H = vsgGetScreenHeightPixels();
		if (xpos > 0)
		{
			xmin = 0; xmax = W + xpos;
		}
		else
		{
			xmin = xpos; xmax = W;
		}
		if (ypos > 0)
		{
			ymin = 0; ymax = H + ypos;
		}
		else
		{
			ymin = ypos; ymax = H;
		}

		buffer = new unsigned char[xmax - xmin];


		units = vsgGetSystemAttribute(vsgSPATIALUNITS);
		vsgSetSpatialUnits(vsgPIXELUNIT);

		unsigned int pmin = 9999;
		unsigned int pmax = 0;
		double dmin = 9999999;
		double dmax = -999999;
		double qmin = 9999999;
		double qmax = -999999;
		double C1 = (double)g1().contrast / 100.0;
		double C2 = (double)g2().contrast / 100.0;

		vsgSetDrawOrigin(0, 0);
		cerr << "draw xposEnd, yposEnd " << xpos << " " << ypos << endl;
		cerr << "draw xmin, xmax " << xmin << ", " << xmax << " ymin, ymax " << ymin << ", " << ymax << endl;
		cerr << "drawPixelLine Y range: " << ymax << " " << ymax - ymin << endl;

		for (int jj = ymin; jj < ymax; jj++)
		{
			int j = jj - ymin;
			for (int ii = xmin; ii < xmax; ii++)
			{
				int i = ii - xmin;
				double d = getFirstLevel() + (getNumLevels() * (0.5 + 0.25 *
					(C1 * cos(getGrPhase(i, j, pixelsPerDegree, g1())) + C2 * cos(getGrPhase(i, j, pixelsPerDegree, g2())))));
				buffer[i] = (unsigned char)d;
			}
			vsgDrawPixelLineFast(0, ymax - jj, buffer, xmax - xmin);
		}
		vsgSetSpatialUnits(units);
		delete buffer;

		return 0;
	}
#endif

	int ARPlaidSpec::drawOverlay()
	{
		return 0;
	}

	double ARPlaidSpec::getGrPhase(double x, double y, double p, const ARPlaidSubGr& gr)
	{
		double phase = 2 * M_PI * gr.sf() / p * ((x - m_x) * gr.calpha() + (y - m_y) * gr.salpha());
		return phase;
	}

	void ARPlaidSpec::setContrast(double contrast)
	{
		select();
		m_contrast = contrast;
		vsgObjSetContrast(contrast);
	};

	ARPlaidSubGr::ARPlaidSubGr(double X, double Y, double W, double H, double C, double SF, double TF, double ori)
		: m_x(X), m_y(Y), m_w(W), m_h(H)
		, m_contrast(C)
		, m_sf(SF)
		, m_tf(TF)
		, m_oriDeg(ori)
	{
		setRotDeg(0);
	}

	void ARPlaidSubGr::setRotDeg(double rotDeg)
	{
		m_calpha = cos(M_PI * (rotDeg + m_oriDeg) / 180.0);
		m_salpha = sin(M_PI * (rotDeg + m_oriDeg) / 180.0);
	}

	std::ostream& operator<<(std::ostream& out, const ARPlaidSubGr& g)
	{
		out << "C=" << g.contrast() << ", sf=" << g.sf() << ", cos(alpha)=" << g.calpha() << ", sin(alpha)=" << g.salpha() << endl;
		return out;
	}

};