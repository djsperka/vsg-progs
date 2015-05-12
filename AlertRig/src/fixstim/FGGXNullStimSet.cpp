#include "FGGXStimSet.h"
#include <iostream>
#include <cstdlib>
#include "AlertUtil.h"
//#include "stopwatch.h"
using namespace std;

static const int f_nlevels = 20;

#undef USE_GRIDS

std::string FGGXNullStimSet::toString() const
{
	std::ostringstream oss;
	oss << "FGGXNullStimSet NI.";
	return oss.str();
}


int FGGXNullStimSet::init(ARvsg& vsg, std::vector<int> pages)
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

		// only allow answer points when you have fixpt
		if (m_pssinfo->getUseAnswerPoints())
		{
			m_ap0.color = m_ap1.color = fixpt().color;
			m_ap0.d = m_ap1.d = fixpt().d;
			m_ap0.init(vsg, 2);
			m_ap1.init(vsg, 2);
		}
	}

#if USE_GRIDS
	m_grid0.init(vsg, 3);
	m_grid1.init(vsg, 3);
#else
	m_cb0.init(vsg, 2);
	m_cb1.init(vsg, 2);
#endif

	draw_scratch_pages();	// xhair and fixpt on scratch page 0'
	draw_pages(true);

	return status;
}




// bDrawAllPages==true means all pages drawn; for updates set to false (then only pages that change are drawn)
void FGGXNullStimSet::draw_pages(bool bDrawAllPages)
{
	int savepage;
	double x, y;
	int lr;

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
	// page 5: this is m_firstGridPage
	// page 5 - m_firstGridPage+m_nGridPages-1 - plaids
	// page m_firstGridPage+m_nGridPages - blank or answer points

	// Set all contrasts to 0 while we draw to prevent flashing them 
	if (has_xhair())
		xhair().setContrast(0);
	if (has_fixpt())
		fixpt().setContrast(0);

#if USE_GRIDS
	m_grid0.setContrast(0);
	m_grid1.setContrast(0);
#else
	m_cb0.setContrast(0);
	m_cb1.setContrast(0);
#endif

	if (bDrawAllPages)
	{
		// Page 0 and 1 - clear to bkgd, draw xhair on 1
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);
		if (has_xhair())
		{
			xhair().draw();
		}

		// page 2-n, copy scratch page
		for (int i=2; i<(m_firstgridpage + m_ngridpages); i++)
			copy_scratch_page(i);

		vsgSetDrawPage(vsgVIDEOPAGE, 3, vsgNOCLEAR);
		vsgSetDrawPage(vsgVIDEOPAGE, 4, vsgNOCLEAR);

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
		vsgSetDrawPage(vsgVIDEOPAGE, m_firstgridpage+m_ngridpages, vsgBACKGROUND);
#else
		vsgSetDrawPage(vsgVIDEOPAGE, 5, vsgNOCLEAR);
		m_cb0.draw();
		m_cb1.draw();
		vsgSetDrawPage(vsgVIDEOPAGE, 6, vsgBACKGROUND);
#endif

		if (has_fixpt() && m_pssinfo->getUseAnswerPoints())
		{
			// Need to know which grating will have the contrast change. That will be grating(1).
			m_pssinfo->getLR(m_itrial, lr);
			if (lr == 0)
			{
				// left grating will have contrast change. 
				m_pssinfo->getCoreXY(0, x, y);
				m_ap1.x = x;
				m_ap1.y = y;
				m_ap1.draw();

				m_pssinfo->getCoreXY(1, x, y);
				m_ap0.x = x;
				m_ap0.y = y;
				m_ap0.draw();
			}
			else
			{
				m_pssinfo->getCoreXY(0, x, y);
				m_ap0.x = x;
				m_ap0.y = y;
				m_ap0.draw();

				// right grating will have contrast change. 
				m_pssinfo->getCoreXY(1, x, y);
				m_ap1.x = x;
				m_ap1.y = y;
				m_ap1.draw();
			}
		}
	}
	else
	{

		copy_scratch_page(3);
		copy_scratch_page(4);

#if USE_GRIDS
		vsgSetDrawPage(vsgVIDEOPAGE, m_firstgridpage+m_ngridpages, vsgBACKGROUND);
#else
		vsgSetDrawPage(vsgVIDEOPAGE, 6, vsgBACKGROUND);
#endif

		if (has_fixpt() && m_pssinfo->getUseAnswerPoints())
		{
			// Need to know which grating will have the contrast change. That will be grating(1).
			m_pssinfo->getLR(m_itrial, lr);
			if (lr == 0)
			{
				// left grating will have contrast change. 
				m_pssinfo->getCoreXY(0, x, y);
				m_ap1.x = x;
				m_ap1.y = y;
				m_ap1.draw();

				m_pssinfo->getCoreXY(1, x, y);
				m_ap0.x = x;
				m_ap0.y = y;
				m_ap0.draw();
			}
			else
			{
				m_pssinfo->getCoreXY(0, x, y);
				m_ap0.x = x;
				m_ap0.y = y;
				m_ap0.draw();

				// right grating will have contrast change. 
				m_pssinfo->getCoreXY(1, x, y);
				m_ap1.x = x;
				m_ap1.y = y;
				m_ap1.draw();
			}
		}
	}

	vsgSetDrawPage(vsgVIDEOPAGE, savepage, vsgNOCLEAR);
	vsgPresent();

	return;
}




int FGGXNullStimSet::handle_trigger(std::string& s)
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
#else
		m_cb0.setContrast(100);
		m_cb1.setContrast(100);
#endif

		if (has_fixpt() && m_pssinfo->getUseAnswerPoints())
		{
			int cSame, cUp;
			double tIgnore;

			// Need to know which grating will have the contrast change. That will be grating(1).
			m_pssinfo->getLR(m_itrial, lr);
			m_pssinfo->getAnswerPointParameters(cUp, cSame, tIgnore);
			m_ap0.setContrast(cSame);
			m_ap1.setContrast(cUp);
			cout << "Set answer point contrast " << cSame << ", " << cUp << endl;
		}

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

#if USE_GRIDS
			m_grid0.setContrast(100);
			m_grid1.setContrast(100);
#else
			m_cb0.setContrast(100);
			m_cb1.setContrast(100);
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
	return status;
}



