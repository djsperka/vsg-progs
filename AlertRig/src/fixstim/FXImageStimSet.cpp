#include "FXImageStimSet.h"
#include <iostream>
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

const DWORD f_truecolorGray = 127 * (1 + 256 + 256 * 256);



bool parseImageInputFile(std::vector<FXImageInfo>& vecInfo, FXGroupsVec& groupsVec, const std::string& filename, double xDefault = 0, double yDefault = 0, double durationDefault = 0, double delayDefault = 0)
{
	bool bReturn = false;

	boost::filesystem::path p(filename);
	if (!exists(p))
	{
		std::cerr << "Error: image list file does not exist: " << filename << endl;
		return false;
	}
	else
	{
		bReturn = true;
		boost::filesystem::path folder = p.parent_path();		// if file has relative pathnames to images, they are relative to dir file lives in
		std::cerr << "Found image list file " << p << " at path " << folder << std::endl;

		// open file, read line-by-line and parse
		string line;
		int linenumber = 0;		// count lines from 0
		int imagecount = 0;
		int groupcount = 0;
		bool doingImages = true;
		bool haveGroups = false;				// only set to true if Groups line found
		double dFixptSec = 0, dImageSec = 0;	// these are only valid if haveGroups == true

		std::ifstream myfile(filename.c_str());
		if (myfile.is_open())
		{
			while (getline(myfile, line))
			{
				linenumber++;

				// line may look like these things
				// 1) c:\path\file.bmp                    filename only. Position will be 0,0 duration=delay=0
				// 2) c:\path\file.bmp,x,y                filename, position (in current units), duration=delay=0
				// 3) c:\path\file.bmp,x,y,durSec,dlySec  all 5 params. 
				// 4) <blank>                             skip this line
				// 5) # anything                          comment, ignored
				// 6) Groups i,f                          start of image groups, i=image sec, f=fixpt sec
				// 7) i0,i1,i2[...]                       list of image indices and order for a single group, only valid after Groups line

				// skip comment line
				if (line[0] == '#')
					continue;

				// skip empty lines
				boost::algorithm::trim(line);
				if (line.size() == 0)
					continue;

				if (doingImages)
				{
					// If "Groups" found on line, break out and read groups.
					if (line.find("Groups") == 0)
					{
						doingImages = false;
						haveGroups = true;
					}
					else
					{
						// tokenize/parse this line. It should be one of the filename variants.
						string sUseThisFilename;
						vector<string> tokens;
						tokenize(line, tokens, ",");
						if (tokens.size() == 0)
							continue;

						// verify file exists
						boost::filesystem::path image(tokens[0]);
						if (image.is_relative())
						{
							image = folder / image;
						}

						if (!exists(image))
						{
							std::cerr << "image file not found,line " << linenumber << " : " << image << endl;
							bReturn = false;
							break;
						}


						double x, y, dur, del;
						switch (tokens.size())
						{
						case 1:
							vecInfo.push_back(std::make_tuple(image.string() , xDefault, yDefault, durationDefault, delayDefault));
							break;
						case 3:
							if (parse_double(tokens[1], x) || parse_double(tokens[2], y))
							{
								std::cerr << "Error at line " << linenumber << ": cannot parse x,y" << endl;
								bReturn = false;
							}
							else
							{
								vecInfo.push_back(std::make_tuple(image.string() , x, y, durationDefault, delayDefault));
							}
							break;
						case 5:
							if (parse_double(tokens[1], x) || parse_double(tokens[2], y) || parse_double(tokens[3], del) || parse_double(tokens[4], dur))
							{
								std::cerr << "Error at line " << linenumber << ": cannot parse x,y,del,dur" << endl;
								bReturn = false;
							}
							else
							{
								vecInfo.push_back(std::make_tuple(image.string() , x, y, del, dur));
							}
							break;
						default:
							std::cerr << "Error - too many tokens (" << tokens.size() << ") at line " << linenumber << endl;
							bReturn = false;
							break;
						}
					}
				}
				else // This is the case doingImages == false
				{
					// Expecting a group list, which is a comma-separated list of integers. 
					// Each integer is the image index, referring to the list already loaded.
					// 0,1,2
					// 2,3,4
					std::vector<int> imageGroup;
					if (parse_int_list(line, imageGroup))
					{
						std::cerr << "Error - cannot parse Group line - expecting e.g. 1,2,3" << endl;
						bReturn = false;
					}
					else
					{
						groupsVec.push_back(imageGroup);
					}
				}

				// if  there was a bad line, exit loop.
				if (!bReturn)
					break;
			}

			myfile.close();
			if (bReturn)
				std::cerr << "Loaded " << vecInfo.size() << " image files." << endl;
		}
	}
	return bReturn;
}


FXImageStimSet *createImageStimSet(const std::string& filename, double x, double y, double duration, double delay, int nlevels)
{
	FXImageStimSet *pStimSet = nullptr;
	std::vector<FXImageInfo> vecInfo;
	FXGroupsVec groupsVec;
	if (parseImageInputFile(vecInfo, groupsVec, filename, x, y, duration, delay))
	{
		pStimSet = new FXImageStimSet(vecInfo, groupsVec, nlevels);
	}
	return pStimSet;
}

FXImageStimSet *createImageStimSet(alert::ARContrastFixationPointSpec& fixpt, const std::string& filename, double x, double y, double duration, double delay, int nlevels)
{
	FXImageStimSet *pStimSet = nullptr;
	std::vector<FXImageInfo> vecInfo;
	FXGroupsVec groupsVec;
	if (parseImageInputFile(vecInfo, groupsVec, filename, x, y, duration, delay))
	{
		pStimSet = new FXImageStimSet(fixpt, vecInfo, groupsVec, nlevels);
	}
	return pStimSet;
}


FXImageStimSet::FXImageStimSet()
	: FXStimSet()
{
}


FXImageStimSet::~FXImageStimSet()
{
}


FXImageStimSet::FXImageStimSet(alert::ARContrastFixationPointSpec& fixpt, const std::vector<FXImageInfo>& vecInfo, const FXGroupsVec& groupsVec, int nlevels)
: FXStimSet(fixpt)
, m_nlevels(nlevels)
, m_imagesInfo(vecInfo)
, m_groupsVec(groupsVec)
, m_current(0)
{
	// groups depends on whether there are any
	m_bUseGroups = m_groupsVec.size() > 0;
}

FXImageStimSet::FXImageStimSet(const std::vector<FXImageInfo>& vecInfo, const FXGroupsVec& vecGroups, int nlevels)
: FXStimSet()
, m_nlevels(nlevels)
, m_imagesInfo(vecInfo)
, m_groupsVec(vecGroups)
, m_current(0)
{
	// groups depends on whether there are any
	m_bUseGroups = m_groupsVec.size() > 0;
}


std::string FXImageStimSet::toString() const
{
	std::ostringstream oss;
	oss << "FXImageStimSet:" << endl;
	if (has_fixpt())
		oss << " Fixpt: " << fixpt() << endl;
	else
		oss << " Fixpt: NONE" << endl;
	oss << " Images: " << m_imagesInfo.size() << endl;
	//for (auto infoTup : m_imagesInfo)
	//{
	//	oss << "   " <<
	//		std::get<0>(infoTup) << "," <<
	//		std::get<1>(infoTup) << "," <<
	//		std::get<2>(infoTup) << "," <<
	//		std::get<3>(infoTup) << "," <<
	//		std::get<4>(infoTup) << std::endl;
	//}
	oss << " Groups(" << m_groupsVec.size() << ")" << std::endl;
	//for (auto vec : m_groupsVec)
	//{
	//	for (auto i : vec)
	//	{
	//		oss << i << " ";
	//	}
	//	oss << std::endl;
	//}
	return oss.str();
}

void FXImageStimSet::cleanup(std::vector<int> pages)
{
	// background color was overwritten in palette. Fix here. 
	vsgSetBackgroundColour(&m_backgroundColorSaved.trival());
	vsgSetVideoMode(vsg8BITPALETTEMODE);
}


int FXImageStimSet::init(std::vector<int> pages, int)
{
	int status = 0;
	m_pageBlank = pages[0];
	m_pageFixpt = pages[1];
	m_pageFixptStim = pages[2];

	std::vector<int>::const_iterator first = pages.begin() + 2;
	std::vector<int>::const_iterator last = pages.end();
	m_pageImages = std::vector<int>(first, last);

	// save background color for cleanup
	m_backgroundColorSaved = ARvsg::instance().background_color();

	// Set the VSG to True colour mode
	// should this also set vsgNOGAMMACORRECT?

	// NOTE: THERE IS A VISIBLE FLASH AT THIS CALL. 
	// The video clear command is an attempt to minimize it, but it still exists. 
	// Probably when switching to 24 bit depth, the buffer has (???) - not sure. Cannot fill buffer before 
	// setting video mode, because subsequent commands will know what video mode vsg is in, and will fill as if 
	// buffer is in 8 bit mode. 
	vsgSetVideoMode(vsgTRUECOLOURMODE);
	vsgSetPen2(f_truecolorGray);
	vsgSetCommand(vsgVIDEOCLEAR);


	//Disable all of the draw modes.
	vsgSetDrawMode(vsgCENTREXY);

	status = drawCurrent();

	return status;
}

int FXImageStimSet::handle_trigger(const std::string& s, const std::string& args)
{
	//std::cerr << "FXImageStimSet::handle_trigger: " << s << std::endl;
	int status = 0;
	if (s == "F")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixpt, vsgNOCLEAR);
		status = 1;
	}
	else if (s == "S")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixptStim, vsgNOCLEAR);
		vsgSetSynchronisedCommand(vsgSYNC_PRESENT, vsgCYCLEPAGEENABLE, 0);
		status = 1;
	}
	else if (s == "a")
	{
		m_current++;
		if (m_bUseGroups)
		{
			if (m_current == m_groupsVec.size())
				m_current = 0;
		}
		else
		{
			if (m_current == m_imagesInfo.size())
				m_current = 0;
		}
		drawCurrent();
		status = 0;
	}
	else if (s == "g")
	{
		std::cerr << "Received \"g\" args: " << args << std::endl;
		// parse index to goto
		size_t new_index;
		std::stringstream ss(args);
		ss >> new_index;
		if (!ss)
		{
			std::cerr << "Input error - expecting number>=0, got \"" << args << "\"" << std::endl;
			std::cerr << "TODO - undefined results here - incorrect stimulus displayed!!!!!!" << std::endl;
		}
		else
		{
			if (m_bUseGroups)
			{
				m_current = new_index % m_groupsVec.size();
				std::cerr << "Got index " << new_index << " m_groupsVec.size()=" << m_groupsVec.size() << " m_current= " << m_current << std::endl;
			}
			else
			{
				m_current = new_index % m_imagesInfo.size();
				std::cerr << "Got index " << new_index << " m_imagesInfo.size()=" << m_imagesInfo.size() << " m_current= " << m_current << std::endl;
			}
			drawCurrent();
			status = 0;
		}

	}
	else if (s == "s")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixpt, vsgNOCLEAR);
		if (m_bUseGroups)
		{
			vsgSetCommand(vsgCYCLEPAGEDISABLE + vsgCYCLELUTDISABLE);
		}
		else
		{
			vsgSetCommand(vsgCYCLEPAGEDISABLE);
		}
		status = 2;	// FixUStim::run_stim will wait before updating digout bits
	}
	else if (s == "X")
	{
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, vsgNOCLEAR);
		if (m_bUseGroups)
		{
			vsgSetCommand(vsgCYCLEPAGEDISABLE + vsgCYCLELUTDISABLE);
		}
		else
		{
			vsgSetCommand(vsgCYCLEPAGEDISABLE);
		}
		status = 2;	// FixUStim::run_stim will wait before updating digout bits
	}
	return status;
}


// Leaves current draw page (m_current is the index to use, depends on whether we use groups or not) as the blank page.
int FXImageStimSet::drawCurrent()
{
	static int ihack = 0;
	int status = 0;
	int diStatus;
	char filename[1024];

	// fixpt page
	vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixpt, f_truecolorGray);
	if (has_fixpt())
	{
		fixpt().ARFixationPointSpec::draw();
	}

	// What we do next depends on whether groups are used or not.
	if (!m_bUseGroups)
	{

		// fixpt + stim page
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageFixptStim, f_truecolorGray);

		//string sfilename = std::get<0>(m_imagesInfo[m_current]);
		strncpy_s(filename, 1024, std::get<0>(m_imagesInfo[m_current]).c_str(), sizeof(filename));

		// draw
		diStatus = vsgDrawImage(vsgBMPPICTURE + vsgPALETTELOAD, std::get<1>(m_imagesInfo[m_current]), -1 * std::get<2>(m_imagesInfo[m_current]), filename);

		if (has_fixpt())
		{
			fixpt().ARFixationPointSpec::draw();
		}

		vsgSetDrawPage(vsgVIDEOPAGE, m_pageBlank, f_truecolorGray);

		// Now setup page cycling
		setupCycling(m_imagesInfo[m_current]);
	}
	else
	{
		// set up a page for each image in the group, with image and fixpt. Pages to be displayed (setupCycling) 
		// are m_pageImages[i], i=0,..., one for each member of group
		for (unsigned int iGroupMember = 0; iGroupMember < m_groupsVec[m_current].size(); iGroupMember++)
		{
			vsgSetDrawPage(vsgVIDEOPAGE, m_pageImages[iGroupMember], f_truecolorGray);

			char filename[1024];
			int imageIndex = m_groupsVec[m_current][iGroupMember];
			strncpy_s(filename, 1024, std::get<0>(m_imagesInfo[imageIndex]).c_str(), sizeof(filename));

			// draw image. Palette will be loaded (below) to LUT buffer array for animation
			diStatus = vsgDrawImage(vsgBMPPICTURE + vsgPALETTELOAD, std::get<1>(m_imagesInfo[imageIndex]), -1 * std::get<2>(m_imagesInfo[imageIndex]), filename);

			if (has_fixpt())
			{
				fixpt().ARFixationPointSpec::draw();
			}
		}
		setupCyclingForCurrentGroup();
	}
	return status;
}

bool FXImageStimSet::loadPaletteFromImage(VSGLUTBUFFER& palImage, char *filename)
{
	int ipal;

	// load palette
	for (int i = 0; i < 256; i++) palImage[i].a = palImage[i].b = palImage[i].c = 0;
	ipal = vsgImageGetPalette(0, filename, &palImage);
	if (ipal)
	{
		switch (ipal)
		{
		case vsgerrorERRORREADINGFILE: std::cerr << "get palette vsgerrorERRORREADINGFILE" << endl; break;
		case vsgerrorUNSUPPORTEDIMAGETYPE: std::cerr << "get palette vsgerrorUNSUPPORTEDIMAGETYPE" << endl; break;
		case vsgerrorUNSUPPORTEDBITMAPFORMAT: std::cerr << "get palette vsgerrorUNSUPPORTEDBITMAPFORMAT" << endl; break;
		case vsgerrorOUTOFPCMEMORY: std::cerr << "get palette vsgerrorOUTOFPCMEMORY" << endl; break;
		case vsgerrorIMAGEHASNOPALETTE: std::cerr << "get palette vsgerrorIMAGEHASNOPALETTE" << endl; break;
		default: std::cerr << "get palette error: " << ipal << " filename " << filename << " len " << strlen(filename) << endl; break;
		}
		return false;
	}
	return true;
}


void FXImageStimSet::setupCycling(const FXImageInfo& info)
{
	VSGCYCLEPAGEENTRY cycle[12];	// warning! No check on usage. You have been warned. 
	int status = 0;
	int count = 0;

	memset(cycle, 0, sizeof(cycle));

	int framesStim = (int)( std::get<3>(info) / vsgGetSystemAttribute(vsgFRAMETIME) * 1000000.0 );
	int framesDelay = (int)(std::get<4>(info) / vsgGetSystemAttribute(vsgFRAMETIME) * 1000000.0);
	std::cerr << "FXImageStimSet::setupCycling: frames delay, stim = " << framesDelay << " " << framesStim << endl;

	if (framesDelay > 0)
	{
		cycle[count].Frames = framesDelay;
		cycle[count].Page = m_pageFixpt + vsgTRIGGERPAGE;
		cycle[count].Stop = 0;
		count++;
	}
	if (framesStim > 0)
	{
		cycle[count].Frames = framesStim;
		cycle[count].Page = m_pageFixptStim + vsgTRIGGERPAGE;
		cycle[count].Stop = 0;
		count++;
		cycle[count].Frames = 0;
		cycle[count].Page = m_pageFixpt + vsgTRIGGERPAGE;
		cycle[count].Stop = 1;
		count++;
	}
	else
	{
		cycle[count].Frames = 1;
		cycle[count].Page = m_pageFixptStim + vsgTRIGGERPAGE;
		cycle[count].Stop = 1;
		count++;
	}
	status = vsgPageCyclingSetup(count, &cycle[0]);
}



void FXImageStimSet::setupCyclingForCurrentGroup()
{
	VSGCYCLEPAGEENTRY cycle[12];	// warning! No check on usage. You have been warned. 
	int status = 0;
	int count = 0;

	memset(cycle, 0, sizeof(cycle));

	for (int i=0; i<m_groupsVec[m_current].size(); i++)
	{
		FXImageInfo info = m_imagesInfo[m_groupsVec[m_current][i]];
		int framesStim = (int)(std::get<3>(info) / vsgGetSystemAttribute(vsgFRAMETIME) * 1000000.0);
		int framesDelay = (int)(std::get<4>(info) / vsgGetSystemAttribute(vsgFRAMETIME) * 1000000.0);
		if (framesStim > 0)
		{
			cycle[count].Frames = framesStim;
			cycle[count].Page = m_pageImages[i]+vsgTRIGGERPAGE;
			cycle[count].Stop = 0;
			count++;
		}
		if (framesDelay > 0 && i < m_groupsVec[m_current].size()-1)		// don't draw fixpt period after last image
		{
			cycle[count].Frames = framesDelay;
			cycle[count].Page = m_pageFixpt + vsgTRIGGERPAGE;
			cycle[count].Stop = 0;
			count++;
		}
	}
	cycle[count].Frames = 0;
	cycle[count].Page = m_pageFixpt + vsgTRIGGERPAGE;
	cycle[count].Stop = 1;
	count++;
	status = vsgPageCyclingSetup(count, &cycle[0]);
}

