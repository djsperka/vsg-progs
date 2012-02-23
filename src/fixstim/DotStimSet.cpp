#include "StimSet.h"
#include <windows.h>
#include <process.h>
#include <iostream>
#define _USE_MATH_DEFINES
#include <cmath>

#include "AlertUtil.h"
using namespace std;


DotStimSet::DotStimSet(alert::ARContrastFixationPointSpec& f, double x, double y, COLOR_TYPE color, double diameter, double speed, double density, int dotsize, vector<double>& angles) : m_fixpt(f), m_bHaveFixpt(true), m_x(x), m_y(y), m_color(color), m_diameter(diameter), m_speed(speed), m_density(density), m_dotsize(dotsize), m_angles(angles), m_nDots(0), m_dots(NULL)
{
	// initialize angle iterator
	m_iterator = m_angles.begin();

	// create event object
	m_event = CreateEvent(	NULL, 
							FALSE,	// auto-reset; resets to nonsignaled after waiting thread is released
							FALSE,	// non-signaled initial state
							NULL);
}


DotStimSet::DotStimSet(double x, double y, COLOR_TYPE color, double speed, double diameter, double density, int dotsize, vector<double>& angles): m_bHaveFixpt(false), m_x(x), m_y(y), m_color(color), m_diameter(diameter), m_speed(speed), m_density(density), m_dotsize(dotsize), m_angles(angles), m_nDots(0), m_dots(NULL)
{
	// initialize angle iterator
	m_iterator = m_angles.begin();

	// create event object
	m_event = CreateEvent(	NULL, 
							FALSE,	// auto-reset; resets to nonsignaled after waiting thread is released
							FALSE,	// non-signaled initial state
							NULL);
}

DotStimSet::~DotStimSet()
{

}
string DotStimSet::toString() const
{
	std::ostringstream oss;
	oss << "DotStimSet" << endl;
	if (m_bHaveFixpt)
	{
		oss << "  fixation point: " << m_fixpt << endl;
	}
	else
	{
		oss << "  fixation point: NONE" << endl;
	}
	oss <<     "  x,y " << m_x << ", " << m_y << endl;
	oss <<     "  diameter " << m_diameter << endl;
	oss <<     "  density " << m_density << endl;
	oss <<     "  dotsize " << m_dotsize << endl;
	oss << endl;
	return oss.str();
}


int DotStimSet::init(ARvsg& vsg, std::vector<int> pages)
{
	int status = 0;
	double pixPerDegree;
	long screenWidthPixels, screenHeightPixels;
	long usecPerFrame;
	m_pageBackground = pages[0];
	m_pages[0] = pages[1];
	m_pages[1] = pages[2];
	m_ipage = 0;

	// VSG should be initialized, so we can convert the input settings to practical values. 
	vsgUnitToUnit(vsgDEGREEUNIT, 1.0, vsgPIXELUNIT, &pixPerDegree);
	screenWidthPixels = vsgGetScreenWidthPixels();
	screenHeightPixels = vsgGetScreenHeightPixels();
	usecPerFrame = vsgGetSystemAttribute(vsgFRAMETIME);
	//m_xPixel = m_x*pixPerDegree - screenWidthPixels/2;
	//m_yPixel = screenHeightPixels/2 - m_y * pixPerDegree;
	m_xPixel = m_x * pixPerDegree;
	m_yPixel = -m_y * pixPerDegree;
	m_diameterPixel = m_diameter * pixPerDegree;
	m_speedPixelsPerFrame = m_speed * pixPerDegree * usecPerFrame / 1000000.0;

	// Allocate array for dots. Dots will be generated in a square box, then masked & translated on each update.
	// This is different allocation than that done in vsgDOTInitilise call. 
	m_nDots = (int)(m_density * m_diameter * m_diameter);
	m_dots = new double[m_nDots*2];
	generate_dots();

	cout << "Dot patch center (degrees/pixels) = " << m_x << ", " << m_y << "/" << m_xPixel << ", " << m_yPixel << endl;
	cout << "Dot patch diameter degrees/pixels) = " << m_diameter << "/" << m_diameterPixel << endl;
	cout << "Speed deg/sec : pixels/frame = " << m_speed << " : " << m_speedPixelsPerFrame << endl;
	cout << "density (dots/deg**2), ndots = " << m_density << ", " << m_nDots << endl;


	// Initialize dot system
	vsgDOTInitialise(1000, 1, vsgDOTAUTORESERVE);


	// If necessary, init fixpt
	if (m_bHaveFixpt)
	{
		m_fixpt.init(2);
	}

	// Get a pixel level for dots
	// TODO need to assign color here!
	vsg.request_single(m_dotLevel);
	arutil_color_to_palette(m_color, m_dotLevel);

	// Start thread. The thread will block on the event above - which indicates "start dot updates". 
	// A bool var will tell the thread to stop and resume waiting on the event.
	m_thread = (HANDLE)_beginthread(DotStimSet::threadfunc, 0, this);


	return 0;
}

void DotStimSet::threadfunc(void *pobj)
{
	DotStimSet* pdss = (DotStimSet *)pobj;
	while (true)
		pdss->do_dots();
	return;
}

void DotStimSet::do_dots()
{
	int i;
	double c, s;
	double rr;

	// before we go anywhere we have to wait for the event signal
	WaitForSingleObject(m_event, INFINITE);

	// Now we're supposed to start the dots
	cout << "Got event, starting dot motion" << endl;

	c = cos(*m_iterator * M_PI/180.0);
	s = sin(*m_iterator * M_PI/180.0);
	rr = m_diameterPixel*m_diameterPixel/4.0;
	
	while (m_bDotsOn)
	{
		m_ipage = 1 - m_ipage;
		vsgSetDrawPage(vsgVIDEOPAGE, m_pages[m_ipage], vsgBACKGROUND);
		shift_dots();
		vsgDOTInitLoadGroup(vsgDOTXY, m_dotsize, m_dotsize, m_dotLevel);
		for (i=0; i<m_nDots; i++)
		{
			// TODO: mask here and rotate
			if (m_dots[i*2]*m_dots[i*2] + m_dots[i*2+1]*m_dots[i*2+1] < rr)
				vsgDOTGenerateNormalDot(m_dots[i*2] * c + m_dots[i*2+1] * s + m_xPixel, 
										-m_dots[i*2] * s + m_dots[i*2+1] * c + m_yPixel, 0);
		}
		vsgDOTLoadGroup();
		//vsgDrawDots(vsgDOTDRAWNORMAL + vsgTRIGGERPAGE);
		vsgDrawDots(vsgDOTDRAWNORMAL);

		if (m_bFixptOn)
		{
			m_fixpt.draw();
		}
	}
}

void DotStimSet::shift_dots()
{
	int i;
	long screenWidthPixels, screenHeightPixels;
	screenWidthPixels = vsgGetScreenWidthPixels();
	screenHeightPixels = vsgGetScreenHeightPixels();

	// shift all dots , wrap if out of the box
	for (i=0; i<m_nDots; i++)
	{
		m_dots[i*2] += m_speedPixelsPerFrame;
		if (fabs(m_dots[i*2]) > m_diameterPixel/2.0)
		{
			m_dots[i*2] -= m_diameterPixel;
		}
	}
}



void DotStimSet::generate_dots()
{
	int i;
	for (i=0; i<m_nDots; i++)
	{
		m_dots[i*2] = ((double)rand()/RAND_MAX -0.5) * m_diameterPixel;
		m_dots[i*2+1] = ((double)rand()/RAND_MAX -0.5) * m_diameterPixel;
	}
	return;
}


int DotStimSet::handle_trigger(std::string& s)
{
	int status = 0;
	if (s == "F")
	{
		m_bFixptOn = true;
		vsgSetDrawPage(vsgVIDEOPAGE, m_pages[m_ipage], vsgBACKGROUND);
		update_page();
		status = 1;
	}
	else if (s == "S")
	{
		m_bDotsOn = true;
		SetEvent(m_event);
		status = 1;
	}
	else if (s == "s")
	{
		m_bDotsOn = false;
		WaitForSingleObject(m_thread, 0);
		update_page();
		status = 1;
	}
	else if (s == "a")
	{
		m_iterator++;
		if (m_iterator == m_angles.end()) 
			m_iterator = m_angles.begin();
		status = 1;
	}
	else if (s == "X")
	{
		m_bFixptOn = false;
		m_bDotsOn = false;
		vsgSetDrawPage(vsgVIDEOPAGE, m_pageBackground, vsgBACKGROUND);
		status = 1;
	}
	return status;
}


void DotStimSet::update_page()
{
	m_ipage = 1 - m_ipage;
	vsgSetDrawPage(vsgVIDEOPAGE, m_pages[m_ipage], vsgBACKGROUND);
	if (m_bFixptOn)
	{
		m_fixpt.draw();
	}
	return;
}