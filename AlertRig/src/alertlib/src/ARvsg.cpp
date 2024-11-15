#include "ARvsg.h"
#include "AlertUtil.h"
#include "RegHelper.h"
#include <iostream>
#include <fstream>
#include <ios>

using namespace alert;
using namespace std;

ARvsg& ARvsg::instance()
{
	static ARvsg vsg;
	return vsg;
};

int ARvsg::loadGammaData(const std::string& filename)
{
	std::ifstream ifs(filename, ios::binary);
	double c[3];
	char tst[4];
	int maxVectors = 5;

	// check the first four bytes. If it starts with 'zz', then the last two bytes 
	// are a version. If no 'zz', then there are just 5 vectors and no color vectors. 
	ifs.read(tst, 4);
	if (tst[0] == 'z' && tst[1] == 'z')
	{
		maxVectors = 11;
		cout << "This gamma file contains DKL and cone iso cvs." << endl;
	}
	else
	{
		ifs.seekg(0);
		maxVectors = 5;
		cout << endl << "WARNING: This gamma file contains DKL but NOT cone iso cvs. Cone iso color vectors will be INCORRECT." << endl << endl;
	}

	for (int i = 0; i < maxVectors; i++)
	{
		ifs.read((char*)c, 3 * sizeof(double));
		m_colors[i].a = c[0];
		m_colors[i].b = c[1];
		m_colors[i].c = c[2];
	}

	long status = 0;
	long length = vsgGetSystemAttribute(vsgCOLOURRESOLUTION);
	short* rs = new short[length];
	short* gs = new short[length];
	short* bs = new short[length];

	cout << "Gamma table length " << length << endl;
	if (!ifs.is_open())
	{
		cout << "Cannot open gamma file " << filename << endl;
		return 1;
	}

	ifs.read((char*)rs, length * sizeof(short));
	if (!ifs)
	{
		cerr << "Error reading red inv gamma table from " << filename << endl;
		ifs.close();
		return 1;
	}
	ifs.read((char*)gs, length * sizeof(short));
	if (!ifs)
	{
		cerr << "Error reading green inv gamma table from " << filename << endl;
		ifs.close();
		return 1;
	}
	ifs.read((char*)bs, length * sizeof(short));
	if (!ifs)
	{
		cerr << "Error reading blue inv gamma table from " << filename << endl;
		ifs.close();
		return 1;
	}
	status = vsgGAMMALoadProfile(length, rs, gs, bs);
	if (!status)
	{
		cout << "Inverse gamma table loaded from " << filename << endl;
	}
	else
	{
		cout << "Error (" << status << " from vsgGAMMALoadProfile while loading inv gamma table to visage" << endl;
		return 1;
	}
	delete[] rs;
	delete[] gs;
	delete[] bs;
	ifs.close();
	return 0;
}

void alert::ARvsg::get_calibration_color(CalibrationColors c, VSGTRIVAL& trival)
{
	//cerr << "WARN: TODO get_calibration_color" << endl;
	trival = m_colors[c];
}


void ARvsg::scaleGammaValues(double* v, short* s, unsigned int length)
{
	for (unsigned int i = 0; i < length; i++)
	{
		int ival = static_cast<unsigned short>(v[i] * 65536);
		if (ival == 65536) ival = 65535;
		s[i] = static_cast<short>(ival - 32767);
	}
}

int ARvsg::setViewDistMM(int screenDistanceMM)
{
	vsgSetViewDistMM(screenDistanceMM);
	return 0;
}

COLOR_TYPE ARvsg::background_color()
{
	return m_background_color;
};

long ARvsg::getScreenHeightPixels()
{
	return m_heightPixels;
};

long ARvsg::getScreenWidthPixels()
{
	return m_widthPixels;
};

double ARvsg::getScreenHeightDegrees()
{
	return m_heightDegrees;
};

double ARvsg::getScreenWidthDegrees()
{
	return m_widthDegrees;
};

ARvsg::~ARvsg()
{
	clear();
};

void ARvsg::setBackgroundColor(const COLOR_TYPE& c)
{
	m_background_color = c;
	// DJS THIS MIGHT BE BAD 
	//arutil_color_to_palette(m_background_color, m_cDummyLevel);
	vsgSetBackgroundColour(&c.trival());
}

void ARvsg::reinit()
{
	// Clear all vsg objects
	cerr << "ARvsg::reinit(): clearing all vsg objects" << endl;
	for (int i = 1; i < vsgGetSystemAttribute(vsgNUMOBJECTS); i++) { vsgObjDestroy(i); }

	// reclaim levels for our level manager
	ARvsg::instance().reset_available_levels();
	cerr << "ARvsg::reinit(): available levels remaining: " << remaining() << endl;

	vsgSetCommand(vsgOVERLAYDISABLE);
	vsgSetDrawMode(vsgSOLIDFILL + vsgCENTREXY);
	vsgSetSpatialUnits(vsgPIXELUNIT);
	vsgSetDrawOrigin(vsgGetScreenWidthPixels() / 2, vsgGetScreenHeightPixels() / 2);

	VSGTRIVAL background;

	vsgSetViewDistMM(m_screenDistanceMM);
	vsgSetSpatialUnits(vsgDEGREEUNIT);
	m_heightPixels = vsgGetScreenHeightPixels();
	m_widthPixels = vsgGetScreenWidthPixels();
	vsgUnitToUnit(vsgPIXELUNIT, m_heightPixels, vsgDEGREEUNIT, &m_heightDegrees);
	vsgUnitToUnit(vsgPIXELUNIT, m_widthPixels, vsgDEGREEUNIT, &m_widthDegrees);

	background = m_background_color.trival();

#if 0
	// Create single dummy object, do not assign it a level
	m_handle = vsgObjCreate();
	//vsgObjSetPixelLevels(m_cDummyLevel, 1);
	cout << "ARvsg::reinit(): Got dummy obj(" << m_handle << ")" << endl;
#else
	cout << "ARvsg::reinit(): No dummy object allocated." << endl;
	m_handle = 999;
#endif

	// Set up triggers and present. A single pulse on DOUT0.
	vsgObjSetTriggers(vsgTRIG_ONPRESENT, 0, 0);
	vsgPresent();

	vsgSetBackgroundColour(&background_color().trival());
	cout << "ARvsg::reinit(): Screen distance = " << m_screenDistanceMM << endl;
	cout << "ARvsg::reinit(): Screen resolution (" << m_widthPixels << "x" << m_heightPixels << ") pixels (" << m_widthDegrees << "x" << m_heightDegrees << ") degrees" << endl;
	//cout << "ARvsg::reinit(): Dummy level " << m_cDummyLevel << " object handle " << m_handle << endl;
	cout << "ARvsg::reinit(): Low/high level " << m_cLowLevel << "/" << m_cHighLevel << endl;
	cout << "ARvsg::reinit(): Background color: " << background_color() << endl;
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	vsgPresent();

}

int ARvsg::init(int screenDistanceMM, COLOR_TYPE i_bg, bool bUseLockFile, bool bSlaveSynch, const std::string& gammaFile)
{
	int status = 0;
	if (!m_initialized)
	{

		// Initialize VSG card.  In this case use the VSG configuration
		// indicated in HKCU\Software\Spike2\VSGConfig IF IT EXISTS. If it does not exist, then init with the current
		// default VSG card (i.e. use "" as the arg to vsgInit). 

		string s;
		if (!GetRegVSGConfig(s))
		{
			cout << "ARvsg::init(): using currently selected VSG configuration (see VSG Desktop)" << endl;
			m_device_handle = vsgInit("");
		}
		else
		{
			cout << "ARvsg::init(): using VSG configuration file \"" << s << "\"" << endl;
			m_device_handle = vsgInit(const_cast<char*>(s.c_str()));
		}
		if (m_device_handle < 0)
		{
			cerr << "Error (" << m_device_handle << ")in vsgInit()." << endl;
			return 1;
		}

		Sleep(500);
		m_initialized = true;

		m_background_color = i_bg;
		m_screenDistanceMM = screenDistanceMM;

		// allocate a HOST page for later (potential) use
		m_hostpage_handle = vsgPAGECreate(vsgHOSTPAGE, vsgGetScreenWidthPixels(), vsgGetScreenHeightPixels(), vsg8BITPALETTEMODE);

		reinit();

		if (gammaFile.size() > 0)
		{
			status = loadGammaData(gammaFile);
		}
	}
	return status;
}

void ARvsg::clear(int ipage)
{
	vsgSetDrawPage(vsgVIDEOPAGE, ipage, vsgBACKGROUND);

	// djs 4-30-2010 
	// modify this to set the display page to the just-cleared page. Not sure why the vsgPresent() was here before. 
	// djs 10-17-2017 drive all outputs to zero on clear
	vsgSetZoneDisplayPage(vsgVIDEOPAGE, ipage + vsgTRIGGERPAGE);

	vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0, 0);

	vsgPresent();
}

void ARvsg::clear()
{
	clear(0);
}

void ARvsg::ready_pulse(int wait_msecs, unsigned int which_bit)
{
	// Issue "ready" triggers to spike2.
	// These commands pulse spike2 port 6. 
	Sleep(wait_msecs);
	vsgFrameSync();
	vsgIOWriteDigitalOut(0xff, which_bit);
	vsgFrameSync();
	//Sleep(10);
	vsgIOWriteDigitalOut(0, which_bit);
	vsgFrameSync();
}


int ARvsg::remaining()
{
	return 250 - m_next_available_level;
}

int ARvsg::request_single(PIXEL_LEVEL& level)
{
	int status = 0;
	if (m_next_available_level < 250)
	{
		level = m_next_available_level++;
	}
	else status = 1;
	return status;
}

int ARvsg::request_range(int num, PIXEL_LEVEL& first)
{
	int status = 0;
	if (remaining() >= num)
	{
		first = m_next_available_level;
		m_next_available_level += num;
	}
	else status = 1;
	return status;
}


