#include "CycleTestStimSet.h"

int CycleTestStimSet::init(ARvsg& vsg, std::vector<int> pages, int)
{
	VSGCYCLEPAGEENTRY cycle[3];	// warning! No check on usage. You have been warned. 
	int status = 0;
	int count = 0;

	vsgSetCommand(vsgPALETTERAMP);

	// these pages used by animation
	m_page_b = pages[1];
	vsgSetDrawPage(vsgVIDEOPAGE, m_page_b, 84);
	m_page_c = pages[2];
	vsgSetDrawPage(vsgVIDEOPAGE, m_page_c, 168);
	m_page_d = pages[3];
	vsgSetDrawPage(vsgVIDEOPAGE, m_page_d, 250);

	// this page used by F
	m_page_a = pages[0];
	vsgSetDrawPage(vsgVIDEOPAGE, m_page_a, 0);

	memset(cycle, 0, sizeof(cycle));
	cycle[0].Frames = 1;
	cycle[0].Page = m_page_c + vsgTRIGGERPAGE;
	cycle[0].Stop = 0;
	cycle[1].Frames = 1;
	cycle[1].Page = m_page_d + vsgTRIGGERPAGE;
	cycle[1].Stop = 0;
	count = 2;
	status = vsgPageCyclingSetup(count, &cycle[0]);
	return status;
}

int CycleTestStimSet::handle_trigger(const std::string& s, const std::string&)
{
	//std::cerr << "CycleTestStimSet::handle_trigger: " << s << std::endl;
	int status = 0;
	if (s == "F")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, m_page_b, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "S")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, m_page_c, vsgNOCLEAR);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "X")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, m_page_a, vsgNOCLEAR);
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		status = 2;
	}
	return status;

}
