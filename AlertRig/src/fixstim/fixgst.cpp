// fixgst.cpp
// gstreamer interface functions for use in fixstim

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <stdint.h>
#include <iostream>
#include <chrono>
using namespace std;

#include "VSGV8.h"
#include "Alertlib.h"

const int f_WG = 320;
const int f_HG = 240;
uint8_t* f_prgb = NULL;		// entire rgb image
uint8_t* f_pline = NULL;	// line of pixel values
VSGLUTBUFFER f_lut;

int counter = 0;
int want = 1;
//uint16_t b_white[385 * 288];
//uint16_t b_black[385 * 288];
bool _bQuit = false;
bool gotImage = false;

void vsgModes()
{
	int drawmode = vsgGetDrawMode();
	int units = vsgGetSystemAttribute(vsgSPATIALUNITS);
	string s;
	if (drawmode & vsgCENTREXY) s += "CENTERXY ";
	if (units & vsgPIXELUNIT) s += "PIXELS ";
	if (units & vsgDEGREEUNIT) s += "DEGREES ";
	cout << s << endl;
}


static void prepare_buffer(GstAppSrc* appsrc) 
{
	static GstClockTime timestamp = 0;
	GstBuffer *buffer;
	guint size;
	GstFlowReturn ret;
	int irow, icol;
	int istart, istep;
	int W = vsgGetScreenWidthPixels();
	int H = vsgGetScreenHeightPixels();
	int ind;

	if (!want) return;
	want = 0;
	//vsgModes();


#if 1

	VSGPAGEDESCRIPTOR descr;
	descr._StructSize = sizeof(VSGPAGEDESCRIPTOR);
	vsgGetCurrentDrawPage(&descr);
	int iDisp = vsgGetZoneDisplayPage(vsgVIDEOPAGE);
	bool bTemp = false;
	//if (descr.Page != iDisp)
	//{
	//	string s;
	//	cout << "counter " << counter << " draw(zone)/disp " << descr.Page << "(" << descr.PageZone << ")/" << iDisp << endl;
	//}
	//cout << "counter " << counter << " draw(zone)/disp " << descr.Page << "(" << descr.PageZone << ")/" << iDisp << " LUTstate:" << vsgGetSystemAttribute(vsgLUTCYCLINGSTATE) << endl;

#if 0
	// Constants used by SetDrawPage, SetDisplayPage
#define vsgVIDEOPAGE   256   //  Start page of VIDEO memory   
#define vsgSCRATCHPAGE 512   //  Scratchpad pages in DRAM memory    
#define vsgOVERLAYPAGE 768   //  Start page of OVERLAY memory 
#define vsgHOSTPAGE    4096  //  host memory based pages 
#endif

	vsgPaletteRead(&f_lut);
	if (counter % 50 == 0)
	{
		for (int i = 0; i < 10; i++)
		{
			cout << i << ": " << (unsigned int)(256 * f_lut[i].a) << "/" << (unsigned int)(256 * f_lut[i].b) << "/" << (unsigned int)(256 * f_lut[i].c) << endl;
		}
		cout << "40: " << (unsigned int)(256 * f_lut[40].a) << "/" << (unsigned int)(256 * f_lut[40].b) << "/" << (unsigned int)(256 * f_lut[40].c) << endl;
		cout << "41: " << (unsigned int)(256 * f_lut[41].a) << "/" << (unsigned int)(256 * f_lut[41].b) << "/" << (unsigned int)(256 * f_lut[41].c) << endl;
	}

	auto start_time = std::chrono::high_resolution_clock::now();

	// set up stepping/stride
	if (counter == 0)
	{
		istart = 0; 
		istep = 1;
	}
	else
	{
		istart = counter % 5;
		istep = 5;
	}

	double halfwidthDeg;
	double halfheightDeg;
	double pixelLineDeg;
	vsgUnitToUnit(vsgPIXELUNIT, W / 2, vsgDEGREEUNIT, &halfwidthDeg);
	vsgUnitToUnit(vsgPIXELUNIT, H / 2, vsgDEGREEUNIT, &halfheightDeg);



	for (irow = istart; irow < H; irow+=istep)
	{
		int units = vsgGetSystemAttribute(vsgSPATIALUNITS);
		int mode = vsgGetDrawMode();
		int diff = 0;

		//vsgSetDrawMode(0);
		//vsgSetSpatialUnits(vsgPIXELUNIT);

		// assume origin at center, degrees,
		// centerxy has no effect here, specify upper left corner of screen.
		// half-width of screen, as a positive number

		//vsgSetDrawPage(vsgVIDEOPAGE, vsgGetZoneDisplayPage(vsgVIDEOPAGE), vsgNOCLEAR);


#ifdef NOSWITCHUNITS

		vsgUnitToUnit(vsgPIXELUNIT, irow, vsgDEGREEUNIT, &pixelLineDeg);
		vsgReadPixelLine(-halfwidthDeg, -halfheightDeg + pixelLineDeg, f_pline, W);

#else

		int dBefore = vsgGetViewDistMM();

		vsgSetSpatialUnits(vsgPIXELUNIT);
		int status = vsgReadPixelLine(-W / 2, -H / 2 + irow, f_pline, W);
		vsgSetSpatialUnits(vsgDEGREEUNIT);
		int dAfter = vsgGetViewDistMM();
		if (dBefore != dAfter) cout << "VIEW DIST " << dBefore << "/" << dAfter << endl;
#endif

		//if (irow == 0)
		//	cout << "row 0: " << (unsigned int)f_pline[0] << "," << (unsigned int)f_pline[1] << endl;

		//vsgReadPixelLine(0, irow*dx, f_pline, dy);
		//vsgReadPixelLine(-dx, -dy, f_pline, W);
		//int status = vsgReadPixelLine(0, -dy, f_pline, W);
		//if (status) cout << "vsgReadPixel err " << status << endl;
		//vsgSetDrawMode(mode);
		//vsgSetSpatialUnits(units);
		//vsgSetSpatialUnits(vsgDEGREEUNIT);
		for (icol = 0; icol < W; icol++)
		{
			ind = (irow * W + icol) * 3;
			f_prgb[ind]   = (uint8_t)(256*f_lut[f_pline[icol]].a);
			f_prgb[ind+1] = (uint8_t)(256*f_lut[f_pline[icol]].b);
			f_prgb[ind+2] = (uint8_t)(256*f_lut[f_pline[icol]].c);
			if (f_pline[icol] != 253) diff++;
		}

		//if (diff > 0) cout << irow << "/" << diff << " " << endl;
	}
	//cout << endl;
	counter++;
	auto end_time = std::chrono::high_resolution_clock::now();

#if 0
	if (counter % 30 == 0)
	{
		for (int ic = 505; ic < 515; ic++)
		{
			irow = 384;
			ind = (irow * 383 + ic) * 3;
			cout << "row 384 mid: " << (unsigned int)f_prgb[ic] << "," << (unsigned int)f_prgb[ind + 1] << "," << (unsigned int)f_prgb[ind + 2] << "  " << endl;
		}
		cout << endl;
	}

	// check draw page and current display
	VSGPAGEDESCRIPTOR descr;
	descr._StructSize = sizeof(VSGPAGEDESCRIPTOR);
	vsgGetCurrentDrawPage(&descr);
	int iDisp = vsgGetZoneDisplayPage(vsgVIDEOPAGE);
	bool bTemp = false;
	if (descr.Page != iDisp)
	{
		if (!bTemp) cout << "diff page " << descr.Page << " " << iDisp << endl;
		bTemp = true;
}



#endif

	//cout << "ms " << (end_time - start_time) / std::chrono::milliseconds(1) << endl;
#else
	auto start_time = std::chrono::high_resolution_clock::now();
	int ind;
	for (irow = 0; irow < H; irow++)
	{
		//vsgReadPixelLine(0, irow, f_pline, W);
		for (icol = 0; icol < W; icol++)
		{
			ind = (irow * W + icol) * 3;
			f_prgb[ind] = (uint8_t)(counter%256);
			f_prgb[ind + 1] = (uint8_t)((counter+100)%256);
			f_prgb[ind + 2] = (uint8_t)((counter+200)%256);
		}
	}
	counter++;
	auto end_time = std::chrono::high_resolution_clock::now();
	cout << "ms " << (end_time - start_time) / std::chrono::milliseconds(1) << endl;



#endif
	size = W * H * 3;

	buffer = gst_buffer_new_wrapped_full(GST_MEMORY_FLAG_READONLY, (gpointer)f_prgb, size, 0, size, NULL, NULL);

	GST_BUFFER_PTS(buffer) = timestamp;
	GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, 4);

	timestamp += GST_BUFFER_DURATION(buffer);

	ret = gst_app_src_push_buffer(appsrc, buffer);

	if (ret != GST_FLOW_OK) 
	{
		cout << "gst_app_src_push_buffer err" << endl;
		/* something wrong, stop pushing */
		// g_main_loop_quit (loop);
	}

//
//#if 0
//	// get current video page, save image
////	if (!gotImage)
//	if (counter < 50)
//	{
//		int savemode = vsgGetDrawMode();
//		int page = vsgGetZoneDisplayPage(vsgVIDEOPAGE);
//		char filename[256];
//
//#endif
//#if 0
//		auto start_time = std::chrono::high_resolution_clock::now();
//		//vsgSetDrawMode(vsgCENTREXY);
//		vsgSetDrawMode(0);
//		vsgSetSpatialUnits(vsgPIXELUNIT);
//		//sprintf(filename, "h:\\image%03d.bmp", counter);
//		sprintf(filename, "c:\\work\\img\\image%03d.bmp", counter);
//		int status = vsgImageExport(vsgBMPPICTURE, 0, 0, vsgGetScreenWidthPixels(), vsgGetScreenHeightPixels(), filename);
//		//int status = vsgImageExport(vsgBMPPICTURE, -vsgGetScreenWidthPixels()/2, -vsgGetScreenHeightPixels()/2, vsgGetScreenWidthPixels(), vsgGetScreenHeightPixels(), "image.bmp");
//		if (!status) gotImage = true;
//		counter++;
//
//		auto end_time = std::chrono::high_resolution_clock::now();
//
//		cout << "count " << counter << " page " << page << " status " << status << " ms " << (end_time - start_time) / std::chrono::milliseconds(1) << endl;
//
//
//		//vsgSetSpatialUnits(vsgPIXELUNIT);
//		//vsgImageExport(vsgBMPPICTURE, -400, -300, 800, 600, "c:\\Documents and Settings\\Lab\\Desktop\\check.bmp");
//
//
//
//
//		vsgSetDrawMode(savemode);
//#else
//		auto start_time = std::chrono::high_resolution_clock::now();
//		int i;
//		char pixels[1024];
//		for (i = 0; i < vsgGetScreenHeightPixels(); i++)
//		{
//			vsgReadPixelLine(0, i, pixels, vsgGetScreenWidthPixels());
//		}
//		auto end_time = std::chrono::high_resolution_clock::now();
//		cout << "ms " << (end_time - start_time) / std::chrono::milliseconds(1) << endl;
//#endif
//	}
//
//
//	size = vsgGetScreenWidthPixels() * vsgGetScreenHeightPixels() * 3;
//
//	buffer = gst_buffer_new_wrapped_full(GST_MEMORY_FLAG_READONLY, (gpointer)f_prgb, size, 0, size, NULL, NULL);
//
//	GST_BUFFER_PTS(buffer) = timestamp;
//	GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, 1);
//
//	timestamp += GST_BUFFER_DURATION(buffer);
//
//	ret = gst_app_src_push_buffer(appsrc, buffer);
//
//	if (ret != GST_FLOW_OK) {
//		/* something wrong, stop pushing */
//		// g_main_loop_quit (loop);
//	}
}

static void cb_need_data(GstElement *appsrc, guint unused_size, gpointer user_data) {
	//prepare_buffer((GstAppSrc*)appsrc);
	want = 1;
//	cout << "need data" << endl;
}



int _run_gstreamer(gpointer data)
{
	GstElement *pipeline, *appsrc, *conv, *videosink, *fpssink, *queue;

	cout << "Initialize gstreamer..." << endl;
	gst_init(NULL, NULL);

	/* setup pipeline */
	pipeline = gst_pipeline_new("pipeline");
	appsrc = gst_element_factory_make("appsrc", "source");
	conv = gst_element_factory_make("videoconvert", "conv");
	queue = gst_element_factory_make("queue", "queue");
	videosink = gst_element_factory_make("d3dvideosink", "videosink");
	fpssink = gst_element_factory_make("fpsdisplaysink", "fpssink");

	/* setup */
	g_object_set(G_OBJECT(appsrc), "caps",
		gst_caps_new_simple("video/x-raw",
			"format", G_TYPE_STRING, "RGB",	// djs - use rgb not rgb16
			"width", G_TYPE_INT, f_WG,
			"height", G_TYPE_INT, f_HG,
			"framerate", GST_TYPE_FRACTION, 0, 1,
			NULL), NULL);

#if 0
	// configure fpsdisoplaysink
	g_object_set(G_OBJECT(fpssink),
		"text-overlay", TRUE,
		"video-sink", videosink, NULL);
	// add and link
	gst_bin_add_many(GST_BIN(pipeline), appsrc, conv, queue, fpssink, NULL);
	gst_element_link_many(appsrc, conv, queue, fpssink, NULL);
#else
	// add and link
	gst_bin_add_many(GST_BIN(pipeline), appsrc, conv, queue, videosink, NULL);
	gst_element_link_many(appsrc, conv, queue, videosink, NULL);
#endif
	//  gst_bin_add_many (GST_BIN (pipeline), appsrc, conv, videosink, NULL);
	//  gst_element_link_many (appsrc, conv, videosink, NULL);

	  /* setup appsrc */
	g_object_set(G_OBJECT(appsrc),
		"stream-type", 0, // GST_APP_STREAM_TYPE_STREAM
		"format", GST_FORMAT_TIME,
		"is-live", TRUE,
		NULL);
	g_signal_connect(appsrc, "need-data", G_CALLBACK(cb_need_data), NULL);

	/* play */
	gst_element_set_state(pipeline, GST_STATE_PLAYING);

	while (!_bQuit) 
	{
		prepare_buffer((GstAppSrc*)appsrc);
		g_main_context_iteration(g_main_context_default(), FALSE);
	}

	/* clean up */
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(GST_OBJECT(pipeline));

	delete[] f_prgb;
	delete[] f_pline;
	return 0;
}


int fix_start_gstreamer()
{
	// allocate rgb array to hold image
	f_prgb = new uint8_t[vsgGetScreenWidthPixels() * vsgGetScreenHeightPixels() * 3];
	f_pline = new uint8_t[vsgGetScreenWidthPixels()];

	// launch thread
	GThread *_loop_thread = NULL;
	if ((_loop_thread = g_thread_new("fixstim-gstreamer", (GThreadFunc)_run_gstreamer, NULL)) == NULL)
	{
		return -1;
	}
	return 0;
}

void fix_stop_gstreamer()
{
	_bQuit = true;
}