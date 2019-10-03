#include "FXImageStimSet.h"
#include <sstream>


FXImageStimSet::FXImageStimSet()
	: FXStimSet()
{
}


FXImageStimSet::~FXImageStimSet()
{
}

FXImageStimSet::FXImageStimSet(ARContrastFixationPointSpec& fixpt, const std::vector<std::string>& vecImages, double x, double y, double durationInSec, int low, int high)
	: FXStimSet(fixpt)
	, m_x(x)
	, m_y(y)
	, m_lowwater(low)
	, m_highwater(high)
	, m_bUseCycling(false)
	, m_images(vecImages)
	, m_current(0)
{
	if (durationInSec <= 0)
		m_stimDurationFrames = 0;
	else
	{
		int f = vsgGetSystemAttribute(vsgFRAMETIME);
		m_stimDurationFrames = durationInSec / f * 1000000.0f;
		m_bUseCycling = true;
	}
}

FXImageStimSet::FXImageStimSet(const std::vector<std::string>& vecImages, double x, double y, double durationInSec, int low, int high)
	: FXStimSet()
	, m_x(x)
	, m_y(y)
	, m_lowwater(low)
	, m_highwater(high)
	, m_bUseCycling(false)
	, m_images(vecImages)
	, m_current(0)
{
	if (durationInSec <= 0)
		m_stimDurationFrames = 0;
	else
	{
		int f = vsgGetSystemAttribute(vsgFRAMETIME);
		m_stimDurationFrames = durationInSec / f * 1000000.0f;
		m_bUseCycling = true;
	}
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

	if (m_highwater > 0)
	{
		cerr << "Preloading " << m_images.size() << " images..." << endl;
		double w, h;
		int currentUnits;

		// load palette
		vsgSetCommand(vsgPALETTERAMP);

		// get resolution of images. Assume first image same as all of them. 
		// Need the resolution and position in pixels for the call to vsgDrawMoveRect (and for creating host pages)
		currentUnits = vsgGetSystemAttribute(vsgSPATIALUNITS);
		vsgSetSpatialUnits(vsgPIXELUNIT);
		char filename[1024];
		strncpy_s(filename, 1024, m_images[0].c_str(), sizeof(filename));
		status = vsgImageGetSize(0, filename, &w, &h);
		vsgSetSpatialUnits(currentUnits);
		cerr << "Got resolution (status " << status << ") " << w << " x " << h << " from " << filename << endl;
		m_imageWPixels = (int)w;
		m_imageHPixels = (int)h;

		// convert x,y
		vsgUnit2Unit(vsgDEGREEUNIT, m_x, vsgPIXELUNIT, &m_imageXPixels);
		vsgUnit2Unit(vsgDEGREEUNIT, m_y, vsgPIXELUNIT, &m_imageYPixels);

		// now load images into host pages, store handles in m_pageHandles
		for (auto f : m_images)
		{
			int i = vsgPAGECreate(vsgHOSTPAGE, (int)w, (int)h, vsg8BITPALETTEMODE);
			if (i < 0)
			{
				cerr << "ERROR loading images: vsgPAGECreate failed." << endl;
				break;
			}
			else
			{
				m_pageHandles.push_back(i);
				strncpy_s(filename, 1024, f.c_str(), sizeof(filename));
				vsgSetDrawPage(vsgHOSTPAGE, i, vsgNOCLEAR);
				i=vsgDrawImage(0, 0, 0, filename);
				cerr << "Loaded file " << f << " status " << i << endl;
			}
		}
	}

	if (m_bUseCycling)
	{
		cerr << "Set up cycling - stim duration is " << m_stimDurationFrames << " frames." << endl;
		setupCycling();
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
		if (m_bUseCycling)
		{
			vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		}
		status = 1;
	}
	else if (s == "a")
	{
		m_current++;
		if (m_current == m_images.size())
			m_current = 0;
		drawCurrent();
		status = 0;
	}
	else if (s == "X")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, vsgNOCLEAR);
		if (m_bUseCycling)
			vsgSetCommand(vsgCYCLEPAGEDISABLE);
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
	if (m_highwater == 0)
	{
		char filename[1024];
		int ipal;
		int mode = vsgGetSystemAttribute(vsgVIDEOMODE);
		strncpy_s(filename, 1024, m_images[m_current].c_str(), sizeof(filename));

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

		diStatus = vsgDrawImage(vsgBMPPICTURE, m_x, -m_y, filename);
	}
	else
	{
		vsgSetSpatialUnits(vsgPIXELUNIT);
		vsgDrawMoveRect(vsgHOSTPAGE, m_pageHandles[m_current], 0, 0, m_imageWPixels, m_imageHPixels, m_imageXPixels, -m_imageYPixels, m_imageWPixels, m_imageHPixels);
		vsgSetSpatialUnits(vsgDEGREEUNIT);
	}

	if (has_fixpt())
	{
		fixpt().draw();
	}

	vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, vsgBACKGROUND);
	return status;
}

void FXImageStimSet::setupCycling()
{
	VSGCYCLEPAGEENTRY cycle[12];	// warning! No check on usage. You have been warned. 
	int status = 0;
	int count = 0;

	memset(cycle, 0, sizeof(cycle));

	cycle[count].Frames = m_stimDurationFrames;
	cycle[count].Page = m_pageFixptStim + vsgTRIGGERPAGE;
	cycle[count].Stop = 0;
	count++;
	cycle[count].Frames = 0;
	cycle[count].Page = m_pageBlank + vsgTRIGGERPAGE;
	cycle[count].Stop = 1;
	count++;

	status = vsgPageCyclingSetup(count, &cycle[0]);
}

