// fixgst.cpp
// gstreamer interface functions for use in fixstim

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <stdint.h>
#include <iostream>
using namespace std;

#include "VSGV8.h"
#include "Alertlib.h"

int want = 1;
uint16_t b_white[385 * 288];
uint16_t b_black[385 * 288];
bool _bQuit = false;
bool gotImage = false;

static void prepare_buffer(GstAppSrc* appsrc) {

	static gboolean white = FALSE;
	static GstClockTime timestamp = 0;
	GstBuffer *buffer;
	guint size;
	GstFlowReturn ret;

	if (!want) return;
	want = 0;

	// get current video page, save image
	if (!gotImage)
	{
		int savemode = vsgGetDrawMode();
		vsgSetDrawMode(0);
		int page = vsgGetZoneDisplayPage(vsgVIDEOPAGE);
		//int status = vsgImageExport(vsgBMPPICTURE, 0, 0, vsgGetScreenWidthPixels(), vsgGetScreenHeightPixels(), "image.bmp");
		int status = vsgImageExport(vsgBMPPICTURE, -vsgGetScreenWidthPixels()/2, -vsgGetScreenHeightPixels()/2, vsgGetScreenWidthPixels(), vsgGetScreenHeightPixels(), "image.bmp");
		vsgSetDrawMode(savemode);
		gotImage = true;
		cout << "Saved image status " << status << endl;
	}


	size = 385 * 288 * 2;

	buffer = gst_buffer_new_wrapped_full(GST_MEMORY_FLAG_READONLY, (gpointer)(white ? b_white : b_black), size, 0, size, NULL, NULL);

	white = !white;

	GST_BUFFER_PTS(buffer) = timestamp;
	GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, 4);

	timestamp += GST_BUFFER_DURATION(buffer);

	ret = gst_app_src_push_buffer(appsrc, buffer);

	if (ret != GST_FLOW_OK) {
		/* something wrong, stop pushing */
		// g_main_loop_quit (loop);
	}
}

static void cb_need_data(GstElement *appsrc, guint unused_size, gpointer user_data) {
	//prepare_buffer((GstAppSrc*)appsrc);
	want = 1;
}



int _run_gstreamer(gpointer data)
{
	GstElement *pipeline, *appsrc, *conv, *videosink, *fpssink;
	GMainContext *context;

	for (int i = 0; i < 385 * 288; i++) { b_black[i] = 0x61a8; b_white[i] = 0x88b8; }

	cout << "Initialize gstreamer..." << endl;
	gst_init(NULL, NULL);

	/* setup pipeline */
	pipeline = gst_pipeline_new("pipeline");
	appsrc = gst_element_factory_make("appsrc", "source");
	conv = gst_element_factory_make("videoconvert", "conv");
	videosink = gst_element_factory_make("xvimagesink", "videosink");
	fpssink = gst_element_factory_make("fpsdisplaysink", "fpssink");

	// configure fpsdisoplaysink
	g_object_set(G_OBJECT(fpssink),
		"text-overlay", TRUE,
		"video-sink", videosink, NULL);

	/* setup */
	g_object_set(G_OBJECT(appsrc), "caps",
		gst_caps_new_simple("video/x-raw",
			"format", G_TYPE_STRING, "RGB16",
			"width", G_TYPE_INT, 384,
			"height", G_TYPE_INT, 288,
			"framerate", GST_TYPE_FRACTION, 0, 1,
			NULL), NULL);
	gst_bin_add_many(GST_BIN(pipeline), appsrc, conv, fpssink, NULL);
	gst_element_link_many(appsrc, conv, fpssink, NULL);

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

	return 0;
}


int fix_start_gstreamer()
{
	// launch thread
	GThread *_loop_thread = NULL;
	if ((_loop_thread = g_thread_new("fixstim-gstreamer", (GThreadFunc)_run_gstreamer, NULL)) == NULL)
	{
		return -1;
	}
	return 0;
	//if ((_loop_thread = g_thread_create((GThreadFunc)_run_gstreamer, NULL, FALSE, NULL)) == NULL)
	//{
	//	return -1;
	//}
}

void fix_stop_gstreamer()
{
	_bQuit = true;
}