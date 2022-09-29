#pragma once
#include "StimSet.h"
#include <string>
#include <vector>
#include <map>
#include <tuple>

// this tuple represents a single stimulus image and its associated timing and x/y. There is a vector of these. 
typedef std::tuple<std::string, double, double, double, double> FXImageInfo;
typedef std::vector<std::vector<int>> FXGroupsVec;

// filename is txt file, comma separated: 
// c:\img\file.bmp,5,3,1,.5
// image center at (5,3)
// stimup for 1s, after 0.5s delay with fixpt only (or blank if no fixpt)

class FXImageStimSet;
FXImageStimSet *createImageStimSet(const std::string& filename, double x = 0, double y = 0, double duration = 0, double delay = 0, int nlevels=230);
FXImageStimSet *createImageStimSet(alert::ARContrastFixationPointSpec& fixpt, const std::string& filename, double x = 0, double y = 0, double duration = 0, double delay = 0, int nlevels = 230);


class FXImageStimSet :
	public FXStimSet
{
private:
	int m_nlevels;					// number of levels to reserve for image color table
	bool m_bUseGroups;				// If using stim groups
	std::vector<FXImageInfo> m_imagesInfo;
	FXGroupsVec m_groupsVec;
	size_t m_current;
	std::vector<std::string>::const_iterator m_iter;	// points to current image. Advance prior to new trial (except for first)
	int m_pageBlank;
	int m_pageFixpt;
	int m_pageFixptStim;
	std::vector<int> m_pageImages;
	PIXEL_LEVEL m_levelImage;
	VSGLUTBUFFER m_lutBufferBase;
	COLOR_TYPE m_backgroundColorSaved;

	FXImageStimSet();		// no default constructor
	int drawCurrent();		// draw current stimulus page. After this, switching to m_pageFixptStim will display next stim image
	void setupCycling(const FXImageInfo& info);
	void setupCyclingForCurrentGroup();
	bool loadPaletteFromImage(VSGLUTBUFFER& lut, char *filename);

public:
	virtual ~FXImageStimSet();
	FXImageStimSet(alert::ARContrastFixationPointSpec& fixpt, const std::vector<FXImageInfo>& vecInfo, const FXGroupsVec& groupsVec, int nlevels = 230);
	FXImageStimSet(const std::vector<FXImageInfo>& vecInfo, const FXGroupsVec& groupsVec, int nlevels = 230);

	int num_pages()
	{
		return 12;
	}

	int num_overlay_pages()
	{
		return 0;
	}

	int init(ARvsg& vsg, std::vector<int> pages, int);
	int handle_trigger(const std::string& s, const std::string& args);
	std::string toString() const;
	void cleanup(std::vector<int> pages);

};

