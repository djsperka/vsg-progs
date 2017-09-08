#include "FXImageStimSet.h"
#include <sstream>


FXImageStimSet::FXImageStimSet()
	: FXStimSet()
{
}


FXImageStimSet::~FXImageStimSet()
{
}

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

	// get levels for image. Assuming 230 levels have been used in 'evert'.
	vsg.request_range(235, m_levelImage);

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
	static int ihack = 0;
	int status = 0;
	int diStatus;
	VSGLUTBUFFER palImage;	// loaded from image

	// fixpt page
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixpt, vsgBACKGROUND);
	if (has_fixpt())
	{
		fixpt().draw();
	}

	// fixpt + stim page
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixptStim, vsgBACKGROUND);
	if (m_iter != m_images.end())
	{
		char filename[1024];
		int ipal;
		int mode = vsgGetSystemAttribute(vsgVIDEOMODE);
		strncpy_s(filename, 1024, m_iter->c_str(), sizeof(filename));

		// load palette
		for (int i = 0; i<256; i++) palImage[i].a = palImage[i].b = palImage[i].c = 0;
		ipal = vsgImageGetPalette(0, filename, &palImage);
		if (ipal)
		{
			switch (ipal)
			{
			case vsgerrorERRORREADINGFILE: cerr << "get palette vsgerrorERRORREADINGFILE" << endl; break;
			case vsgerrorUNSUPPORTEDIMAGETYPE: cerr << "get palette vsgerrorUNSUPPORTEDIMAGETYPE" << endl; break;
			case vsgerrorUNSUPPORTEDBITMAPFORMAT: cerr << "get palette vsgerrorUNSUPPORTEDBITMAPFORMAT" << endl; break;
			case vsgerrorOUTOFPCMEMORY: cerr << "get palette vsgerrorOUTOFPCMEMORY" << endl; break;
			case vsgerrorIMAGEHASNOPALETTE: cerr << "get palette vsgerrorIMAGEHASNOPALETTE" << endl; break;
			default: cerr << "get palette error: " << ipal << " filename " << filename << " len " << strlen(filename) << endl; break;
			}
		}
		else
			vsgPaletteWrite((VSGLUTBUFFER*)palImage, 0, 230);

		diStatus = vsgDrawImage(vsgBMPPICTURE, 0, 0, filename);
	}
	else
	{
		cerr << "NO IMAGE TO DRAW!" << endl;
	}

	if (has_fixpt())
	{
		fixpt().draw();
	}

	vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, vsgBACKGROUND);
	return status;
}