#pragma once
#include "VSGV8.H"
#include "ARtypes.h"
#include <string>

namespace alert
{

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
		const COLOR_TYPE& get_color(unsigned int i);


		// This function initializes all pages to background color. That color is set as 
		// level 0 - this func does not use vsgBACKGROUND. This func should be used if 
		// you draw gratings with draw(true). Why? Well, initially your pages are initialized
		// to a low level - 0 or 1. Then draw(true) draws an ellipse using level 250. Next, 
		// the grating is drawn using vsgTRANSONHIGHER, meaning that only the portion of the 
		// grating that overlays the ellipse will actually be drawn. 
		int init_video();

		int init_overlay();

		int init_video_pages(voidfunc func_before_objects, voidfunc func_after_objects, void* data);

		/* lock/unlock */
		bool acquire_lock();
		void release_lock();

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

		/*
		 * Select specifies the vsg to which commands are directed.
		 */
		void select();

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
		void reset_available_levels() { m_next_available_level = 0; };
		VSGOBJHANDLE dummyObjectHandle() { return m_handle; };
	private:
		ARvsg()
			: m_initialized(false)
			, m_handle(0)
			, m_background_level(-1)
			, m_background_color(gray)
			, m_screenDistanceMM(0)
			, m_heightPixels(0)
			, m_widthPixels(0)
			, m_heightDegrees(0)
			, m_widthDegrees(0)
			, m_device_handle(-999)
			, m_next_available_level(0)
			, m_colors{ COLOR_TYPE(gray), COLOR_TYPE(black), COLOR_TYPE(white), COLOR_TYPE(black), COLOR_TYPE(white) }

		{};

		ARvsg(ARvsg const&);	// prohibited
		ARvsg& operator=(ARvsg const&) {};
		int loadGammaData(const std::string& filename);
		void scaleGammaValues(double* v, short* s, unsigned int length);
		bool m_initialized;
		VSGOBJHANDLE m_handle;
		PIXEL_LEVEL m_background_level;
		COLOR_TYPE m_background_color;
		int m_screenDistanceMM;
		long m_heightPixels;
		long m_widthPixels;
		double m_heightDegrees;
		double m_widthDegrees;
#if 0
		static bool c_bHaveLock;
#endif
		long m_device_handle;
		int m_next_available_level;
		COLOR_TYPE m_colors[5];
	};
};
