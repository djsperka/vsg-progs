#include "StimSetFGGX.h"
#include <iostream>
#include <cstdlib>
//#include <windows.h>
//#include <process.h>
#include "AlertUtil.h"
#include "stopwatch.h"
using namespace std;

static const int f_nlevels = 20;

#define USE_GRIDS 1





StimSetFGGXDonut::StimSetFGGXDonut(shared_ptr<SSInfo> pssinfo, double xOffset, double yOffset) : StimSetFGGX(pssinfo, xOffset, yOffset)
{
 	double x, y;
	ARGratingSpec g0 = m_pssinfo->getDonutGrating();
	m_pssinfo->getDonutXY(0, x, y);
	g0.x = x;
	g0.y = y;
	set_grating(g0, xOffset, yOffset);
	m_pssinfo->getDonutXY(1, x, y);
	g0.x = x;
	g0.y = y;
	set_grating(g0, xOffset, yOffset);
}

void StimSetFGGXDonut::per_trial_predraw_updates()
{
	double diam;
	m_pssinfo->getDiam(m_itrial, diam);
	grating(0).w = diam;
	grating(0).h = diam;
	grating(1).w = diam;
	grating(1).h = diam;
}


int StimSetFGGXDonut::handle_trigger(std::string& s)
{
	int status = 0;
	if (s == "F")
	{
		// set to page 2, return 1 to ask for present()
		if (has_fixpt()) fixpt().setContrast(100);
		if (has_xhair()) xhair().setContrast(100);
		vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "S")
	{
		grating(0).setContrast(contrast(0));
		grating(1).setContrast(contrast(1));
		if (has_fixpt()) fixpt().setContrast(100);
		if (has_xhair()) xhair().setContrast(100);
#if USE_GRIDS
		m_grid0.setContrast(100);
		m_grid1.setContrast(100);
#endif

		setup_cycling();
		//SetEvent(m_event);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "s")
	{
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "X")
	{
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "a")
	{	
		advance();
		draw_pages(false);
		status = 1;
	}
	return status;
}



// bDrawAllPages==true means all pages drawn; for updates set to false (then only pages that change are drawn)
void StimSetFGGXDonut::draw_pages(bool bDrawAllPages)
{
	int savepage;
	Stopwatch w;
	w.split("Start");

	// Before any drawing, save the current display page
	savepage = vsgGetZoneDisplayPage(vsgVIDEOPAGE);

	// per-trial updates
	per_trial_predraw_updates();


	// Now do the drawing. Note the page usage:
	// Page 0: blank (background)
	// Page 1: xhair only (no fixpt)
	// page 2: xhair and fixpt
	// page 3: xhair, fixpt, gratings 0 and 1
	// page 4: xhair, fixpt, gratings 2 and 3
	// page 5: xhair, fixpt, plaid#1
	// page 6: xhair, fixpt, plaid#2
	// page 7: xhair, fixpt, plaid#3

	// Set all contrasts to 0 while we draw to prevent flashing them 
	if (has_xhair())
		xhair().setContrast(0);
	if (has_fixpt())
		fixpt().setContrast(0);
	grating(0).setContrast(0);

#if USE_GRIDS
	m_grid0.setContrast(0);
	m_grid1.setContrast(0);
#endif

	w.split("contrast, pretrial done.");
	if (bDrawAllPages)
	{
		// Page 0 and 1 - clear to bkgd, draw xhair on 1
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);
		if (has_xhair())
		{
			xhair().draw();
		}
		vsgPresent();
		w.split("1,2 done");

		// page 2-n, copy scratch page
		for (int i=2; i<m_firstgridpage + m_ngridpages; i++)
			copy_scratch_page(i);
		vsgPresent();
		w.split("copied sp 2-8");

		// gratings. Note these had their x,y set in constructor. The donut positions do no 
		// change. 
		vsgSetDrawPage(vsgVIDEOPAGE, 3, vsgNOCLEAR);
		grating(0).draw();
		grating(1).draw();
		//vsgPresent();

		w.split("page 3 gratings done.");
		vsgSetDrawPage(vsgVIDEOPAGE, 4, vsgNOCLEAR);
		grating(0).draw();
		grating(1).draw();
		//vsgPresent();
		w.split("page 4 gratings done.");

		// grids
#if	USE_GRIDS
		for (int i=0; i<m_ngridpages; i++)
		{
			vsgSetDrawPage(vsgVIDEOPAGE, m_firstgridpage + i, vsgNOCLEAR);
			m_grid0.draw();
			m_grid1.draw();
			vsgPresent();
			w.split("grid page done.");
		}
#endif
	}
	else
	{
		copy_scratch_page(3);
		grating(0).draw();
		grating(1).draw();
		//vsgPresent();
		w.split("page 3 gratings done.");

		copy_scratch_page(4);
		grating(0).draw();
		grating(1).draw();
		//vsgPresent();
		w.split("page 4 gratings done.");
	}

	vsgSetDrawPage(vsgVIDEOPAGE, savepage, vsgNOCLEAR);
	vsgPresent();
	w.split("end");

	cout << "draw_pages(" << (bDrawAllPages ? "true" : "false") << ")" << endl << w << endl;

	return;
}

int StimSetFGGXDonut::init(ARvsg& vsg)
{	
	int status = 0;
	vsg.select();

	// Init other objects
	if (has_xhair())
	{
		xhair().init(vsg, 16);
	}
	if (has_fixpt())
	{
		fixpt().init(vsg, 2);
	}
	grating(0).init(vsg, f_nlevels);
	grating(0).setContrast(0);
	grating(1).init(vsg, f_nlevels);
	grating(1).setContrast(0);

#if USE_GRIDS
	double gDiam = getGridDiam();
	m_grid0.x = grating(0).x;
	m_grid0.y = grating(0).y;
	m_grid0.w = gDiam;
	m_grid0.h = gDiam;
	m_grid0.nc = m_grid0.nr = 8;
	m_grid1.x = grating(1).x;
	m_grid1.y = grating(1).y;
	m_grid1.w = gDiam;
	m_grid1.h = gDiam;
	m_grid1.nc = m_grid1.nr = 8;

	m_grid0.init(vsg, 3);
	m_grid1.init(vsg, 3);
#endif

	Stopwatch w;

	w.split("start");
	draw_scratch_pages();	// xhair and fixpt on scratch page 0'
	w.split("draw_scratch_pages");
	draw_pages(true);
	w.split("draw_pages");

	cout << "init pages" << endl << w << endl;

	return status;
}