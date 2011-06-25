Alertlib usage
--------------

Apps with gratings should use the following sequence of calls:

// Init vsg
if (ARvsg::instance().init(m_screenDistanceMM, m_background))
{
	cerr << "VSG init failed!" << endl;
	return 1;
}

// Init video pages - clears all pages to vsgBACKGROUND (m_background)
if (ARvsg::instance().init_video())
{
	cerr << "VSG video initialization failed!" << endl;
	return 1;
}


// Set display page to a page you are not going to draw on
vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);

// We will draw on page 0, so we will not see drawing artifacts. 
// The draw page is not displayed until vsgPresent() is called. 
vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgNOCLEAR);

// draw grating. Note that with the above setup, gratings should be drawn
// with draw() or draw((long)vsgTRANSONLOWER) or draw(false)
grating->init(islice);
grating->draw();
grating->setContrast(0);


// Make current draw page the display page and display grating. 
// In the code above contrast was set to 0, so grating will not be visible. 
vsgPresent();







Apps without gratings (all drawings in pixel coords)
-------------------------------------------------------



// Init vsg - if screen distance is <= 0 you get pixel mode. 
if (ARvsg::instance().init(-1, f_background))
{
	cerr << "VSG init failed!" << endl;
	return 1;
}
