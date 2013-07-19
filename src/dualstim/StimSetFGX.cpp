#include "StimSetFGX.h"
#include <iostream>
using namespace std;

static const int f_nlevels = 100;




int StimSetFGX::init(ARvsg& vsg)
{
	
	int status = 0;

	vsg.select();

	// Set initial params in grating(s)
	set_initial_parameters();

	if (has_xhair())
	{
		xhair().init(vsg, 16);
	}
	if (has_fixpt())
	{
		fixpt().init(vsg, 2);
	}
	if (has_grating())
	{
		// grating contrast is saved. Whenever grating is drawn it must
		// be drawn with contrast 0. That way it isn't seen until later, 
		// when we set the contrast to its intended value. 
		m_contrast = grating().contrast;
		grating().init(vsg, f_nlevels);
		grating().setContrast(0);
	}

	// page 2 - xhair, fixpt and stim. 
	vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgBACKGROUND);
	if (has_xhair())
	{
		xhair().draw();
	}
	if (has_fixpt())
	{
		fixpt().draw();
	}
	if (has_grating())
	{
		grating().draw();
	}
	vsgPresent();

	// page 1 - xhair and fixpt
	vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);
	if (has_xhair())
	{
		xhair().draw();
	}
	if (has_fixpt())
	{
		fixpt().draw();
	}
	vsgPresent();

	// page 0 - xhair only
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	if (has_xhair())
	{
		xhair().draw();
	}
	vsgPresent();


	return status;
}

int StimSetFGX::handle_trigger(std::string& s)
{
	int status = 0;
	if (s == "F")
	{
		// s/b draw page 1, so set contrast and present
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		if (has_fixpt())
		{
			fixpt().setContrast(100);
		}
		status = 1;
	}
	else if (s == "S")
	{
		if (has_grating())
		{
			grating().select();
			vsgObjResetDriftPhase();
			//vsgObjSetSpatialPhase(get_spatial_phase());
			grating().setContrast(m_contrast);
		}
		setup_cycling(1, 2, 1);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "s")
	{
		if (has_grating())
		{
			grating().setContrast(0);
		}
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEDISABLE, 0);
		vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "X")
	{
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		if (has_fixpt())
		{
			fixpt().setContrast(0);
		}
		if (has_grating())
		{
			grating().setContrast(0);
		}
		status = 1;
	}
	else if (s == "a")
	{	
		if (has_grating()) grating().setContrast(m_contrast);
		advance();
		if (has_grating()) 
		{
			m_contrast = grating().contrast;
			grating().setContrast(0);
		}

		// SUBCLASS: redraw page 2 if necessary
		int ipage = vsgGetZoneDisplayPage(vsgVIDEOPAGE);
		vsgSetDrawPage(vsgVIDEOPAGE, 2, vsgBACKGROUND);
		if (has_xhair())
		{
			xhair().draw();
		}
		if (has_fixpt())
		{
			fixpt().draw();
		}
		if (has_grating())
		{
			grating().draw();
		}
		vsgPresent();
		vsgSetDrawPage(vsgVIDEOPAGE, ipage, vsgNOCLEAR);
		status = 1;
	}
	return status;
}



int StimSetFGX::setup_cycling(int firstpage, int secondpage, int lastpage)
{
	VSGCYCLEPAGEENTRY cycle[32768];
	int status = 0;

	memset(cycle, 0, 3*sizeof(VSGCYCLEPAGEENTRY));

	cycle[0].Frames = 1 + get_frames_delay();
	cycle[0].Page = firstpage;
	cycle[0].Stop = 0;

	if (lastpage >= 0)
	{
		cycle[1].Frames = get_frames_on();
		cycle[1].Page = secondpage + vsgTRIGGERPAGE;
		cycle[1].Stop = 0;

		cycle[2].Frames = 1;
		cycle[2].Page = lastpage + vsgTRIGGERPAGE;
		cycle[2].Stop = 1;

		status = vsgPageCyclingSetup(3, &cycle[0]);
		//cout << "StimSetFGX: Page cycling ready status=" << status << "  frames delay/on=" << get_frames_delay() << "/" << get_frames_on() << endl;
	}
	else
	{
		cycle[1].Frames = 1;
		cycle[1].Page = secondpage + vsgTRIGGERPAGE;
		cycle[1].Stop = 1;

		status = vsgPageCyclingSetup(2, &cycle[0]);
		//cout << "StimSetFGX: Page cycling ready status=" << status << "  frames delay=" << get_frames_delay() << endl;	
	}
	status = 0;
	return status;
}

int StimSetFGX::setup_fixpt_cycling(int firstpage, int fixptpage)
{
	VSGCYCLEPAGEENTRY cycle[3];
	int status = 0;

	memset(cycle, 0, 3*sizeof(VSGCYCLEPAGEENTRY));

	cycle[0].Frames = 1 + get_frames_fixpt_delay();
	cycle[0].Page = firstpage;
	cycle[0].Stop = 0;

	cycle[1].Frames = 1;
	cycle[1].Page = fixptpage + vsgTRIGGERPAGE;
	cycle[1].Stop = 1;

	status = vsgPageCyclingSetup(2, &cycle[0]);
	//cout << "StimSetFGX: Page cycling ready for fixpt, status=" << status << "  frames delay=" << get_frames_fixpt_delay() << endl;

	return status;
}

void StimSetFGX::set_initial_parameters()
{
	if (has_grating())
	{
		for (ptr_vector<StimParameterList>::iterator it = begin(); it != end(); it++)
		{
			it->set_current_parameter(grating());
		}
	}
	return;
}

void StimSetFGX::advance()
{
	if (has_grating())
	{
		for (ptr_vector<StimParameterList>::iterator it = begin(); it != end(); it++)
		{
			it->advance(grating());
		}
	}
	return;
}
