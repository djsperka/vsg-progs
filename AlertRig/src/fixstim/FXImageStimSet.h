#pragma once
#include "StimSet.h"
#include <string>
#include <vector>

class FXImageStimSet :
	public FXStimSet
{
private:
	double m_x, m_y;// where to draw images (fixpt can/will have its own location)
	int m_stimDurationFrames;
	int m_lowwater, m_highwater;	// for loading/caching images 
	bool m_bUseCycling;				// if precise stim time and bkgd time given.
	double m_imageXPixels, m_imageYPixels;
	int m_imageWPixels, m_imageHPixels;
	std::vector<std::string> m_images;	// filenames, verified to exist
	std::vector<int> m_pageHandles;
	size_t m_current;
	std::vector<std::string>::const_iterator m_iter;	// points to current image. Advance prior to new trial (except for first)
	int m_pageBlank;
	int m_pageFixpt;
	int m_pageFixptStim;
	PIXEL_LEVEL m_levelImage;

	FXImageStimSet();		// no default constructor
	int drawCurrent();		// draw current stimulus page. After this, switching to m_pageFixptStim will display next stim image
	void setupCycling();

public:
	virtual ~FXImageStimSet();
	//FXImageStimSet(ARContrastFixationPointSpec& fixpt);	// all images added at constructor time. sorry.
	FXImageStimSet(ARContrastFixationPointSpec& fixpt, const std::vector<std::string>& vecImages, double x=0, double y=0, double stimDurationSec=0, int low_water=0, int high_water=0);
	FXImageStimSet(const std::vector<std::string>& vecImages, double x=0, double y=0, double stimDurationSec=0, int low_water=0, int high_water=0);

	//void addImages(const std::vector<std::string>& vecImages);

	int num_pages()
	{
		return 3;
	}

	int num_overlay_pages()
	{
		return 0;
	}

	int init(ARvsg& vsg, std::vector<int> pages);
	int handle_trigger(std::string& s);
	std::string toString() const;
	void cleanup(std::vector<int> pages);

};

