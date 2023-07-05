#pragma once
#include "VSGV8.H"
#include "ARtypes.h"
#include <string>

namespace alert
{
	typedef enum { background, dkl_rg_plus, dkl_rg_minus, dkl_s_plus, dkl_s_minus, l_minus, l_plus, m_minus, m_plus, s_minus, s_plus } CalibrationColors;


	// Singleton class representing the VSG. 
	// Modified 4-27-2010 djs
	// Changed to a "double singleton" class to accomodate the dual vsg setup. 
	// Modified 9-17-2021 djs
	// Revert to single singleton.

	class ARvsg
	{
	public:
		~ARvsg();
		int init(int screenDistanceMM, COLOR_TYPE i_bg, bool bUseLockFile = true, bool bSlaveSynch = false, const std::string& gammaFile = std::string());
		void reinit();	// re-do init except for re-init of vsg. 

		// set view distance
		int setViewDistMM(int screenDistanceMM);

		// get gamma color - this will only give meaningful results if you have also 
		// loaded a gamma data file.
		void get_calibration_color(CalibrationColors c, VSGTRIVAL& trival);

		/* Clear any page and display it. */
		void clear(int i);

		/* Clear page 0 and display it. */
		void clear();

		/* Send ready pulse -- used for VSG/Spike2 expts. */
		void ready_pulse(int wait_msecs = 2000, unsigned int which_bit = vsgDIG6);

		/*
		 * instance() will return a singleton instance of this class. Use this when you are using a single
		 * vsg. When using dual vsg setup, use master() and slave() to get instances for each separately.
		 */
		static ARvsg& instance();

		void setBackgroundColor(const COLOR_TYPE& c);
		COLOR_TYPE background_color();
		PIXEL_LEVEL background_level();
		long getScreenHeightPixels();
		long getScreenWidthPixels();
		double getScreenHeightDegrees();
		double getScreenWidthDegrees();

		//		LevelManager& getLevelManager();
				// Use these functions to get available levels
		int request_single(PIXEL_LEVEL& level);
		int request_range(int num, PIXEL_LEVEL& first);
		int remaining();
		void reset_available_levels() { m_next_available_level = m_cLowLevel+1; };
		VSGOBJHANDLE dummyObjectHandle() { return m_handle; };
		long hostpage_handle() const { return m_hostpage_handle; };
		PIXEL_LEVEL lowlevel() const { return m_cLowLevel; };
		PIXEL_LEVEL highlevel() const { return m_cHighLevel; };

	private:
		ARvsg()
			: m_initialized(false)
			, m_handle(0)
			, m_background_color(gray)
			, m_screenDistanceMM(0)
			, m_heightPixels(0)
			, m_widthPixels(0)
			, m_heightDegrees(0)
			, m_widthDegrees(0)
			, m_next_available_level(m_cLowLevel+1)
			, m_hostpage_handle(-1)
//			, m_colors{ COLOR_TYPE(gray), COLOR_TYPE(black), COLOR_TYPE(white), COLOR_TYPE(black), COLOR_TYPE(white) }

		{};

		ARvsg(ARvsg const&);	// prohibited
		ARvsg& operator=(ARvsg const&) {};
		int loadGammaData(const std::string& filename);
		void scaleGammaValues(double* v, short* s, unsigned int length);
		bool m_initialized;
		VSGOBJHANDLE m_handle;
		COLOR_TYPE m_background_color;
		int m_screenDistanceMM;
		long m_heightPixels;
		long m_widthPixels;
		double m_heightDegrees;
		double m_widthDegrees;
		long m_device_handle;
		int m_next_available_level;
		long m_hostpage_handle;
		VSGTRIVAL m_colors[11];

		static const PIXEL_LEVEL m_cDummyLevel = 0;
		static const PIXEL_LEVEL m_cLowLevel = 1;
		static const PIXEL_LEVEL m_cHighLevel = 251;
	};
};
