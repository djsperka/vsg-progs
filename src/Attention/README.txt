Attention

Designed for the attention task. 
Handles variations for both Cornelius (Corey) and for Betty (Farran). 
Modified Jan 2013 for new variations due to Henry's expt. 

Two gratings are displayed. The grating specified with "s" ("g") has its 
contrast changed up/down with C/c (D/d). 

Single stim option (-S) means only one of the two stim are shown with the "S" 
trigger. Which stim displayed depends on the cue used: If red cue ("F" 
trigger), then the grating specified with "s" is displayed. Only the C/c 
contrast change triggers will have an effect. If green cue ("G" trigger), 
then the grating specified with "g" is displayed. Only the D/d contrast 
change triggers will have an effect. 

White spot fixation: When cue circles are NOT used, the "W" trigger will turn 
on a white fixation point. This was used as an initial fixation point. After 
fixation, the "F" or "G" was issued to change the fixpt to the colored cue. 
When cue circles are used this option is not available due to VSG limitations.

Cue circles: Using -Q option (specify -c as well) allows use of cue circles. 
The cue circle is drawn (on the "W" trigger) around the stim indicated by the 
fixation point trigger. So, if an "F" is sent, then the "W" will draw the cue 
circle around the "s" stim. The "w" will turn off the cue circle. 

Lollipops: These go on and off with the fixation point. Does not have a 
separate trigger. This and cue circles are mutually exclusive. The -c, -l 
and -G options can be used here (though they have defaults of 
-c .1 -l .75 -G .5). The length of the stick is the distance from the edge 
of the cue circle to the fixpt center MINUS the gap. The fraction "-l" 
specifies how to place the line in that space; .75 centers it with equal 
spaces at either end. 

Command Line options:

-a		Use ascii triggers.
-v		Verbose. Might write more msgs, might not. 
-f		Fixation point parameters (required). 
		Format: -f x,y,diam,color
		x,y,diam are in degrees, color can be red, blue or green 
		or custom color {r,g,b}, 0 <= r,g,b <= 255
-b		background color, gray, black or white or custom (required)
-d		distance to screen in MM (required)
-s		stimulus grating specifier
-g		distractor grating specifier
-t		contrast triplet, format -t down,base,up : 
		0 <= down,base,up <= 100 e.g. -t 50,75,100
		Modified: There can be a list of contrast triplets here, comma 
		separated. The total number of contrasts in the comma-separated 
		list must be a multiple of 3 - following the pattern 
		down,base,up,down,base,up.... When a list is used, the 'a' 
		trigger will step to the next triplet. After advancing through 
		all triplets, start over at the beginning. 
-A		answer point offset, in degrees (offset up/down from fixpt)
-D		answer point diameter, in degrees
-c		circle diameter differential; difference (in degrees) between 
		diameter of stimulus and the cue circle (if used)
-l		cue line fraction; fraction of distance from fixpt to cue circle 
		that cue line covers
-G		cue line gap
-L      use lollipop cues
-p		lollipop contrast
-P		lollipop stick contrast
-N		no answer points
-Q		use cue circles
-S		use single stim (may be stim or distractor depending on fixpt cue used)
-h		print help


Triggers

		The ready pulse issued is 0x20, seen on CED port 6.
        This program uses the following triggers and trigger responses:
        

        Action/visual  | Ascii Trigger | Input mask/val | Out mask/val | port 
        -----------------------------------------------------------------------
        FixptR         |       F       |    0x66/0x02   |  0x1/0x1     |   1
        FixptG         |       G       |    0x66/0x04   |  0x1/0x1     |   1
        Stim           |       S       |    0x08/0x08   |  0x2/0x2     |   2
        CueCircle ON   |       W       |    0x10/0x10   |  0x4/0x4     |   3
        CueCircle OFF  |       w       |    0x10/0x00   |  0x4/0x0     |   3
        Chg-C          |       C       |    0x6E/0x2A   |  0x8/0x8     |   4
        Chg-c          |       c       |    0x6E/0x4A   |  0x8/0x8     |   4
        Chg-D          |       D       |    0x6E/0x2C   |  0x8/0x8     |   4
        Chg-d          |       d       |    0x6E/0x4C   |  0x8/0x8     |   4
        AllOff         |       X       |    0x6E/0x00   |  0xf/0x0     |   1
        Advance        |       a       |    0x80/0x80(T)|  0x10/0x10(T)|   5
        Quit		   |       q       |    0x7E/0x7E   |  0xff/0x0    |     

        (T) - this bit toggled
        