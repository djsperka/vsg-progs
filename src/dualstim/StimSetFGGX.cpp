#include "StimSetFGGX.h"
#include <iostream>
#include <cstdlib>
//#include <windows.h>
//#include <process.h>
#include "AlertUtil.h"
#include "stopwatch.h"
using namespace std;

static const int f_nlevels = 20;

#undef USE_GRIDS
#undef USE_SCRATCH 

StimSetFGGX::StimSetFGGX(shared_ptr<SSInfo> pssinfo, double xOffset, double yOffset) 
: StimSetMultipleGrating()
, m_itrial(0)
, m_ngridpages(7)
, m_firstgridpage(5)
, m_pssinfo(pssinfo)
{
}


void StimSetFGGX::advance()
{
	m_itrial++;
}



int StimSetFGGX::setup_cycling()
{
	VSGCYCLEPAGEENTRY cycle[16];
	int status = 0;
	double factor;
	double t1, t2, t3;
	int iIgnore1, iIgnore2;
	double tAnswerPoints;
	int nFramesPerGrid;
	vector<int> pages;
	int nGrids = 8;

#if USE_GRIDS
	if (nGrids > 8) 
	{
		cerr << "nHC grids capped at 8!" << endl;
		nGrids = 8;
	}
	getRandomList(pages, m_ngridpages, nGrids);
	cerr << "Grid pages: ";
	for (int i=0; i<nGrids; i++)
	{
		cerr << pages[i] << ", ";
	}
	cerr << endl;
#endif

	m_pssinfo->getT1(m_itrial, t1);
	t2 = m_pssinfo->getT2();
	t3 = m_pssinfo->getT3();

	m_pssinfo->getAnswerPointParameters(iIgnore1, iIgnore2, tAnswerPoints);

	// factor (frames/sec) will convert those times to frames
	factor = 1000000.0f / vsgGetSystemAttribute(vsgFRAMETIME);
#if USE_GRIDS
	nFramesPerGrid = (int)(factor * m_pssinfo->getTHC()/nGrids);
#endif

	memset(cycle, 0, sizeof(cycle));
	cycle[0].Frames = (WORD)(t1 * factor);
	cycle[0].Page = 2 + vsgTRIGGERPAGE;
	cycle[0].Stop = 0;

	cycle[1].Frames = (WORD)(t2 * factor);
	cycle[1].Page = 3 + vsgTRIGGERPAGE;
	cycle[1].Stop = 0;

	cycle[2].Frames = (WORD)(t3 * factor);
	cycle[2].Page = 4 + vsgTRIGGERPAGE;
	cycle[2].Stop = 0;

#if USE_GRIDS
	for (int i = 0; i < nGrids; i++)
	{
		cycle[3+i].Frames = nFramesPerGrid;
		cycle[3+i].Page = m_firstgridpage + pages[i] + vsgTRIGGERPAGE;
		cycle[3+i].Stop = 0;
	}

	cycle[3+nGrids].Frames = (WORD)(tAnswerPoints * factor);
	cycle[3+nGrids].Page = m_firstgridpage+m_ngridpages + vsgTRIGGERPAGE;
	cycle[3+nGrids].Stop = 0;

	cycle[3+nGrids+1].Frames = 1;
	cycle[3+nGrids+1].Page = 0 + vsgTRIGGERPAGE;
	cycle[3+nGrids+1].Stop = 1;
	status = vsgPageCyclingSetup(5+nGrids, &cycle[0]);
#else
	cycle[3].Frames = (WORD)(factor * m_pssinfo->getTHC());
	cycle[3].Page = 5 + vsgTRIGGERPAGE;
	cycle[3].Stop = 0;

	cycle[4].Frames = (WORD)(tAnswerPoints * factor);
	cycle[4].Page = 6 + vsgTRIGGERPAGE;
	cycle[4].Stop = 0;

	cycle[5].Frames = 1;
	cycle[5].Page = 0 + vsgTRIGGERPAGE;
	cycle[5].Stop = 1;
	status = vsgPageCyclingSetup(6, &cycle[0]);
#endif

	return status;
}


// gets a random list of 'num' ints between 0 and N-1. No repeats. num <= N. 
void StimSetFGGX::getRandomList(vector<int>& result, int N, int num)
{
	vector<int> s;
	int m;
	for (int i=0; i<N; i++) s.push_back(i);
	//cout << "getRandomList: N = " << N << " num = " << num << endl;
	for (int i=0; i<num; i++)
	{
		m = rand() % (num-i);
		//cout << "i = " << i << " got m " << m << endl;
		result.push_back(s.at(m));
		s.erase(s.begin()+m);
		//cout << "i = " << i << " m = " << m << " remain ";
		//for (vector<int>::const_iterator it = s.begin(); it!=s.end(); it++) cout << *it << ",";
		//cout << endl;
	}
	//cout << "grid pages selected "; 
	//for (vector<int>::const_iterator it = result.begin(); it!=result.end(); it++) cout << *it << ",";
	return;
}


double StimSetFGGX::getGridDiam()
{
	unsigned int i;
	double dmax = -9999999;
	double d;
	for (i=0; i<m_pssinfo->getSize(); i++)
	{
		m_pssinfo->getDiam(i, d);
		if (d > dmax) dmax = d;
	}
	return dmax;
}


void StimSetFGGX::draw_scratch_pages()
{
#if USE_SCRATCH
	vsgSetDrawPage(vsgSCRATCHPAGE, 1, vsgBACKGROUND);
	if (has_xhair())
	{
		xhair().draw();
	}
	if (has_fixpt())
	{
		fixpt().draw();
	}
	//vsgPresent();
#endif
	return;
}

void StimSetFGGX::copy_scratch_page(int dest)
{
#if USE_SCRATCH
	long w;
	long h; 
	double ww, wpix, hpix;
	// get width and height of xhair and only copy that. 
	if (has_xhair())
	{
		ww = 2*max(max(xhair().r1, xhair().r2), xhair().ro);
		vsgUnitToUnit(vsgDEGREEUNIT, ww, vsgPIXELUNIT, &wpix);
		hpix = wpix;
		//ww = wwmax = 300;
		//vsgDrawMoveRect(vsgSCRATCHPAGE, 0, 0, 0, 2*ww, 2*ww, 0, 0, 2*ww, 2*ww);
		//vsgUnitToUnit(vsgPIXELUNIT, vsgGetScreenWidthPixels(), vsgDEGREEUNIT, &wwmax);
		//cout << "ww is " << ww << " max w " << wwmax << endl;
		//vsgSetSpatialUnits(vsgPIXELUNIT);
		//vsgDrawMoveRect(vsgSCRATCHPAGE, 0, 0, 0, 400, 600, 0, 0, 400, 600);
		//vsgSetSpatialUnits(vsgDEGREEUNIT);	}
	}
	else
	{
		w = vsgGetScreenWidthPixels();
		h = vsgGetScreenHeightPixels();
		wpix = w;
		hpix = h;
	}

	vsgSetDrawPage(vsgVIDEOPAGE, dest, vsgBACKGROUND);
	vsgSetSpatialUnits(vsgPIXELUNIT);
	if (vsgDrawMoveRect(vsgSCRATCHPAGE, 1, 0, 0, wpix, hpix, 0, 0, wpix, hpix))
		cout << "drawmoverect FAILED." << endl;
	vsgSetSpatialUnits(vsgDEGREEUNIT);

	cout << "copy_scratch_page(" << dest << ") wpix, hpix " << wpix << ", " << hpix << endl;

#else
	//cout << "WARN: no SCRATCH pages used, drawing fixpt, xhair...." << endl;
	vsgSetDrawPage(vsgVIDEOPAGE, dest, vsgBACKGROUND);
	if (has_xhair()) xhair().draw();
	if (has_fixpt()) fixpt().draw();
#endif
}




