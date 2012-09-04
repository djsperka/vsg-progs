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



StimSetFGGXCore::StimSetFGGXCore(shared_ptr<SSInfo> pssinfo, double xOffset, double yOffset) : StimSetFGGX(pssinfo, xOffset, yOffset)
{
 	double x, y;
	double diam;
	ARGratingSpec g0 = m_pssinfo->getCoreGrating();
	ARGratingSpec g1 = m_pssinfo->getCoreGrating();
	/*
	ARGratingSpec g2 = m_pssinfo->getCoreGrating();
	ARGratingSpec g3 = m_pssinfo->getCoreGrating();
	m_pssinfo->getCoreXY(0, x, y);
	g0.x = x;
	g0.y = y;
	m_pssinfo->getCoreXY(1, x, y);
	g1.x = x;
	g1.y = y;
	m_pssinfo->getCoreXY(0, x, y);
	g2.x = x;
	g2.y = y;
	m_pssinfo->getCoreXY(1, x, y);
	g3.x = x;
	g3.y = y;
	set_grating(g2, xOffset, yOffset);
	set_grating(g3, xOffset, yOffset);
	*/
	set_grating(g0, xOffset, yOffset);
	set_grating(g1, xOffset, yOffset);

	m_pssinfo->getCoreXY(0, x, y);
	m_grid0.x = x;
	m_grid0.y = y;
	diam = getGridDiam();
	m_grid0.w = diam;
	m_grid0.h = diam;
	m_grid0.nc = m_grid0.nr = 8;

	m_pssinfo->getCoreXY(1, x, y);
	m_grid1.x = x;
	m_grid1.y = y;
	m_grid1.w = diam;
	m_grid1.h = diam;
	m_grid1.nc = m_grid1.nr = 8;

	// create event object
	//m_event = CreateEvent(	NULL, 
	//						FALSE,	// auto-reset; resets to nonsignaled after waiting thread is released
	//						FALSE,	// non-signaled initial state
	//						NULL);

	// Start thread. The thread will block on the event above - which indicates "start dot updates". 
	// A bool var will tell the thread to stop and resume waiting on the event.
	//m_thread = (HANDLE)_beginthread(StimSetFGGXCore::threadfunc, 0, this);

}


void StimSetFGGXCore::per_trial_predraw_updates()
{
	double ori;
	m_pssinfo->getOri(m_itrial, ori);
	grating(0).orientation = ori;
	grating(1).orientation = ori;
}


int StimSetFGGXCore::init(ARvsg& vsg)
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
	grating(1).init(vsg, f_nlevels);

#if USE_GRIDS
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




// bDrawAllPages==true means all pages drawn; for updates set to false (then only pages that change are drawn)
void StimSetFGGXCore::draw_pages(bool bDrawAllPages)
{
	int savepage;
	double x, y;
	int lr;
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
	grating(0).contrast = 0;
	grating(1).contrast = 0;
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
		//vsgPresent();
		w.split("1,2 done");

		// page 2-n, copy scratch page
		for (int i=2; i<m_firstgridpage + m_ngridpages; i++)
			copy_scratch_page(i);
		//vsgPresent();
		w.split("copied sp 2-8");

		// gratings
		vsgSetDrawPage(vsgVIDEOPAGE, 3, vsgNOCLEAR);
		m_pssinfo->getCoreXY(0, x, y);
		grating(0).x = x;
		grating(0).y = y;
		grating(0).draw();
		m_pssinfo->getCoreXY(1, x, y);
		grating(0).x = x;
		grating(0).y = y;
		grating(0).draw();
		//vsgPresent();

		w.split("page 3 gratings done.");
		vsgSetDrawPage(vsgVIDEOPAGE, 4, vsgNOCLEAR);

		// Need to know which grating will have the contrast change. That will be grating(1).
		m_pssinfo->getLR(m_itrial, lr);
		if (lr == 0)
		{
			// left grating will have contrast change. 
			m_pssinfo->getCoreXY(0, x, y);
			grating(1).x = x;
			grating(1).y = y;
			grating(1).draw();

			m_pssinfo->getCoreXY(1, x, y);
			grating(0).x = x;
			grating(0).y = y;
			grating(0).draw();
		}
		else
		{
			m_pssinfo->getCoreXY(0, x, y);
			grating(0).x = x;
			grating(0).y = y;
			grating(0).draw();

			// right grating will have contrast change. 
			m_pssinfo->getCoreXY(1, x, y);
			grating(1).x = x;
			grating(1).y = y;
			grating(1).draw();
		}
		//vsgPresent();
		w.split("page 4 gratings done.");

		// grids
#if	USE_GRIDS
		for (int i=0; i<m_ngridpages; i++)
		{
			vsgSetDrawPage(vsgVIDEOPAGE, m_firstgridpage + i, vsgNOCLEAR);
			m_grid0.draw();
			m_grid1.draw();
			//vsgPresent();
			w.split("grid page done.");
		}
#endif
	}
	else
	{

		copy_scratch_page(3);
		m_pssinfo->getCoreXY(0, x, y);
		grating(0).x = x;
		grating(0).y = y;
		grating(0).draw();
		m_pssinfo->getCoreXY(1, x, y);
		grating(0).x = x;
		grating(0).y = y;
		grating(0).draw();
		w.split("page 3 gratings done.");

		copy_scratch_page(4);

		// Need to know which grating will have the contrast change. That will be grating(1).
		m_pssinfo->getLR(m_itrial, lr);
		if (lr == 0)
		{
			// left grating will have contrast change. 
			m_pssinfo->getCoreXY(0, x, y);
			grating(1).x = x;
			grating(1).y = y;
			grating(1).draw();

			m_pssinfo->getCoreXY(1, x, y);
			grating(0).x = x;
			grating(0).y = y;
			grating(0).draw();
		}
		else
		{
			m_pssinfo->getCoreXY(0, x, y);
			grating(0).x = x;
			grating(0).y = y;
			grating(0).draw();

			// right grating will have contrast change. 
			m_pssinfo->getCoreXY(1, x, y);
			grating(1).x = x;
			grating(1).y = y;
			grating(1).draw();
		}
		//vsgPresent();
		w.split("page 4 gratings done.");
	}

	vsgSetDrawPage(vsgVIDEOPAGE, savepage, vsgNOCLEAR);
	vsgPresent();
	w.split("end");

	return;
}




int StimSetFGGXCore::handle_trigger(std::string& s)
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
		int lr, cup, cbase;
		m_pssinfo->getLR(m_itrial, lr);
		m_pssinfo->getCUp(m_itrial, cup);
		cbase = m_pssinfo->getCBase();

		if (has_fixpt()) 
		{
			fixpt().setContrast(100);
		}
		if (has_xhair()) 
		{
			xhair().setContrast(100);
		}
#if USE_GRIDS
		m_grid0.setContrast(100);
		m_grid1.setContrast(100);
#endif

		grating(0).select();
		vsgObjResetDriftPhase();
		grating(0).setContrast(cbase);

		grating(1).select();
		vsgObjResetDriftPhase();
		grating(1).setContrast(cup);

		setup_cycling();
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
	else if (s == "C")
	{
		int ipage;
		string stmp;
		cout << "Enter page: ";
		cin >> ipage;
		if (cin && ipage >= 0)
		{
			xhair().setContrast(100);
			fixpt().setContrast(100);

			grating(0).select();
			vsgObjResetDriftPhase();
			grating(0).setContrast(100);

			grating(1).select();
			vsgObjResetDriftPhase();
			grating(1).setContrast(100);

#if USE_GRIDS
			m_grid0.setContrast(100);
			m_grid1.setContrast(100);
#endif

			vsgSetDrawPage(vsgVIDEOPAGE, ipage, vsgNOCLEAR);
			status = 1;
		}
		else
		{
			cout << "Enter int >=0" << endl;
			status = 0;
		}
	}
	else if (s == "D")
	{
		int icon;
		string stmp;
		cout << "Enter contrast: ";
		cin >> icon;
		if (cin && icon >= 0 && icon <= 100)
		{
			fixpt().setContrast(icon);
			xhair().setContrast(icon);
			status = 1;
		}
	}
	else if (s == "E")
	{
		cout << "grating(0): " << (long)grating(0).handle() << ":" << grating(0) << endl;
		cout << "grating(1): " << (long)grating(1).handle() << ":" << grating(1) << endl;
		status = 0;
	}
	return status;
}



#if 0
void StimSetFGGXCore::threadfunc(void *obj)
{
	StimSetFGGXCore* pss = (StimSetFGGXCore *)obj;
	while (true)
		pss->do_updates();
	return;
}
void StimSetFGGXCore::do_updates()
{
	long p;

	// before we go anywhere we have to wait for the event signal
	WaitForSingleObject(m_event, INFINITE);

	// Now we're supposed to start the dots
	cout << "Got event, starting update loop" << endl;

	p = vsgGetSystemAttribute(vsgPAGECYCLINGSTATE);
	while (p > -1 && p < 3) 
	{
		cout << "P=" << p << endl;
		p = vsgGetSystemAttribute(vsgPAGECYCLINGSTATE);
	}
	cout << "Exited p=" << p << endl;
	return;
}

void StimSetFGGXDonut::threadfunc(void *obj)
{
	return;
}
#endif



