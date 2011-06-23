#include "Alertlib.h"

using namespace alert;

int arutil_color_to_overlay_palette(ARFixationPointSpec& fp, PIXEL_LEVEL level);
int arutil_color_to_overlay_palette(COLOR_TYPE c, PIXEL_LEVEL level);
int arutil_color_to_palette(COLOR_TYPE c, PIXEL_LEVEL level);
int	arutil_draw_overlay(ARFixationPointSpec& fp, PIXEL_LEVEL level, int overlayPage);
int	arutil_draw_video(ARFixationPointSpec& fp, PIXEL_LEVEL level, int videoPage);
int arutil_draw_grating_fullscreen(ARGratingSpec& gr, int videoPage);
int arutil_draw_grating(ARGratingSpec& gr, int videoPage);
int arutil_draw_grating_noaperture(ARGratingSpec& gr, int videoPage);
int arutil_draw_aperture(ARGratingSpec& gr, int overlayPage);
int arutil_load_mseq(char **ppseq, std::string& filename, int iOrder=15);
int arutil_load_sequence(char **ppseq, std::string& filename);

