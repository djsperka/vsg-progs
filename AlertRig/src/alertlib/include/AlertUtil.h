#include "Alertlib.h"

using namespace alert;

int arutil_color_to_overlay_palette(ARFixationPointSpec& fp, PIXEL_LEVEL level);
int	arutil_draw_overlay(ARFixationPointSpec& fp, PIXEL_LEVEL level, int overlayPage);
int arutil_draw_grating_fullscreen(ARGratingSpec& gr, int videoPage);
int arutil_draw_aperture(ARGratingSpec& gr, int overlayPage);

