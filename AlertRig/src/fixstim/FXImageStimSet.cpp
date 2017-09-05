#include "FXImageStimSet.h"
#include <sstream>


FXImageStimSet::FXImageStimSet()
	: FXStimSet()
{
}


FXImageStimSet::~FXImageStimSet()
{
}

//FXImageStimSet::FXImageStimSet(ARContrastFixationPointSpec& fixpt)
//	: FXStimSet(fixpt)
//{
//
//}

FXImageStimSet::FXImageStimSet(ARContrastFixationPointSpec& fixpt, const std::vector<std::string>& vecImages)
	: FXStimSet(fixpt)
	, m_images(vecImages)
{
	m_iter = m_images.begin();
}

FXImageStimSet::FXImageStimSet(const std::vector<std::string>& vecImages)
	: FXStimSet()
	, m_images(vecImages)
{
	m_iter = m_images.begin();
}

// djs - CHECK WHETHER THIS IS NEEDED, AND WHETHER ITERATOR BEHAVES AS EXPECTED AFTER IMAGES ADDED
//void FXImageStimSet::addImages(const std::vector<std::string>& vecImages)
//{
//	m_images.insert(m_images.end(), vecImages.begin(), vecImages.end());
//}

std::string FXImageStimSet::toString() const
{
	std::ostringstream oss;
	oss << "FXImageStimSet:" << endl;
	if (has_fixpt())
		oss << " Fixpt: " << fixpt() << endl;
	else
		oss << " Fixpt: NONE" << endl;
	oss << " Images: " << m_images.size() << endl;
	for (std::vector<std::string>::const_iterator it = m_images.begin(); it != m_images.end(); it++)
		oss << "   " << *it << endl;
	return oss.str();
}

int FXImageStimSet::init(ARvsg& vsg, std::vector<int> pages)
{
	int status = 0;
	m_pageBlank = pages[0];
	m_pageFixpt = pages[1];
	m_pageFixptStim = pages[2];

	vsgSetVideoMode(vsgTRUECOLOURMODE);
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, vsgBACKGROUND);

	if (has_fixpt())
	{
		fixpt().init(vsg, 2);
		fixpt().setContrast(100);
	}

	status = drawCurrent();

	return status;
}

void FXImageStimSet::cleanup(std::vector<int> pages)
{
	cerr << "cleanup" << endl;
	vsgSetVideoMode(vsg8BITPALETTEMODE | vsgGAMMACORRECT);
}


int FXImageStimSet::handle_trigger(std::string& s)
{
	int status = 0;
	if (s == "F")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixpt, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "S")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixptStim, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "a")
	{
		m_iter++;
		if (m_iter == m_images.end())
			m_iter = m_images.begin();
		drawCurrent();
		status = 0;
	}
	else if (s == "X")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, vsgNOCLEAR);
		status = 1;
	}
	return status;
}


// Leaves current draw page as the blank page.
int FXImageStimSet::drawCurrent()
{
	int status = 0;
	int diStatus;
	// fixpt page
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixpt, vsgBACKGROUND);
	if (has_fixpt())
		fixpt().draw();

	// fixpt + stim page
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixptStim, vsgBACKGROUND);
	if (m_iter != m_images.end())
	{
		char filename[1024];
		strncpy_s(filename, 1024, m_iter->c_str(), sizeof(filename));
		diStatus = vsgDrawImage(vsgBMPPICTURE, 0, 0, filename);
		cerr << "drawImage status " << diStatus << endl;
	}
	else
		cerr << "NO IMAGE TO DRAW!" << endl;
	if (has_fixpt()) fixpt().draw();

	vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, vsgNOCLEAR);
	return status;
}