#include "DPICalStimSet.h"

int DPICalStimSet::init(std::vector<int> pages, int num_stim_pages)
{
	int status = 0;
	int nlevels = 40; // default up to 6 gratings
	m_pages[0] = pages[0];
	m_pages[1] = pages[1];
	m_ipage = 1;

	fixpt().init(2);
	fixpt().setContrast(100);

	//drawCurrent();

	// restore blank page
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);

	return 0;
}

void DPICalStimSet::drawCurrent()
{
	m_ipage = 1 - m_ipage;
	vsgSetDrawPage(vsgVIDEOPAGE, m_pages[m_ipage], vsgBACKGROUND);
	fixpt().draw();
	//vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
	cout << "drawCurrent: page " << m_pages[m_ipage] << " bkgd color " << ARvsg::instance().background_color() << " fixpt " << fixpt() << endl;
	return;
}

void DPICalStimSet::cleanup(std::vector<int> pages)
{
	return;
}

int DPICalStimSet::handle_trigger(const std::string& s, const std::string& args)
{
	int status = 0;
	if (s == "F")
	{
		drawCurrent();
		//cout << "handle(F): page " << m_pages[m_ipage] << " fixpt " << fixpt() << endl;
		//fixpt().setContrast(100);
		//vsgSetDrawPage(vsgVIDEOPAGE, m_pages[m_ipage], vsgNOCLEAR);
		status = 1;
	}
	else if (s == "D")
	{
		stringstream ss(args);
		alert::ARContrastFixationPointSpec f;
		ss >> f;
		if (ss)
		{
			cout << "Got D arg " << f << endl;
			fixpt().x = f.x;
			fixpt().y = f.y;
			fixpt().d = f.d;
			fixpt().color = f.color;
			drawCurrent();
			status = 1;
		}
		else
		{
			cerr << "Error parsing D arg: " << args << endl;
		}
	}
	else if (s == "X")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "1")
	{
		cout << "Got 1" << endl;
		status = 1;
	}
	return status;
}

std::string DPICalStimSet::toString() const
{
	return std::string("DPICalStimSet");
}
