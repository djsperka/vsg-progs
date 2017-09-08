#pragma once
#include "StimSet.h"
#include <string>
#include <vector>

class FXImageStimSet :
	public FXStimSet
{
private:
	std::vector<std::string> m_images;	// filenames, verified to exist
	std::vector<std::string>::const_iterator m_iter;	// points to current image. Advance prior to new trial (except for first)
	int m_pageBlank;
	int m_pageFixpt;
	int m_pageFixptStim;
	PIXEL_LEVEL m_levelImage;

	FXImageStimSet();		// no default constructor
	int drawCurrent();		// draw current stimulus page. After this, switching to m_pageFixptStim will display next stim image
public:
	virtual ~FXImageStimSet();
	//FXImageStimSet(ARContrastFixationPointSpec& fixpt);	// all images added at constructor time. sorry.
	FXImageStimSet(ARContrastFixationPointSpec& fixpt, const std::vector<std::string>& vecImages);
	FXImageStimSet(const std::vector<std::string>& vecImages);

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

