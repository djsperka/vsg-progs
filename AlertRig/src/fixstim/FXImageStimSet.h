#pragma once
#include "StimSet.h"
#include <string>
#include <vector>
#include <map>
#include <tuple>

typedef std::tuple<std::string, double, double, double, double> FXImageInfo;

// filename is txt file, comma separated: 
// c:\img\file.bmp,5,3,1,.5
// image center at (5,3)
// stimup for 1s, after 0.5s delay with fixpt only (or blank if no fixpt)

class FXImageStimSet;
FXImageStimSet *createImageStimSet(const std::string& filename, double x = 0, double y = 0, double duration = 0, double delay = 0, int nlevels=230);
FXImageStimSet *createImageStimSet(ARContrastFixationPointSpec& fixpt, const std::string& filename, double x = 0, double y = 0, double duration = 0, double delay = 0, int nlevels = 230);


class FXImageStimSet :
	public FXStimSet
{
private:
	//double m_x, m_y;// where to draw images (fixpt can/will have its own location)
	//int m_stimDurationFrames;
	int m_nlevels;					// number of levels to reserve for image color table
	//int m_lowwater, m_highwater;	// for loading/caching images 
	bool m_bUseCycling;				// if precise stim time and bkgd time given for stim drawn by drawCurrent()
	//double m_imageXPixels, m_imageYPixels;
	//int m_imageWPixels, m_imageHPixels;
	//std::vector<std::string> m_images;	// filenames, verified to exist
	std::vector<FXImageInfo> m_imagesInfo;
	//std::vector<int> m_pageHandles;
	//std::map<std::string, int> m_pageHandleMap;
	size_t m_current;
	std::vector<std::string>::const_iterator m_iter;	// points to current image. Advance prior to new trial (except for first)
	int m_pageBlank;
	int m_pageFixpt;
	int m_pageFixptStim;
	PIXEL_LEVEL m_levelImage;

	FXImageStimSet();		// no default constructor
	int drawCurrent();		// draw current stimulus page. After this, switching to m_pageFixptStim will display next stim image
	void setupCycling(const FXImageInfo& info);
	bool loadPaletteFromImage(char *filename, int nlevels);

public:
	virtual ~FXImageStimSet();
	//FXImageStimSet(ARContrastFixationPointSpec& fixpt);	// all images added at constructor time. sorry.
	FXImageStimSet(ARContrastFixationPointSpec& fixpt, const std::vector<std::string>& vecImages, double x=0, double y=0, double stimDurationSec=0, int nlevels=230);
	FXImageStimSet(const std::vector<std::string>& vecImages, double x=0, double y=0, double stimDurationSec=0, int nlevels=230);
	FXImageStimSet(ARContrastFixationPointSpec& fixpt, const std::vector<FXImageInfo>& vecInfo, int nlevels = 230);
	FXImageStimSet(const std::vector<FXImageInfo>& vecInfo, int nlevels = 230);

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

