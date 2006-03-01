colorflash
----------

This program uses vsg page cycling to flash solid colors full screen at specified intervals.

The command line looks like this:

colorflash [-v] -p r1,g1,b1,secs_1[,r2,g2,b2,secs_2[,...]

For example, to display the following sequence of pages ad nauseum,
(r,g,b)=(0,0,0) for 1 second
(r,g,b)=(0.5,0.5,0.5) for 3 seconds
(r,g,b)=(1,1,1) for 1 second

use this command line

colorflash -p 1,1,1,1,.5,.5,.5,3,0,0,0,1



Spike2 notes:
-------------

The vsg triggers are on port 0. The triggers are pulses one frame wide, and the leading edge 
corresponds to the transition between the different color pages you specified. For the example 
above, there will be a trigger at the onset of the first (0,0,0) page, the onset of the second
(.5, .5, .5) page and the onset of the third (1,1,1) page. 

Data acquisition - when setting the spike2 sampling config by hand, use these settings for the pulses:

Channel/title (any is OK)
Type: Event+  (will mark only the leading edge of the pulses)
1401 Port: 0 (Dig In 8)
Maximum Event rate: 10000

The maximum event rate value you enter will determine the sampling rate, and hence the resolution, 
you get for the event times. A value of 10000 will give resolution < 1ms.

Interval values:
----------------

You can run the spike2 script "DUMPEVE.S2S" (found in the "scripts" folder under the Spike2 installation
folder. This script will dump the interval times for a region of the data file you select. 
