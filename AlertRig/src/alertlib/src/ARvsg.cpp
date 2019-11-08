#include "ARvsg.h"
#include "RegHelper.h"
#include "AlertUtil.h"

namespace alert
{
	ARvsg& ARvsg::instance()
	{
		static ARvsg vsg;
		return vsg;
	};

	ARvsg& ARvsg::instance(bool is_master, bool is_slave)
	{
		if (is_master && !is_slave) return ARvsg::master();
		if (!is_master && is_slave) return ARvsg::slave();
		return ARvsg::instance();
	}

	ARvsg& ARvsg::master()
	{
		static ARvsg master(true, false);
		return master;
	};

	ARvsg& ARvsg::slave()
	{
		static ARvsg slave(false, true);
		return slave;
	};

	int ARvsg::setViewDistMM(int screenDistanceMM)
	{
		vsgSetViewDistMM(screenDistanceMM);
		return 0;
	}

	COLOR_TYPE ARvsg::background_color()
	{
		return m_background_color;
	};

	PIXEL_LEVEL ARvsg::background_level()
	{
		return m_background_level;
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


	bool ARvsg::acquire_lock()
	{
		return true;
	}


	void ARvsg::release_lock()
	{
		return;
	}

	bool ARvsg::is_master()
	{
		return m_is_master;
	}

	bool ARvsg::is_slave()
	{
		return m_is_slave;
	}


	void ARvsg::select()
	{
		vsgInitSelectDevice(m_device_handle);
	}

	void ARvsg::setBackgroundColor(const COLOR_TYPE& c)
	{
		m_background_color = c;
		if (m_background_level < 0)
			request_single(m_background_level);
		arutil_color_to_palette(m_background_color, m_background_level);
		vsgSetBackgroundColour(&c.trival());
	}

	void ARvsg::reinit()
	{
		// Clear all vsg objects
		std::cerr << "ARvsg::reinit(): clearing all vsg objects" << std::endl;
		for (int i = 1; i < vsgGetSystemAttribute(vsgNUMOBJECTS); i++) { vsgObjDestroy(i); }

		// reclaim levels for our level manager
		ARvsg::instance().reset_available_levels();

		vsgSetCommand(vsgOVERLAYDISABLE);
		//vsgSetCommand(vsgCYCLEPAGEDISABLE);
		//vsgSetCommand(vsgCYCLELUTDISABLE);
		//vsgSetCommand(vsgDISABLELUTANIM);
		vsgSetDrawMode(vsgSOLIDFILL + vsgCENTREXY);
		vsgSetSpatialUnits(vsgPIXELUNIT);
		vsgSetDrawOrigin(vsgGetScreenWidthPixels() / 2, vsgGetScreenHeightPixels() / 2);


		// I don't think we use the negative screen distance scheme anywhere. 
		if (m_screenDistanceMM <= 0)
		{
			// djs 3-22-11
			// spatial units are set to pixel units with the init() call. 
			//vsgSetSpatialUnits(vsgPIXELUNIT);
			m_heightPixels = vsgGetScreenHeightPixels();
			m_widthPixels = vsgGetScreenWidthPixels();
			request_single(m_background_level);
			arutil_color_to_palette(m_background_color, m_background_level);
			std::cout << "ARvsg::reinit(): Screen distance = " << m_screenDistanceMM << ", will use PIXEL units." << std::endl;
			std::cout << "ARvsg::reinit(): Background level " << m_background_level << " color set to " << m_background_color << std::endl;
		}
		else
		{
			VSGTRIVAL background;

			vsgSetViewDistMM(m_screenDistanceMM);
			vsgSetSpatialUnits(vsgDEGREEUNIT);
			m_heightPixels = vsgGetScreenHeightPixels();
			m_widthPixels = vsgGetScreenWidthPixels();
			vsgUnitToUnit(vsgPIXELUNIT, m_heightPixels, vsgDEGREEUNIT, &m_heightDegrees);
			vsgUnitToUnit(vsgPIXELUNIT, m_widthPixels, vsgDEGREEUNIT, &m_widthDegrees);

			background = m_background_color.trival();
			// this level gets used later, but we request it now to insure we get level 0
			//request_single(m_background_level);
			m_background_level = 250;	// djs HACK HACK HACK

			// Create single dummy object and assign it a level
			m_handle = vsgObjCreate();
			vsgObjSetPixelLevels(m_background_level, 1);
			std::cout << "ARvsg::reinit(): Got dummy obj(" << m_handle << ")" << std::endl;

			// Set up triggers and present. A single pulse on DOUT0.
			vsgObjSetTriggers(vsgTRIG_ONPRESENT, 0, 0);
			vsgPresent();

			vsgSetBackgroundColour(&background);
			std::cout << "ARvsg::reinit(): Screen distance = " << m_screenDistanceMM << ", will use DEGREE units." << std::endl;
			std::cout << "ARvsg::reinit(): Background level " << m_background_level << " color set to " << m_background_color << std::endl;
			vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
			vsgPresent();
		}

	}

	int ARvsg::init(int screenDistanceMM, COLOR_TYPE i_bg, bool bUseLockFile, bool bSlaveSynch)
	{
		int status = 0;
		if (!m_initialized)
		{
			if (bUseLockFile)
			{
				if (!acquire_lock())
				{
					std::cerr << "ARvsg::init(): cannot acquire VSG lock!" << std::endl;
					return 1;
				}
			}
			else
			{
				std::cout << "ARvsg::init(): ignoring lock file!" << std::endl;
			}


			// Initialize VSG card. 
			// Exactly WHICH vsg card should be initialize depends on the particulars of what this ARvsg represents. 
			// If running master/slave setup, then m_is_master==true or m_is_slave==true, and then specific 
			// initializations should be done using the VSG configurations indicated in the registry key/value 
			// HKCU\Software\Spike2\VSGMaster (or VSGSlave). 
			// If running single VSG, then m_is_master == m_is_slave == false. In this case use the VSG configuration
			// indicated in HKCU\Software\Spike2\VSGConfig IF IT EXISTS. If it does not exist, then init with the current
			// default VSG card (i.e. use "" as the arg to vsgInit). 

			if (!m_is_master && !m_is_slave)
			{
				std::string s;
				if (!GetRegVSGConfig(s))
				{
					std::cout << "ARvsg::init(): using currently selected VSG configuration (see VSG Desktop)" << std::endl;
					m_device_handle = vsgInit("");
				}
				else
				{
					std::cout << "ARvsg::init(): using VSG configuration file \"" << s << "\"" << std::endl;
					m_device_handle = vsgInit(const_cast<char *>(s.c_str()));
				}
			}
			else if (m_is_master)
			{
				std::string s;
				if (!GetRegVSGMaster(s))
				{
					std::cerr << "ERROR: Registry key HKCU\\Software\\Spike2\\VSGMaster not found!" << std::endl;
					return 1;
				}
				else
				{
					std::cout << "ARvsg::init(): master is using configuration file \"" << s << "\"" << std::endl;
					m_device_handle = vsgInit(const_cast<char *>(s.c_str()));
				}
			}
			else if (m_is_slave)
			{
				std::string s;
				if (!GetRegVSGSlave(s))
				{
					std::cerr << "ERROR: Registry key HKCU\\Software\\Spike2\\VSGSlave not found!" << std::endl;
					return 1;
				}
				else
				{
					std::cout << "ARvsg::init(): slave is using configuration file \"" << s << "\"" << std::endl;
					if (bSlaveSynch)
					{
						m_device_handle = vsgAdvancedInit(const_cast<char *>(s.c_str()), vsgADVINITSLAVEVIDEO);
					}
					else
					{
						std::cout << "WARNING: Non-enslavement initialization!!! No frame sync!!" << std::endl;
						m_device_handle = vsgInit(const_cast<char *>(s.c_str()));
					}
				}
			}

			if (m_device_handle < 0)
			{
				std::cerr << "Error (" << m_device_handle << ")in vsgInit()." << std::endl;
				return 1;
			}

			Sleep(500);
			m_initialized = true;

			/*
			 * If screen distance is negative, then set spatial units to PIXEL units.
			 */

			vsgInitSelectDevice(m_device_handle);

			m_background_color = i_bg;
			m_screenDistanceMM = screenDistanceMM;

			reinit();

			////if (screenDistanceMM <= 0) 
			////{
			////	// djs 3-22-11
			////	// spatial units are set to pixel units with the init() call. 
			////	//vsgSetSpatialUnits(vsgPIXELUNIT);
			////	m_heightPixels = vsgGetScreenHeightPixels();
			////	m_widthPixels = vsgGetScreenWidthPixels();
			////	m_background_color = i_bg;
			////	request_single(m_background_level);
			////	arutil_color_to_palette(m_background_color, m_background_level);
			////	std::cout << "ARvsg::init(): Screen distance = " << screenDistanceMM << ", will use PIXEL units." << std::endl;
			////	std::cout << "ARvsg::init(): Background level " << m_background_level << " color set to " << m_background_color << std::endl;
			////}
			////else
			////{
			////	vsgSetViewDistMM(screenDistanceMM);
			////	vsgSetSpatialUnits(vsgDEGREEUNIT);
			////	m_heightPixels = vsgGetScreenHeightPixels();
			////	m_widthPixels = vsgGetScreenWidthPixels();
			////	vsgUnitToUnit(vsgPIXELUNIT, m_heightPixels, vsgDEGREEUNIT, &m_heightDegrees);
			////	vsgUnitToUnit(vsgPIXELUNIT, m_widthPixels, vsgDEGREEUNIT, &m_widthDegrees);

			////	m_background_color = i_bg;
			////	background = m_background_color.trival();
			////	// this level gets used later, but we request it now to insure we get level 0
			////	//request_single(m_background_level);
			////	m_background_level = 250;	// djs HACK HACK HACK

			////	// Create single dummy object and assign it a level
			////	m_handle = vsgObjCreate();
			////	vsgObjSetPixelLevels(m_background_level, 1);
			////	
			////	// Set up triggers and present. A single pulse on DOUT0.
			////	vsgObjSetTriggers(vsgTRIG_ONPRESENT, 0, 0);
			////	vsgPresent();

			////	vsgSetBackgroundColour(&background);
			////	std::cout << "ARvsg::init(): Screen distance = " << screenDistanceMM << ", will use DEGREE units." << std::endl;
			////	std::cout << "ARvsg::init(): Background level " << m_background_level << " color set to " << m_background_color << std::endl;
			////	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
			////	vsgPresent();
			////}
		}
		return status;
	}



	int ARvsg::init_overlay()
	{
		int status = 0;
		VSGTRIVAL background;

		if (!m_initialized)
		{
			std::cerr << "init_overlay: Must call init first!" << std::endl;
			status = 1;
		}
		else
		{
			vsgSetCommand(vsgOVERLAYMASKMODE);
			background = m_background_color.trival();

			// Get the number of overlay pages, then clear them all. 
			int npages = vsgGetSystemAttribute(vsgNUMOVERLAYPAGES);
			std::cout << "There are " << npages << " overlay pages." << std::endl;
			vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&background, 1, 1);
			for (int i = npages - 1; i >= 0; i--)
			{
				vsgSetDrawPage(vsgOVERLAYPAGE, i, 1);
			}
		}
		return status;
	}



	int ARvsg::init_video()
	{
		return init_video_pages(NULL, NULL, NULL);
	}




	int ARvsg::init_video_pages(voidfunc func_before_objects, voidfunc func_after_objects, void *data)
	{
		int i;
		int status = 0;
		VSGTRIVAL background;
		PIXEL_LEVEL dummy_level;

		if (!m_initialized)
		{
			std::cerr << "Must call ARvsg::init first!" << std::endl;
			return 1;
		}
		else
		{
			// set background color, er colour, in palette
			// We'll set the vsgBACKGROUND color later, after the vsgObject is created. 
			// The background level was obtained in the init() call.
			background = m_background_color.trival();

			VSGLUTBUFFER buffer;
			for (i = 0; i < 256; i++) buffer[i] = background;
			vsgLUTBUFFERWrite(0, &buffer);
			vsgLUTBUFFERtoPalette(0);





			//		vsgPaletteWrite((VSGLUTBUFFER*)&background, m_background_level, 1);
			//		vsgPaletteWrite((VSGLUTBUFFER*)&background, 250, 1);
			//		for (i = 0; i<252; i++) vsgPaletteWrite((VSGLUTBUFFER*)&background, i, 1);

					// Now clear all pages to background level, then call the before_objects callback, 
					// if it exists. 
			for (i = 0; i < vsgGetSystemAttribute(vsgNUMVIDEOPAGES); i++)
			{
				vsgSetDrawPage(vsgVIDEOPAGE, i, m_background_level);
				if (func_before_objects)
				{
					func_before_objects(i, data);
				}
			}


			// Create a single vsg object and set vsgBACKGROUND color
			m_handle = vsgObjCreate();
			request_single(dummy_level);
			vsgObjSetPixelLevels(dummy_level, 1);
			std::cerr << " Dummy object " << (int)m_handle << " level " << (int)dummy_level << std::endl;
			vsgSetBackgroundColour(&background);


			// Now call after_objects callback if not null
			if (func_after_objects)
			{
				for (i = 0; i < vsgGetSystemAttribute(vsgNUMVIDEOPAGES); i++)
				{
					vsgSetDrawPage(vsgVIDEOPAGE, i, vsgNOCLEAR);
					func_after_objects(i, data);
				}
			}

			// Finally, set page 0 as the current page and present. 
			vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);
			vsgPresent();
		}
		return status;
	};

	void ARvsg::clear(int ipage)
	{
		vsgSetDrawPage(vsgVIDEOPAGE, ipage, vsgBACKGROUND);

		// djs 4-30-2010 
		// modify this to set the display page to the just-cleared page. Not sure why the vsgPresent() was here before. 
		// djs 10-17-2017 drive all outputs to zero on clear
		vsgSetZoneDisplayPage(vsgVIDEOPAGE, ipage + vsgTRIGGERPAGE);
		vsgSetTriggerOptions(vsgTRIGOPT_PRESENT, 0, vsgTRIG_OUTPUTMARKER, 0.5, 0, 0, 0x1FE);
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

};
