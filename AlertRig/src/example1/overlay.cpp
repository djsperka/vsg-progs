#include "vsgv8.h"
#include "Alertlib.h"
#include "ARvsg.h"
#include "ARtypes.h"
#include "AlertUtil.h"
//#include "alert-triggers.h"
#include <iostream>

using namespace std;
using namespace alert;

int main(int argc, char** argv)
{
    int Width, Height;
    double Buff[6];
    int Ypos, Offset;
    int Shift;


    // INit vsg
    if (ARvsg::instance().init(800, COLOR_TYPE(gray)))
    {
        cerr << "VSG init failed!" << endl;
        return 1;
    }

    vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);

    PIXEL_LEVEL level;
    ARvsg::instance().request_single(level);
    arutil_color_to_palette(COLOR_TYPE(0, 1, 1), level);
    vsgSetPen1(level);
    vsgDrawRect(0, 0, 10, 10);






    vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);

    cout << "Done" << endl;

    ////Find out the horizontal and vertical resolution of the vsg screen.
    //Width = vsgGetScreenWidthPixels();
    //Height = vsgGetScreenHeightPixels();

    ////Clear a drawing page in memory then display it.
    //vsgSetDrawPage(vsgVIDEOPAGE, 1, 5);
    //vsgSetDisplayPage(1);

    ////Set lookup table 0 in palette 0 & 1 to orange and red.
    //Buff[0] = 1.0;
    //Buff[1] = 0.4;
    //Buff[2] = 0.0;
    //Buff[3] = 1.0;
    //Buff[4] = 0;
    //Buff[5] = 0;
    //vsgPaletteWrite((VSGLUTBUFFER*)&Buff, 0, 2);
    ////Draw the chess board to cover the whole page.

    //vsgSetPen1(1);
    //vsgSetPen2(0);
    //vsgSetDrawMode(vsgCENTREXY + vsgSOLIDFILL);
    //vsgDrawChessboard(0, 0, Width, Height, div(Width, 8).quot, div(Height, 8).quot, 0);
    //vsgPresent();

    ////Change the drawing modes to target the Overlay.
    //vsgSetCommand(vsgOVERLAYMASKMODE);

    ////Clear all of the Overlay memory to pixel-level(1).
    //vsgSetPen2(1);
    //vsgSetCommand(vsgOVERLAYCLEAR);

    ////Fill Overlay pixel-level(1) with black.
    //Buff[0] = 0.0;
    //Buff[1] = 0.0;
    //Buff[2] = 0.0;
    //vsgSetDrawPage(vsgOVERLAYPAGE, 1, vsgNOCLEAR);
    //vsgPaletteWriteOverlayCols((VSGLUTBUFFER*)&Buff, 1, 1);

    ////Draw a window in the Overlay page (in vsgOVERLAYMASKMODE anything drawn in pixel-
    ////level(0) is transparent).
    //vsgSetPen1(0);
    //vsgDrawRect(0, 0, div(Width, 2).quot, div(Height, 4).quot);

    ////Display OVERLAYPAGE 1.
    //vsgSetDisplayPage(1);

    ////Change the drawing mode so that the Video and Overlay pages can be
    ////drifted together.
    //vsgSetCommand(vsgVIDEODRIFT + vsgOVERLAYDRIFT);

    ////Move the Video and Overlay pages up and down.
    //Ypos = 0;
    //Shift = 1;
    //Offset = div(Height, 4).quot;
    //int iCount = 0;
    //do
    //{
    //    iCount++;
    //    vsgMoveScreen(0, Ypos);
    //    if ((Ypos <= -Offset) | (Ypos >= Offset))
    //    {
    //        Shift = Shift * -1;
    //    }
    //    Ypos = Ypos + Shift;
    //} while (iCount < 500);

    return 0;
}
